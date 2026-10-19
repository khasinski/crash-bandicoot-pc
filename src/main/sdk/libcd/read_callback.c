#include "common.h"

/* Consume a completed sector, advance read state and dispatch completion. */
#include "cd_read_state.h"
                                  /* extern */
                        /* extern */
                                  /* extern */
                          /* extern */

void cb_read(s32 arg0, void *arg1) {
    s32 var_a0;

    if (arg0 == 1) {
        if (g_cd_read_state.remaining > 0) {
            CD_getsector(g_cd_read_state.cursor, g_cd_read_state.size);
            g_cd_read_state.cursor += g_cd_read_state.size * 4;
            g_cd_read_state.remaining--;
        }
    } else {
        g_cd_read_state.remaining = -1;
    }
    g_cd_read_state.time = VSync(-1);
    if ((g_cd_read_state.remaining < 0) && (g_cd_read_state.state > 0)) {
        CD_read();
    }
    if (g_cd_read_state.remaining <= 0) {
        g_cd_sync_cb = g_cd_read_state.word20;
        g_cd_ready_cb = g_cd_read_state.word24;
        CD_cw(9, 0, 0, 0);
        if (g_cd_data_cb != NULL) {
            var_a0 = 5;
            if (g_cd_read_state.remaining == 0) {
                var_a0 = 2;
            }
            g_cd_data_cb(var_a0, arg1);
        }
    }
}
