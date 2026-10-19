#include "common.h"
#include <libetc.h>
#include "render.h"

void Gpu_SwapBuffers(void)
{
    RAM_REF(RenderBuffer) swap;
    RAM_REF(RenderBuffer) *active = g_db_cur;
    swap = g_db_cur[1];
    *active = swap;
    g_db_cur[1] = g_db_cur[2];
    g_db_cur[2] = swap;
    VSync(0);
    PutDispEnv(&RAM_AT(RenderBuffer, *active)->display);
    PutDrawEnv(&RAM_AT(RenderBuffer, g_db_cur[1])->drawing);
}
