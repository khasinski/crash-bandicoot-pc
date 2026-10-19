#include "common.h"

typedef struct ZoneFogHeader {
    s32 count;
    u32 words[0xB9];
    s32 depth_offset;
    s32 fog_mode;
} ZoneFogHeader;

typedef struct ZoneHeader {
    u8 reserved[0x10];
    RAM_REF(ZoneFogHeader) data;
} ZoneHeader;

typedef struct NsdHeader {
    s32 word_0;
    s32 type;
} NsdHeader;

typedef struct Sp {
    u8 reserved[0x48];
    s32 a, b, c, d, e, f, g, h;
} Sp;

#define CRASH_EXT_GOOL_ENTRIES_A8CC
#include "gool_externs.h"
#undef CRASH_EXT_GOOL_ENTRIES_A8CC

void Gfx_TransformWorldsDark(s32 arg0)
{
    u32 sp20;
    s32 sp24;
    Sp *sp = RAM_AT(Sp, 0x1F800000);

    if (*RAM_AT(u16, gp_level_entry[0]) != 0) {
        SetRotMatrix((MATRIX *)g_m_view);
        SetLightMatrix((MATRIX *)(g_m_view + 0x110));
        RMemcpy(RAM_AT(ZoneFogHeader, RAM_AT(ZoneHeader, g_pads[0])->data)->words, RAM_AT(void, 0x1F800100), 0x100);
        sp->a = gp_fade_step << 4;
        sp->b = gp_shader_color_g << 4;
        sp->c = gp_shader_color_b << 4;
        sp->d = gp_ripple_amp;
        sp->e = gp_ripple_freq << 4;
        sp->f = gp_shader_tint_r << 4;
        sp->g = gp_shader_tint_g << 4;
        sp->h = gp_lightning_state;
        /* The retail code inlines Gfx_GetFar here (same zone fog fields). */
        Gfx_GetFar(&sp20, &sp24);
        Gte_RTransformWorldsDark(RAM_AT(void, gp_level_entry[0]), arg0, 0x800 - (g_view_proj[0] / 2), g_draw_stamp[0], Gpu_GetCurPrimsTail(), g_wgeo_tex_anim_masks, sp20, sp24);
    }
}
