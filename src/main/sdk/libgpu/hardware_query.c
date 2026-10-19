#include "common.h"

s32 _version(s32 arg0) {
    *g_gpu_stat_reg = 0x10000007;
    if ((*g_gpu_gp0_reg & 0xFFFFFF) != 2) {
        *g_gpu_gp0_reg = (*g_gpu_stat_reg & 0x3FFF) | 0xE1001000;
        /* Retail consumes this GPU data read before querying status. */
        (void)*g_gpu_gp0_reg;
        if (!(*g_gpu_stat_reg & 0x1000)) {
            return 0;
        }
        if (!(arg0 & 8)) {
            return 1;
        }
        *g_gpu_stat_reg = 0x20000504;
        return 2;
    }
    if (!(arg0 & 8)) return 3;
    *g_gpu_stat_reg = 0x09000001;
    return 4;
}
