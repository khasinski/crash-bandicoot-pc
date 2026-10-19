#include "common.h"

s32 Gool_AngDiff(s32 current,s32 target) {
    s32 delta;
    current &= 0xFFF;
    target &= 0xFFF;
    delta = target - current;
    if (abs(delta) < 0x801) return delta;
    return delta > 0 ? delta - 0x1000 : delta + 0x1000;
}

s32 Gool_Seek(s32 current, s32 target, s32 step) {
    s32 delta = target - current;
    if (step > 0 && abs(delta) < step) return target;
    return delta > 0 ? current + step : current - step;
}
