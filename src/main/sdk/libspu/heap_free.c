#include "common.h"

void SpuFree(u32 address)
{
    s32 i;

    for (i = 0; i < g_spu_tsa_shadow; i++) {
        if (g_spu_memList[i].address & 0x40000000) {
            break;
        }
        if (g_spu_memList[i].address == address) {
            g_spu_memList[i].address = address | 0x80000000;
            break;
        }
    }
    _spu_gcSPU();
}
