#include "common.h"

typedef void (*Callback)(void);

void SsQuit(void)
{
    if (g_ss_tick_mode_cur == 0) {
        g_ss_vblank_flag = 0;
        if (g_ss_max_programs != 0x7F) {
            EnterCriticalSection();
            if (g_ss_started_flag != 0) {
                VSyncCallback(0);
                g_ss_started_flag = 0;
            } else if (g_ss_max_programs == 0) {
                InterruptCallback(0, g_ss_vsync_prev_cb);
                g_ss_vsync_prev_cb = NULL;
            } else {
                InterruptCallback(6, NULL);
            }
            ExitCriticalSection();
            g_ss_max_programs = 0x7F;
        }
    }
}
