#include "common.h"

/* Exchange the hardware interrupt mask. */

s32 SetIntrMask(s32 arg0) {
    u16 temp_v0;

    temp_v0 = *g_intr_i_mask_reg;
    *g_intr_i_mask_reg = arg0;
    return temp_v0;
}
