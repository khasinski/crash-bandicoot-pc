#include "common.h"

/* Known fields and the 0xAC stride are recovered from retail loads/stores. */
typedef char SoundStateStride[(sizeof(SequenceState) == 0xAC) ? 1 : -1];
typedef struct { u8 bytes[0x19]; } Master;
#define CRASH_EXT_SDK_LIBSPU_SEQUENCE_VOLUME_0
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_LIBSPU_SEQUENCE_VOLUME_0
typedef struct { s16 program; u8 rest[50]; } ProgramVoice;
typedef struct { u8 pan; u8 rest[51]; } PanVoice;
#define CRASH_EXT_SDK_LIBSPU_SEQUENCE_VOLUME_1
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_LIBSPU_SEQUENCE_VOLUME_1

s16 SpuVmSetSeqVol(s16 arg0, u16 arg1, u16 arg2, s16 arg3) {
    s16 temp_a1_2;
    s16 left_result, right_result;
    s16 var_t3;
    s32 temp_a1;
    s32 scaled_velocity;
    s32 temp_v0;
    s32 temp_v1;
    s32 temp_v1_3;
    u32 temp_lo;
    u32 temp_lo_2;
    u32 temp_lo_3;
    u16 var_a1;
    u16 var_a3;
    u8 temp_a0;
    u8 temp_a0_2;
    u8 temp_v1_2;
    SequenceState *temp_t2;

    temp_t2 = g_ss_score_table[arg0 & 0xFF] + ((u32) (arg0 << 0x10) >> 0x18);
    g_spu_vm_cur_seq_sep_id = arg0;
    *(u16 *)(void *)&temp_t2->voll = arg1;
    temp_t2->volr = arg2;
    if ((u16) *(u16 *)(void *)&temp_t2->voll >= 0x80U) {
        *(u16 *)(void *)&temp_t2->voll = 0x7FU;
    }
    if ((u16) temp_t2->volr >= 0x80U) {
        temp_t2->volr = 0x7FU;
    }
    if (arg3 == 1) {
        var_t3 = 0;
        if (gp_spawn_count != 0) {
            u32 seq_key = arg0 & 0xFFFF;
            do {
                s32 volume_max = 127;
                s32 volume_square = 16129;
                s32 pan_max = 63;

                s16 *left_shadow = (s16 *)g_midi_seq_table;
                s16 *right_shadow = left_shadow + 1;
                temp_a1 = var_t3 * 0x34;
                if (*(u16 *)(g_spu_vm_voice_seq_sep + temp_a1) == seq_key) {
                    scaled_velocity = ((s32) (*(s16 *)(g_spu_vm_voice_vol + temp_a1) * temp_t2->gains[temp_t2->channel]) / volume_max);
                    scaled_velocity = (s32)(g_vab_cur_master_vol->bytes[0x18] * (((u32)scaled_velocity << 14) - (u32)scaled_velocity)) / volume_square;
                    temp_v1 = g_spu_vm_voice_tone_prog[var_t3].program * 0x10;
                    scaled_velocity *= g_vab_cur_prog_table[temp_v1 + 1];
                    temp_a1_2 = *(s16 *)(g_spu_vm_voice_tone_idx + temp_a1);
                    temp_lo = (u32)(scaled_velocity * g_ns_vpage_pool[((temp_v1 + temp_a1_2) << 5) + 2]) / (u32)volume_square;
                    temp_lo_2 = (u32) (temp_lo * *(u16 *)(void *)&temp_t2->voll) / (u32)volume_max;
                    temp_lo_3 = (u32) (temp_lo * temp_t2->volr) / (u32)volume_max;
                    temp_v1_2 = g_ns_vpage_pool[(temp_a1_2 << 5) + 3];
                    if (temp_v1_2 < 0x40U) {
                        var_a1 = temp_lo_2;
                        var_a3 = (u32) (temp_lo_3 * temp_v1_2) / (u32)pan_max;
                    } else {
                        var_a3 = temp_lo_3;
                        var_a1 = (u32) (temp_lo_2 * (volume_max - temp_v1_2)) / (u32)pan_max;
                    }
                    temp_a0 = g_vab_cur_prog_table[(g_spu_vm_voice_tone_prog[var_t3].program * 16) + 4];
                    if (temp_a0 < 0x40U) {
                        var_a3 = (u32) ((s32) (var_a3 * temp_a0) / pan_max);
                    } else {
                        var_a1 = (u32) ((s32) (var_a1 * (volume_max - temp_a0)) / pan_max);
                    }
                    temp_a0_2 = g_spu_vm_voice_pan[var_t3].pan;
                    if (temp_a0_2 < 0x40U) {
                        var_a3 = (u32) ((s32) (var_a3 * temp_a0_2) / pan_max);
                    } else {
                        var_a1 = (u32) ((s32) (var_a1 * (volume_max - temp_a0_2)) / pan_max);
                    }
                    if (g_spu_vm_mono_flag == 1) {
                        if (var_a1 < var_a3) {
                            var_a1 = var_a3;
                        } else {
                            var_a3 = var_a1;
                        }
                    }
                    temp_v0 = var_a3;
                    temp_v1_3 = var_t3 * 0x10;
                    left_result = (s16) ((s32) ((u32)var_a1 * var_a1) / 16383);
                    right_result = (s16) ((s32) (temp_v0 * temp_v0) / 16383);
                    *(s16 *)(temp_v1_3 + (u32)left_shadow) = left_result;
                    *(s16 *)(temp_v1_3 + (u32)right_shadow) = right_result;
                    g_card_ev_table[var_t3] = g_card_ev_table[var_t3] | 3;
                }
                var_t3++;
            } while (var_t3 < (s32) gp_spawn_count);
        }
    }
    return g_spu_vm_cur_seq_sep_id;
}
