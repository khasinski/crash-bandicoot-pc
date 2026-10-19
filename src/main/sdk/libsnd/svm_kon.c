#include "common.h"

typedef struct {
    u16 word_0;
    u16 word_2;
    u8 reserved_4[2];
    u16 word_6;
    u8 reserved_8[16];
    s16 word_18;
    u8 reserved_1a;
    u8 word_1b;
    u8 reserved_1c[24];
} VoiceStruct;

typedef struct { u16 value; u8 reserved[50]; } VoiceU16;
typedef struct { s16 value; u8 reserved[50]; } VoiceS16;
typedef struct { u8 value; u8 reserved[51]; } VoiceU8;

typedef struct {
    u8 word_0[7];
    u8 word_7;
    u8 word_8[4];
    u8 word_c;
    u8 word_d[2];
    u8 word_f;
    u8 word_10[8];
    s16 word_18;
    s16 word_1a;
    s16 word_1c;
    s16 word_1e;
} SvmCur;

#define CRASH_EXT_SDK_LIBSND_SVM_KON
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_LIBSND_SVM_KON

void SpuVmDoAllocate(void) {
    s32 var_t0;
    s32 reserved[2];

    g_midi_sep.word_1c = g_midi_sep.word_1a * 8;
    g_midi_sep.word_1e = (g_midi_sep.word_7 << 4) + g_midi_sep.word_c;
    g_spu_vm_voice_envx[g_midi_sep.word_1a].value = 0x7FFF;
    for (var_t0 = 0; var_t0 < 0x10; var_t0++) {
        g_spu_vm_seq_voice_mask[var_t0] &= ~(1 << g_midi_sep.word_1a);
    }
    if ((g_midi_sep.word_18 & 1) > 0) {
        g_spu_vm_alloc_prog_attr[g_midi_sep.word_1c] = *(u16 *)(g_vab_cur_prog_table + ((((s16) g_midi_sep.word_18 - 1) / 2) * 0x10) + 0xC);
        g_card_ev_table[g_midi_sep.word_1a] |= 8;
    } else {
        g_spu_vm_alloc_prog_attr[g_midi_sep.word_1c] = *(u16 *)(g_vab_cur_prog_table + ((((s16) g_midi_sep.word_18 - 1) / 2) * 0x10) + 0xE);
        g_card_ev_table[g_midi_sep.word_1a] |= 8;
    }
    g_spu_vm_alloc_adsr1[g_midi_sep.word_1c] = *(u16 *)(((((g_midi_sep.word_7 << 4) + g_midi_sep.word_c) << 5) + (*(u32 *)&g_ns_vpage_pool)) + 0x10);
    g_spu_vm_alloc_adsr2[g_midi_sep.word_1c] = *(u16 *)(((((g_midi_sep.word_7 << 4) + g_midi_sep.word_c) << 5) + (*(u32 *)&g_ns_vpage_pool)) + 0x12) + g_spu_vm_damper_mask;
    g_card_ev_table[g_midi_sep.word_1a] |= 0x30;
}
