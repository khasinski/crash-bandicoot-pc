#include "common.h"

/* Configure GPU DMA address, block count and control registers. */

void _cwc(s32 arg0) {
    *g_gpu_stat_reg = 0x04000002;
    *g_gpu_dma2_madr_reg = arg0;
    *g_gpu_dma2_bcr_reg = 0;
    *g_gpu_dma2_chcr_reg = 0x01000401;
}
