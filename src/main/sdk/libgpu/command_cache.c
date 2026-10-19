#include "common.h"
void _ctl(u32 command) {
    *g_gpu_stat_reg = command;
    g_gpu_ctrl_cache[command >> 24] = command;
}
u8 _getctl(s32 index) {
    return g_gpu_ctrl_cache[index];
}
s32 _cwb(u32 *data, s32 count) {
    *g_gpu_stat_reg = 0x04000000;
    while (--count != -1) {
        *g_gpu_gp0_reg = *data++;
    }
    return 0;
}
