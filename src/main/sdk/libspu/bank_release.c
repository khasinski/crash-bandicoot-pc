#include "common.h"

void SsVabClose(s32 requested)
{
    s16 index;
    if ((u16)requested < 16) {
        index = requested;
        if (g_midi_tracks[index] == 1) {
            SpuFree(g_vab_spu_addrs[index]);
            g_midi_tracks[index] = 0;
            g_vab_open_count--;
        }
    }
}
