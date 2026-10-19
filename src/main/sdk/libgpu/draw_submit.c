#include "gpu.h"

/* Submit the drawing packet and retain its 92-byte environment snapshot. */
typedef GpuDrawing Drawing;
typedef struct {u8 debug;u8 reserved_01[13];u8 drawing[sizeof(Drawing)];} GpuState;
#define CRASH_EXT_SDK_LIBGPU_DRAW_SUBMIT
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_LIBGPU_DRAW_SUBMIT

void *PutDrawEnv(Drawing *arg0) {
    u32 *temp_s0;
    GpuState *state = &g_gpu_debug_level;

    if (state->debug >= 2U) {
        g_gpu_ot_status(g_str_draw_submit_putdrawenv_08x_n, arg0);
    }
    temp_s0 = arg0->packet;
    SetDrawEnv(temp_s0, arg0);
    arg0->packet[0] = (s32) (arg0->packet[0] | 0xFFFFFF);
    g_gpu_ctl->dispatch08(g_gpu_ctl->word18, temp_s0, 0x40, 0);
    BIOS_MemCpy(state->drawing, arg0, 0x5C);
    return arg0;
}

void DrawOTagEnv(s32 arg0, Drawing *arg1) {
    u32 *temp_s0;
    GpuState *state = &g_gpu_debug_level;

    if (state->debug >= 2U) {
        g_gpu_ot_status(g_str_draw_submit_drawotagenv_08x_08x_n, arg0, arg1);
    }
    temp_s0 = arg1->packet;
    SetDrawEnv(temp_s0, arg1);
    arg1->packet[0] = (s32) ((arg1->packet[0] & 0xFF000000) | (arg0 & 0xFFFFFF));
    g_gpu_ctl->dispatch08(g_gpu_ctl->word18, temp_s0, 0x40, 0);
    BIOS_MemCpy(state->drawing, arg1, 0x5C);
}
