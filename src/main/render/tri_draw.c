#include "common.h"
#include "gpu.h"
#include "gte.h"

#define CRASH_EXT_RENDER_TRI_DRAW_0
#include "render_externs.h"
#undef CRASH_EXT_RENDER_TRI_DRAW_0
typedef struct PrimBuf9398 {
    u8 reserved_00[0x78];
    void *prim_ptr;
} PrimBuf9398;

#define CRASH_EXT_RENDER_TRI_DRAW_1
#include "render_externs.h"
#undef CRASH_EXT_RENDER_TRI_DRAW_1

typedef struct Vert9DCC {
    u32 v0;
    s32 v1;
} Vert9DCC;

typedef struct Tri9DCC {
    u32 w0;
    u32 w1;
} Tri9DCC;

typedef struct Group9DCC {
    s32 word_00;
    volatile s32 trx;
    s32 try;
    s32 trz;
    s32 word_10;
    u8 *tris;
    u8 *verts;
    u8 *tex_tab;
    u8 reserved_20[0x20];
} Group9DCC;

typedef struct ZoneEntry9DCC {
    u8 reserved_00[0x10];
    u8 *header;
} ZoneEntry9DCC;

typedef struct SortList9DCC {
    u16 count;
    u16 reserved_02;
    u16 entries[1];
} SortList9DCC;

typedef struct UvEntry9DCC {
    u32 lo;
    u16 hi;
    u16 reserved_06;
} UvEntry9DCC;

typedef struct PolyGT3_9DCC {
    u32 tag;
    u32 color0;
    u32 xy0;
    u16 uv0;
    u16 clut;
    u32 color1;
    u32 xy1;
    u16 uv1;
    u16 tpage;
    u32 color2;
    u32 xy2;
    u16 uv2;
    u16 reserved_26;
} PolyGT3_9DCC;

typedef struct PolyG3_9DCC {
    u32 tag;
    u32 color0;
    u32 xy0;
    u32 color1;
    u32 xy1;
    u32 color2;
    u32 xy2;
} PolyG3_9DCC;

#define CRASH_EXT_RENDER_TRI_DRAW_2
#include "render_externs.h"
#undef CRASH_EXT_RENDER_TRI_DRAW_2

void Gfx_UpdateCamMatrices(u32 *arg0)
{
    Group9DCC *groups;
    s32 ot_idx;
    s32 prev_group;
    s32 i;
    Group9DCC *group_ptr;
    u32 entry;
    Tri9DCC *tri_ptr;
    u32 tri_w1;
    u8 *verts;
    u32 tri_w0;
    Vert9DCC *vtx0;
    Vert9DCC *vtx1;
    Vert9DCC *vtx2;
    s32 a0;
    u32 xy;
    s32 z;
    u32 w0;
    PolyGT3_9DCC *poly;
    u32 t3;
    u32 w1;
    u32 uv_base;
    u32 tpage;
    UvEntry9DCC *uv_ptr;
    s32 sz1;
    s32 temp;
    PolyG3_9DCC *poly_g3;
    s32 reserved[2];

    groups = (Group9DCC *)(g_pads->header + 4);
    ot_idx = 0x7FF;
    SetRotMatrix(g_m_view);
    SetLightMatrix(g_m_view + 0x110);
    prev_group = -1;
    for (i = gp_level_entry->count - 1; i >= 0; i--) {
        entry = gp_level_entry->entries[i];
        if ((s32)(entry >> 12) != prev_group) {
            s32 trx, try, trz;
            prev_group = entry >> 12;
            group_ptr = &groups[prev_group];
            trx = group_ptr->trx;
            try = group_ptr->try;
            trz = group_ptr->trz;
            gte_ldtr(trx, try, trz);
        }
        tri_ptr = (Tri9DCC *)(((entry & 0xFFF) << 3) + (u32)group_ptr->tris);
        tri_w1 = tri_ptr->w1;
        verts = group_ptr->verts;
        vtx0 = (Vert9DCC *)(((tri_w1 >> 20) << 3) + (u32)verts);
        a0 = vtx0->v1;
        tri_w0 = tri_ptr->w0;
        xy = (((a0 >> 16) & ~7) << 16) | (a0 & 0xFFF8);
        z = ((s32)(((a0 << 13) & 0xE0000000) | ((a0 << 26) & 0x18000000) | ((vtx0->v0 >> 5) & 0x07F80000)) >> 16) & ~7;
        gte_ldv0(xy, z);

        vtx1 = (Vert9DCC *)((((tri_w1 >> 8) & 0xFFF) << 3) + (u32)verts);
        a0 = vtx1->v1;
        xy = (((a0 >> 16) & ~7) << 16) | (a0 & 0xFFF8);
        z = ((s32)(((a0 << 13) & 0xE0000000) | ((a0 << 26) & 0x18000000) | ((vtx1->v0 >> 5) & 0x07F80000)) >> 16) & ~7;
        gte_ldv1(xy, z);

        vtx2 = (Vert9DCC *)(((tri_w0 >> 20) << 3) + (u32)verts);
        a0 = vtx2->v1;
        xy = (((a0 >> 16) & ~7) << 16) | (a0 & 0xFFF8);
        z = ((s32)(((a0 << 13) & 0xE0000000) | ((a0 << 26) & 0x18000000) | ((vtx2->v0 >> 5) & 0x07F80000)) >> 16) & ~7;
        gte_ldv2(xy, z);
        gte_rtpt();

        xy = (((tri_w0 >> 8) & 0xFFF) << 2) + (u32)group_ptr->tex_tab;
        w0 = *(u32 *)xy;
        gte_ldrgb(w0);
        w0 >>= 24;
        if (w0 & 0x80) {
            void *p = g_ot_cur->prim_ptr;
            g_ot_cur->prim_ptr = (PolyGT3_9DCC *)p + 1;
            t3 = **(u32 ***)(((tri_w0 & 0xE0) >> 3) + (u32)group_ptr->tex_tab - 0x20);
            poly = (PolyGT3_9DCC *)p;
            poly->color0 = vtx0->v0;
            poly->color1 = vtx1->v0;
            poly->color2 = vtx2->v0;
            if (tri_w1 & 0x1E) {
                a0 = (tri_w1 & 0x1E) | 1;
                w1 = ((u32 *)xy)[1 + (((g_draw_stamp[0] >> ((tri_w1 >> 5) & 7)) + (tri_w0 & 0x1F)) & a0)];
            } else {
                w1 = ((u32 *)xy)[1];
            }
            gte_stsxy0(poly->xy0);
            uv_base = (w1 >> 20) & 3;
            tpage = uv_base << 7;
            uv_base = ((w1 & 0x3E000) >> (uv_base + 10)) | ((((w1 & 0x1F) << 2) | (t3 & 0x80)) << 8);
            poly->clut = ((t3 >> 4) & 0xFFF0) | (w1 & 0x1FC0) | (w0 & 0xF);
            poly->tpage = tpage | (t3 & 0x1C) | ((w1 >> 18) & 3) | (w0 & 0x60);
            uv_ptr = &((UvEntry9DCC *)g_wgeo_tex_anim_masks)[w1 >> 22];
            xy = uv_ptr->lo;
            z = uv_ptr->hi;
            poly->uv0 = xy + uv_base;
            poly->uv1 = (xy >> 16) + uv_base;
            poly->uv2 = z + uv_base;
            gte_getsz1(sz1);
            ((u8 *)&poly->color0)[3] = ((w0 & 0x60) == 0x60) ? 0x34 : 0x36;
            gte_getsz2(xy);
            gte_stsxy1(poly->xy1);
            sz1 += (s32)xy;
            gte_getsz3(xy);
            gte_stsxy2(poly->xy2);
            temp = 0x800 - ((u32)(sz1 + (s32)xy) >> 5);
            sz1 = temp - (g_view_proj / 2);
            ot_idx = (sz1 >= ot_idx) ? ot_idx : ((sz1 >= 0) ? sz1 : 0);
            xy = (u32)&arg0[ot_idx];
            poly->tag = *(u32 *)xy | 0x09000000;
            *(u32 *)xy = (u32)poly & 0xFFFFFF;
        } else {
            void *p = g_ot_cur->prim_ptr;
            g_ot_cur->prim_ptr = (PolyG3_9DCC *)p + 1;
            poly_g3 = (PolyG3_9DCC *)p;
            poly_g3->color0 = vtx0->v0;
            poly_g3->color1 = vtx1->v0;
            poly_g3->color2 = vtx2->v0;
            gte_stsxy0(poly_g3->xy0);
            gte_getsz1(sz1);
            ((u8 *)&poly_g3->color0)[3] = ((w0 & 0x60) == 0x60) ? 0x30 : 0x32;
            gte_getsz2(xy);
            gte_stsxy1(poly_g3->xy1);
            sz1 += (s32)xy;
            gte_getsz3(xy);
            gte_stsxy2(poly_g3->xy2);
            temp = 0x800 - ((u32)(sz1 + (s32)xy) >> 5);
            sz1 = temp - (g_view_proj / 2);
            ot_idx = (sz1 >= ot_idx) ? ot_idx : ((sz1 >= 0) ? sz1 : 0);
            xy = (u32)&arg0[ot_idx];
            poly_g3->tag = *(u32 *)xy | 0x06000000;
            *(u32 *)xy = (u32)poly_g3 & 0xFFFFFF;
        }
    }
}
