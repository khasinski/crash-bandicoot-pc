#include "common.h"

/* Configure the SPU transfer address and issue a transfer command. */

s32 _spu_read(s32 arg0, s32 arg1) {
    _spu_t(2, g_spu_adsr_dummy_1 << g_spu_trans_mode);
    _spu_t(0);
    _spu_t(3, arg0, arg1);
    return arg1;
}
