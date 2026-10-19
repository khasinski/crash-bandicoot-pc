#include "common.h"

s32 CdSyncCallback(s32 arg0) {
    s32 temp_v0;

    temp_v0 = g_cd_sync_cb;
    g_cd_sync_cb = arg0;
    return temp_v0;
}
