#include "common.h"

typedef void (*CdCallback)(s32, u8 *);

typedef struct {
    u8 sync;
    u8 ready;
    u8 data;
    u8 reserved[13];
} CdStatus;

typedef struct {
    s32 ack_clear[32];
    s32 unused[32];
    s32 param_count[32];
} CdCmdTable;

#define CRASH_EXT_SDK_LIBCD_BIOS_COM
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_LIBCD_BIOS_COM

static inline void cd_set_timeout(const char *name)
{
    g_cd_timeout_deadline = VSync(-1) + 0x1E0;
    g_cd_timeout_ticks = 0;
    g_cd_timeout_cmd_name[0] = name;
}

static inline s32 cd_check_timeout(void)
{
    if (VSync(-1) > g_cd_timeout_deadline || g_cd_timeout_ticks++ > 0x1E0000) {
        puts(g_str_bios_com_cd_timeout);
        BIOS_Printf(g_str_bios_com_s_s_sync_s, g_cd_timeout_cmd_name[0], g_cd_com_str_ptrs[g_cd_com_flags_tab[0]], g_cd_intr_str_ptrs[g_cd_status.sync], g_cd_intr_str_ptrs[g_cd_status.ready]);
        CD_initvol();
        return -1;
    }
    return 0;
}

static inline void cd_poll_interrupts(void)
{
    s32 event;
    u8 bank = *g_cd_reg0 & 3;
    while ((event = getintr()) != 0) {
        if ((event & 4) && g_cd_ready_cb != NULL) {
            g_cd_ready_cb(g_cd_status.ready, g_cd_ready_result_buf);
        }
        if ((event & 2) && g_cd_sync_cb != NULL) {
            g_cd_sync_cb(g_cd_status.sync, g_cd_sync_result_buf);
        }
    }
    *g_cd_reg0 = bank;
}

static inline void cd_publish_response(u8 *dst, u8 *src)
{
    s32 count;
    if (dst != NULL) {
        s32 end;
        count = 7;
        end = -1;
        do {
            u8 val = *src++;
            count--;
            *dst++ = val;
        } while (count != end);
    }
}

s32 CD_sync(s32 mode, u8 *result)
{
    s32 status;
    cd_set_timeout(g_str_bios_com_cd_sync);
    do {
        if (cd_check_timeout() != 0) {
            return -1;
        }
        if (CheckCallback() != 0) {
            cd_poll_interrupts();
        }
        status = g_cd_status.sync;
        if (status == 2 || status == 5) {
            g_cd_status.sync = 2;
            cd_publish_response(result, g_cd_sync_result_buf);
            return status;
        }
    } while (mode == 0);
    return 0;
}

s32 CD_ready(s32 mode, u8 *result)
{
    s32 status;
    cd_set_timeout(g_str_bios_com_cd_ready);
    do {
        if (cd_check_timeout() != 0) {
            return -1;
        }
        if (CheckCallback() != 0) {
            cd_poll_interrupts();
        }
        status = g_cd_status.data;
        if (status != 0) {
            g_cd_status.data = 0;
            cd_publish_response(result, g_cd_intr_result_buf);
            return status;
        }
        status = g_cd_status.ready;
        if (status != 0) {
            g_cd_status.ready = 0;
            cd_publish_response(result, g_cd_ready_result_buf);
            return status;
        }
    } while (mode == 0);
    return 0;
}

s32 CD_cw(u8 com, u8 *param, u8 *result, s32 arg3)
{
    s32 i;
    if (g_cd_read_pos >= 2) {
        BIOS_Printf(g_str_bios_com_s_n, g_cd_com_str_ptrs[com]);
    }
    if (g_cd_com_sent_counters.param_count[com] != 0 && param == NULL) {
        if (g_cd_read_pos > 0) {
            BIOS_Printf(g_str_bios_com_s_no_param_n, g_cd_com_str_ptrs[com]);
        }
        return -2;
    }
    CD_sync(0, NULL);
    if (com == 2) {
        for (i = 0; i < 4; i++) {
            g_cd_read_retry_cnt[i] = param[i];
        }
    }
    g_cd_status.sync = 0;
    if (g_cd_com_sent_counters.ack_clear[com] != 0) {
        g_cd_status.ready = 0;
    }
    *g_cd_reg0 = 0;
    for (i = 0; i < g_cd_com_sent_counters.param_count[com]; i++) {
        *g_cd_reg2_ptr = param[i];
    }
    g_cd_com_flags_tab[0] = com;
    *g_cd_reg1_ptr = com;
    if (arg3 != 0) {
        return 0;
    }
    cd_set_timeout(g_str_bios_com_cd_cw);
    while (g_cd_status.sync == 0) {
        if (cd_check_timeout() != 0) {
            return -1;
        }
        if (CheckCallback() != 0) {
            cd_poll_interrupts();
        }
    }
    if (g_cd_status.sync == 2 && com == 0x0E) {
        g_cd_read_mode_val = *param;
    }
    cd_publish_response(result, g_cd_sync_result_buf);
    return -(g_cd_status.sync == 5);
}
