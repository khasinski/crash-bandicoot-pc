#include "common.h"

#define CRASH_EXT_SDK_LIBGPU_HARDWARE_VERSION
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_LIBGPU_HARDWARE_VERSION

s32 GetGraphType(void)
{
    return g_gpu_mode;
}
