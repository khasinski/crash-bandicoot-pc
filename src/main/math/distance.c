#include "common.h"

/* Absolute difference of two signed values, preserving the retail subtraction order. */

s32 Math_Absdiff(s32 a, s32 b)
{
    s32 delta = a - b;

    if (delta < 0)
        delta = b - a;
    return delta;
}
