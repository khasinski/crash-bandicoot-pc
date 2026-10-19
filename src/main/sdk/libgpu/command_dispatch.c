#include "common.h"
#include "gpu.h"

#define CRASH_EXT_SDK_LIBGPU_COMMAND_DISPATCH
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_LIBGPU_COMMAND_DISPATCH

void checkRECT(const char *name, GpuRect *rect) {
    switch (g_gpu_debug_level) {
    case 1:
        if (rect->w > g_gpu_vram_max_w || rect->w + rect->x > g_gpu_vram_max_w
            || rect->y > g_gpu_vram_max_h || rect->y + rect->h > g_gpu_vram_max_h
            || rect->w <= 0 || rect->x < 0 || rect->y < 0 || rect->h <= 0) {
            g_gpu_ot_status(g_str_command_dispatch_s_bad_rect, name);
            g_gpu_ot_status(g_str_command_dispatch_d_d_d_d, rect->x, rect->y, rect->w, rect->h);
        }
        break;
    case 2:
        g_gpu_ot_status(g_str_command_dispatch_s, name);
        g_gpu_ot_status(g_str_command_dispatch_d_d_d_d, rect->x, rect->y, rect->w, rect->h);
        break;
    }
}

s32 ClearImage(GpuRect *rect, u8 r, u8 g, u8 b) {
    checkRECT(g_str_command_dispatch_clearimage, rect);
    return g_gpu_ctl->dispatch08(g_gpu_ctl->word0C, rect, 8, (b << 16) | (g << 8) | r);
}

void LoadImage(s32 arg0, s32 arg1) {
    checkRECT(g_str_command_dispatch_loadimage, (GpuRect *)arg0);
    g_gpu_ctl->dispatch08(g_gpu_ctl->word20, (void *)arg0, 8, arg1);
}

void StoreImage(s32 arg0, s32 arg1) {
    checkRECT(g_str_command_dispatch_storeimage, (GpuRect *)arg0);
    g_gpu_ctl->dispatch08(g_gpu_ctl->word1C, (void *)arg0, 8, arg1);
}

s32 MoveImage(s32 *rect, s32 x, s32 y) {
    s32 dst;

    checkRECT(g_str_command_dispatch_moveimage, (GpuRect *)rect);
    if (((GpuRect *)rect)->w == 0 || ((GpuRect *)rect)->h == 0) {
        return -1;
    }
    dst = (y << 16) | (x & 0xFFFF);
    g_libc_ctype_table[2] = rect[0];
    g_libc_ctype_table[3] = dst;
    g_libc_ctype_table[4] = rect[1];
    return g_gpu_ctl->dispatch08(g_gpu_ctl->word18, g_libc_ctype_table, 0x14, 0);
}
