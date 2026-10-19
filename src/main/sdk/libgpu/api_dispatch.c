#include "common.h"

#define CRASH_EXT_SDK_LIBGPU_API_DISPATCH
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_LIBGPU_API_DISPATCH

void DrawOTag(s32 arg0) {
    if ((u8) g_gpu_debug_level >= 2U) {
        g_gpu_ot_status(g_str_api_dispatch_drawotag_08x_n, arg0);
    }
    g_gpu_ctl->dispatch08(g_gpu_ctl->word18, arg0, 0, 0);
}
