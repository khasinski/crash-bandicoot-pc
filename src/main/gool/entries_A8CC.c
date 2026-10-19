#include "common.h"

typedef struct ZoneFogHeader {
    s32 count;
    u32 words[0xB9];
    s32 depth_offset;
    s32 fog_mode;
} ZoneFogHeader;

typedef struct ZoneHeader {
    u8 reserved[0x10];
    ZoneFogHeader *data;
} ZoneHeader;

typedef struct NsdHeader {
    s32 word_0;
    s32 type;
} NsdHeader;

typedef struct {
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

    if (*gp_level_entry[0] != 0) {
        SetRotMatrix(g_m_view);
        SetLightMatrix(g_m_view + 0x110);
        RMemcpy(g_pads[0]->data->words, 0x1F800100, 0x100);
        {
            u32 *_arg0 = &sp20;
            s32 _type;
            u32 _offset;
            u32 _val;
            s32 _flag;
            s32 _zero = 0;
            s32 _h;
            NsdHeader *_hdr;
            do { } while (0);
            if (_zero) {
                if (arg0) { _offset = 1; _zero = 1; }
                _zero = 2;
            }
            ((Sp *)0x1F800000)->a = gp_fade_step << 4;
            ((Sp *)0x1F800000)->b = gp_shader_color_g << 4;
            ((Sp *)0x1F800000)->c = gp_shader_color_b << 4;
            ((Sp *)0x1F800000)->d = gp_ripple_amp;
            ((Sp *)0x1F800000)->e = gp_ripple_freq << 4;
            ((Sp *)0x1F800000)->f = gp_shader_tint_r << 4;
            ((Sp *)0x1F800000)->g = gp_shader_tint_g << 4;
            _h = gp_lightning_state;
            do { } while (0);
            _hdr = g_ns_page_pool[0];
            ((Sp *)0x1F800000)->h = _h;
            _type = _hdr->type;
            if (_type == 0x14 || _type == 0x16) {
                _offset = g_pause_state.wordAA8 * 0xC80 + 0x64000;
                _val = ((u32)g_pads[0]->data->depth_offset - _offset) >> 8;
                g_pause_state.wordAC8 = _val + 0x7D0;
                _flag = g_pads[0]->data->fog_mode;
                *_arg0 = _val;
            } else {
                _val = (g_pads[0]->data->depth_offset - 0x32000) >> 8;
                _flag = g_pads[0]->data->fog_mode;
                if (_flag == 1) {
                    _val -= 0x4B0;
                }
                g_pause_state.wordAC8 = _val + 0x320;
                *_arg0 = _val;
            }
            *(&sp24) = _flag;
        }
        Gte_RTransformWorldsDark(gp_level_entry[0], arg0, 0x800 - (g_view_proj[0] / 2), g_draw_stamp[0], Gpu_GetCurPrimsTail(), g_wgeo_tex_anim_masks, sp20, sp24);
    }
}
