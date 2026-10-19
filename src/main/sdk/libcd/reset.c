#include "common.h"

/* Acknowledge controller interrupts and reset CD status. */

#define CRASH_EXT_SDK_LIBCD_RESET
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_LIBCD_RESET

void CD_initvol(void) {
    u32 acknowledge = 7;
    u8 status;
    *g_cd_reg0 = 1;
    if (*g_cd_reg3_ptr & 7) {
        do {
            *g_cd_reg0 = 1;
            *g_cd_reg3_ptr = acknowledge;
            *g_cd_reg2_ptr = acknowledge;
        } while (*g_cd_reg3_ptr & 7);
    }
    g_cd_sense_result_buf = 0;
    status = g_cd_sense_result_buf;
    g_cd_read_mode_val = 0;
    *g_cd_in_callback = status;
    g_cd_status = 2;
    *g_cd_reg0 = 0;
    *g_cd_reg3_ptr = 0;
    *g_cd_com_delay_reg = 0x1325;
}
