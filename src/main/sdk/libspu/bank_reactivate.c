#include "common.h"

s32 SsVabFakeBody(s32 requested)
{
    s16 index;
    if ((u16)requested < 17) {
        index = requested;
        if (g_midi_tracks[index] == 2) {
            SsSetMono(0);
            g_midi_tracks[index] = 1;
            return index;
        }
    }
    return -1;
}
