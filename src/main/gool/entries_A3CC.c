#include "common.h"

typedef struct ZoneHeader {
    u8 reserved[0x10];
    RAM_REF(GoolAnimData) data;
} ZoneHeader;

typedef struct NsdHeader {
    s32 word_0;
    s32 type;
} NsdHeader;

#define CRASH_EXT_GOOL_ENTRIES_A3CC
#include "gool_externs.h"
#undef CRASH_EXT_GOOL_ENTRIES_A3CC

typedef struct WgeoHeaderView {
    u8 reserved_00[0x1C];
    s32 flags;
} WgeoHeaderView;

typedef struct GoolEntry64 {
    u32 word0;
    u32 reserved[3];
    RAM_REF(WgeoHeaderView) ptr10;
    u32 reserved_2[11];
} GoolEntry64;

typedef struct GoolAnimData {
    s32 count;
    u32 data[1];
} GoolAnimData;

void Gfx_TransformWorldsFog(s32 arg0)
{
    GoolAnimData *data;
    u32 sp18;
    s32 sp1C;
    s32 i;
    GoolEntry64 *base;

    base = RAM_AT(GoolEntry64, 0x1F800100);
    data = RAM_AT(GoolAnimData, RAM_AT(ZoneHeader, g_pads[0])->data);
    if (*RAM_AT(u16, gp_level_entry[0]) != 0) {
        SetRotMatrix((MATRIX *)g_m_view);
        SetLightMatrix((MATRIX *)(g_m_view + 0x110));
        RMemcpy(data->data, base, data->count << 5);
        Gfx_GetFar(&sp18, &sp1C);
        for (i = 0; i < data->count; i++) {
            if (RAM_AT(WgeoHeaderView, base[i].ptr10)->flags == 1) {
                base[i].word0 = (sp1C << 16) | 0xFFFF;
            } else {
                base[i].word0 = (sp1C << 16) | sp18;
            }
        }
        Gte_RTransformWorldsFog(RAM_AT(void, gp_level_entry[0]), arg0, 0x800 - (g_view_proj[0] / 2), g_draw_stamp[0], Gpu_GetCurPrimsTail(), g_wgeo_tex_anim_masks);
    }
}
