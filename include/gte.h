#ifndef CRASH_GTE_H
#define CRASH_GTE_H

#include "common.h"

#ifdef __psyz
/*
 * Native build: the same macros drive PsyZ's GTE register file. Names that
 * PsyZ's libgte.h also defines are replaced with this header's semantics.
 */
#include <psyz/gte.h>

#undef gte_ldv0
#undef gte_ldrgb
#undef gte_ldtr
#undef gte_rtps
#undef gte_rtpt
#undef gte_nclip
#undef gte_avsz3
#undef gte_avsz4
#undef gte_ldclmv
#undef gte_stsz3
#undef gte_stlvnl

#define gte_mtc2(val, reg) Psyz_GteDataWrite(reg, (u32)(val))
#define gte_mfc2(out, reg) ((out) = Psyz_GteDataRead(reg))
#define gte_ctc2(val, reg) Psyz_GteCtrlWrite(reg, (u32)(val))
#define gte_cfc2(out, reg) ((out) = Psyz_GteCtrlRead(reg))
#define gte_lwc2(mem, reg) Psyz_GteDataWrite(reg, *(const u32 *)&(mem))
#define gte_swc2(mem, reg) (*(u32 *)&(mem) = Psyz_GteDataRead(reg))

#define gte_ldvxy0(vxy) gte_mtc2(vxy, 0)
#define gte_ldvz0(vz)   gte_mtc2(vz, 1)
#define gte_ldvxy1(vxy) gte_mtc2(vxy, 2)
#define gte_ldvz1(vz)   gte_mtc2(vz, 3)
#define gte_ldvxy2(vxy) gte_mtc2(vxy, 4)
#define gte_ldvz2(vz)   gte_mtc2(vz, 5)

#define gte_ldv0(vxy, vz) \
    do { gte_ldvxy0(vxy); gte_ldvz0(vz); } while (0)
#define gte_ldv1(vxy, vz) \
    do { gte_ldvxy1(vxy); gte_ldvz1(vz); } while (0)
#define gte_ldv2(vxy, vz) \
    do { gte_ldvxy2(vxy); gte_ldvz2(vz); } while (0)

#define gte_ldrgb(rgb) gte_mtc2(rgb, 6)

#define gte_ldtrx(trx) gte_ctc2(trx, 5)
#define gte_ldtry(try) gte_ctc2(try, 6)
#define gte_ldtrz(trz) gte_ctc2(trz, 7)

#define gte_ldtr(trx, try, trz) \
    do { gte_ldtrx(trx); gte_ldtry(try); gte_ldtrz(trz); } while (0)

#define gte_stsxy0(out) gte_swc2(out, 12)
#define gte_stsxy1(out) gte_swc2(out, 13)
#define gte_stsxy2(out) gte_swc2(out, 14)

#define gte_getsz0(out) gte_mfc2(out, 16)
#define gte_getsz1(out) gte_mfc2(out, 17)
#define gte_getsz2(out) gte_mfc2(out, 18)
#define gte_getsz3(out) gte_mfc2(out, 19)

#define gte_getmac0(out)     gte_mfc2(out, 24)
#define gte_getflag(out)     gte_cfc2(out, 31)
#define gte_getflag_nop(out) gte_cfc2(out, 31)

/* Command words: rtps, rtpt, nclip, avsz3, avsz4, mvmva 1,0,0,0,0. */
#define gte_rtps()  Psyz_GteCommand(0x0180001)
#define gte_rtpt()  Psyz_GteCommand(0x0280030)
#define gte_nclip() Psyz_GteCommand(0x1400006)
#define gte_avsz3() Psyz_GteCommand(0x158002D)
#define gte_avsz4() Psyz_GteCommand(0x168002E)

#define gte_ldclmv(m, c0, c1, c2, c3, c4) \
    do { \
        const u32 *gte_m_ = (const u32 *)(m); \
        gte_ctc2(gte_m_[0], c0); \
        gte_ctc2(gte_m_[1], c1); \
        gte_ctc2(gte_m_[2], c2); \
        gte_ctc2(gte_m_[3], c3); \
        gte_ctc2(gte_m_[4], c4); \
    } while (0)

#define gte_ldrotmatrix(m)   gte_ldclmv(m, 0, 1, 2, 3, 4)
#define gte_ldlightmatrix(m) gte_ldclmv(m, 8, 9, 10, 11, 12)
#define gte_ldcolormatrix(m) gte_ldclmv(m, 16, 17, 18, 19, 20)

#define gte_ldtransmatrix(m) \
    do { \
        const u32 *gte_m_ = (const u32 *)(m); \
        gte_ctc2(gte_m_[5], 5); \
        gte_ctc2(gte_m_[6], 6); \
        gte_ctc2(gte_m_[7], 7); \
    } while (0)

#define gte_stsz3(sz0, sz1, sz2) \
    do { \
        gte_swc2(*(sz0), 17); \
        gte_swc2(*(sz1), 18); \
        gte_swc2(*(sz2), 19); \
    } while (0)

#define gte_ldv0_p(v0) \
    do { \
        gte_lwc2(((u32 *)(v0))[0], 0); \
        gte_lwc2(((u32 *)(v0))[1], 1); \
    } while (0)

#define gte_rtps_v0(v0) \
    do { gte_ldv0_p(v0); gte_rtps(); } while (0)

#define gte_mvmva_v0(v0) \
    do { gte_ldv0_p(v0); Psyz_GteCommand(0x0480012); } while (0)

#define gte_stlvnl(v1) \
    do { \
        gte_swc2(((u32 *)(v1))[0], 25); \
        gte_swc2(((u32 *)(v1))[1], 26); \
        gte_swc2(((u32 *)(v1))[2], 27); \
    } while (0)

#define gte_obj_align(id)

#else /* !__psyz */

/*
 * PSY-Q-style GTE (COP2) register transfer and command macros.
 */

/* Generic single-instruction COP2 transfers */
#define gte_mtc2(val, reg) \
    asm volatile("mtc2 %0, $" #reg : : "r"(val))
#define gte_mfc2(out, reg) \
    asm volatile("mfc2 %0, $" #reg : "=r"(out))
#define gte_ctc2(val, reg) \
    asm volatile("ctc2 %0, $" #reg : : "r"(val))
#define gte_cfc2(out, reg) \
    asm volatile("cfc2 %0, $" #reg : "=r"(out))
#define gte_lwc2(mem, reg) \
    asm volatile("lwc2 $" #reg ", %0" : : "m"(mem))
#define gte_swc2(mem, reg) \
    asm volatile("swc2 $" #reg ", %0" : "=m"(mem))

/* Vertex input registers (V0: $0-$1, V1: $2-$3, V2: $4-$5) */
#define gte_ldvxy0(vxy) gte_mtc2(vxy, 0)
#define gte_ldvz0(vz)   gte_mtc2(vz, 1)
#define gte_ldvxy1(vxy) gte_mtc2(vxy, 2)
#define gte_ldvz1(vz)   gte_mtc2(vz, 3)
#define gte_ldvxy2(vxy) gte_mtc2(vxy, 4)
#define gte_ldvz2(vz)   gte_mtc2(vz, 5)

#define gte_ldv0(vxy, vz) \
    do { gte_ldvxy0(vxy); gte_ldvz0(vz); } while (0)
#define gte_ldv1(vxy, vz) \
    do { gte_ldvxy1(vxy); gte_ldvz1(vz); } while (0)
#define gte_ldv2(vxy, vz) \
    do { gte_ldvxy2(vxy); gte_ldvz2(vz); } while (0)

/* Colour register (RGBC: $6) */
#define gte_ldrgb(rgb) gte_mtc2(rgb, 6)

/* Translation vector control registers (TRX: $5, TRY: $6, TRZ: $7) */
#define gte_ldtrx(trx) gte_ctc2(trx, 5)
#define gte_ldtry(try) gte_ctc2(try, 6)
#define gte_ldtrz(trz) gte_ctc2(trz, 7)

#define gte_ldtr(trx, try, trz) \
    do { gte_ldtrx(trx); gte_ldtry(try); gte_ldtrz(trz); } while (0)

/* Screen XY FIFO stores (SXY0: $12, SXY1: $13, SXY2: $14) */
#define gte_stsxy0(out) gte_swc2(out, 12)
#define gte_stsxy1(out) gte_swc2(out, 13)
#define gte_stsxy2(out) gte_swc2(out, 14)

/* Screen Z FIFO reads (SZ0: $16, SZ1: $17, SZ2: $18, SZ3: $19) */
#define gte_getsz0(out) gte_mfc2(out, 16)
#define gte_getsz1(out) gte_mfc2(out, 17)
#define gte_getsz2(out) gte_mfc2(out, 18)
#define gte_getsz3(out) gte_mfc2(out, 19)

/* MAC0 ($24) and FLAG ($31) reads */
#define gte_getmac0(out)     gte_mfc2(out, 24)
#define gte_getflag(out)     gte_cfc2(out, 31)
#define gte_getflag_nop(out) asm volatile("cfc2 %0, $31; nop" : "=r"(out))

/* GTE commands */
#define gte_rtps()  asm volatile("rtps")
#define gte_rtpt()  asm volatile("rtpt")
#define gte_nclip() asm volatile("nclip")
#define gte_avsz3() asm volatile("avsz3")
#define gte_avsz4() asm volatile("avsz4")

/* Matrix & vector register block transfers used by libgte */
#define gte_ldclmv(m, c0, c1, c2, c3, c4) \
    asm volatile( \
        "lw $8, 0(%0)\n\t" \
        "lw $9, 4(%0)\n\t" \
        "lw $10, 8(%0)\n\t" \
        "lw $11, 12(%0)\n\t" \
        "lw $12, 16(%0)\n\t" \
        "ctc2 $8, $" #c0 "\n\t" \
        "ctc2 $9, $" #c1 "\n\t" \
        "ctc2 $10, $" #c2 "\n\t" \
        "ctc2 $11, $" #c3 "\n\t" \
        "ctc2 $12, $" #c4 \
        : : "r"(m) : "$8", "$9", "$10", "$11", "$12")

#define gte_ldrotmatrix(m)   gte_ldclmv(m, 0, 1, 2, 3, 4)
#define gte_ldlightmatrix(m) gte_ldclmv(m, 8, 9, 10, 11, 12)
#define gte_ldcolormatrix(m) gte_ldclmv(m, 16, 17, 18, 19, 20)

#define gte_ldtransmatrix(m) \
    asm volatile( \
        "lw $8, 20(%0)\n\t" \
        "lw $9, 24(%0)\n\t" \
        "lw $10, 28(%0)\n\t" \
        "ctc2 $8, $5\n\t" \
        "ctc2 $9, $6\n\t" \
        "ctc2 $10, $7" \
        : : "r"(m) : "$8", "$9", "$10")

#define gte_stsz3(sz0, sz1, sz2) \
    do { \
        gte_swc2(*(sz0), 17); \
        gte_swc2(*(sz1), 18); \
        gte_swc2(*(sz2), 19); \
    } while (0)

#define gte_ldv0_p(v0) \
    do { \
        gte_lwc2(((u32 *)(v0))[0], 0); \
        gte_lwc2(((u32 *)(v0))[1], 1); \
    } while (0)

#define gte_rtps_v0(v0) \
    do { \
        gte_ldv0_p(v0); \
        asm volatile("nop\n\trtps"); \
    } while (0)

#define gte_mvmva_v0(v0) \
    do { \
        gte_ldv0_p(v0); \
        asm volatile("nop\n\tmvmva 1, 0, 0, 0, 0"); \
    } while (0)

#define gte_stlvnl(v1) \
    do { \
        gte_swc2(((u32 *)(v1))[0], 25); \
        gte_swc2(((u32 *)(v1))[1], 26); \
        gte_swc2(((u32 *)(v1))[2], 27); \
    } while (0)

#define gte_obj_align(id) \
    void __maspsx_include_asm_hack_align_##id(void) { \
        asm(".balign 16 # maspsx-keep\n"); \
    }

#endif /* __psyz */

#endif
