#include "common.h"

#define CRASH_EXT_GOOL_ENTRIES_9D08
#include "gool_externs.h"
#undef CRASH_EXT_GOOL_ENTRIES_9D08

void Gfx_TransformWorlds(s32 arg0)
{
    if (*gp_level_entry != 0) {
        SetRotMatrix(g_m_view);
        SetLightMatrix(g_m_view + 0x110);
        RMemcpy(g_pads->data + 4, 0x1F800100, 0x100);
        Gte_RTransformWorlds(gp_level_entry, arg0, 0x800 - (g_view_proj / 2), g_draw_stamp[0], Gpu_GetCurPrimsTail(), g_wgeo_tex_anim_masks);
    }
}

