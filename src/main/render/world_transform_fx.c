#include "common.h"
#include "gte.h"

/*
 * World geometry transforms with special lighting, from the hand-written
 * MIPS block (0x80036340..0x80037D50):
 *
 * - Gte_RTransformWorldsLightning: storm levels. Vertices are depth-cued
 *   towards a far colour while lightning flashes; flagged vertices use a
 *   second colour set.
 * - Gte_RTransformWorldsDark: the same two-set lighting for every triangle,
 *   followed by distance fog towards the zone's far colour.
 * - Gte_RTransformWorldsDark2: dark levels lit only around a point (Crash);
 *   each vertex is darkened by its Manhattan distance from that point.
 *
 * Like Gte_RTransformWorlds they walk the level's list of visible world
 * triangles back to front, transform each with rtpt, append a POLY_GT3 or
 * POLY_G3 to the primitive buffer and link it into the ordering table.
 * The callers copy the per-world state (Gfx_TransformWorlds*) to the
 * scratchpad first.
 *
 * The original saves the caller's registers at scratchpad 0x00..0x3C and
 * restores them on return; nothing else reads them, so the C version
 * leaves that area alone.
 */

/* Per-world state at scratchpad 0x1F800100, 8 slots of 64 bytes. */
typedef struct WorldSlot {
    u32 word_00;
    s32 trans[3];          /* GTE translation (TRX, TRY, TRZ) */
    u32 word_10;
    RAM_REF(u32) tris;     /* two words per triangle */
    RAM_REF(u32) verts;    /* two words per vertex: colour, packed xyz */
    RAM_REF(u32) texinfos; /* texture info words, indexed per triangle */
    u32 tex_pages[8];      /* texture page/CLUT base words */
} WorldSlot;

/* Two depth-cue settings at scratchpad 0x1F800048 (Lightning and Dark). */
typedef struct DepthCueSet {
    s32 far_color[3]; /* GTE RFC, GFC, BFC */
    s32 ir0;          /* interpolation factor */
} DepthCueSet;

/* Light parameters of Dark2 at scratchpad 0x1F800048. */
typedef struct DarkLight {
    s32 center[3];  /* lit point */
    s32 shift_add;  /* falloff: d + (d >> shift_add) - (d >> shift_sub) */
    s32 shift_sub;
    s32 bias[2];    /* added to the falloff; bias[1] for flagged vertices */
} DarkLight;

/* A world triangle as the GTE sees it: tri words and its three vertices. */
typedef struct WorldTri {
    u32 w0, w1;
    u32 color[3]; /* vertex colour; the top byte holds low z bits */
    u32 pos[3];   /* packed x, y and high z bits; bit 0 flags the vertex */
} WorldTri;

/* POLY_GT3 and POLY_G3 in game memory. */
typedef struct WorldPolyGT3 {
    u32 tag;
    u32 rgbc0;
    u32 xy0;
    u32 uv0_clut;
    u32 rgbc1;
    u32 xy1;
    u32 uv1_tpage;
    u32 rgbc2;
    u32 xy2;
    u16 uv2;
} WorldPolyGT3;

typedef struct WorldPolyG3 {
    u32 tag;
    u32 rgbc0;
    u32 xy0;
    u32 rgbc1;
    u32 xy1;
    u32 rgbc2;
    u32 xy2;
} WorldPolyG3;

/* Primitive output state shared by all triangles of one call. */
typedef struct WorldPass {
    u8 *ot;              /* ordering table */
    u32 far;             /* far plane, in OT bytes */
    u32 ot_slot;         /* nearest OT offset used so far */
    u8 *prim;            /* next free primitive */
    u8 *tex_anim_masks;  /* UV descriptors, 8 bytes each */
    WorldSlot *slot;     /* world of the current triangle */
    s32 world;           /* its index, 0xFFFF before the first */
} WorldPass;

#define SCRATCH_LIGHT 0x1F800048u
#define SCRATCH_STAMP 0x1F8000E0u
#define SCRATCH_E4 0x1F8000E4u
#define SCRATCH_WORLDS 0x1F800100u
#define SCRATCH_ORIGINS 0x1F800300u

#define GTE_CMD_DPCS 0x0780010u

#define CODE_POLY_GT3 0x34000000u
#define CODE_POLY_G3 0x30000000u
#define CODE_SEMI_TRANS 0x02000000u

static void pass_init(WorldPass *pass, s32 ot, s32 far, u32 *prims_tail, u8 *tex_anim_masks)
{
    pass->ot = RAM_AT(u8, ot);
    pass->far = (u32)far << 2;
    pass->ot_slot = 0x1FFC;
    pass->prim = RAM_AT(u8, *prims_tail);
    pass->tex_anim_masks = tex_anim_masks;
    pass->slot = NULL;
    pass->world = 0xFFFF;
}

/*
 * Entry of the visible-triangle list: world index in the top 4 bits,
 * triangle index in the low 12. Loads the world's translation into the GTE
 * when the world changes, and fetches the triangle and its vertices.
 */
static void fetch_tri(WorldPass *pass, u32 entry, WorldTri *tri)
{
    const u32 *t, *verts;
    s32 world = (s32)(entry >> 12);

    if (world != pass->world) {
        pass->world = world;
        pass->slot = RAM_AT(WorldSlot, SCRATCH_WORLDS) + world;
        gte_ldtr(pass->slot->trans[0], pass->slot->trans[1], pass->slot->trans[2]);
    }
    t = RAM_AT(u32, pass->slot->tris) + (entry & 0xFFF) * 2;
    verts = RAM_AT(u32, pass->slot->verts);
    tri->w0 = t[0];
    tri->w1 = t[1];
    /* Vertex indices: a = w1 >> 20, b = (w1 >> 8) & 0xFFF, c = w0 >> 20. */
    tri->color[0] = verts[(tri->w1 >> 20) * 2];
    tri->pos[0] = verts[(tri->w1 >> 20) * 2 + 1];
    tri->color[1] = verts[((tri->w1 >> 8) & 0xFFF) * 2];
    tri->pos[1] = verts[((tri->w1 >> 8) & 0xFFF) * 2 + 1];
    tri->color[2] = verts[(tri->w0 >> 20) * 2];
    tri->pos[2] = verts[(tri->w0 >> 20) * 2 + 1];
}

/* Vertex x/y word for the GTE (VXY). */
static u32 vertex_xy(u32 pos)
{
    return pos & 0xFFF8FFF8u;
}

/* Vertex z (VZ): colour bits 24..31 and position bits 1..2 and 16..18. */
static u32 vertex_z(u32 color, u32 pos)
{
    u32 hi = pos & 0x70006u;
    return ((color >> 24) << 3) | (hi << 10) | (hi >> 3);
}

static void load_tri_xy(const WorldTri *tri)
{
    gte_ldvxy0(vertex_xy(tri->pos[0]));
    gte_ldvxy1(vertex_xy(tri->pos[1]));
    gte_ldvxy2(vertex_xy(tri->pos[2]));
}

static void load_tri_z(const WorldTri *tri)
{
    gte_ldvz0(vertex_z(tri->color[0], tri->pos[0]));
    gte_ldvz1(vertex_z(tri->color[1], tri->pos[1]));
    gte_ldvz2(vertex_z(tri->color[2], tri->pos[2]));
}

/* Bit i set when vertex i is flagged (bit 0 of its position word). */
static u32 flagged_vertices(const WorldTri *tri)
{
    return (tri->pos[0] & 1) | (tri->pos[1] & 1) << 1 | (tri->pos[2] & 1) << 2;
}

static void load_far_color(const s32 *rgb)
{
    gte_ctc2(rgb[0], 21);
    gte_ctc2(rgb[1], 22);
    gte_ctc2(rgb[2], 23);
}

/* dpcs on a colour; returns the full RGB2 result. */
static u32 dpcs(u32 color)
{
    u32 rgb;

    gte_ldrgb(color);
    Psyz_GteCommand(GTE_CMD_DPCS);
    gte_mfc2(rgb, 22);
    return rgb;
}

/* dpcs on a colour, keeping its top byte. */
static u32 depth_cue(u32 color)
{
    return (color & 0xFF000000u) | (dpcs(color) & 0xFFFFFFu);
}

/*
 * Two-set lighting of Lightning and Dark. The first set's far colour is
 * loaded. Unflagged vertices are depth-cued with it, then flagged vertices
 * with the second set, each group in vertex order.
 */
static void light_tri_two_sets(WorldTri *tri, u32 flagged, const DepthCueSet *second)
{
    s32 i;

    for (i = 0; i < 3; i++)
        if (!(flagged >> i & 1))
            tri->color[i] = depth_cue(tri->color[i]);
    if (flagged == 0)
        return;
    gte_mtc2(second->ir0, 8);
    load_far_color(second->far_color);
    for (i = 0; i < 3; i++)
        if (flagged >> i & 1)
            tri->color[i] = depth_cue(tri->color[i]);
}

/* Fills the texture fields of a POLY_GT3 from the triangle's texture info. */
static void set_tri_texture(const WorldPass *pass, WorldPolyGT3 *prim, const WorldTri *tri,
                            const u32 *texinfo, u32 flags)
{
    u32 page = pass->slot->tex_pages[(tri->w0 >> 5) & 7];
    u32 anim_mask = tri->w1 & 0x1E;
    u32 tex, mode, offset, tpage, clut, uv01;
    const u8 *uv;

    /* Animated textures: one word per frame, chosen from the draw stamp. */
    if (anim_mask != 0) {
        u32 stamp = *RAM_AT(u32, SCRATCH_STAMP);
        u32 frame = (stamp >> ((tri->w1 >> 5) & 7)) + (tri->w0 & 0x1F);
        texinfo += frame & (anim_mask | 1);
    }
    tex = texinfo[1];
    uv = pass->tex_anim_masks + (tex >> 22) * 8;
    memcpy(&uv01, uv, 4);
    mode = (tex >> 20) & 3;
    tpage = (mode << 7) | (page & 0x1C) | ((tex >> 18) & 3) | (flags & 0x60);
    offset = ((((tex & 0x1F) << 2) | (page & 0x80)) << 8) | (((tex >> 10) & 0xF8) >> mode);
    clut = (flags & 0xF) | (tex & 0x1FC0) | ((page >> 4) & 0xFFF0);
    prim->uv0_clut = (clut << 16) | ((uv01 & 0xFFFF) + offset);
    prim->uv1_tpage = (tpage << 16) | ((uv01 >> 16) + offset);
    prim->uv2 = (u16)(*(const u16 *)(uv + 4) + offset);
}

/* Links a primitive of `len` words into the ordering table by its depth. */
static void link_prim(WorldPass *pass, u32 *tag, u32 len)
{
    u32 sz1, sz2, sz3, z, offset;
    u32 *entry;

    gte_mfc2(sz1, 17);
    gte_mfc2(sz2, 18);
    gte_mfc2(sz3, 19);
    z = ((sz1 + sz2 + sz3) >> 5) << 2;
    offset = (pass->far - z) & 0x1FFC;
    /* Never in front of a primitive linked earlier in this call. */
    if (offset < pass->ot_slot)
        pass->ot_slot = offset;
    entry = (u32 *)(pass->ot + pass->ot_slot);
    *tag = *entry | (len << 24);
    *entry = Ram_Addr(tag) & 0xFFFFFF;
}

/* Writes the transformed triangle as a POLY_GT3 or POLY_G3 and links it. */
static void emit_tri(WorldPass *pass, const WorldTri *tri)
{
    const u32 *texinfo = RAM_AT(u32, pass->slot->texinfos) + ((tri->w0 >> 8) & 0xFFF);
    u32 flags = texinfo[0] >> 24;
    u32 rgbc0 = tri->color[0] & 0xFFFFFF;

    if (flags & 0x80) {
        WorldPolyGT3 *prim = (WorldPolyGT3 *)pass->prim;
        gte_stsxy0(prim->xy0);
        gte_stsxy1(prim->xy1);
        gte_stsxy2(prim->xy2);
        rgbc0 |= CODE_POLY_GT3;
        if ((flags & 0x60) != 0x60)
            rgbc0 |= CODE_SEMI_TRANS;
        prim->rgbc0 = rgbc0;
        prim->rgbc1 = tri->color[1];
        prim->rgbc2 = tri->color[2];
        set_tri_texture(pass, prim, tri, texinfo, flags);
        link_prim(pass, &prim->tag, 9);
        pass->prim += 0x28;
    } else {
        WorldPolyG3 *prim = (WorldPolyG3 *)pass->prim;
        gte_stsxy0(prim->xy0);
        gte_stsxy1(prim->xy1);
        gte_stsxy2(prim->xy2);
        prim->rgbc0 = rgbc0 | CODE_POLY_G3;
        prim->rgbc1 = tri->color[1];
        prim->rgbc2 = tri->color[2];
        link_prim(pass, &prim->tag, 6);
        pass->prim += 0x1C;
    }
}

/* The wide view's list (world_transform.c), or the baked one. */
void *World_VisibleListFor(void *baked, s32 far);

/* The visible-triangle list: a count, then entries walked back to front. */
static u32 tri_count(const void *worlds)
{
    return *(const u32 *)worlds;
}

static u32 tri_entry(const void *worlds, u32 i)
{
    return ((const u16 *)((const u8 *)worlds + 4))[i];
}

/* Storm levels: lightning flashes depth-cue the world towards a colour. */
void Gte_RTransformWorldsLightning(void *worlds, s32 flag, s32 far, s32 stamp, s32 *prims_tail,
                                   u8 *tex_anim_masks)
{
    const DepthCueSet *sets = RAM_AT(DepthCueSet, SCRATCH_LIGHT);
    WorldPass pass;
    WorldTri tri;
    u32 i;

    worlds = World_VisibleListFor(worlds, far);
    *RAM_AT(s32, SCRATCH_STAMP) = stamp;
    pass_init(&pass, flag, far, (u32 *)prims_tail, tex_anim_masks);
    for (i = tri_count(worlds); i != 0; i--) {
        u32 flagged;

        fetch_tri(&pass, tri_entry(worlds, i - 1), &tri);
        flagged = flagged_vertices(&tri);
        gte_mtc2(sets[0].ir0, 8);
        /* Nothing to light without a flash or a flagged vertex. */
        if ((flagged | (u32)sets[0].ir0) != 0) {
            load_far_color(sets[0].far_color);
            light_tri_two_sets(&tri, flagged, &sets[1]);
        }
        load_tri_xy(&tri);
        load_tri_z(&tri);
        gte_rtpt();
        emit_tri(&pass, &tri);
    }
    *prims_tail = (s32)Ram_Addr(pass.prim);
}

/*
 * Two-set lighting as in Lightning, always applied, then distance fog:
 * vertices beyond fog_far are depth-cued towards the GTE's far colour from
 * entry by (sz - fog_far) << fog_mode.
 */
void Gte_RTransformWorldsDark(void *worlds, s32 flag, s32 far, s32 stamp, s32 *prims_tail,
                              u8 *tex_anim_masks, u32 fog_far, s32 fog_mode)
{
    const DepthCueSet *sets = RAM_AT(DepthCueSet, SCRATCH_LIGHT);
    s32 *fog_color = RAM_AT(s32, SCRATCH_E4);
    WorldPass pass;
    WorldTri tri;
    u32 i;
    s32 v;

    worlds = World_VisibleListFor(worlds, far);
    *RAM_AT(s32, SCRATCH_STAMP) = stamp;
    gte_cfc2(fog_color[0], 21);
    gte_cfc2(fog_color[1], 22);
    gte_cfc2(fog_color[2], 23);
    pass_init(&pass, flag, far, (u32 *)prims_tail, tex_anim_masks);
    for (i = tri_count(worlds); i != 0; i--) {
        fetch_tri(&pass, tri_entry(worlds, i - 1), &tri);
        gte_mtc2(sets[0].ir0, 8);
        load_far_color(sets[0].far_color);
        light_tri_two_sets(&tri, flagged_vertices(&tri), &sets[1]);
        load_far_color(fog_color);
        load_tri_xy(&tri);
        load_tri_z(&tri);
        gte_rtpt();
        for (v = 0; v < 3; v++) {
            u32 sz;

            gte_mfc2(sz, 17 + v);
            gte_ldrgb(tri.color[v]);
            if ((s32)fog_far < (s32)sz) {
                gte_mtc2((sz - fog_far) << (fog_mode & 31), 8);
                Psyz_GteCommand(GTE_CMD_DPCS);
                gte_mfc2(tri.color[v], 22);
            }
        }
        emit_tri(&pass, &tri);
    }
    *prims_tail = (s32)Ram_Addr(pass.prim);
}

/* |a - b| with the original's wrapping arithmetic. */
static u32 abs_diff(u32 a, u32 b)
{
    u32 d = a - b;
    return (s32)d < 0 ? -d : d;
}

/*
 * Dark levels: each vertex is depth-cued towards the GTE's far colour by its
 * Manhattan distance from the lit point, plus a bias, clamped to 0..0xFFF.
 * Each world's origin is at scratchpad 0x1F800300 + 16 * world.
 */
void Gte_RTransformWorldsDark2(void *worlds, s32 flag, s32 far, s32 stamp, s32 *prims_tail,
                               u8 *tex_anim_masks)
{
    const DarkLight *light = RAM_AT(DarkLight, SCRATCH_LIGHT);
    WorldPass pass;
    WorldTri tri;
    u32 i;
    s32 v;

    worlds = World_VisibleListFor(worlds, far);
    *RAM_AT(s32, SCRATCH_STAMP) = stamp;
    *RAM_AT(u32, SCRATCH_E4) = Ram_Addr(tex_anim_masks);
    pass_init(&pass, flag, far, (u32 *)prims_tail, tex_anim_masks);
    for (i = tri_count(worlds); i != 0; i--) {
        const s32 *origin;
        u32 dist[3];

        fetch_tri(&pass, tri_entry(worlds, i - 1), &tri);
        origin = RAM_AT(s32, SCRATCH_ORIGINS + pass.world * 16);
        load_tri_xy(&tri);
        for (v = 0; v < 3; v++) {
            u32 xy = vertex_xy(tri.pos[v]);
            dist[v] = abs_diff((u32)origin[0] + (u32)(s16)xy, (u32)light->center[0]) +
                      abs_diff((u32)origin[1] + (u32)((s32)xy >> 16), (u32)light->center[1]);
        }
        load_tri_z(&tri);
        for (v = 0; v < 3; v++) {
            u32 z = (u32)(s16)vertex_z(tri.color[v], tri.pos[v]);
            u32 d = dist[v] + abs_diff((u32)origin[2] + z, (u32)light->center[2]);
            s32 ir0 = (s32)(d + (d >> (light->shift_add & 31)) - (d >> (light->shift_sub & 31)) +
                            (u32)light->bias[tri.pos[v] & 1]);

            if (ir0 >= 0x1000)
                ir0 = 0xFFF;
            else if (ir0 < 0)
                ir0 = 0;
            gte_mtc2(ir0, 8);
            tri.color[v] = dpcs(tri.color[v]);
        }
        gte_rtpt();
        emit_tri(&pass, &tri);
    }
    *prims_tail = (s32)Ram_Addr(pass.prim);
}
