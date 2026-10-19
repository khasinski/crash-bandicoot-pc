#include "common.h"

#define CRASH_EXT_SDK_LIBSPU_VOICE_TRIGGER
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_LIBSPU_VOICE_TRIGGER

/* Retail caller passes a byte in a0 and the pitch in a1; a0 is unused here. */
void SpuVmKeyOnNow(u8 arg0, u16 arg1) {
    s32 master_volume;
    s16 shadow_index;
    u16 *left_destination, *right_destination;
    union { s32 signed_value; u32 unsigned_value; } gain;
    s32 var_a2_2;
    s32 var_a3_2;
    u16 temp_a1;
    u16 temp_v1_2;
    u32 var_a1;
    u32 var_a2;
    u32 var_a3;
    union { s32 signed_value; u32 unsigned_value; u8 *pointer; } squared;
    SequenceState *temp_a0;

    master_volume = g_vab_cur_master_vol[0x18];
    var_a3 = (s32)(g_spu_vm_cur_pan * ((master_volume << 14) - master_volume)) / 16129;
    gain.signed_value = (s32)var_a3 * g_spu_vm_cur_prog_vol;
    var_a2 = (u32)(gain.unsigned_value * g_spu_vm_cur_tone_vol) / 16129U;
    shadow_index = g_spu_vm_last_voice[0] * 8;
    temp_a0 = g_ss_score_table[g_spu_vm_cur_seq_sep_id & 255] + ((u32)(g_spu_vm_cur_seq_sep_id << 16) >> 24);
    var_a3 = var_a2;
    if ((s16) g_spu_vm_cur_seq_sep_id != 0x21) {
        var_a3 = (u32) (var_a2 * temp_a0->voll) / 127U;
        var_a2 = (u32) (var_a2 * temp_a0->volr) / 127U;
    }
    if ((u8) g_spu_vm_cur_seq_pan < 0x40U) {
        var_a2 = (u32) (var_a2 * g_spu_vm_cur_seq_pan) / 63U;
        var_a1 = var_a3;
    } else {
        var_a1 = (u32) (var_a3 * (0x7F - g_spu_vm_cur_seq_pan)) / 63U;
    }
    if ((u8) g_spu_vm_cur_tone_pan < 0x40U) {
        var_a2 = (u32) (var_a2 * g_spu_vm_cur_tone_pan) / 63U;
    } else {
        var_a1 = (u32) (var_a1 * (0x7F - g_spu_vm_cur_tone_pan)) / 63U;
    }
    if ((u8) g_spu_vm_cur_prog_pan < 0x40U) {
        var_a2 = (u32) (var_a2 * g_spu_vm_cur_prog_pan) / 63U;
    } else {
        var_a1 = (u32) (var_a1 * (0x7F - g_spu_vm_cur_prog_pan)) / 63U;
    }
    if (g_spu_vm_mono_flag == 1) {
        if (var_a1 < var_a2) {
            var_a1 = var_a2;
        } else {
            var_a2 = var_a1;
        }
    }
    squared.unsigned_value = (u32)(var_a1 * var_a1);
    var_a1 = (u32)squared.signed_value / 16383U;
    squared.unsigned_value = (u32)(var_a2 * var_a2);
    var_a2 = (u32)squared.signed_value / 16383U;
    squared.pointer = g_spu_hw_voice_pitch_ptr;
    ((s16 *)g_spu_hw_voice_pitch_ptr)[(u16)shadow_index] = arg1;
    left_destination = (u16 *)(squared.pointer - 4);
    left_destination += (u16)shadow_index;
    squared.pointer -= 2;
    right_destination = (u16 *)squared.pointer + (u16)shadow_index;
    *left_destination = var_a1;
    *right_destination = var_a2;
    g_card_ev_table[g_spu_vm_last_voice[0]] = g_card_ev_table[g_spu_vm_last_voice[0]] | 7;
    *(s16 *)(g_spu_vm_voice_pitch + g_spu_vm_last_voice[0] * 0x34) = arg1;
    g_spu_vm_voice_auto_flag[g_spu_vm_last_voice[0] * 0x34] = 1;
    if (g_spu_vm_last_voice[0] < 0x10) {
        var_a3_2 = 1 << g_spu_vm_last_voice[0];
        var_a2_2 = 0;
    } else {
        var_a3_2 = 0;
        var_a2_2 = 1 << (g_spu_vm_last_voice[0] - 0x10);
    }
    if (g_spu_vm_cur_tone_mode & 4) {
        g_spu_vm_noise_mask_lo = var_a3_2 | g_spu_vm_noise_mask_lo;
        g_spu_vm_noise_mask_hi = var_a2_2 | g_spu_vm_noise_mask_hi;
    } else {
        g_spu_vm_noise_mask_lo &= ~var_a3_2;
        g_spu_vm_noise_mask_hi &= ~var_a2_2;
    }
    temp_v1_2 = var_a3_2 | g_spu_vm_key_on_lo;
    g_spu_vm_key_on_lo = temp_v1_2;
    temp_a1 = var_a2_2 | g_spu_vm_key_on_hi;
    g_spu_vm_key_on_hi = temp_a1;
    g_spu_vm_active_mask_lo &= ~temp_v1_2;
    g_spu_vm_active_mask_hi &= ~temp_a1;
}
