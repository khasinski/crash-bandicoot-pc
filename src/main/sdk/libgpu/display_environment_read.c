#include "common.h"

#define CRASH_EXT_SDK_LIBGPU_DISPLAY_ENVIRONMENT_READ
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_LIBGPU_DISPLAY_ENVIRONMENT_READ

/* Copy the complete cached environment into the caller's buffer. */
void *GetDispEnv(void *out)
{
    BIOS_MemCpy(out, g_gpu_cur_dispenv, 0x14);
    return out;
}
