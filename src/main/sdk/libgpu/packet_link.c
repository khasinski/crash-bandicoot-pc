#include "common.h"

#define CRASH_EXT_SDK_LIBGPU_PACKET_LINK
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_LIBGPU_PACKET_LINK

s32 *ClearOTagR(s32 *arg0, s32 arg1) {
    if ((u8) g_gpu_debug_level >= 2U) {
        g_gpu_ot_status(g_str_packet_link_clearotagr_08x_d_n, arg0, arg1);
    }
    g_gpu_ctl->callback2C(arg0, arg1);
    *arg0 = (s32) g_gpu_ot_clear_step & 0xFFFFFF;
    return arg0;
}
