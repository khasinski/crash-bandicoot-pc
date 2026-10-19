#include "common.h"

#define CRASH_EXT_SDK_LIBGPU_RESET
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_LIBGPU_RESET

/* Reset the GPU state and cached environments, or invoke the driver reset. */
s32 ResetGraph(s32 requested)
{
    s32 operation = requested & 7;
    if (operation == 0 || operation == 3) {
        u8 *state = g_gpu_mode;
        BIOS_Printf(g_str_reset_resetgraph_jtb_08x_env, g_gpu_driver_ops, state);
        memset(state, 0, 0x80);
        ResetCallback();
        GPU_cw((u32)g_gpu_ctl & 0xFFFFFF);
        state[0] = _reset(operation != 0);
        g_gpu_debug_level_2 = 1;
        g_gpu_vram_max_w = *(u16 *)(g_gpu_vram_width_tab + state[0] * 4);
        g_gpu_vram_max_h = *(u16 *)(g_gpu_vram_height_tab + state[0] * 4);
        memset(state + 0x10, -1, 0x5C);
        memset(state + 0x6C, -1, 0x14);
        return state[0];
    }
    if (g_gpu_debug_level >= 2) g_gpu_ot_status(g_str_reset_resetgraph_d_n, requested);
    return g_gpu_ctl->reset34(1);
}
