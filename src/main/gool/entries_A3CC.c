#include "common.h"

typedef struct ZoneFogHeader {
    u8 reserved_00[0x2E8];
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

#define CRASH_EXT_GOOL_ENTRIES_A3CC
#include "gool_externs.h"
#undef CRASH_EXT_GOOL_ENTRIES_A3CC

static __inline__ void Gfx_GetFar(u32 *arg0, s32 *arg1)
{
    s32 type;
    u32 offset;
    u32 val;
    s32 flag;

    type = g_ns_page_pool[0]->type;
    if (type == 0x14 || type == 0x16) {
        offset = g_pause_state.wordAA8 * 0xC80 + 0x64000;
        val = ((u32)g_pads[0]->data->depth_offset - offset) >> 8;
        g_pause_state.wordAC8 = val + 0x7D0;
        flag = g_pads[0]->data->fog_mode;
        *arg0 = val;
    } else {
        val = (g_pads[0]->data->depth_offset - 0x32000) >> 8;
        flag = g_pads[0]->data->fog_mode;
        if (flag == 1) {
            val -= 0x4B0;
        }
        g_pause_state.wordAC8 = val + 0x320;
        *arg0 = val;
    }
    *arg1 = flag;
}

#define GOOL_CALC_FOG(arg0, arg1) \
    { \
        u32 *_arg0 = (arg0); \
        s32 _type; \
        u32 _offset; \
        u32 _val; \
        s32 _flag; \
        s32 _zero = 0; \
        do { } while (0); \
        if (_zero) { \
            if (g_ns_page_pool[0]) { _offset = 1; _zero = 1; } \
            _zero = 2; \
        } \
        _type = g_ns_page_pool[0]->type; \
        if (_type == 0x14 || _type == 0x16) { \
            _offset = g_pause_state.wordAA8 * 0xC80 + 0x64000; \
            _val = ((u32)g_pads[0]->data->depth_offset - _offset) >> 8; \
            g_pause_state.wordAC8 = _val + 0x7D0; \
            _flag = g_pads[0]->data->fog_mode; \
            *_arg0 = _val; \
        } else { \
            _val = (g_pads[0]->data->depth_offset - 0x32000) >> 8; \
            _flag = g_pads[0]->data->fog_mode; \
            if (_flag == 1) { \
                _val -= 0x4B0; \
            } \
            g_pause_state.wordAC8 = _val + 0x320; \
            *_arg0 = _val; \
        } \
        *(arg1) = _flag; \
    }

typedef struct {
    u8 reserved_00[0x1C];
    s32 flags;
} WgeoHeaderView;

typedef struct {
    u32 word0;
    u32 reserved[3];
    WgeoHeaderView *ptr10;
    u32 reserved_2[11];
} GoolEntry64;

typedef struct {
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

    base = (GoolEntry64 *)0x1F800100;
    data = (GoolAnimData *)g_pads[0]->data;
    if (*gp_level_entry[0] != 0) {
        SetRotMatrix(g_m_view);
        SetLightMatrix(g_m_view + 0x110);
        RMemcpy(data->data, (u32)base, data->count << 5);
        GOOL_CALC_FOG(&sp18, &sp1C);
        for (i = 0; i < data->count; i++) {
            if (base[i].ptr10->flags == 1) {
                *(u32 *)(base + i) = (sp1C << 16) | 0xFFFF;
            } else {
                *(u32 *)(base + i) = (sp1C << 16) | sp18;
            }
        }
        Gte_RTransformWorldsFog(gp_level_entry[0], arg0, 0x800 - (g_view_proj[0] / 2), g_draw_stamp[0], Gpu_GetCurPrimsTail(), g_wgeo_tex_anim_masks);
    }
}

