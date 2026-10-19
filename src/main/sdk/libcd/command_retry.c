#include "common.h"

typedef void (*CdCallback)(s32, u8 *);

s32 CdControl(u8 command, u8 *params, u8 *result) {
    s32 retries = 3;
    s32 code = command & 0xff;
    s32 *table = g_cd_com_param_len_tab;
    CdCallback saved_callback = g_cd_sync_cb;
    s32 *command_flags = &table[code];
    s32 status = 0;
    do {
        if ((code == 0x10) && !((*(u8 *)&g_cd_read_sectors) & 0x20)) {
            retries--;
            CD_flush();
        } else {
            if (code != 1) {
                g_cd_sync_cb = 0;
                CD_flush();
                if (params && *command_flags && CD_cw(2, params, result, 0)) goto retry;
                g_cd_sync_cb = saved_callback;
            }
            if (!CD_cw(command, params, result, 0)) goto done;
retry:
            retries--;
        }
    } while (retries != -1);
    g_cd_sync_cb = saved_callback;
    status = -1;
done:
    return status + 1;
}

s32 CdControlF(u8 command, u8 *params) {
    s32 retries = 3;
    s32 code = command & 0xff;
    s32 *table = g_cd_com_param_len_tab;
    CdCallback saved_callback = g_cd_sync_cb;
    s32 *command_flags = &table[code];
    s32 status = 0;
    do {
        if ((code == 0x10) && !((*(u8 *)&g_cd_read_sectors) & 0x20)) {
            retries--;
            CD_flush();
        } else {
            if (code != 1) {
                g_cd_sync_cb = 0;
                CD_flush();
                if (params && *command_flags && CD_cw(2, params, NULL, 0)) goto retry;
                g_cd_sync_cb = saved_callback;
            }
            if (!CD_cw(command, params, NULL, 1)) goto done;
retry:
            retries--;
        }
    } while (retries != -1);
    g_cd_sync_cb = saved_callback;
    status = -1;
done:
    return status + 1;
}

s32 CdControlB(u8 command, u8 *params, u8 *result) {
    s32 retries = 3;
    s32 code = command & 0xff;
    s32 *table = g_cd_com_param_len_tab;
    CdCallback saved_callback = g_cd_sync_cb;
    s32 *command_flags = &table[code];
    s32 status;
    s32 success;
    do {
        if (code == 0x10 && !((*(u8 *)&g_cd_read_sectors) & 0x20)) {
            retries--;
            CD_flush();
            status = -1;
        } else {
            if (code != 1) {
                g_cd_sync_cb = 0;
                CD_flush();
                if (params && *command_flags && CD_cw(2,params,result,0)) goto retry;
                g_cd_sync_cb = saved_callback;
            }
            if (CD_cw(command,params,result,0) == 0) {
                status = 0;
                goto done;
            }
retry:
            retries--;
            status = -1;
        }
    /* Retry paths set status to -1, the exhausted-counter sentinel. */
    } while (retries != status);
    g_cd_sync_cb = saved_callback;
done:
    if (status == 0) success = CD_sync(0,result) == 2;
    else success = 0;
    return success;
}
