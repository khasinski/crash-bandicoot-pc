#include "common.h"

void Zone_ColorSeek(s16 *current, s16 *target, s32 step) {
    s16 start;
    s16 result;
    s32 delta;
    if(step != 0) {
        start = *current;
        delta = *target - start;
        if(step < delta) {
            result = start + step;
            *target = result;
        } else if(delta < -step) {
            result = start - step;
            *target = result;
        }
    }
    *current = (u16)*target;
}
