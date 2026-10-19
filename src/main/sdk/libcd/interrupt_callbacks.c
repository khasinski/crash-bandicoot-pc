#include "common.h"
typedef void (*CdCallback)(s32,void*);
#define CRASH_EXT_SDK_LIBCD_INTERRUPT_CALLBACKS
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_LIBCD_INTERRUPT_CALLBACKS
void cd_callback(void) {
    s32 event;
    s32 pending;
    u8 bank;
    volatile u8 *ready_status;

    bank = *g_cd_reg0 & 3;
    ready_status=g_cd_in_callback;
loop_1:
    pending = getintr();
    event = pending;
    if (pending != 0) {
        pending = event;
        if (event & 4) {
            if (g_cd_ready_cb[0] != NULL) {
                g_cd_ready_cb[0](ready_status[0], g_cd_ready_result_buf);
            }
            pending = event;
        }
        if (((pending & 2) != 0) && (g_cd_sync_cb[0] != NULL)) {
            g_cd_sync_cb[0](g_cd_status[0], g_cd_sync_result_buf);
        }
        goto loop_1;
    }
    *g_cd_reg0 = bank;
}
