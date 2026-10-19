#include "common.h"

void _spu_gcSPU(void) {
    s32 i;
    s32 j;
    u32 temp_addr;
    u32 temp_size;

    for (i = 0; i <= g_spu_alloc_num; ) {
        if (g_spu_memList[i].address & 0x80000000) {
            for (j = i + 1; ; j++) {
                if (g_spu_memList[j].address != 0x2FFFFFFF) {
                    goto found;
                }
            }
        found:
            if ((g_spu_memList[j].address & 0x80000000) &&
                ((g_spu_memList[j].address & 0x0FFFFFFF) == (g_spu_memList[i].address & 0x0FFFFFFF) + g_spu_memList[i].size)) {
                g_spu_memList[j].address = 0x2FFFFFFF;
                g_spu_memList[i].size += g_spu_memList[j].size;
                continue;
            }
        }
        i++;
    }
    for (i = 0; i <= g_spu_alloc_num; i++) {
        if (g_spu_memList[i].size == 0) {
            g_spu_memList[i].address = 0x2FFFFFFF;
        }
    }
    for (i = 0; i <= g_spu_alloc_num; i++) {
        if (g_spu_memList[i].address & 0x40000000) {
            break;
        }
        for (j = i + 1; j <= g_spu_alloc_num; j++) {
            if (g_spu_memList[j].address & 0x40000000) {
                break;
            }
            if ((g_spu_memList[j].address & 0x0FFFFFFF) < (g_spu_memList[i].address & 0x0FFFFFFF)) {
                temp_addr = g_spu_memList[i].address;
                g_spu_memList[i].address = g_spu_memList[j].address;
                temp_size = g_spu_memList[i].size;
                g_spu_memList[i].size = g_spu_memList[j].size;
                g_spu_memList[j].address = temp_addr;
                g_spu_memList[j].size = temp_size;
            }
        }
    }
    for (i = 0; i <= g_spu_alloc_num; i++) {
        if (g_spu_memList[i].address & 0x40000000) {
            break;
        }
        if (g_spu_memList[i].address == 0x2FFFFFFF) {
            g_spu_memList[i].address = g_spu_memList[g_spu_alloc_num].address;
            g_spu_memList[i].size = g_spu_memList[g_spu_alloc_num].size;
            g_spu_alloc_num = i;
            break;
        }
    }
    for (i = g_spu_alloc_num - 1; i >= 0; i--) {
        if (!(g_spu_memList[i].address & 0x80000000)) {
            break;
        }
        g_spu_memList[i].address = (g_spu_memList[i].address & 0x0FFFFFFF) | 0x40000000;
        g_spu_memList[i].size += g_spu_memList[g_spu_alloc_num].size;
        g_spu_alloc_num = i;
    }
}
