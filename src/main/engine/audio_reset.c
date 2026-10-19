#include "common.h"
#include "audio_channels.h"

typedef struct {
    u32 mask;
    s16 volume_left, volume_right;
    s16 mode_left, mode_right;
    u8 reserved_0c[4];
    u16 cd_left, cd_right;
    s32 cd_mix, cd_reverb;
    u16 external_left, external_right;
    s32 external_mix, external_reverb;
} SpuCommonAttributes;

typedef struct {
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
    void *word_0c;
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
    SpuCommonAttributes attr;

    for (i = 0; i < 24; i++) {
        g_voices[i].b3 = 0;
    }
    SsInit();
    SsSetTableSize(g_ss_seq_attr_table, 1, 2);
    SpuInitMalloc(0, gp_spu_malloc_rec);
    attr.mask = 3;
    attr.volume_left = 0x3FFF;
    attr.volume_right = 0x3FFF;
    SpuSetCommonAttr(&attr);
    SpuSetTransferMode(0);
    g_audio_vol_scale.word_0c = NULL;
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
