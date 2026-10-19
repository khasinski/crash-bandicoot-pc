#include "common.h"
#define CRASH_EXT_SDK_LIBSPU_VOICE_TONE_LIST
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_LIBSPU_VOICE_TONE_LIST
s32 SpuVmSelectToneAndVag(u8 *tones,u8 *channels) {
    u8 count=0;
    u8 tone=0;
    u8 *state=g_midi_sep;
    u8 *program;
    u8 *count_state;
    u8 *attrs;
    u8 note;
    s32 channel_index;
    u32 lower;
    u8 output;
    if (state[0]!=0) {
        program=state+7;
        count_state=state;
        do {
            attrs=g_ns_vpage_pool+((*program*16+(tone&0xFF))<<5);
            lower=attrs[6];
            note = *(program - 5);
            if (note>=lower) {
                if (attrs[7]>=note) {
                    channel_index=count&0xFF;
                    output=count;
                    count++;
                    channels[channel_index]=attrs[0x16];
                    tones[output&0xFF]=tone;
                }
            }
            tone++;
        } while ((u32)(tone&0xFF)<count_state[0]);
    }
    return count&0xFF;
}
