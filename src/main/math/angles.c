#include "common.h"

/* Compute 12-bit angles between two points in the XZ and XY planes. */

s32 Math_AngDistXZ(s32 *origin, s32 *target)
{
    return ratan2(target[0] - origin[0], target[2] - origin[2]) & 0xFFF;
}

s32 Math_AngDistXY(s32 *origin, s32 *target)
{
    return ratan2(target[0] - origin[0], target[1] - origin[1]) & 0xFFF;
}

/* Limit the shortest 12-bit angular difference to the supplied step. */
s32 Math_AngDiffClamp(s32 current, s32 target, s32 step) {
    s32 delta;
    s32 magnitude;
    s32 limited;

    current &= 0xFFF;
    target &= 0xFFF;
    delta = target - current;
    magnitude = abs(delta);
    if (magnitude >= 0x801) {
        if (delta > 0) {
            delta -= 0x1000;
        } else {
            delta += 0x1000;
        }
    }
    magnitude = abs(delta);
    if (step < magnitude) {
        magnitude = abs(step);
        if (delta <= 0) {
            limited = 0;
            if (delta != 0) {
                limited = -magnitude;
            }
        } else {
            limited = magnitude;
        }
        delta = limited;
    }
    return delta;
}
