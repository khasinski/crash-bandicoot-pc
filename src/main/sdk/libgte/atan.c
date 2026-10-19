#include "common.h"

/* Compute a 12-bit signed angle with the retail lookup table and overflow-aware ratio scaling. */

s32 ratan2(s32 x, s32 y) {
    s32 angle;
    s32 negative_y;
    s32 negative_x;

    negative_y = 0;
    negative_x = 0;
    if (y < 0) {
        negative_y = 1;
        y = -y;
    }
    if (x < 0) {
        negative_x = 1;
        x = -x;
    }
    if ((y == 0) && (x == 0)) {
        return 0;
    }
    if (x < y) {
        if (x & 0x7FE00000) {
            x = x / (y >> 0xA);
        } else {
            x = (x << 0xA) / y;
        }
        angle = g_gte_atan_lut[x];
    } else {
        if (y & 0x7FE00000) {
            x = y / (x >> 0xA);
        } else {
            x = (y << 0xA) / x;
        }
        angle = 0x400 - g_gte_atan_lut[x];
    }
    if (negative_y != 0) {
        angle = 0x800 - angle;
    }
    if (negative_x != 0) {
        angle = -angle;
    }
    return angle;
}
