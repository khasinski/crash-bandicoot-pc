#include "common.h"

typedef struct { s16 value; u8 reserved[50]; } VoiceWord;
#define CRASH_EXT_SDK_LIBSPU_VOICE_UPDATE_0
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_LIBSPU_VOICE_UPDATE_0
typedef struct { s16 left; s16 right; u8 reserved[12]; } VoiceVol;

#define CRASH_EXT_SDK_LIBSPU_VOICE_UPDATE_1
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_LIBSPU_VOICE_UPDATE_1

s32 SpuVmSetVol(s16 arg0, s16 arg1, s16 arg2, s32 arg3, u16 arg4) {
    s16 temp_a0_2;
    s32 var_s6;
    u8 var_t2;
    u32 temp_lo;
    u32 temp_lo_2;
    u32 var_a1;
    u32 var_t1;
    u8 temp_v1_3;
    u8 temp_v1_4;
    u32 temp_v1_5;
    s32 master_volume;
    SequenceState *temp_s2;
    u8 r;
    u32 r2;

    var_s6 = 0;
    temp_s2 = g_ss_score_table[arg0 & 255] + (((u32)(arg0 << 0x10)) >> 24);
    SpuVmVSetUp(arg1, arg2);
    g_spu_vm_cur_seq_sep_id = arg0;
    if (arg4 == 0) {
        arg4 = 1;
    }
    if (!(arg3 & 0xFFFF)) {
        arg3 = 1;
    }
    for (var_t2 = 0; var_t2 < gp_spawn_count; var_t2++) {
        if ((g_spu_vm_voice_seq_sep[var_t2].value == arg0) && (g_spu_vm_voice_tone_prog[var_t2].value == arg2) && (g_spu_vm_voice_vab_id[var_t2].value == arg1)) {
            if ((temp_s2->gains[temp_s2->channel] != (arg3 & 0xFFFF)) && (temp_s2->gains[temp_s2->channel] == 0)) {
                temp_s2->gains[temp_s2->channel] = 1;
            }
            temp_lo = (g_spu_vm_voice_vol[var_t2].value * (arg3 & 0xFFFF)) / 127;
            master_volume = g_vab_cur_master_vol[0x18];
            temp_lo_2 = ((s32)temp_lo * ((master_volume << 14) - master_volume)) / 16129;
            temp_lo = temp_lo_2 * g_vab_cur_prog_table[arg2 * 16 + 1];
            temp_a0_2 = g_spu_vm_voice_tone_idx[var_t2].value;
            temp_lo = ((u32)temp_lo * g_ns_vpage_pool[((arg2 * 16 + temp_a0_2) << 5) + 2]) / 16129U;
            temp_lo_2 = (u32) (temp_lo * temp_s2->voll) / 127U;
            var_t1 = (u32) (temp_lo * temp_s2->volr) / 127U;
            temp_v1_3 = g_ns_vpage_pool[(temp_a0_2 << 5) + 3];
            if (temp_v1_3 < 0x40U) {
                var_t1 = (u32) (var_t1 * temp_v1_3) / 63U;
                var_a1 = temp_lo_2;
            } else {
                var_a1 = (u32) (temp_lo_2 * (0x7F - temp_v1_3)) / 63U;
            }
            temp_v1_4 = g_vab_cur_prog_table[g_spu_vm_voice_tone_prog[var_t2].value * 16 + 4];
            if (temp_v1_4 < 0x40U) {
                var_t1 = (u32) (var_t1 * temp_v1_4) / 63U;
            } else {
                var_a1 = (u32) (var_a1 * (0x7F - temp_v1_4)) / 63U;
            }
            r = var_t2 * 12;
            temp_lo_2 = (arg4 & 0x7F00);
            temp_lo_2 |= arg4;
            temp_v1_5 = temp_lo_2 & 0xFF;
            r2 = (temp_v1_5 < 0x40U) | (r & 0x7F00);
            r2 |= (r2 & 0x7F00) | (r2 & 0x3F00) | (r2 & 0x1F00) | (r2 & 0x0F00) | (r2 & 0x0700) | (r2 & 0x0300);
            if (r2) {
                var_t1 = (u32) (var_t1 * temp_v1_5) / 63U;
            } else {
                var_a1 = (u32) (var_a1 * (0x7F - temp_v1_5)) / 63U;
            }
            if (g_spu_vm_mono_flag == 1) {
                if (var_a1 < var_t1) {
                    var_a1 = var_t1;
                } else {
                    var_t1 = var_a1;
                }
            }
            var_a1 = (var_a1 * var_a1) / 16383U;
            var_t1 = (var_t1 * var_t1) / 16383U;
            *(s16 *)((u32)&g_midi_seq_table[0].left + var_t2 * 16) = var_a1;
            *(s16 *)((u32)&g_midi_seq_table[0].right + var_t2 * 16) = var_t1;
            var_s6 += 1;
            g_card_ev_table[var_t2] = g_card_ev_table[var_t2] | 3;
        }
    }
    return var_s6;
}
