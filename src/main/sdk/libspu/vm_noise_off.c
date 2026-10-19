#include "common.h"
typedef struct {
    u8 gap00[0x194];
    u16 voice_off_lo;
    u16 voice_off_hi;
} SpuPorts;
#define CRASH_EXT_SDK_LIBSPU_VM_NOISE_OFF
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_LIBSPU_VM_NOISE_OFF
void vmNoiseOff(s32 arg0) {
    s32 offset;
    SpuPorts *ports;
    offset = (arg0 & 0xFF) * 0x34;
    g_spu_vm_voice_auto_flag[offset] = 0;
    ports = g_snd_tempo;
    *(u16 *)(g_spu_vm_voice_pitch + offset) = 0;
    ports->voice_off_lo = 0;
    ports->voice_off_hi = 0;
}
