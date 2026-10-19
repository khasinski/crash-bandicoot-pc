#include "common.h"

s32 note2pitch(void) {
    s32 temp_v0;
    s32 semitone_index;
    s32 quotient;
    s32 remainder;
    s32 temp_v1;
    u32 note;
    u16 var_v1;
    u32 var_v1_2;
    s32 var_a0;

    var_a0 = g_spu_vm_cur_tone_shift;
    note = (u32)g_spu_vm_cur_note + 0x3C;
    temp_v1 = note - g_spu_vm_cur_tone_center;
    if ((s32) var_a0 < 0) {
        var_a0 += 7;
    }
    quotient = temp_v1 / 12;
    remainder = temp_v1 % 12;
    var_v1_2 = (u32)var_a0 >> 3;
    if (var_v1_2 >= 0x10U) {
        var_v1_2 = 0xF;
    }
    semitone_index = remainder * 0x10;
    semitone_index += var_v1_2 & 0xFFFF;
    var_v1 = g_spu_vm_reverb_mask_hi[semitone_index];
    temp_v0 = quotient - 5;
    if (temp_v0 > 0) {
        var_v1 <<= temp_v0;
    } else if (temp_v0 < 0) {
        var_v1 = (u16) ((s32) (var_v1 & 0xFFFF) >> -temp_v0);
    }
    return var_v1 & 0xFFFF;
}
