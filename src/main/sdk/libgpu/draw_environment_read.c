#include "common.h"

/* Copy the complete cached environment into the caller's buffer. */
void *GetDrawEnv(void *out)
{
    BIOS_MemCpy(out, g_gpu_cur_drawenv, 0x5C);
    return out;
}
