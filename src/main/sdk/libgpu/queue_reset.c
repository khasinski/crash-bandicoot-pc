#include "common.h"

/* The queue consumer callback also updates this index. */

#define CRASH_EXT_SDK_LIBGPU_QUEUE_RESET
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_LIBGPU_QUEUE_RESET

/* Reset queue indices and hardware, then restore interrupt state. */
s32 _reset(s32 arg0) {
    s32 temp_v0;
    s32 temp_v1;
    s32 var_v0;

    temp_v0 = SetIntrMask(0);
    g_gpu_q_read = 0;
    g_gpu_reset_cnt = temp_v0;
    g_gpu_q_write = g_gpu_q_read;
    temp_v1 = arg0 & 7;
    switch (temp_v1) {                              /* irregular */
    case 0:
        *g_gpu_dma2_chcr_reg = 0x401;
        *g_gpu_dpcr_reg |= 0x800;
        *g_gpu_stat_reg = 0;
        memset(g_gpu_ctrl_cache, 0, 0x100);
        memset(g_gpu_queue_entries, 0, 0x1800);
        break;
    case 1:
        *g_gpu_dma2_chcr_reg = 0x401;
        *g_gpu_dpcr_reg |= 0x800;
        *g_gpu_stat_reg = 0x02000000;
        *g_gpu_stat_reg = 0x01000000;
        break;
    }
    SetIntrMask(g_gpu_reset_cnt);
    if (!(arg0 & 7)) {
        var_v0 = _version(arg0);
    } else {
        var_v0 = 0;
    }
    return var_v0;
}
