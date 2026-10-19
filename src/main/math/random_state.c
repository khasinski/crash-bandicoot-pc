#include "common.h"

/* The two retail generator states both start at one. */
s32 gp_seed_a = 1;
s32 gp_seed_b = 1;

/* Advance the corresponding LCG state, normalise through unsigned division
 * and reduction, then take a signed remainder against the supplied bound.
 */

s32 Math_Randa(s32 bound)
{
    s32 value;
    if (bound != 0) {
        gp_seed_a = (s32)((u32)gp_seed_a * 0x41C64E6Du + 0x3039u);
        value = ((u32)gp_seed_a / 15) % 0x07FFFFFF;
        if (value < 0) {
            value = -value;
        }
        return value % bound;
    }
    return 0;
}

void Math_Sranda2(void)
{
    gp_seed_a = 0x3039;
}

void Math_Sranda(s32 value)
{
    gp_seed_a = value;
}

s32 Math_Randb(s32 bound)
{
    s32 value;
    if (bound != 0) {
        gp_seed_b = (s32)((u32)gp_seed_b * 0x41C64E6Du + 0x3039u);
        value = ((u32)gp_seed_b / 15) % 0x07FFFFFF;
        if (value < 0) {
            value = -value;
        }
        return value % bound;
    }
    return 0;
}

void Math_Srandb2(void)
{
    gp_seed_b = 0x3039;
}

void Math_Srandb(s32 value)
{
    gp_seed_b = value;
}
