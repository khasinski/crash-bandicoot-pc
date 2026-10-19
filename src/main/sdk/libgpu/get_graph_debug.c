#include "common.h"

#define CRASH_EXT_SDK_LIBGPU_GET_GRAPH_DEBUG
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_LIBGPU_GET_GRAPH_DEBUG

u8 GetGraphDebug(void) {
    return g_gpu_debug_level;
}
