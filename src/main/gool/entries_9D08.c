#include "common.h"
#include "resource.h"

typedef struct GoolLevelHeader {
    s32 word_0;
    s32 type;
} GoolLevelHeader;

#define CRASH_EXT_GOOL_ENTRIES_9D08
#include "gool_externs.h"
#undef CRASH_EXT_GOOL_ENTRIES_9D08

void Gfx_TransformWorlds(s32 arg0)
{
    if (*RAM_AT(u16, gp_level_entry) != 0) {
        SetRotMatrix((MATRIX *)g_m_view);
        SetLightMatrix((MATRIX *)(g_m_view + 0x110));
        RMemcpy(RAM_AT(u8, RAM_AT(ResourceEntry, g_pads)->data) + 4, RAM_AT(void, 0x1F800100), 0x100);
        Gte_RTransformWorlds(RAM_AT(void, gp_level_entry), arg0, 0x800 - (g_view_proj / 2), g_draw_stamp[0], Gpu_GetCurPrimsTail(), g_wgeo_tex_anim_masks);
    }
}
