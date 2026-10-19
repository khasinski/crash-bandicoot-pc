#include "common.h"

#define CRASH_EXT_SDK_LIBSPU_NOISE_VOICE_0
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_LIBSPU_NOISE_VOICE_0
typedef struct { s16 pitch; u8 reserved[50]; } VoicePitch;
#define CRASH_EXT_SDK_LIBSPU_NOISE_VOICE_1
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_LIBSPU_NOISE_VOICE_1
typedef struct { u8 flags; u8 reserved[51]; } VoiceFlags;
#define CRASH_EXT_SDK_LIBSPU_NOISE_VOICE_2
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_LIBSPU_NOISE_VOICE_2

typedef struct { s16 left; s16 right; u8 reserved[12]; } VoiceVol;
#define CRASH_EXT_SDK_LIBSPU_NOISE_VOICE_3
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_LIBSPU_NOISE_VOICE_3

void vmNoiseOn(s16 arg0) {
    s32 reserved[2];
    s16 var_a1_2;
    s16 var_a2_2;
    s16 var_a3;
    s32 temp_v1_2;
    u16 temp_a0_2;
    u16 temp_v0_2;
    u32 temp_a0;
    u32 temp_lo;
    u32 temp_lo_2;
    u32 var_a1;
    u32 var_a2;
    s32 left_scale, right_scale;
    u16 control;
    u32 packed_id;
    SequenceState *temp_v1;

    packed_id = (*(u16 *)&g_spu_vm_cur_seq_sep_id);
    temp_v1 = g_ss_score_table[packed_id & 255] + (packed_id >> 8);
    left_scale = temp_v1->voll;
    temp_lo = (u32)(((left_scale << 7) + left_scale) * g_spu_vm_cur_prog_vol) / 127U;
    right_scale = temp_v1->volr;
    temp_lo_2 = (u32)(((right_scale << 7) + right_scale) * g_spu_vm_cur_prog_vol) / 127U;
    temp_lo = (temp_lo * g_spu_vm_cur_tone_vol) / 127U;
    temp_lo_2 = (temp_lo_2 * g_spu_vm_cur_tone_vol) / 127U;
    control = g_snd_tempo[0x1AA / 2];
    if ((u8) g_spu_vm_cur_seq_pan < 0x40U) {
        var_a2 = (u32) (temp_lo_2 * g_spu_vm_cur_seq_pan) / 63U;
        var_a1 = temp_lo;
    } else {
        var_a2 = temp_lo_2;
        var_a1 = (u32) (temp_lo * (0x7F - g_spu_vm_cur_seq_pan)) / 63U;
    }
    if ((u8) g_spu_vm_cur_tone_pan < 0x40U) {
        var_a2 = (u32) (var_a2 * g_spu_vm_cur_tone_pan) / 63U;
    } else {
        var_a1 = (u32) (var_a1 * (0x7F - g_spu_vm_cur_tone_pan)) / 63U;
    }
    if ((u8) g_spu_vm_cur_prog_pan < 0x40U) {
        var_a2 = (u32) (g_spu_vm_cur_prog_pan * var_a2) / 63U;
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
    control &= 0xC0FF;
    control |= ((g_spu_vm_cur_note - g_spu_vm_cur_tone_center) & 0x3F) << 8;
    g_snd_tempo[0x1AA / 2] = control;
    temp_a0 = arg0 & 0xFF;
    temp_v1_2 = temp_a0 * 0x10;
    *(s16 *)((u32)&g_midi_seq_table[0].right + temp_v1_2) = (s16) var_a2;
    *(s16 *)((u32)&g_midi_seq_table[0].left + temp_v1_2) = (s16) var_a1;
    g_card_ev_table[temp_a0] = g_card_ev_table[temp_a0] | 3;
    if (temp_a0 < 0x10U) {
        var_a3 = 1 << temp_a0;
        var_a2_2 = 0;
    } else {
        var_a3 = 0;
        var_a2_2 = 1 << (temp_a0 - 0x10);
    }
    g_spu_vm_voice_pitch[arg0 & 0xFF].pitch = 0xA;
    var_a1_2 = 0;
    if (gp_spawn_count != 0) {
        do {
            g_spu_vm_voice_auto_flag[var_a1_2].flags &= 1;
            var_a1_2++;
        } while (var_a1_2 < gp_spawn_count);
    }
    g_spu_vm_voice_auto_flag[arg0 & 0xFF].flags = 2;
    temp_v0_2 = var_a3 | g_spu_vm_key_on_lo;
    g_spu_vm_key_on_lo = temp_v0_2;
    temp_a0_2 = var_a2_2 | g_spu_vm_key_on_hi;
    g_spu_vm_key_on_hi = temp_a0_2;
    g_spu_vm_active_mask_lo &= ~temp_v0_2;
    g_spu_vm_active_mask_hi &= ~temp_a0_2;
    if (g_spu_vm_cur_tone_mode & 4) {
        g_spu_vm_noise_mask_lo |= var_a3;
        g_spu_vm_noise_mask_hi |= var_a2_2;
    } else {
        g_spu_vm_noise_mask_lo &= ~var_a3;
        g_spu_vm_noise_mask_hi &= ~var_a2_2;
    }
    g_snd_tempo[0x194 / 2] = var_a3;
    g_snd_tempo[0x196 / 2] = var_a2_2;
}
