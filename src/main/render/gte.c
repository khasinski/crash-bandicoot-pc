#include "common.h"

/* Pass the matrix at g_m_rot to the two GTE setup routines. */

#define CRASH_EXT_RENDER_GTE
#include "render_externs.h"
#undef CRASH_EXT_RENDER_GTE

void Gfx_SetGteMatrices(void)
{
    SetRotMatrix(g_m_rot);
    SetTransMatrix(g_m_rot);
}
