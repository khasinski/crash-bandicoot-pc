#include "common.h"
#include "gpu.h"

#define CRASH_EXT_SDK_LIBGPU_ORDERING_TABLE_DMA
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_LIBGPU_ORDERING_TABLE_DMA

s32 _otc(u32 *table, s32 count)
{
    *g_gpu_dpcr_reg |= 0x08000000;
    *g_gpu_dma6_chcr_reg = 0;
    *g_gpu_dma6_madr_reg = (u32)(table - 1 + count);
    *g_gpu_dma6_bcr_reg = count;
    *g_gpu_dma6_chcr_reg = 0x11000002;
    set_alarm();
    while (*g_gpu_dma6_chcr_reg & 0x01000000) {
        if (get_alarm()) return -1;
    }
    return count;
}

s32 _clr(GpuRect *rect, u32 color)
{
    rect->w = (rect->w < 0) ? 0 : ({ s32 max_x = g_gpu_vram_max_w[0] - 1; (rect->w > max_x) ? max_x : rect->w; });
    rect->h = (rect->h < 0) ? 0 : ({ s32 max_y = g_gpu_vram_max_h[0] - 1; (rect->h > max_y) ? max_y : rect->h; });

    if ((rect->x & 0x3F) || (rect->w & 0x3F)) {
        g_gpu_ot_dma_start[0] = ((u32)g_gpu_ot_dma_end & 0xFFFFFF) | 0x07000000;
        g_gpu_ot_dma_start[1] = 0xE3000000;
        g_gpu_ot_dma_start[2] = 0xE4FFFFFF;
        g_gpu_ot_dma_start[3] = 0xE5000000;
        g_gpu_ot_dma_start[4] = 0xE6000000;
        g_gpu_ot_dma_start[5] = (color & 0xFFFFFF) | 0x60000000;
        g_gpu_ot_dma_start[6] = *(u32 *)&rect->x;
        g_gpu_ot_dma_start[7] = *(u32 *)&rect->w;
        g_gpu_ot_dma_end[0] = 0x03FFFFFF;
        g_gpu_ot_dma_end[1] = _param(3) | 0xE3000000;
        g_gpu_ot_dma_end[2] = _param(4) | 0xE4000000;
        g_gpu_ot_dma_end[3] = _param(5) | 0xE5000000;
    } else {
        g_gpu_ot_dma_start[0] = 0x04FFFFFF;
        g_gpu_ot_dma_start[1] = 0xE6000000;
        g_gpu_ot_dma_start[2] = (color & 0xFFFFFF) | 0x02000000;
        g_gpu_ot_dma_start[3] = *(u32 *)&rect->x;
        g_gpu_ot_dma_start[4] = *(u32 *)&rect->w;
    }
    _cwc((s32)g_gpu_ot_dma_start);
    return 0;
}

s32 _dws(GpuRect *rect, u32 *data)
{
    s32 total, blocks, rem, flag;

    set_alarm();
    flag = 0;
    rect->w = (rect->w < 0) ? 0 : (g_gpu_vram_max_w[0] < rect->w) ? g_gpu_vram_max_w[0] : rect->w;
    rect->h = (rect->h < 0) ? 0 : (g_gpu_vram_max_h[0] < rect->h) ? g_gpu_vram_max_h[0] : rect->h;
    total = (rect->w * rect->h + 1) / 2;
    if (total <= 0) {
        return -1;
    }
    rem = total % 16;
    blocks = total / 16;
    while (!(*g_gpu_stat_reg & 0x04000000)) {
        if (get_alarm()) {
            return -1;
        }
    }
    *g_gpu_stat_reg = 0x04000000;
    *g_gpu_gp0_reg = 0x01000000;
    *g_gpu_gp0_reg = flag ? 0xB0000000 : 0xA0000000;
    *g_gpu_gp0_reg = *(u32 *)&rect->x;
    *g_gpu_gp0_reg = *(u32 *)&rect->w;
    while (--rem != -1) {
        *g_gpu_gp0_reg = *data++;
    }
    if (blocks != 0) {
        *g_gpu_stat_reg = 0x04000002;
        *g_gpu_dma2_madr_reg = (u32)data;
        *g_gpu_dma2_bcr_reg = (blocks << 16) | 0x10;
        *g_gpu_dma2_chcr_reg = 0x01000201;
    }
    return 0;
}

s32 _drs(GpuRect *rect, u32 *data)
{
    s32 total, blocks, rem;

    set_alarm();
    rect->w = (rect->w < 0) ? 0 : (g_gpu_vram_max_w[0] < rect->w) ? g_gpu_vram_max_w[0] : rect->w;
    rect->h = (rect->h < 0) ? 0 : (g_gpu_vram_max_h[0] < rect->h) ? g_gpu_vram_max_h[0] : rect->h;
    total = (rect->w * rect->h + 1) / 2;
    if (total <= 0) {
        return -1;
    }
    rem = total % 16;
    blocks = total / 16;
    while (!(*g_gpu_stat_reg & 0x04000000)) {
        if (get_alarm()) {
            return -1;
        }
    }
    *g_gpu_stat_reg = 0x04000000;
    *g_gpu_gp0_reg = 0x01000000;
    *g_gpu_gp0_reg = 0xC0000000;
    *g_gpu_gp0_reg = *(u32 *)&rect->x;
    *g_gpu_gp0_reg = *(u32 *)&rect->w;
    while (!(*g_gpu_stat_reg & 0x08000000)) {
        if (get_alarm()) {
            return -1;
        }
    }
    while (--rem != -1) {
        *data++ = *g_gpu_gp0_reg;
    }
    if (blocks != 0) {
        *g_gpu_stat_reg = 0x04000003;
        *g_gpu_dma2_madr_reg = (u32)data;
        *g_gpu_dma2_bcr_reg = (blocks << 16) | 0x10;
        *g_gpu_dma2_chcr_reg = 0x01000200;
    }
    return 0;
}
