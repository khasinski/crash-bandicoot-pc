#include "common.h"

void SsSetMono(s32 arg0) {
    if (arg0 == 1) {
        g_spu_trans_start_addr = 0;
        return;
    }
    g_spu_trans_start_addr = 1;
}
