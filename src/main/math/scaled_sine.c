#include "common.h"
s32 Math_SinScale(s32 value,s32 scale) {
    return (Math_Sin((value << 10) / scale) * scale) >> 12;
}
