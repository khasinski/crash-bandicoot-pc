#include "common.h"

/* Cache the encoded SPU transfer address and return the supplied address. */

s32 SpuSetTransferStartAddr(s32 arg0) {
    g_spu_adsr_dummy_1 = _spu_FsetRXXa(-1, arg0);
    return arg0;
}
