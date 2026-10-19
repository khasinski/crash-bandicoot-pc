#include "common.h"

s32 PadRead(void) {
    PAD_dr();
    return ~g_pad_mode_state;
}
