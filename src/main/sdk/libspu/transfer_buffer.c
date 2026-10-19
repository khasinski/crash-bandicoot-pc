#include "common.h"

/* Clamp transfer size, forward buffer and size, and update completion state. */

u32 SpuWrite(s32 arg0,u32 arg1) {
    u32 var_s0;

    var_s0 = arg1;
    if (var_s0 > 0x7F000U) {
        var_s0 = 0x7F000;
    }
    _spu_write(arg0,var_s0);
    if (g_spu_dma_done_cb == 0) {
        g_spu_trans_start_addr = 0;
    }
    return var_s0;
}
