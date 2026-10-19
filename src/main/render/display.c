#include "common.h"

/* Rebuild the camera matrices into the ordering table when the level entry is set.
 * The original passed its own a0 (the table) straight through. */

#define CRASH_EXT_RENDER_DISPLAY
#include "render_externs.h"
#undef CRASH_EXT_RENDER_DISPLAY

void Gfx_UpdateMatrices2(u32 *ot)
{
    if (*RAM_AT(u16, gp_level_entry))
        Gfx_UpdateCamMatrices(ot);
}
