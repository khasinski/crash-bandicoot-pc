#include "common.h"

/* Establish the GPU operation timeout and reset its polling counter. */

#define CRASH_EXT_SDK_LIBGPU_TIMEOUT_INIT
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_LIBGPU_TIMEOUT_INIT

void set_alarm(void)
{
    g_gpu_alarm_active = VSync(-1) + 240;
    g_gpu_alarm_timeout_cnt = 0;
}

s32 get_alarm(void)
{
    if (VSync(-1) > g_gpu_alarm_active || g_gpu_alarm_timeout_cnt++ > 0xF0000) {
        *g_gpu_stat_reg;
        BIOS_Printf(g_str_timeout_init, (g_gpu_q_write - g_gpu_q_read) & 0x3F, *g_gpu_stat_reg, *g_gpu_dma2_chcr_reg, *g_gpu_dma2_madr_reg);
        BIOS_Printf(g_str_timeout_init_func_08x_08x_08x, g_gpu_queue_enabled[0], g_gpu_alarm_deadline, g_gpu_alarm_counters);
        g_gpu_reset_cnt = SetIntrMask(0);
        g_gpu_q_read = 0;
        g_gpu_q_write = g_gpu_q_read;
        *g_gpu_dma2_chcr_reg = 0x401;
        *g_gpu_dpcr_reg |= 0x800;
        *g_gpu_stat_reg = 0x02000000;
        *g_gpu_stat_reg = 0x01000000;
        SetIntrMask(g_gpu_reset_cnt);
        return -1;
    }
    return 0;
}
