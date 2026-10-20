/*
 * Differential check of the C ports of the hand-written MIPS routines
 * (src/main/engine/gte_native.h). Built with -DCRASH_GTE_ORACLE=ON, each call
 * of a ported routine runs the translated original first, keeps its results,
 * restores the state from before the call and runs the C version. The two
 * must leave the same game memory, scratchpad, GTE registers and result.
 *
 * Compared memory: the game's 2 MiB and the Ram_StackAlloc area, and the
 * game stack above the checking wrapper (the caller's locals). The C
 * version's own frames and the translation's stack are not compared.
 *
 * CRASH_GTE_ORACLE_CONTINUE=1 reports mismatches without stopping; the run
 * then continues from the C version's state.
 */
#ifdef CRASH_GTE_ORACLE

#include <stdio.h>
#include <stdlib.h>
#include <psyz/gte.h>
#include "common.h"

#define LOW_START RAM_PS1_BASE
#define LOW_SIZE (RAM_STACK_BASE + RAM_STACK_SIZE - RAM_PS1_BASE)
/* Top of the game stack (src/platform/main.c: GAME_STACK_BASE + SIZE). */
#define STACK_TOP 0x807C0000u

/*
 * Scratchpad ranges a routine writes and nothing reads: the original's
 * register save areas. What they hold depends on the caller's registers (in
 * the translation, its emulated ones), so the C ports do not write them and
 * the check takes the C version's contents there.
 */
static const struct {
    const char *name;
    u32 start, end;
} scratch_unread[] = {
    {"Slst_RDecodeForward", 0x1F800000u, 0x1F800040u},
    {"Slst_RDecodeBackward", 0x1F800000u, 0x1F800040u},
    {"Gte_RTransformWorlds", 0x1F800000u, 0x1F800040u},
    {"Gte_RTransformWorldsFog", 0x1F800000u, 0x1F800040u},
    {"Gte_RTransformWorldsRipple", 0x1F800000u, 0x1F800040u},
};

static int depth;
static u32 stack_from, stack_size;
static u8 *before_low, *before_stack, *mips_low, *mips_stack_copy;
static u8 before_scratch[RAM_SCRATCH_SIZE], mips_scratch[RAM_SCRATCH_SIZE];
static u8 *before_gte, *mips_gte, *native_gte;
static unsigned long calls, mismatches;
static struct {
    const char *name;
    unsigned long calls;
} counts[32];

void Oracle_Report(void);

static void *must_alloc(size_t size)
{
    /* Zeroed, so the padding of GTE snapshots compares equal. */
    void *p = calloc(1, size);
    if (p == NULL) {
        fprintf(stderr, "gte oracle: out of memory\n");
        abort();
    }
    return p;
}

int Oracle_Begin(void *frame)
{
    u32 addr;
    if (depth != 0)
        return 0;
    if (before_low == NULL) {
        before_low = must_alloc(LOW_SIZE);
        mips_low = must_alloc(LOW_SIZE);
        before_stack = must_alloc(STACK_TOP - RAM_PS1_BASE);
        mips_stack_copy = must_alloc(STACK_TOP - RAM_PS1_BASE);
        before_gte = must_alloc(Psyz_GteStateSize());
        mips_gte = must_alloc(Psyz_GteStateSize());
        native_gte = must_alloc(Psyz_GteStateSize());
        atexit(Oracle_Report);
    }
    depth = 1;
    calls++;
    /* The caller's frames start above the wrapper's frame record. */
    addr = Ram_Addr((u8 *)frame + 16);
    stack_from = addr < STACK_TOP ? addr : STACK_TOP;
    stack_size = STACK_TOP - stack_from;
    memcpy(before_low, Ram_Ptr(LOW_START), LOW_SIZE);
    memcpy(before_stack, Ram_Ptr(stack_from), stack_size);
    memcpy(before_scratch, g_ps1_scratch, RAM_SCRATCH_SIZE);
    Psyz_GteSaveState(before_gte);
    return 1;
}

void Oracle_Swap(void)
{
    memcpy(mips_low, Ram_Ptr(LOW_START), LOW_SIZE);
    memcpy(mips_stack_copy, Ram_Ptr(stack_from), stack_size);
    memcpy(mips_scratch, g_ps1_scratch, RAM_SCRATCH_SIZE);
    Psyz_GteSaveState(mips_gte);
    memcpy(Ram_Ptr(LOW_START), before_low, LOW_SIZE);
    memcpy(Ram_Ptr(stack_from), before_stack, stack_size);
    memcpy(g_ps1_scratch, before_scratch, RAM_SCRATCH_SIZE);
    Psyz_GteLoadState(before_gte);
}

/* Prints up to `limit` differing words; returns the number of them. */
static int compare(const char *what, u32 base, const u8 *before, const u8 *expect, const u8 *got,
                   u32 size, int limit)
{
    int count = 0;
    u32 i;
    for (i = 0; i + 4 <= size; i += 4) {
        u32 b, e, g;
        memcpy(&e, expect + i, 4);
        memcpy(&g, got + i, 4);
        if (e == g)
            continue;
        if (count < limit) {
            memcpy(&b, before + i, 4);
            fprintf(stderr, "  %s %08X: before %08X mips %08X native %08X\n", what, base + i, b, e, g);
        }
        count++;
    }
    return count;
}

/* GTE registers as the code reads them: data 0..31, then control 0..31. */
static void read_gte(const void *state, u32 *regs)
{
    unsigned i;
    Psyz_GteLoadState(state);
    for (i = 0; i < 32; i++) {
        regs[i] = Psyz_GteDataRead(i);
        regs[32 + i] = Psyz_GteCtrlRead(i);
    }
}

static int compare_gte(void)
{
    u32 b[64], e[64], g[64];
    int count = 0, i;
    read_gte(before_gte, b);
    read_gte(mips_gte, e);
    read_gte(native_gte, g);
    for (i = 0; i < 64; i++) {
        if (e[i] == g[i])
            continue;
        fprintf(stderr, "  gte %s %2d: before %08X mips %08X native %08X\n",
            i < 32 ? "data" : "ctrl", i & 31, b[i], e[i], g[i]);
        count++;
    }
    if (count == 0 && memcmp(mips_gte, native_gte, Psyz_GteStateSize()) != 0) {
        fprintf(stderr, "  gte: internal state differs (FIFO or unread register)\n");
        count++;
    }
    return count;
}

void Oracle_End(const char *name, u32 mips_result, u32 native_result)
{
    int diff = 0;
    int i;
    for (i = 0; i < 32; i++) {
        if (counts[i].name == NULL)
            counts[i].name = name;
        if (counts[i].name == name) {
            counts[i].calls++;
            break;
        }
    }
    for (i = 0; i < (int)(sizeof(scratch_unread) / sizeof(scratch_unread[0])); i++) {
        u32 offset = scratch_unread[i].start - RAM_SCRATCH_BASE;
        if (strcmp(scratch_unread[i].name, name) == 0)
            memcpy(mips_scratch + offset, g_ps1_scratch + offset,
                scratch_unread[i].end - scratch_unread[i].start);
    }
    Psyz_GteSaveState(native_gte);
    if (mips_result != native_result || memcmp(mips_low, Ram_Ptr(LOW_START), LOW_SIZE) != 0 ||
        memcmp(mips_stack_copy, Ram_Ptr(stack_from), stack_size) != 0 ||
        memcmp(mips_scratch, g_ps1_scratch, RAM_SCRATCH_SIZE) != 0 ||
        memcmp(mips_gte, native_gte, Psyz_GteStateSize()) != 0) {
        mismatches++;
        fprintf(stderr, "gte oracle: %s differs (call %lu)\n", name, calls);
        if (mips_result != native_result)
            fprintf(stderr, "  result: mips %08X native %08X\n", mips_result, native_result);
        diff += compare("ram", LOW_START, before_low, mips_low, Ram_Ptr(LOW_START), LOW_SIZE, 16);
        diff += compare("stack", stack_from, before_stack, mips_stack_copy, Ram_Ptr(stack_from),
            stack_size, 16);
        diff += compare("scratch", RAM_SCRATCH_BASE, before_scratch, mips_scratch, g_ps1_scratch,
            RAM_SCRATCH_SIZE, 32);
        diff += compare_gte();
        Psyz_GteLoadState(native_gte);
        fprintf(stderr, "  %d differences\n", diff);
        if (getenv("CRASH_GTE_ORACLE_CONTINUE") == NULL)
            abort();
    }
    depth = 0;
}

void Oracle_Report(void)
{
    int i;
    for (i = 0; i < 32 && counts[i].name != NULL; i++)
        fprintf(stderr, "gte oracle: %-32s %lu calls\n", counts[i].name, counts[i].calls);
    fprintf(stderr, "gte oracle: %lu checked calls, %lu mismatches\n", calls, mismatches);
}

#endif
