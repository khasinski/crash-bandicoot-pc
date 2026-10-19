#include "common.h"

/* Dispatch the transfer through the configured mode. */

s32 _spu_write(s32 arg0, s32 arg1) {
    if (g_spu_reverb_depth_l == 0) {
        _spu_t(2, g_spu_adsr_dummy_1 << g_spu_trans_mode);
        _spu_t(1);
        _spu_t(3, arg0, arg1);
    } else {
        _spu_writeByIO(arg0, arg1);
    }
    return arg1;
}
