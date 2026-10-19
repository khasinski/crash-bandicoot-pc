#include "common.h"

/* Read a packet link as a KSEG0 address, or test its 24-bit end-of-chain sentinel. */

u32 NextPrim(u32 *packet)
{
    return (*packet & 0xFFFFFF) | 0x80000000;
}

s32 IsEndPrim(u32 *packet)
{
    return (*packet & 0xFFFFFF) == 0xFFFFFF;
}
