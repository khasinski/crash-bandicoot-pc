#include "common.h"

void _SsStart(s32 arg0) {
    s32 i;
    u16 target;
    u32 rcnt;
    u8 *flag_p;

    i = 0;
loop:
    i += 3;
    if (i < 3000) {
        goto loop;
    }
    flag_p = &g_ss_started_flag;
    *flag_p = 0;
    g_ss_vblank_flag = 0;
    g_ss_vsync_prev_cb = 0;
    g_ss_max_programs = 6;
    rcnt = 0xF2000002;
    switch (g_ss_ticks_per_sec) {
    case 0:
        g_ss_max_programs = 0x7F;
        return;
    case 5:
        g_ss_max_programs = 0;
        if (arg0 == 0) {
            *flag_p = 1;
        } else {
            rcnt = 0xF2000003;
            target = 1;
        }
        break;
    case 3:
        target = 0x89D0;
        break;
    case 2:
        target = 0x44E8;
        break;
    default:
        {
            struct { s32 val; u8 reserved[9]; u8 flag_d; } *p = (void *)&g_ss_tick_mode_cur;
            s32 tmp;
            if (*(s32 *)p != 0) {
                return;
            }
            tmp = (s32)p & 1;
            if ((tmp >> 1) == 0) {
                goto L_after;
            }
            if (*flag_p) {
                goto L_after;
            }
            *flag_p = 0;
            return;
        L_after:
            if (*((s32 *)p - 1) < 70) {
                target = 0x204CC0 / *((s32 *)p - 1);
                p->flag_d++;
            } else {
                target = 0x409980 / *((s32 *)p - 1);
            }
        }
        break;
    }
    if (g_ss_started_flag != 0) {
        EnterCriticalSection();
        VSyncCallback(g_ss_tick_cb);
    } else {
        EnterCriticalSection();
        ResetRCnt(rcnt);
        SetRCnt(rcnt, target, 0x1000);
        if (g_ss_max_programs == 0) {
            g_ss_vsync_prev_cb = InterruptCallback(0, 0);
            InterruptCallback(g_ss_max_programs, _SsTrapIntrVSync);
        } else if (g_ss_vblank_flag == 0) {
            InterruptCallback(g_ss_max_programs, g_ss_tick_cb);
        } else {
            InterruptCallback(g_ss_max_programs, _SsSeqCalledTbyT_1per2);
        }
    }
    ExitCriticalSection();
}
