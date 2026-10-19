#include "common.h"

s32 note2pitch2(s32 arg0, s32 arg1) {
    s16 temp_v0;
    s16 temp_v0_2;
    s32 temp_a1;
    s16 var_a0;
    u32 note;
    s32 var_a2;
    s32 var_v0;
    u16 var_v1;
    u8 *temp_v1;

    temp_v1 = (u8 *)(((g_spu_vm_cur_tone_idx + (g_spu_vm_cur_prog_tones * 0x10)) << 5) + g_ns_vpage_pool);
    var_v0 = (arg1 & 0xFFFF) + temp_v1[5];
    var_a2 = 0;
    if (var_v0 < 0) {
        var_v0 += 7;
    }
    temp_a1 = var_v0 >> 3;
    var_a0 = temp_a1;
    if (temp_a1 >= 0x10) {
        var_a2 = 1;
        var_a0 = temp_a1 - 0x10;
    }
    note = (u32)arg0 + 0x3C;
    temp_v0 = var_a2 + (note - temp_v1[4]);
    temp_v0_2 = (temp_v0 / 12) - 5;
    var_v1 = g_spu_vm_reverb_mask_hi[((temp_v0 % 12) * 0x10) + (s16) var_a0];
    if (temp_v0_2 > 0) {
        var_v1 <<= temp_v0_2;
    } else if (temp_v0_2 < 0) {
        var_v1 = (u16) ((s32) (var_v1 & 0xFFFF) >> -temp_v0_2);
    }
    return var_v1 & 0xFFFF;
}
