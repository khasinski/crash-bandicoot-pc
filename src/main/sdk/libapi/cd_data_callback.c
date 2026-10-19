#include "common.h"

s32 CdDataCallback(s32 arg0) {
    s32 temp_v0;

    temp_v0 = g_cd_data_cb;
    g_cd_data_cb = arg0;
    return temp_v0;
}
