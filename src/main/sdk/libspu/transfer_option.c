#include "common.h"

void SpuSetTransferMode(s32 mode)
{
    s32 enabled;
    switch(mode) {
    case 0: enabled = 0; break;
    case 1: enabled = 1; break;
    default: enabled = 0; break;
    }
    g_spu_transfer_mode_cur = mode;
    g_spu_reverb_depth_l = enabled;
}
