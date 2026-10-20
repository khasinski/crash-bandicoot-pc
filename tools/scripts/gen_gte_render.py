#!/usr/bin/env python3
"""Translate the hand-written MIPS routines of gte_render to C.

The original executable has a block of hand-written R3000 code
(0x800334A0..0x8003A94C) for SLST decoding, transformation, projection and
collision queries. Its routines jump into each other's unrolled code and
call each other with jal, so the whole block becomes one C function with a
label per instruction. The translation keeps the R3000 semantics the code
relies on: branch delay slots, load delay slots, and GTE (COP2) operations,
which go to PsyZ's register-level GTE. Each routine is an entry point with a
C wrapper whose signature is its prototype in include/functions.h.

The routines are ported to C; the translation only serves as the reference
of the -DCRASH_GTE_ORACLE=ON check build, which runs this script on the
decompilation's executable and linked ELF (needs pyelftools):

    tools/scripts/gen_gte_render.py path/to/main.exe path/to/main.elf --output OUT.inc
"""
import argparse
import re
import struct
from pathlib import Path

from elftools.elf.elffile import ELFFile

ROOT = Path(__file__).resolve().parents[2]
START, END = 0x800334A0, 0x8003A94C
EXE_BASE = 0x80010000 - 0x800
SENTINEL = 0xFFFFFFF0  # return address meaning "return to the C caller"
# Entries defined in C with no translated counterpart (gte_render.c).
NATIVE_ONLY = {"RIncTickCount"}

REG = ["zero", "at", "v0", "v1", "a0", "a1", "a2", "a3",
       "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
       "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
       "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"]


def sx16(v):
    return v - 0x10000 if v & 0x8000 else v


class Insn:
    def __init__(self, addr, word):
        self.addr, self.word = addr, word
        self.op = word >> 26
        self.rs = (word >> 21) & 31
        self.rt = (word >> 16) & 31
        self.rd = (word >> 11) & 31
        self.sa = (word >> 6) & 31
        self.fn = word & 63
        self.imm = word & 0xFFFF
        self.simm = sx16(self.imm)
        self.target = (addr & 0xF0000000) | ((word & 0x3FFFFFF) << 2)

    # Control transfer -------------------------------------------------
    def is_branch(self):
        return self.op in (4, 5, 6, 7) or (self.op == 1 and self.rt in (0, 1, 16, 17))

    def is_jump(self):
        return self.op in (2, 3) or (self.op == 0 and self.fn in (8, 9))

    def has_delay(self):
        return self.is_branch() or self.is_jump()

    def branch_target(self):
        return (self.addr + 4 + (self.simm << 2)) & 0xFFFFFFFF

    # Register effects used for load-delay analysis -------------------
    def load_dest(self):
        """GPR written with a one-instruction delay (loads, mfc2, cfc2)."""
        if self.op in (0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26):
            return self.rt
        if self.op == 0x12 and self.rs in (0, 2):
            return self.rt
        return None

    def reads(self):
        o, f = self.op, self.fn
        if self.word == 0:
            return set()
        if o == 0:
            if f in (0, 2, 3):
                return {self.rt}
            if f in (4, 6, 7):
                return {self.rs, self.rt}
            if f in (8, 9):
                return {self.rs}
            if f in (0x10, 0x12, 0x0C, 0x0D):
                return set()
            if f in (0x11, 0x13):
                return {self.rs}
            return {self.rs, self.rt}
        if o == 1:
            return {self.rs}
        if o in (2, 3):
            return set()
        if o in (4, 5):
            return {self.rs, self.rt}
        if o in (6, 7):
            return {self.rs}
        if 8 <= o <= 14:
            return {self.rs}
        if o == 15:
            return set()
        if o == 0x12:
            if self.rs in (4, 6):
                return {self.rt}
            return set()
        if o in (0x22, 0x26):  # lwl/lwr merge into rt
            return {self.rs, self.rt}
        if 0x20 <= o <= 0x26:
            return {self.rs}
        if 0x28 <= o <= 0x2E:
            return {self.rs, self.rt}
        if o in (0x32, 0x3A):
            return {self.rs}
        return set()


def r(n):
    return "0" if n == 0 else f"R({REG[n]})"


def setr(n, expr):
    if n == 0:
        return f"(void)({expr});"
    return f"R({REG[n]}) = {expr};"


def translate(i, data_labels):
    """C statement for a non-control instruction."""
    o, f, rs, rt, rd = i.op, i.fn, i.rs, i.rt, i.rd
    if i.word == 0:
        return ";"
    if o == 0:
        ops = {
            0x00: lambda: setr(rd, f"{r(rt)} << {i.sa}"),
            0x02: lambda: setr(rd, f"{r(rt)} >> {i.sa}"),
            0x03: lambda: setr(rd, f"(u32)((s32){r(rt)} >> {i.sa})"),
            0x04: lambda: setr(rd, f"{r(rt)} << ({r(rs)} & 31)"),
            0x06: lambda: setr(rd, f"{r(rt)} >> ({r(rs)} & 31)"),
            0x07: lambda: setr(rd, f"(u32)((s32){r(rt)} >> ({r(rs)} & 31))"),
            0x10: lambda: setr(rd, "c->hi"),
            0x11: lambda: f"c->hi = {r(rs)};",
            0x12: lambda: setr(rd, "c->lo"),
            0x13: lambda: f"c->lo = {r(rs)};",
            0x18: lambda: f"{{ s64 p = (s64)(s32){r(rs)} * (s32){r(rt)}; c->lo = (u32)p; c->hi = (u32)((u64)p >> 32); }}",
            0x19: lambda: f"{{ u64 p = (u64){r(rs)} * {r(rt)}; c->lo = (u32)p; c->hi = (u32)(p >> 32); }}",
            0x1A: lambda: f"mips_div(c, {r(rs)}, {r(rt)});",
            0x1B: lambda: f"mips_divu(c, {r(rs)}, {r(rt)});",
            0x20: lambda: setr(rd, f"{r(rs)} + {r(rt)}"),
            0x21: lambda: setr(rd, f"{r(rs)} + {r(rt)}"),
            0x22: lambda: setr(rd, f"{r(rs)} - {r(rt)}"),
            0x23: lambda: setr(rd, f"{r(rs)} - {r(rt)}"),
            0x24: lambda: setr(rd, f"{r(rs)} & {r(rt)}"),
            0x25: lambda: setr(rd, f"{r(rs)} | {r(rt)}"),
            0x26: lambda: setr(rd, f"{r(rs)} ^ {r(rt)}"),
            0x27: lambda: setr(rd, f"~({r(rs)} | {r(rt)})"),
            0x2A: lambda: setr(rd, f"(s32){r(rs)} < (s32){r(rt)}"),
            0x2B: lambda: setr(rd, f"{r(rs)} < {r(rt)}"),
            0x0D: lambda: "mips_break(c);",
        }
        if f in ops:
            return ops[f]()
    imm, simm = i.imm, i.simm
    if o in (8, 9):
        return setr(rt, f"{r(rs)} + (u32){simm}")
    if o == 10:
        return setr(rt, f"(s32){r(rs)} < {simm}")
    if o == 11:
        return setr(rt, f"{r(rs)} < (u32){simm}")
    if o == 12:
        return setr(rt, f"{r(rs)} & 0x{imm:X}u")
    if o == 13:
        return setr(rt, f"{r(rs)} | 0x{imm:X}u")
    if o == 14:
        return setr(rt, f"{r(rs)} ^ 0x{imm:X}u")
    if o == 15:
        return setr(rt, f"0x{imm << 16:X}u")
    ea = f"{r(rs)} + (u32){simm}"
    if o == 0x20:
        return setr(rt, f"(u32)(s32)(s8)mem_lb({ea})")
    if o == 0x24:
        return setr(rt, f"mem_lb({ea})")
    if o == 0x21:
        return setr(rt, f"(u32)(s32)(s16)mem_lh({ea})")
    if o == 0x25:
        return setr(rt, f"mem_lh({ea})")
    if o == 0x23:
        return setr(rt, f"mem_lw({ea})")
    if o == 0x22:
        return setr(rt, f"mem_lwl({ea}, {r(rt)})")
    if o == 0x26:
        return setr(rt, f"mem_lwr({ea}, {r(rt)})")
    if o == 0x28:
        return f"mem_sb({ea}, {r(rt)});"
    if o == 0x29:
        return f"mem_sh({ea}, {r(rt)});"
    if o == 0x2B:
        return f"mem_sw({ea}, {r(rt)});"
    if o == 0x2A:
        return f"mem_swl({ea}, {r(rt)});"
    if o == 0x2E:
        return f"mem_swr({ea}, {r(rt)});"
    if o == 0x32:
        return f"Psyz_GteDataWrite({rt}, mem_lw({ea}));"
    if o == 0x3A:
        return f"mem_sw({ea}, Psyz_GteDataRead({rt}));"
    if o == 0x12:
        if rs == 0:
            return setr(rt, f"Psyz_GteDataRead({rd})")
        if rs == 2:
            return setr(rt, f"Psyz_GteCtrlRead({rd})")
        if rs == 4:
            return f"Psyz_GteDataWrite({rd}, {r(rt)});"
        if rs == 6:
            return f"Psyz_GteCtrlWrite({rd}, {r(rt)});"
        if rs & 0x10:
            return f"Psyz_GteCommand(0x{i.word & 0x1FFFFFF:07X}u);"
    raise ValueError(f"unsupported instruction {i.word:08X} at {i.addr:08X}")


def emit(insns, by_addr, entries, data_words):
    out = []
    label = lambda a: f"L_{a:08X}"
    code_addrs = [i.addr for i in insns if i.addr not in data_words]

    def stmt(i):
        return translate(i, data_words)

    def with_load_delay(i, nxt):
        """i is a delayed load whose result nxt must not see yet."""
        dest = i.load_dest()
        tmp = stmt(i).replace(f"R({REG[dest]}) = ", "loaded = ", 1)
        return tmp, f"R({REG[dest]}) = loaded;"

    def control(i, delay):
        """Statements for a branch or jump with its delay slot."""
        o, f = i.op, i.fn
        dly = stmt(delay) if delay is not None else ";"
        if delay is not None and delay.has_delay():
            raise ValueError(f"control in delay slot at {delay.addr:08X}")
        lines = []
        if i.is_branch():
            cond = {
                4: f"{r(i.rs)} == {r(i.rt)}",
                5: f"{r(i.rs)} != {r(i.rt)}",
                6: f"(s32){r(i.rs)} <= 0",
                7: f"(s32){r(i.rs)} > 0",
            }.get(o)
            link = False
            if o == 1:
                cond = f"(s32){r(i.rs)} < 0" if i.rt in (0, 16) else f"(s32){r(i.rs)} >= 0"
                link = i.rt in (16, 17)
            lines.append(f"taken = {cond};")
            if link:
                lines.append(f"R(ra) = 0x{i.addr + 8:08X}u;")
            lines.append(dly)
            lines.append(f"if (taken) goto {label(i.branch_target())};")
            return lines
        if o == 2:
            return [dly, f"goto {label(i.target)};"]
        if o == 3:
            return [f"R(ra) = 0x{i.addr + 8:08X}u;", dly, f"goto {label(i.target)};"]
        if o == 0 and f == 8:
            lines.append(f"target = {r(i.rs)};")
            lines.append(dly)
            lines.append("if (target == SENTINEL) return;")
            lines.append("goto dispatch;")
            return lines
        if o == 0 and f == 9:
            lines.append(f"target = {r(i.rs)};")
            if i.rd:
                lines.append(f"R({REG[i.rd]}) = 0x{i.addr + 8:08X}u;")
            lines.append(dly)
            lines.append("mips_call_native(c, target);")
            lines.append(f"goto {label(i.addr + 8)};")
            return lines
        raise ValueError(f"unknown control {i.word:08X}")

    out.append("static void mips_run(MipsCpu *c, u32 entry)")
    out.append("{")
    out.append("    u32 target = entry;")
    out.append("    u32 loaded;")
    out.append("    int taken;")
    out.append("    (void)loaded;")
    out.append("    (void)taken;")
    out.append("dispatch:")
    out.append("    switch (target) {")
    for a in code_addrs:
        out.append(f"    case 0x{a:08X}u: goto {label(a)};")
    out.append("    default: mips_bad_jump(target);")
    out.append("    }")
    n = len(insns)
    idx = 0
    warnings = []
    while idx < n:
        i = insns[idx]
        if i.addr in data_words:
            out.append(f"    /* 0x{i.addr:08X}: data word */")
            idx += 1
            continue
        out.append(f"{label(i.addr)}:")
        if i.addr in entries:
            out.append(f"    /* {entries[i.addr]} */")
        if i.has_delay():
            delay = insns[idx + 1] if idx + 1 < n else None
            for s in control(i, delay):
                out.append(f"    {s}")
            # The delay slot is also reachable by itself; skip that copy when
            # the branch falls through.
            if delay is not None:
                if i.is_branch():
                    out.append(f"    goto {label(delay.addr + 4)};")
                out.append(f"{label(delay.addr)}:")
                out.append(f"    {stmt(delay)}")
            idx += 2
            continue
        dest = i.load_dest()
        nxt = insns[idx + 1] if idx + 1 < n else None
        if dest and nxt is not None and nxt.addr not in data_words and dest in nxt.reads():
            if nxt.has_delay() or nxt.load_dest() is not None:
                raise ValueError(f"unhandled load delay chain at {nxt.addr:08X}")
            # R3000 load delay: run sequentially, the next instruction still
            # sees the old register; reached by a jump, it sees the current one.
            load = stmt(i).replace(f"R({REG[dest]}) = ", "loaded = ", 1)
            out.append(f"    {load}")
            out.append(f"    {stmt(nxt)}")
            out.append(f"    R({REG[dest]}) = loaded;")
            after = nxt.addr + 4
            out.append(f"    goto {label(after)};")
            out.append(f"{label(nxt.addr)}:")
            out.append(f"    {stmt(nxt)}")
            warnings.append(f"load delay used at {nxt.addr:08X}")
            idx += 2
            continue
        out.append(f"    {stmt(i)}")
        idx += 1
    out.append("}")
    return out, warnings


def split_params(text):
    params, depth, cur = [], 0, ""
    for ch in text:
        if ch == "," and depth == 0:
            params.append(cur.strip())
            cur = ""
            continue
        depth += ch == "("
        depth -= ch == ")"
        cur += ch
    if cur.strip():
        params.append(cur.strip())
    return [] if params == ["void"] else params


def wrappers(entries, functions_h):
    """C entry points for entries with a prototype.

    mips_<name> runs the translated routine with its arguments as PS1 words.
    The public <name> runs it and the C port, Native_<name>, from the same
    state and compares the results (src/main/engine/gte_oracle.c).
    """
    out = []
    for addr, name in sorted(entries.items()):
        if name in NATIVE_ONLY:
            continue
        m = re.search(r"^([\w \*]+?)\b" + name + r"\((.*)\);", functions_h, re.M)
        if not m or m.group(2).strip() == "":
            continue
        ret = m.group(1).strip()
        params = split_params(m.group(2))
        args, decls, names = [], [], []
        for k, p in enumerate(params):
            if "(*" in p:
                pname = re.search(r"\(\s*\*\s*(\w+)\s*\)", p).group(1)
                args.append(f"Ram_FnAddr((const void *){pname})")
            else:
                pname = re.search(r"(\w+)\s*$", p).group(1)
                args.append(f"Ram_Addr({pname})" if "*" in p else f"(u32){pname}")
            decls.append(p)
            names.append(pname)
        plist = ", ".join(decls) or "void"
        nlist = ", ".join(names)
        call = f"mips_call(0x{addr:08X}u, {len(args)}, args)"
        out.append(f"static {ret} mips_{name}({plist})")
        out.append("{")
        if args:
            out.append(f"    u32 args[{len(args)}] = {{{', '.join(args)}}};")
        else:
            out.append("    u32 *args = NULL;")
        if ret == "void":
            out.append(f"    {call};")
        elif "*" in ret:
            out.append(f"    return ({ret})Ram_Ptr({call});")
        else:
            out.append(f"    return ({ret}){call};")
        out.append("}")
        out.append(f"{ret} Native_{name}({plist});")
        out.append(f"{ret} {name}({plist})")
        out.append("{")
        word = (lambda v: f"Ram_Addr({v})") if "*" in ret else (lambda v: f"(u32){v}")
        if ret == "void":
            out.append("    if (!Oracle_Begin(__builtin_frame_address(0))) {")
            out.append(f"        Native_{name}({nlist});")
            out.append("        return;")
            out.append("    }")
            out.append(f"    mips_{name}({nlist});")
            out.append("    Oracle_Swap();")
            out.append(f"    Native_{name}({nlist});")
            out.append(f'    Oracle_End("{name}", 0, 0);')
        else:
            out.append(f"    {ret} mips_result, native_result;")
            out.append("    if (!Oracle_Begin(__builtin_frame_address(0)))")
            out.append(f"        return Native_{name}({nlist});")
            out.append(f"    mips_result = mips_{name}({nlist});")
            out.append("    Oracle_Swap();")
            out.append(f"    native_result = Native_{name}({nlist});")
            out.append(f'    Oracle_End("{name}", {word("mips_result")}, {word("native_result")});')
            out.append("    return native_result;")
        out.append("}")
        out.append("")
    return out


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("exe")
    parser.add_argument("elf")
    parser.add_argument("--output", required=True)
    args = parser.parse_args()

    exe = Path(args.exe).read_bytes()
    elf = ELFFile(open(args.elf, "rb"))
    entries, data_words = {}, set()
    for sym in elf.get_section_by_name(".symtab").iter_symbols():
        a = sym["st_value"]
        if not (START <= a < END) or "." in sym.name or not sym.name:
            continue
        if sym["st_info"]["type"] == "STT_FUNC":
            entries[a] = sym.name
        elif sym["st_info"]["type"] == "STT_OBJECT":
            for k in range(0, max(sym["st_size"], 4), 4):
                data_words.add(a + k)
    insns = []
    for a in range(START, END, 4):
        word = struct.unpack_from("<I", exe, a - EXE_BASE)[0]
        insns.append(Insn(a, word))
    body, warnings = emit(insns, {i.addr: i for i in insns}, entries, data_words)
    body += ["", "/* Entry points for C callers. */", ""]
    body += wrappers(entries, (ROOT / "include/functions.h").read_text())
    for w in warnings:
        print("note:", w)
    header = [
        "/* Generated by tools/scripts/gen_gte_render.py from the original",
        " * hand-written MIPS at 0x%08X..0x%08X. Do not edit. */" % (START, END),
        "",
    ]
    Path(args.output).write_text("\n".join(header + body) + "\n")
    print(f"{len(insns)} instructions, {len(entries)} entries -> {args.output}")


if __name__ == "__main__":
    main()
