#include "common.h"

/* Select sequence timing using the mode returned by the video-mode query.
 * Flag 0x1000 is stored separately; values below six select preset timings.
 */

void SsSetTickMode(s32 arg0)
{
    s32 mode = GetVideoMode();

    if (arg0 & 0x1000) {
        g_ss_tick_mode_cur = 1;
        g_ss_ticks_per_sec = arg0 & 0xFFF;
    } else {
        g_ss_tick_mode_cur = 0;
        g_ss_ticks_per_sec = arg0;
    }
    if (g_ss_ticks_per_sec < 6) {
        switch (g_ss_ticks_per_sec) {
        case 4:
            g_spawns_base = 50;
            if (mode == 1) {
                g_ss_ticks_per_sec = 5;
            } else {
                g_ss_ticks_per_sec = 50;
            }
            break;
        case 1:
            g_spawns_base = 60;
            if (mode == 0) {
                g_ss_ticks_per_sec = 5;
            } else {
                g_ss_ticks_per_sec = 60;
            }
            break;
        case 3:
            g_spawns_base = 120;
            break;
        case 2:
            g_spawns_base = 240;
            break;
        case 5:
            switch (mode) {
            case 0:
                g_spawns_base = 60;
                break;
            case 1:
                g_spawns_base = 50;
                break;
            default:
                g_spawns_base = 60;
                break;
            }
            break;
        case 0:
            if (mode == 0) {
                g_spawns_base = 60;
            } else if (mode == 1) {
                g_spawns_base = 50;
            } else {
                g_spawns_base = 60;
            }
            break;
        default:
            g_spawns_base = 60;
            break;
        }
    } else {
        g_spawns_base = g_ss_ticks_per_sec;
    }
}
