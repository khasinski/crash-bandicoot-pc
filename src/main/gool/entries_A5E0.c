#include "common.h"
#include "render.h"

#define CRASH_EXT_GOOL_ENTRIES_A5E0
#include "gool_externs.h"
#undef CRASH_EXT_GOOL_ENTRIES_A5E0

void Gfx_TransformWorldsRipple(s32 arg0)
{
    s32 i;
    s32 val;

    if (arg0 == 0) {
        for (i = 0; i < 16; i++) {
            gp_ripple_offsets[i] = ((gp_shader_ripple_amp + 1) / 8) * i - gp_shader_ripple_amp;
        }
        return;
    }
    if (*gp_level_entry[0] != 0) {
        if (gp_next_lid == 0) {
            for (i = 0; i < 16; i++) {
                val = gp_ripple_offsets[i] + gp_shader_ripple_speed;
                gp_ripple_offsets[i] = val;
                if (val > gp_shader_ripple_amp) {
                    val = -gp_shader_ripple_amp + 1;
                    gp_ripple_offsets[i] = val;
                }
            }
        }
        for (i = 0; i < 16; i++) {
            val = gp_ripple_offsets[i];
            ((s32 *)0x1F800048)[i] = val >= 0 ? val : -val;
        }
        SetRotMatrix(g_m_view);
        SetLightMatrix(g_m_view + 0x110);
        RMemcpy(g_pads[0]->data + 4, 0x1F800100, 0x100);
        Gte_RTransformWorldsRipple(gp_level_entry[0], arg0, 0x800 - (g_view_proj.distance / 2), g_draw_stamp[0], Gpu_GetCurPrimsTail(), g_wgeo_tex_anim_masks);
    }
}

void Gfx_TransformWorldsLightning(s32 arg0)
{
    if (*gp_level_entry[0] != 0) {
        SetRotMatrix(g_m_view);
        SetLightMatrix(g_m_view + 0x110);
        RMemcpy(g_pads[0]->data + 4, 0x1F800100, 0x100);
        *(s32 *)0x1F800048 = gp_fade_step << 4;
        *(s32 *)0x1F80004C = gp_shader_color_g << 4;
        *(s32 *)0x1F800050 = gp_shader_color_b << 4;
        *(s32 *)0x1F800054 = gp_ripple_amp;
        *(s32 *)0x1F800058 = gp_ripple_freq << 4;
        *(s32 *)0x1F80005C = gp_shader_tint_r << 4;
        *(s32 *)0x1F800060 = gp_shader_tint_g << 4;
        *(s32 *)0x1F800064 = gp_lightning_state;
        Gte_RTransformWorldsLightning(gp_level_entry[0], arg0, 0x800 - (g_view_proj.distance / 2), g_draw_stamp[0], Gpu_GetCurPrimsTail(), g_wgeo_tex_anim_masks);
    }
}

