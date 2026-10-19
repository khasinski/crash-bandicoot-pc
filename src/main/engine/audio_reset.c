#include "common.h"
#include "audio_channels.h"

typedef struct VoiceEntry {
    s32 word_00;
    u32 b0 : 1;
    u32 b1 : 1;
    u32 b2 : 1;
    u32 b3 : 1;
    u32 b4 : 1;
    u32 b5 : 1;
    u32 b6 : 1;
    u32 b7 : 1;
    u32 b8 : 1;
    u32 b9 : 1;
    u32 b10 : 1;
    u32 b11 : 1;
    s32 word_08;
    RAM_REF(void) word_0c;
    u8 word_10;
    u8 word_11;
    u16 word_12;
    u16 word_14;
    u16 word_16;
    s32 word_18;
    s32 word_1c;
    s32 word_20;
    u8 reserved_24[0x20];
} VoiceEntry;

#define CRASH_EXT_ENGINE_AUDIO_RESET
#include "engine_externs.h"
#undef CRASH_EXT_ENGINE_AUDIO_RESET

void Audio_Init(void) {
    s32 i;
    SpuCommonAttr attr;

    for (i = 0; i < 24; i++) {
        g_voices[i].b3 = 0;
    }
    SsInit();
    SsSetTableSize(g_ss_seq_attr_table, 1, 2);
    SpuInitMalloc(0, gp_spu_malloc_rec);
    attr.mask = 3;
    attr.mvol.left = 0x3FFF;
    attr.mvol.right = 0x3FFF;
    SpuSetCommonAttr(&attr);
    SpuSetTransferMode(0);
    g_audio_vol_scale.word_0c = 0;
    g_audio_vol_scale.word_10 = 1;
    g_audio_vol_scale.word_11 = 0x80;
    g_audio_vol_scale.word_12 = 0x3FFF;
    g_audio_vol_scale.word_14 = 0;
    g_audio_vol_scale.word_16 = 0x1000;
    g_audio_vol_scale.word_18 = 0;
    g_audio_vol_scale.word_1c = 0;
    g_audio_vol_scale.word_20 = 0;
    g_audio_vol_scale.b1 = 0;
    g_audio_vol_scale.b0 = 0;
    g_audio_vol_scale.b2 = 0;
    g_audio_vol_scale.b3 = 0;
    g_audio_vol_scale.b4 = 0;
    g_audio_vol_scale.b5 = 0;
    g_audio_vol_scale.b6 = 0;
    g_audio_vol_scale.b7 = 0;
    g_audio_vol_scale.b8 = 0;
    g_audio_vol_scale.b9 = 1;
    g_audio_vol_scale.b10 = 1;
    g_audio_vol_scale.b11 = 0;
    switch (gp_pbak_mode) {
    case 0x5:
    case 0x1C:
    case 0x1D:
    case 0x34:
        Audio_SetReverbAttr(5, 0x2000, 0x2000, 0, 0);
        break;
    case 0x25:
        Audio_SetReverbAttr(3, 0x2000, 0x2000, 0, 0);
        break;
    }
    Audio_SetVoiceMVol((g_cur_map_level[0] * 0x3FFF) >> 8);
    gp_spu_master_vol = 0x3FFF;
    gp_midi_fade_step = 0;
    SsSetMVol(0x7F, 0x7F);
    gp_midi_default_rvol = 0;
}
