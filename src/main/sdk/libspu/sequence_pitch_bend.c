#include "common.h"

#define CRASH_EXT_SDK_LIBSPU_SEQUENCE_PITCH_BEND
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_LIBSPU_SEQUENCE_PITCH_BEND

s16 SpuVmPBVoice(s16 voice, s16 arg1, s16 arg2, s16 arg3, u16 arg4) {
    s16 bend;
    u16 tone_idx;
    u16 note;
    u16 fine;
    s32 offset;
    s32 prod;

    bend = arg4 - 0x40;
    offset = voice * 0x34;
    if (*(s16 *)(g_spu_vm_voice_seq_sep + offset) == arg1) {
        if (*(s16 *)(g_spu_vm_voice_vab_id + offset) == arg2) {
            if (*(s16 *)(g_spu_vm_voice_tone_prog + offset) == arg3) {
                tone_idx = *(u16 *)(g_spu_vm_voice_tone_idx + offset) + (g_spu_vm_cur_prog_tones * 0x10);
                note = *(u16 *)(g_spu_vm_voice_note + offset);
                if (bend > 0) {
                    note += (bend * g_ns_vpage_pool[(tone_idx << 5) + 0xD]) / 0x3F;
                    fine = ((bend * g_ns_vpage_pool[(tone_idx << 5) + 0xD]) % 0x3F) * 2;
                } else if (bend < 0) {
                    prod = bend * g_ns_vpage_pool[(tone_idx << 5) + 0xC];
                    note = note + (prod / 64) - 1;
                    fine = ((prod % 64) * 2) + 0x7F;
                } else {
                    fine = 0;
                }
                g_spu_vm_cur_tone_idx = g_spu_vm_voice_tone_idx[voice * 0x34];
                g_spu_vm_last_voice[0] = voice;
                *(volatile s16 *)(g_spu_hw_voice_pitch_ptr + (voice << 4)) = note2pitch2(note, fine);
                *(u8 *)((s32)g_card_ev_table + voice) = *(volatile u8 *)((s32)g_card_ev_table + voice) | 4;
                return 1;
            }
        }
    }
    return 0;
}

s32 SpuVmPitchBend(s16 arg0, s16 arg1, s16 arg2, u16 arg3) {
    s16 i;
    s32 count = 0;

    SpuVmVSetUp(arg1, arg2);
    g_spu_vm_cur_seq_sep_id = arg0;
    for (i = 0; i < gp_spawn_count; i++) {
        count += SpuVmPBVoice(i, arg0, arg1, arg2, arg3);
    }
    return count;
}
