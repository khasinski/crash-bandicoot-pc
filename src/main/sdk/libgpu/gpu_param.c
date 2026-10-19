#include "common.h"

s32 _param(s32 arg0) {
    *g_gpu_stat_reg = arg0 | 0x10000000;
    return *g_gpu_gp0_reg & 0xFFFFFF;
}
