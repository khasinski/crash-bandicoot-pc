#include "common.h"

/* Synchronise cached disc state while temporarily suppressing the callback. */

void CD_flush(void) {
    s32 saved_callback;
    s32 poll_count;

    poll_count = 0;
    if (g_cd_sync_disc_state < g_cd_read_retry_max) {
        saved_callback = g_cd_sync_cb;
        g_cd_sync_cb = 0;
        while (g_cd_read_sectors & 0x10) {
            if ((poll_count++ & 0xFF) == 0) {
                puts(g_str_bios_init_cd_opening_n);
            }
            CD_cw(1, 0, 0, 0);
        }
        while (CD_cw(0x16, g_cd_sync_disc_mode, 0, 0) != 0) {
            CD_cw(1, 0, 0, 0);
            puts(g_str_bios_init_cd_closing_n);
        }
        g_cd_sync_cb = saved_callback;
        g_cd_sync_disc_state = g_cd_read_retry_max;
    }
}
