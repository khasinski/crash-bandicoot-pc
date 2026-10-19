#include "common.h"

typedef struct { s16 value; u8 reserved[50]; } VoiceWord;
typedef struct { u8 value; u8 reserved[51]; } VoiceByte;

#define CRASH_EXT_SDK_LIBSPU_VOICE_RELEASE
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_LIBSPU_VOICE_RELEASE

void SpuVmKeyOffNow(void) {
    u16 voice;
    s32 var_a3;
    s32 var_a2;
    u16 temp_a0;
    u16 temp_a1;

    voice = g_spu_vm_last_voice[0];
    if (voice < 0x10) {
        var_a3 = 1 << voice;
        var_a2 = 0;
    } else {
        var_a3 = 0;
        var_a2 = 1 << (voice - 0x10);
    }
    g_spu_vm_voice_auto_flag[voice].value = 0;
    g_spu_vm_voice_pitch[voice].value = 0;
    g_spu_vm_voice_table[voice].value = 0;
    temp_a0 = var_a3 | g_spu_vm_active_mask_lo;
    g_spu_vm_active_mask_lo = temp_a0;
    temp_a1 = var_a2 | g_spu_vm_active_mask_hi;
    g_spu_vm_active_mask_hi = temp_a1;
    g_spu_vm_key_on_lo &= ~temp_a0;
    g_spu_vm_key_on_hi &= ~temp_a1;
}
