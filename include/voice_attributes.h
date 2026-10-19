#ifndef CRASH_SDK_LIBSPU_VOICE_ATTRIBUTES_H
#define CRASH_SDK_LIBSPU_VOICE_ATTRIBUTES_H

#include "common.h"

/* Layout recovered from SPU attribute readback and update code. */
typedef struct VoiceAttributes {
    s32 voice;
    s32 mask;
    u16 vol_left;
    u16 vol_right;
    s16 volmode_left;
    s16 volmode_right;
    u16 volx_left;
    u16 volx_right;
    u16 pitch;
    u16 note;
    u16 sample_note;
    u16 envx;
    s32 addr;
    s32 loop_addr;
    s32 a_mode;
    s32 s_mode;
    s32 r_mode;
    u16 ar;
    u16 dr;
    u16 sr;
    u16 rr;
    u16 sl;
    u16 adsr1;
    u16 adsr2;
} VoiceAttributes;

#endif
