#include "common.h"

#define CRASH_EXT_SDK_LIBSPU_SEQUENCE_VOICE_STOP
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_LIBSPU_SEQUENCE_VOICE_STOP
void SpuVmSeqKeyOff(s16 selector) {
    u8 voice=0;
    if (gp_spawn_count!=0) {
        do {
            if (g_spu_vm_voice_seq_sep[voice&0xFF][0]==selector) {
                g_spu_vm_last_voice[0]=voice&0xFF;
                SpuVmKeyOffNow(0);
            }
            voice++;
        } while ((u32)(voice&0xFF)<gp_spawn_count);
    }
}
