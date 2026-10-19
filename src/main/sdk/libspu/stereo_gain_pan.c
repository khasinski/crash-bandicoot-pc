#include "common.h"
void SpuVmSeKeyOn(s16 arg0, s16 arg1, s16 arg2, s32 arg3, u16 arg4, u16 arg5) {
    s16 var_v1;
    u16 var_a0;
    u32 temp_a0;
    u32 temp_v1;

    temp_a0 = arg4 & 0xFFFF;
    temp_v1 = arg5 & 0xFFFF;
    if (temp_a0 == temp_v1) {
        var_v1 = 0x40;
        var_a0 = arg4;
    } else if (temp_v1 < temp_a0) {
        var_a0 = arg4;
        var_v1 = (s32) (temp_v1 << 6) / (s32) temp_a0;
    } else {
        var_a0 = arg5;
        var_v1 = 0x7F - ((s32) (temp_a0 << 6) / (s32) temp_v1);
    }
    SpuVmKeyOn(0x21, arg0, arg1, arg2 & 0xFFFF, var_a0 & 0xFFFF, var_v1 & 0xFFFF);
}
