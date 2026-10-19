#include "common.h"

u8 SsSetReservedVoice(s32 count) {
    s32 bit;
    u32 requested;
    s32 voice;

    requested = count & 0xFF;
    if (requested >= 25) {
        return 255;
    }
    if (requested == 0) {
        return 255;
    }
    gp_spawn_count = count;
    voice = gp_spawn_count;
    if (voice < 0x18) {
        do {
            bit = 1 << voice;
            voice += 1;
            g_spu_vm_max_voice |= bit;
        } while (voice < 0x18);
    }
    return gp_spawn_count;
}
