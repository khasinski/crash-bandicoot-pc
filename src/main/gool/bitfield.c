#include "common.h"

/* Sign-extend the low bits of a GOOL operand. */

s32 Gool_SignShift(s32 value, s32 bits)
{
    s32 shift = 0x20 - bits;
    return (s32)((u32)value << shift) >> shift;
}
