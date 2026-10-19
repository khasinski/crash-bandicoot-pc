#include "common.h"

s32 VSync(s32 mode)
{
    u32 status = *g_vsync_gpu_stat_reg;
    s32 hsync = (*g_vsync_rcnt1_reg - g_vsync_prev_hblank) & 0xFFFF;

    if (mode < 0) {
        return g_vsync_cb_active_cnt;
    }
    if (mode == 1) {
        return hsync;
    }
    v_wait(mode > 0 ? g_vsync_total_hblanks + (mode - 1) : g_vsync_total_hblanks, mode > 0 ? mode - 1 : 0);
    status = *g_vsync_gpu_stat_reg;
    v_wait(g_vsync_cb_active_cnt + 1, 1);
    if (status & 0x80000) {
        while (((status ^ *g_vsync_gpu_stat_reg) & 0x80000000) == 0) {
        }
    }
    g_vsync_total_hblanks = g_vsync_cb_active_cnt;
    g_vsync_prev_hblank = *g_vsync_rcnt1_reg;
    return hsync;
}

void v_wait(s32 target, s32 count)
{
    volatile s32 timeout = count << 15;

    while (g_vsync_cb_active_cnt < target) {
        if (--timeout == -1) {
            puts(g_str_vsync_vsync_timeout_n);
            ChangeClearPAD(0);
            ChangeClearRCnt(3, 0);
            break;
        }
    }
}
