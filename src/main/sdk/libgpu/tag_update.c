#include "common.h"

/* Replace or terminate the 24-bit packet link while preserving the high length byte. */

void CatPrim(u32 *packet, u32 address)
{
    *packet = (*packet & 0xFF000000) | (address & 0xFFFFFF);
}

void TermPrim(u32 *packet)
{
    *packet |= 0xFFFFFF;
}
