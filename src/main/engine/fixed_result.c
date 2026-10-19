#include "common.h"

s32 Solid_BinfInit(void)
{
    s32 i;

    gp_grid_rows = 0x1F800100;
    gp_solid_scratch_grid = 0x1F800180;
    gp_solid_scratch_workspace = 0x1F800380;
    for (i = 0; i < 32; i++) {
        RAM_AT(u32, gp_solid_scratch_workspace)[i] = g_solid_binf_default_header[i];
    }
    return -255;
}

s32 Solid_BinfKill(void)
{
    return -255;
}

