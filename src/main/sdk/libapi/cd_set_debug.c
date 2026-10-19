#include "common.h"

s32 CdSetDebug(s32 arg0) {
    s32 temp_v0;

    temp_v0 = g_cd_read_pos;
    g_cd_read_pos = arg0;
    return temp_v0;
}
