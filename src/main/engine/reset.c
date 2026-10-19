#include "common.h"

/* Clear the two engine state words; their precise role is still unidentified. */

#define CRASH_EXT_ENGINE_RESET
#include "engine_externs.h"
#undef CRASH_EXT_ENGINE_RESET

void Pbak_Init(void)
{
    gp_cur_pbak_frame = 0;
    g_pbak_frame_step = 0;
}

void Pbak_Kill(void)
{
}

void Pbak_Noop(void)
{
}
