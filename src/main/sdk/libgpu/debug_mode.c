#include "common.h"

/* Update GPU debug and mode bytes, returning their previous values. */
#define CRASH_EXT_SDK_LIBGPU_DEBUG_MODE_0
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_LIBGPU_DEBUG_MODE_0
typedef struct {u8 debug;u8 reserved_01[13];u8 drawing[92];u8 display[20];} GpuState;
#define CRASH_EXT_SDK_LIBGPU_DEBUG_MODE_1
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_LIBGPU_DEBUG_MODE_1
typedef struct {u8 reverse;u8 reserved_01[12];u8 drawing[92];u8 display[20];} GpuReverseState;
#define CRASH_EXT_SDK_LIBGPU_DEBUG_MODE_2
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_LIBGPU_DEBUG_MODE_2

u8 SetGraphReverse(s32 requested) {
    GpuReverseState *state = &g_gpu_reverse_flag;
    u8 previous = state->reverse;
    if (g_gpu_debug_level.debug >= 2) {
        g_gpu_ot_status(g_str_debug_mode_setgraphreverse_d_n, requested);
    }
    state->reverse = requested;
    g_gpu_ctl->ctl10(g_gpu_ctl->ctl28(8) | (state->reverse ? 0x08000080 : 0x08000000));
    if (g_gpu_mode == 2) {
        g_gpu_ctl->ctl10(g_gpu_reverse_flag.reverse ? 0x20000501 : 0x20000504);
    }
    return previous;
}

s32 SetGraphDebug(s32 arg0) {
    u8 temp_s0;
    GpuState *state = &g_gpu_debug_level;

    temp_s0 = state->debug;
    state->debug = arg0;
    if (arg0 & 0xFF) {
        g_gpu_ot_status(g_str_debug_mode, state->debug, g_gpu_mode, g_gpu_reverse_flag.reverse);
    }
    return temp_s0;
}

typedef struct {u8 mode,debug;u8 reserved_02[12];u8 drawing[92];u8 display[20];} GpuModeState;
#define CRASH_EXT_SDK_LIBGPU_DEBUG_MODE_3
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_LIBGPU_DEBUG_MODE_3

u8 SetGraphQueue(s32 requested) {
    GpuModeState *state = &g_gpu_debug_level_2;
    u8 previous = state->mode;
    if (g_gpu_debug_level.debug >= 2) g_gpu_ot_status(g_str_debug_mode_setgrapque_d_n,requested);
    if (requested != state->mode) {
        g_gpu_ctl->reset34(1);
        state->mode = requested;
        DMACallback(2,0);
    }
    return previous;
}
