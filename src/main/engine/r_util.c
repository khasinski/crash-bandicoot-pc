#include "common.h"

/*
 * Small routines of the hand-written MIPS block (0x80033FBC..0x80034608).
 */

/* Copies `count` halfwords. The original unrolls eight halfwords at a time
 * with word accesses for each source/destination alignment, and ends in the
 * tails the symbol table calls Gpu_RResetOT_0..3. */
void RMemcpy(const void *src, void *dst, s32 count)
{
    if (count > 0)
        memmove(dst, src, (size_t)count * 2);
}

/* Links `count` ordering-table entries forward, each to the next one (24-bit
 * PS1 address), and terminates the last with 0xFFFFFF. The original fills 64
 * entries per pass, so `count` is a multiple of 64. */
void Gpu_RResetOT(u32 *ot, s32 count)
{
    u32 link = Ram_Addr(ot) & 0xFFFFFF;
    s32 i, n = (count >> 6) << 6;

    for (i = 0; i < n; i++) {
        link += 4;
        ot[i] = link;
    }
    ot[n - 1] = 0xFFFFFF;
}

/* (x * x + y * y) >> 13 with a 64-bit sum. */
s32 RSqrMagnitude2(s32 x, s32 y)
{
    u64 sum = (u64)((s64)x * x) + (u64)((s64)y * y);
    return (s32)(u32)(sum >> 13);
}

/* (x * x + y * y + z * z) >> 20 with a 64-bit sum. */
s32 RSqrMagnitude3(s32 x, s32 y, s32 z)
{
    u64 sum = (u64)((s64)x * x) + (u64)((s64)y * y) + (u64)((s64)z * z);
    return (s32)(u32)(sum >> 20);
}
