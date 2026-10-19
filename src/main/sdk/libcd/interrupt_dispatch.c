#include "common.h"

#define CRASH_EXT_SDK_LIBCD_INTERRUPT_DISPATCH
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_LIBCD_INTERRUPT_DISPATCH

/* The SDK uses both scalar and array views of these event bytes. */

static inline void cd_publish_response(u8 *destination, u8 *source)
{
    s32 count;
    if (destination) {
        count = 7;
        do {
            u8 value = *source++;
            count--;
            *destination++ = value;
        } while (count != -1);
    }
}

s32 getintr(void)
{
    volatile u8 interrupt;
    u8 response[8];
    s32 status, count, errors, status_mask;
    u8 *write, *remaining;
    u8 sampled;

    *g_cd_reg0 = 1;
    sampled = *g_cd_reg3_ptr & 7;
    interrupt = sampled;
    errors = 0;
    if (interrupt == 0) return 0;
    while (interrupt != (*g_cd_reg3_ptr & 7)) {
        interrupt = *g_cd_reg3_ptr & 7;
    }
    write = response;
    count = 0;
read_response:
    if (*g_cd_reg0 & 0x20) {
        count++;
        *write = *g_cd_reg1_ptr;
        write++;
        if (count < 8) goto read_response;
    }
    /* The SDK caches the response status in a word after byte normalization. */
    status_mask = 0xFF;
    if (count < 8) {
        remaining = (u8 *)((u32)count + (u32)response);
        do {
            *remaining = 0;
            count++;
            remaining++;
        } while (count < 8);
    }
    *g_cd_reg0 = 1;
    *g_cd_reg3_ptr = 7;
    *g_cd_reg2_ptr = 7;
    if ((interrupt & 0xFF) != 3 || g_cd_ack_counters_b[g_cd_com_flags_tab[0]] != 0) {
        if (!(g_cd_read_sectors & 0x10) && (response[0] & 0x10)) g_cd_read_retry_max++;
        status = response[0] & status_mask;
        errors = status & 0x1D;
        g_cd_read_sectors = status;
        g_cd_read_buf_ptr = response[1];
    }
    if ((interrupt & 0xFF) == 5) {
        puts(g_str_interrupt_dispatch_diskerror);
        if (g_cd_read_pos > 0)
            BIOS_Printf(g_str_interrupt_dispatch_com_s_code_02x, g_cd_com_str_ptrs[g_cd_com_flags_tab[0]], g_cd_read_sectors, g_cd_read_buf_ptr);
    }
    switch (interrupt) {
    case 3:
        if (errors) {
            g_cd_status[0] = 5;
            cd_publish_response(g_cd_sync_result_buf, response);
            return 2;
        }
        if (g_cd_ack_counters_a[g_cd_com_flags_tab[0]]) {
            g_cd_status[0] = 3;
            cd_publish_response(g_cd_sync_result_buf, response);
            return 1;
        }
        g_cd_status[0] = 2;
        cd_publish_response(g_cd_sync_result_buf, response);
        return 2;
    case 2:
        cd_sync_status = errors ? 5 : 2;
        cd_publish_response(g_cd_sync_result_buf, response);
        return 2;
    case 1:
        g_cd_in_callback = errors ? 5 : 1;
        cd_publish_response(g_cd_ready_result_buf, response);
        return 4;
    case 4:
        g_cd_sense_result_buf = 4;
        cd_ready_status[0] = g_cd_sense_result_buf;
        cd_publish_response(g_cd_intr_result_buf, response);
        cd_publish_response(g_cd_ready_result_buf, response);
        return 4;
    case 5:
        g_cd_in_callback = 5;
        g_cd_status[0] = g_cd_in_callback;
        cd_publish_response(g_cd_sync_result_buf, response);
        cd_publish_response(g_cd_ready_result_buf, response);
        return 6;
    default:
        puts(g_str_interrupt_dispatch_cdrom_unknown_intr);
        BIOS_Printf(g_str_interrupt_dispatch_d_n, (s32)interrupt);
        return -1;
    }
}
