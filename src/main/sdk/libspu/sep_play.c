#include "common.h"

void SsSepPlay(s16 arg0, s16 arg1, s32 arg2, s16 arg3) {
    Snd_SetPlayMode(arg0, arg1, arg2 & 0xFF, arg3);
}
