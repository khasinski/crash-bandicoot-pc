#include "common.h"

#define CRASH_EXT_SDK_LIBGPU_CONTROL_DISPATCH
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_LIBGPU_CONTROL_DISPATCH

void DrawSync(s32 arg0) {
    if ((u8) g_gpu_debug_level >= 2U) {
        g_gpu_ot_status(g_str_control_dispatch_drawsync_d_n, arg0);
    }
    g_gpu_ctl->callback3C(arg0);
}
