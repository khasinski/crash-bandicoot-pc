#include "common.h"

typedef void (*CdCallback)(s32, u8 *);

typedef struct CdReadState {
    volatile s32 state;
    s32 buffer;
    s32 count;
    volatile s32 cursor;
    volatile s32 mode;
    s32 size;
    volatile s32 remaining;
    s32 time;
    CdCallback word20;
    CdCallback word24;
} CdReadState;
#define CRASH_EXT_SDK_LIBCD_BIOS_INIT_0
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_LIBCD_BIOS_INIT_0

typedef struct {
    u8 sync;
    u8 ready;
    u8 data;
    u8 reserved[13];
} CdStatus;

#define CRASH_EXT_SDK_LIBCD_BIOS_INIT_1
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_LIBCD_BIOS_INIT_1

static inline void cd_sync_disc(void)
{
    CdCallback saved_callback;
    s32 poll_count = 0;

    if (g_cd_sync_disc_state < g_cd_read_retry_max) {
        saved_callback = g_cd_sync_cb;
        g_cd_sync_cb = NULL;
        while (g_cd_read_sectors & 0x10) {
            if ((poll_count++ & 0xFF) == 0) {
                puts(g_str_bios_init_cd_opening_n);
            }
            CD_cw(1, NULL, NULL, 0);
        }
        while (CD_cw(0x16, g_cd_sync_disc_mode, NULL, 0) != 0) {
            CD_cw(1, NULL, NULL, 0);
            puts(g_str_bios_init_cd_closing_n);
        }
        g_cd_sync_cb = saved_callback;
        g_cd_sync_disc_state = g_cd_read_retry_max;
    }
}

static inline void cd_reset_hw(void)
{
    *g_cd_reg0 = 1;
    while (*g_cd_reg3_ptr & 7) {
        *g_cd_reg0 = 1;
        *g_cd_reg3_ptr = 7;
        *g_cd_reg2_ptr = 7;
    }
    g_cd_read_mode_val = 0;
    g_cd_status.ready = (g_cd_status.data = 0);
    g_cd_status.sync = 2;
    *g_cd_reg0 = 0;
    *g_cd_reg3_ptr = 0;
    *g_cd_com_delay_reg = 0x1325;
}

static inline void cd_init_callbacks(void)
{
    s32 *ptr = (s32 *)&g_cd_read_state;
    s32 count = 9;
    g_cd_ready_cb = NULL;
    g_cd_sync_cb = NULL;
    g_cd_read_buf_ptr = 0;
    g_cd_read_sectors = 0;
    do {
        *ptr = 0;
        count--;
        ptr++;
    } while (count != -1);
    ResetCallback();
    InterruptCallback(2, &cd_callback);
}

s32 CD_reset(void)
{
    puts(g_str_bios_init_cd_init);
    BIOS_Printf(g_str_bios_init_addr_08x_n, g_spu_voice_reg_base);
    g_cd_com_flags_tab[0] = 0;
    g_cd_read_mode_val = 0;
    cd_init_callbacks();
    cd_reset_hw();
    CD_cw(1, NULL, NULL, 0);
    if (g_cd_read_sectors & 0x10) {
        CD_cw(1, NULL, NULL, 0);
    }
    cd_sync_disc();
    if (CD_cw(0x0A, NULL, NULL, 0) != 0) {
        return -1;
    }
    if (CD_cw(0x0C, NULL, NULL, 0) != 0) {
        return -1;
    }
    return -(CD_sync(0, NULL) != 2);
}

s32 CD_read(void)
{
    u8 mode;
    g_cd_ready_cb = NULL;
    g_cd_sync_cb = NULL;
    while (g_cd_read_state.state-- > 0) {
        if (g_cd_read_state.state < 7) {
            puts(g_str_bios_init_cd_read_retry);
            BIOS_Printf(g_str_bios_init_d_pos_02x_02x, g_cd_read_state.state, g_cd_read_retry_cnt[0], g_cd_read_retry_cnt[1], g_cd_read_retry_cnt[2]);
            cd_sync_disc();
            if (CD_cw(9, NULL, NULL, 0) != 0) {
                continue;
            }
            if (CD_cw(2, g_cd_read_retry_cnt, NULL, 0) != 0) {
                continue;
            }
        }
        cd_reset_hw();
        if ((mode = g_cd_read_state.mode) != g_cd_read_mode_val) {
            if (CD_cw(0x0E, &mode, NULL, 0) != 0) {
                continue;
            }
        }
        g_cd_ready_cb = cb_read;
        g_cd_read_state.cursor = g_cd_read_state.count;
        CD_cw(6, NULL, NULL, 1);
        g_cd_read_state.remaining = g_cd_read_state.buffer;
        g_cd_read_state.time = VSync(-1) + 0x1E0;
        return g_cd_read_state.remaining;
    }
    return g_cd_read_state.remaining = -1;
}
