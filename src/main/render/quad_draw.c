#include "common.h"
#include "gpu.h"
#include "gte.h"

typedef struct Rect9398 {
    u16 x0;
    u16 reserved_02;
    s32 y0;
    s32 reserved_08;
    u16 x1;
    u16 reserved_0e;
    s32 y1;
} Rect9398;

typedef struct PrimBuf9398 {
    u8 reserved_00[0x78];
    RAM_REF(void) prim_ptr;
} PrimBuf9398;

typedef struct UvPair9398 {
    u32 lo;
    u32 hi;
} UvPair9398;

typedef struct PolyFT4_9398 {
    u32 tag;
    u32 color_code;
    u32 xy0;
    u16 uv0;
    u16 clut;
    u32 xy1;
    u16 uv1;
    u16 tpage;
    u32 xy2;
    u16 uv2;
    u16 reserved_1e;
    u32 xy3;
    u16 uv3;
    u16 reserved_26;
} PolyFT4_9398;

#define CRASH_EXT_RENDER_QUAD_DRAW
#include "render_externs.h"
#undef CRASH_EXT_RENDER_QUAD_DRAW

void Gfx_TransformFragment(u32 *arg0, s32 arg1, u32 arg2, Rect9398 *arg3, u32 *arg4)
{
    u32 *p0;
    u32 y1;
    u32 x0;
    s32 sz1;
    s32 v;
    void *p;
    PolyFT4_9398 *poly;
    u32 w0;
    s32 ot_bias;
    u32 *ot_entry;
    u32 w1;
    u32 tpage;
    u32 uv_base;
    UvPair9398 *uv_ptr;
    u32 uv1;

    y1 = arg3->y1 << 16;
    x0 = arg3->x0;
    v = x0 | y1;
    p0 = arg0;
    gte_ldv0(v, 0);
    v = arg3->x1 | y1;
    gte_ldv1(v, 0);
    v = x0 | (arg3->y0 << 16);
    gte_ldv2(v, 0);
    gte_rtpt();
    gte_getflag_nop(v);
    if (v < 0) {
        return;
    }

    p = g_ot_cur->prim_ptr;
    g_ot_cur->prim_ptr = (PolyFT4_9398 *)p + 1;
    w0 = p0[0];
    poly = (PolyFT4_9398 *)p;
    poly->color_code = w0;
    w0 >>= 24;
    gte_getsz1(sz1);
    ((u8 *)&poly->color_code)[3] = ((w0 & 0x60) == 0x60) ? 0x2C : 0x2E;
    gte_getsz2(v);
    gte_stsxy2(poly->xy2);
    sz1 += v;
    gte_getsz3(v);
    gte_stsxy1(poly->xy1);

    ot_bias = arg1 + 0x800;
    ot_bias -= g_view_proj / 2;
    sz1 = ot_bias - ((u32)(sz1 + v) >> 5);
    if (sz1 < 0x7FF) {
        s32 idx = sz1;
        if (idx < 0) {
            idx = 0;
        }
        ot_entry = &arg4[idx];
    } else {
        ot_entry = &arg4[0x7FF];
    }
    poly->tag = *ot_entry | 0x09000000;
    *ot_entry = (u32)poly & 0xFFFFFF;

    v = arg3->x1 | (arg3->y0 << 16);
    gte_stsxy0(poly->xy0);
    gte_ldv0(v, 0);
    gte_rtps();
    gte_stsxy2(poly->xy3);

    w1 = p0[1];
    uv_base = (w1 >> 20) & 3;
    tpage = uv_base << 7;
    uv_base = ((w1 & 0x3E000) >> (uv_base + 10)) | ((((w1 & 0x1F) << 2) | (arg2 & 0x80)) << 8);
    poly->clut = ((arg2 >> 4) & 0xFFF0) | (w1 & 0x1FC0) | (w0 & 0xF);
    poly->tpage = tpage | (arg2 & 0x1C) | ((w1 >> 18) & 3) | (w0 & 0x60);
    uv_ptr = &((UvPair9398 *)g_wgeo_tex_anim_masks)[w1 >> 22];
    v = uv_ptr->lo;
    uv1 = uv_ptr->hi;
    poly->uv0 = v + uv_base;
    poly->uv1 = ((u32)v >> 16) + uv_base;
    poly->uv2 = uv1 + uv_base;
    poly->uv3 = (uv1 >> 16) + uv_base;
}

typedef struct PolyGT4_9398 {
    u32 tag;
    u8 r0;
    u8 g0;
    u8 b0;
    u8 code;
    u32 xy0;
    u16 uv0;
    u16 clut;
    u8 r1;
    u8 g1;
    u8 b1;
    u8 reserved_13;
    u32 xy1;
    u16 uv1;
    u16 tpage;
    u8 r2;
    u8 g2;
    u8 b2;
    u8 reserved_1f;
    u32 xy2;
    u16 uv2;
    u16 reserved_26;
    u8 r3;
    u8 g3;
    u8 b3;
    u8 reserved_2b;
    u32 xy3;
    u16 uv3;
    u16 reserved_32;
} PolyGT4_9398;

typedef struct LightCols9398 {
    u8 reserved_00[0x48];
    s16 c[4][3];
} LightCols9398;

void Gfx_TransformFontChar(LightCols9398 *arg0, u32 *arg1, s32 arg2, u32 arg3, Rect9398 *arg4, u32 *arg5, s32 arg6)
{
    u32 y1;
    u32 x0;
    s32 sz1;
    s32 v;
    void *p;
    PolyGT4_9398 *poly;
    u32 w0;
    s32 ot_bias;
    u32 *ot_entry;
    u32 w1;
    u32 tpage;
    u32 uv_base;
    UvPair9398 *uv_ptr;
    u32 uv1;

    y1 = arg4->y1 << 16;
    x0 = arg4->x0;
    v = x0 | y1;
    gte_ldv0(v, 0);
    v = arg4->x1 | y1;
    gte_ldv1(v, 0);
    v = x0 | (arg4->y0 << 16);
    gte_ldv2(v, 0);
    gte_rtpt();
    gte_getflag_nop(v);
    if (v < 0) {
        return;
    }

    p = g_ot_cur->prim_ptr;
    g_ot_cur->prim_ptr = (PolyGT4_9398 *)p + 1;
    w0 = arg1[0];
    poly = (PolyGT4_9398 *)p;
    if (arg6 != 0) {
        poly->r0 = (((u8 *)arg1)[0] * arg0->c[0][0]) >> 8;
        poly->g0 = (((u8 *)arg1)[1] * arg0->c[0][1]) >> 8;
        poly->b0 = (((u8 *)arg1)[2] * arg0->c[0][2]) >> 8;
        poly->r1 = (((u8 *)arg1)[0] * arg0->c[1][0]) >> 8;
        poly->g1 = (((u8 *)arg1)[1] * arg0->c[1][1]) >> 8;
        poly->b1 = (((u8 *)arg1)[2] * arg0->c[1][2]) >> 8;
        poly->r2 = (((u8 *)arg1)[0] * arg0->c[2][0]) >> 8;
        poly->g2 = (((u8 *)arg1)[1] * arg0->c[2][1]) >> 8;
        poly->b2 = (((u8 *)arg1)[2] * arg0->c[2][2]) >> 8;
        poly->r3 = (((u8 *)arg1)[0] * arg0->c[3][0]) >> 8;
        poly->g3 = (((u8 *)arg1)[1] * arg0->c[3][1]) >> 8;
        poly->b3 = (((u8 *)arg1)[2] * arg0->c[3][2]) >> 8;
    } else {
        *(u32 *)&poly->r0 = w0;
        *(u32 *)&poly->r1 = w0;
        *(u32 *)&poly->r2 = w0;
        *(u32 *)&poly->r3 = w0;
    }
    w0 >>= 24;
    gte_getsz1(sz1);
    poly->code = ((w0 & 0x60) == 0x60) ? 0x3C : 0x3E;
    gte_getsz2(v);
    gte_stsxy2(poly->xy2);
    sz1 += v;
    gte_getsz3(v);
    gte_stsxy1(poly->xy1);

    ot_bias = arg2 + 0x800;
    ot_bias -= g_view_proj / 2;
    sz1 = ot_bias - ((u32)(sz1 + v) >> 5);
    if (sz1 < 0x7FF) {
        s32 idx = sz1;
        if (idx < 0) {
            idx = 0;
        }
        ot_entry = &arg5[idx];
    } else {
        ot_entry = &arg5[0x7FF];
    }
    poly->tag = *ot_entry | 0x0C000000;
    *ot_entry = (u32)poly & 0xFFFFFF;

    v = arg4->x1 | (arg4->y0 << 16);
    gte_stsxy0(poly->xy0);
    gte_ldv0(v, 0);
    gte_rtps();
    gte_stsxy2(poly->xy3);

    w1 = arg1[1];
    uv_base = (w1 >> 20) & 3;
    tpage = uv_base << 7;
    uv_base = ((w1 & 0x3E000) >> (uv_base + 10)) | ((((w1 & 0x1F) << 2) | (arg3 & 0x80)) << 8);
    poly->clut = ((arg3 >> 4) & 0xFFF0) | (w1 & 0x1FC0) | (w0 & 0xF);
    poly->tpage = tpage | (arg3 & 0x1C) | ((w1 >> 18) & 3) | (w0 & 0x60);
    uv_ptr = &((UvPair9398 *)g_wgeo_tex_anim_masks)[w1 >> 22];
    v = uv_ptr->lo;
    uv1 = uv_ptr->hi;
    poly->uv0 = v + uv_base;
    poly->uv1 = ((u32)v >> 16) + uv_base;
    poly->uv2 = uv1 + uv_base;
    poly->uv3 = (uv1 >> 16) + uv_base;
}

