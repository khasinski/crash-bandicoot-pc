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

#define CRASH_EXT_SDK_LIBSND_SVM_VOICE_ALLOC
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_LIBSND_SVM_VOICE_ALLOC

s32 SpuVmAlloc(void)
{
    u8 var_s0;
    u16 var_t3;
    u8 var_t2;
    u16 var_t0;
    u16 var_t4;
    u8 var_t1;
    u8 var_a3;

    var_s0 = 0x63;
    var_t3 = 0xFFFF;
    var_t2 = 0;
    var_t0 = 0;
    var_t4 = g_midi_sep.word_f;
    var_t1 = 0x63;
    for (var_a3 = 0; var_a3 < gp_spawn_count; var_a3++) {
        if ((g_spu_vm_voice_auto_flag[var_a3].value == 0) && (g_spu_vm_voice_envx[var_a3].value == 0)) {
            var_s0 = var_a3;
            break;
        }
        if (g_spu_vm_voice_prior[var_a3].value < var_t4) {
            var_t4 = g_spu_vm_voice_prior[var_a3].value;
            var_t3 = g_spu_vm_voice_envx[var_a3].value;
            var_t0 = g_spu_vm_voice_age[var_a3].value;
            var_t1 = var_a3;
            var_t2 = 1;
        } else if (g_spu_vm_voice_prior[var_a3].value == var_t4) {
            var_t2 += 1;
            if (g_spu_vm_voice_envx[var_a3].value < var_t3) {
                var_t3 = g_spu_vm_voice_envx[var_a3].value;
                var_t0 = g_spu_vm_voice_age[var_a3].value;
                var_t1 = var_a3;
            } else if (g_spu_vm_voice_envx[var_a3].value == var_t3) {
                if (var_t0 < (s16)g_spu_vm_voice_age[var_a3].value) {
                    var_t0 = g_spu_vm_voice_age[var_a3].value;
                    var_t1 = var_a3;
                }
            }
        }
    }
    if (var_s0 == 0x63) {
        var_s0 = var_t1;
        if (var_t2 == 0) {
            var_s0 = gp_spawn_count;
        }
    }
    if (var_s0 < gp_spawn_count) {
        for (var_a3 = 0; var_a3 < gp_spawn_count; var_a3++) {
            (g_spu_vm_voice_table + var_a3)->word_2 = g_spu_vm_voice_age[var_a3].value + 1;
        }
        g_spu_vm_voice_age[var_s0].value = 0;
        g_spu_vm_voice_prior[var_s0].value = g_midi_sep.word_f;
        if (g_spu_vm_voice_auto_flag[var_s0].value == 2) {
            _SpuVmSetNoiseVoice(0, 0xFFFFFF);
        }
    }
    return var_s0;
}
