#include "common.h"

typedef struct {
    u16 word_0;
    u16 word_2;
    u16 word_4;
    volatile u16 word_6;
    u8 reserved_8[19];
    u8 word_1b;
    s16 word_1c;
    u8 reserved_1e[10];
    s16 word_28;
    u8 reserved_2a[10];
} VoiceStruct;

#define CRASH_EXT_SDK_LIBSPU_VOICE_ADSR_TABLE
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_LIBSPU_VOICE_ADSR_TABLE

void SpuVmFlush(void) {
    s32 i = 0;
    u32 Mask2;
    u32 mask = 0;

    g_spu_vm_flush_tick = (g_spu_vm_flush_tick + 1) & 0xF;
    g_spu_vm_seq_voice_mask[g_spu_vm_flush_tick] = 0;
    for (i = 0; i < gp_spawn_count; i++) {
        g_spu_vm_voice_table[i].word_6 = g_snd_tempo[i * 8 + 6];
        if (g_spu_vm_voice_table[i].word_6 == 0) {
            g_spu_vm_seq_voice_mask[g_spu_vm_flush_tick] |= (1 << i);
        }
    }
    if (g_spu_vm_flush_cond == 0) {
        Mask2 = 0xFFFFFFFF;
        for (i = 0; i < 15; i++) {
            Mask2 &= g_spu_vm_seq_voice_mask[i];
        }
        for (i = 0; i < gp_spawn_count; i++) {
            if (Mask2 & (1 << i)) {
                if (g_spu_vm_voice_table[i].word_1b == 2) {
                    _SpuVmSetNoiseVoice(0, 0xFFFFFF);
                }
                g_spu_vm_voice_table[i].word_1b = 0;
            }
        }
    }
    i = 0;
    g_spu_vm_key_on_lo &= ~g_spu_vm_active_mask_lo;
    g_spu_vm_key_on_hi &= ~g_spu_vm_active_mask_hi;
    for (i = 0; i < 24; i++) {
        if (g_spu_vm_voice_table[i].word_1c != 0) {
            g_spu_vm_flush_cb_b(i);
        }
        if (g_spu_vm_voice_table[i].word_28 != 0) {
            g_spu_vm_flush_cb_a(i);
        }
    }
    for (i = 0; i < 24; i++) {
        if (g_card_ev_table[i] & 1) {
            g_snd_tempo[i * 8 + 0] = g_midi_seq_table[i * 8 + 0];
            g_snd_tempo[i * 8 + 1] = g_midi_seq_table[i * 8 + 1];
        }
        if (g_card_ev_table[i] & 4) {
            g_snd_tempo[i * 8 + 2] = g_midi_seq_table[i * 8 + 2];
        }
        if (g_card_ev_table[i] & 8) {
            g_snd_tempo[i * 8 + 3] = g_midi_seq_table[i * 8 + 3];
        }
        if (g_card_ev_table[i] & 0x10) {
            g_snd_tempo[i * 8 + 4] = g_midi_seq_table[i * 8 + 4];
            g_snd_tempo[i * 8 + 5] = g_midi_seq_table[i * 8 + 5];
        }
        g_card_ev_table[i] = 0;
    }
    g_snd_tempo[0xC6] = g_spu_vm_active_mask_lo;
    g_snd_tempo[0xC7] = g_spu_vm_active_mask_hi;
    g_snd_tempo[0xC4] = g_spu_vm_key_on_lo;
    g_snd_tempo[0xC5] = g_spu_vm_key_on_hi;
    if (gp_spawn_count < 24) {
        mask = (g_snd_tempo[0xCD] << 16) | g_snd_tempo[0xCC];
        mask &= g_spu_vm_max_voice;
    }
    g_spu_vm_active_mask_lo = 0;
    g_spu_vm_active_mask_hi = 0;
    g_spu_vm_key_on_lo = 0;
    g_spu_vm_key_on_hi = 0;
    g_snd_tempo[0xCC] = g_spu_vm_noise_mask_lo | mask;
    g_snd_tempo[0xCD] = g_spu_vm_noise_mask_hi | (mask >> 16);
}
