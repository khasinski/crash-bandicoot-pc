#include "common.h"
#include "gte.h"
#include "platform.h"
#include <stdlib.h>

/*
 * World geometry transformation (0x80035438..0x80036320 of the hand-written
 * MIPS block): projects the visible polygons of the loaded worlds and links
 * them into the ordering table as POLY_GT3 (textured) or POLY_G3 (shaded)
 * packets. Gte_RTransformWorldsFog also depth-cues the vertex colours, and
 * Gte_RTransformWorldsRipple displaces flagged vertices vertically.
 *
 * The caller copies the zone's world table to the scratchpad (0x1F800100)
 * and loads the view rotation into the GTE before the call.
 */

/* One world of the scratchpad world table (64 bytes per world). */
typedef struct WorldEntry {
    u32 fog;              /* Fog: near depth (low half) and shift (high half) */
    s32 trans[3];         /* view-space translation, loaded into TR */
    RAM_REF(void) wgeo;
    RAM_REF(WorldPoly) polys;
    RAM_REF(WorldVertex) vertices;
    RAM_REF(u32) texinfos; /* per-texture flags word followed by frame words */
    u32 tpages[8];
} WorldEntry;

/* A world vertex: colour (low 24 bits) and the top bits of z (high byte),
 * then x and y in 13.3 with the low bits of z and a ripple flag (bit 0). */
typedef struct WorldVertex {
    u32 color_z;
    u32 xy;
} WorldVertex;

/* A world triangle. word0: vertex 2 (bits 20..31), texinfo (8..19), tpage
 * slot (5..7), animation phase (0..4). word1: vertex 0 (20..31), vertex 1
 * (8..19), animation speed shift (5..7), animation frame mask (1..4). */
typedef struct WorldPoly {
    u32 word0;
    u32 word1;
} WorldPoly;

/* The visible polygon list: count, then (world << 12 | polygon) per entry,
 * drawn last to first. */
typedef struct WorldPolyList {
    u32 count;
    u16 polys[1];
} WorldPolyList;

/* Texture coordinates of a texinfo: uv0, uv1, uv2 (one u/v byte pair each). */
typedef struct WorldUv {
    u16 uv0;
    u16 uv1;
    u16 uv2;
    u16 pad;
} WorldUv;

typedef struct WorldPolyGT3 {
    u32 tag;
    u32 rgb0;
    u32 xy0;
    u32 clut_uv0;
    u32 rgb1;
    u32 xy1;
    u32 tpage_uv1;
    u32 rgb2;
    u32 xy2;
    u16 uv2;
    u16 pad;
} WorldPolyGT3;

typedef struct WorldPolyG3 {
    u32 tag;
    u32 rgb0;
    u32 xy0;
    u32 rgb1;
    u32 xy1;
    u32 rgb2;
    u32 xy2;
} WorldPolyG3;

/* Scratchpad use: the ripple offsets the caller fills (16 words), the
 * animation stamp, and the world table. The original also saves its
 * caller's registers at 0x1F800000..0x1F80003F, which nothing else reads. */
#define SCRATCH_RIPPLE     0x1F800048u
#define SCRATCH_STAMP      0x1F8000E0u
#define SCRATCH_WORLDS     0x1F800100u

/* GTE command word of dpcs. */
#define GTE_DPCS 0x0780010

typedef enum WorldEffect {
    WORLD_PLAIN,
    WORLD_FOG,
    WORLD_RIPPLE
} WorldEffect;

/* What the polygon loop carries between polygons. */
typedef struct WorldDraw {
    u8 *ot;
    u32 far;       /* far plane, as an ordering-table byte offset */
    u32 slot;      /* byte offset of the last slot used (never increases) */
    u8 *prim;      /* next free packet */
    u32 stamp;
    const u8 *uvs;
    WorldEntry *world;
    u32 world_index;
    u32 fog_near, fog_shift; /* Fog only */
} WorldDraw;

/* Makes world `index` current: loads its translation into the GTE and, for
 * fog, its near depth and shift. */
static void World_Select(WorldDraw *d, u32 index, WorldEffect effect)
{
    WorldEntry *world = RAM_AT(WorldEntry, SCRATCH_WORLDS) + index;

    d->world_index = index;
    d->world = world;
    gte_ldtr(world->trans[0], world->trans[1], world->trans[2]);
    if (effect == WORLD_FOG) {
        d->fog_near = world->fog & 0xFFFF;
        d->fog_shift = world->fog >> 16;
    }
}

/* Ripple: a vertex with bit 0 set has y shifted by the scratchpad offset its
 * position selects. The low half keeps x and the flag bits unmasked. */
static u32 World_RippleXY(u32 word)
{
    u32 xy = word & 0xFFF8FFF8;
    u32 index;
    s32 offset;

    if (!(word & 1))
        return xy;
    index = ((xy >> 19) + ((xy & 0xFFF8) >> 3)) & 0xF;
    offset = RAM_AT(s32, SCRATCH_RIPPLE)[index];
    return (((u32)((s32)xy >> 16) + (u32)offset) << 16) | (word & 0xFFFF);
}

/* Loads the three vertices of `poly` into the GTE and projects them (rtpt).
 * Returns the vertex colour words. */
static void World_Project(const WorldDraw *d, const WorldPoly *poly, WorldEffect effect, u32 color[3])
{
    const WorldVertex *vertices = RAM_AT(WorldVertex, d->world->vertices);
    const WorldVertex *v[3];
    s32 i;

    v[0] = &vertices[poly->word1 >> 20];
    v[1] = &vertices[(poly->word1 >> 8) & 0xFFF];
    v[2] = &vertices[poly->word0 >> 20];
    for (i = 0; i < 3; i++) {
        u32 xy = effect == WORLD_RIPPLE ? World_RippleXY(v[i]->xy) : v[i]->xy & 0xFFF8FFF8;
        gte_mtc2(xy, 2 * i);
    }
    for (i = 0; i < 3; i++) {
        /* z: high byte of the colour word above the low bits kept in xy. */
        u32 low = v[i]->xy & 0x70006;
        color[i] = v[i]->color_z;
        gte_mtc2(((color[i] >> 24) << 3) | (low << 10) | (low >> 3), 2 * i + 1);
    }
    gte_rtpt();
}

/* Fog: depth-cues each colour past the near depth (dpcs with IR0 =
 * (sz - near) << shift). Returns SZ1 + SZ2 + SZ3. */
static u32 World_Fog(const WorldDraw *d, u32 color[3])
{
    u32 sum = 0;
    s32 i;

    for (i = 0; i < 3; i++) {
        u32 sz;
        gte_mfc2(sz, 17 + i);
        sum += sz;
        gte_mtc2(color[i], 6);
        if ((s32)d->fog_near < (s32)sz) {
            gte_mtc2((sz - d->fog_near) << (d->fog_shift & 31), 8);
            Psyz_GteCommand(GTE_DPCS);
            gte_mfc2(color[i], 22);
        }
    }
    return sum;
}

static u32 World_DepthSum(void)
{
    u32 sz1, sz2, sz3;

    gte_mfc2(sz1, 17);
    gte_mfc2(sz2, 18);
    gte_mfc2(sz3, 19);
    return sz1 + sz2 + sz3;
}

/* Texture page, CLUT and coordinates of a textured polygon. Animated
 * textures pick a frame from the stamp. */
static void World_Texture(const WorldDraw *d, const WorldPoly *poly, const u32 *texinfo, u32 flags,
                          WorldPolyGT3 *prim)
{
    u32 anim_mask = poly->word1 & 0x1E;
    u32 tex, tpage, mode, offset, hi;
    const WorldUv *uv;

    if (anim_mask != 0) {
        u32 frame = (d->stamp >> ((poly->word1 >> 5) & 7)) + (poly->word0 & 0x1F);
        texinfo += frame & (anim_mask | 1);
    }
    tex = texinfo[1];
    tpage = d->world->tpages[(poly->word0 >> 5) & 7];
    uv = (const WorldUv *)(d->uvs + (tex >> 22) * 8);
    mode = (tex >> 20) & 3;

    /* Offset of the texture in its page (v << 8 | u). */
    offset = ((((tex & 0x1F) << 2) | (tpage & 0x80)) << 8) | (((tex >> 10) & 0xF8) >> mode);
    hi = (mode << 7) | (tpage & 0x1C) | ((tex >> 18) & 3) | (flags & 0x60);
    prim->tpage_uv1 = (hi << 16) | (uv->uv1 + offset);
    hi = (flags & 0xF) | (tex & 0x1FC0) | ((tpage >> 4) & 0xFFF0);
    prim->clut_uv0 = (hi << 16) | (uv->uv0 + offset);
    prim->uv2 = (u16)(uv->uv2 + offset);
}

/* Links a packet of `words` words into the ordering table at the depth of
 * `depth_sum`, but never behind the slot of the previous polygon. */
static void World_Link(WorldDraw *d, u32 *tag, u32 depth_sum, u32 words)
{
    u32 slot = (d->far - ((depth_sum >> 5) << 2)) & 0x1FFC;
    u32 *entry;

    if (slot < d->slot)
        d->slot = slot;
    entry = (u32 *)(d->ot + d->slot);
    *tag = *entry | (words << 24);
    *entry = Ram_Addr(tag) & 0xFFFFFF;
}

/*
 * Extra polygons for the wide view.
 *
 * The level's list of visible polygons is baked offline per camera-path point
 * for a 4:3 view, so it lacks what a wider view shows at the sides. In the wide
 * view the baked list is kept and the zone's other polygons are tested here:
 * one is added when it is facing the camera, in front of the near limit, its
 * screen rectangle touches the screen, and it is no farther than the farthest
 * baked polygon (the baked lists drop what is distant). The result is merged
 * by depth, the order the baked lists have (World_Link never moves a polygon
 * behind the previous one). The extras are limited, as the primitive buffers
 * are sized for the baked lists.
 */
extern u32 g_pads[];

typedef struct ZoneDataView {
    u8 reserved_00[0x10];
    RAM_REF(s32) data; /* the zone's world count, then the world table */
} ZoneDataView;

#define WIDE_MAX_WORLDS 4 /* the callers copy four worlds to the scratchpad */
#define WIDE_MAX_POLYS (WIDE_MAX_WORLDS * 0x1000)
#define WIDE_MAX_EXTRA 6000
#define WIDE_MAX_TOTAL 5000 /* the primitive buffer also holds objects and effects */
#define WIDE_NEAR 0x40
#define SCREEN_HALF_W 256
#define SCREEN_HALF_H 120

static u32 wide_list[1 + WIDE_MAX_POLYS / 2 + 1];
static u64 wide_keys[WIDE_MAX_POLYS];
static u8 wide_seen[WIDE_MAX_POLYS];

static int wide_key_order(const void *a, const void *b)
{
    u64 x = *(const u64 *)a, y = *(const u64 *)b;

    return x < y ? -1 : x > y;
}

static int wide_in_ram(u32 address)
{
    return address >= 0x80000000u && address < 0x80200000u;
}

/* Projects a polygon of the selected world; false when it cannot be shown. */
static int World_WideProject(WorldDraw *d, const WorldEntry *entry, u32 i, u32 counts[3], u32 xy[3],
                             u32 sz[3])
{
    const WorldPoly *poly = RAM_AT(WorldPoly, entry->polys) + i;
    u32 color[3];

    /* Indices outside the world's tables are not polygons. */
    if ((poly->word1 >> 20) >= counts[0] || ((poly->word1 >> 8) & 0xFFF) >= counts[0]
        || (poly->word0 >> 20) >= counts[0] || ((poly->word0 >> 8) & 0xFFF) >= counts[1])
        return 0;
    World_Project(d, poly, WORLD_PLAIN, color);
    gte_stsxy0(xy[0]);
    gte_stsxy1(xy[1]);
    gte_stsxy2(xy[2]);
    gte_mfc2(sz[0], 17);
    gte_mfc2(sz[1], 18);
    gte_mfc2(sz[2], 19);
    return 1;
}

static int World_OffScreen(const u32 xy[3])
{
    s32 x_low = 0x7FFF, x_high = -0x7FFF, y_low = 0x7FFF, y_high = -0x7FFF, k;

    for (k = 0; k < 3; k++) {
        s32 x = (s16)xy[k], y = (s16)(xy[k] >> 16);

        x_low = x < x_low ? x : x_low;
        x_high = x > x_high ? x : x_high;
        y_low = y < y_low ? y : y_low;
        y_high = y > y_high ? y : y_high;
    }
    return x_high < -SCREEN_HALF_W || x_low > SCREEN_HALF_W || y_high < -SCREEN_HALF_H
           || y_low > SCREEN_HALF_H;
}

/* A one-sided polygon facing away is not drawn (flag bit 3 marks two-sided). */
static int World_BackFacing(const WorldDraw *d, const WorldEntry *entry, u32 i, const u32 xy[3])
{
    const WorldPoly *poly = RAM_AT(WorldPoly, entry->polys) + i;
    s32 ax = (s16)xy[1] - (s16)xy[0], ay = (s16)(xy[1] >> 16) - (s16)(xy[0] >> 16);
    s32 bx = (s16)xy[2] - (s16)xy[0], by = (s16)(xy[2] >> 16) - (s16)(xy[0] >> 16);
    u32 flags = RAM_AT(u32, entry->texinfos)[(poly->word0 >> 8) & 0xFFF] >> 24;

    (void)d;
    return ax * by - ay * bx < 0 && !(flags & 0x08);
}

static WorldPolyList *World_VisibleList(WorldPolyList *baked, s32 far)
{
    WorldDraw d;
    WorldPolyList *list = (WorldPolyList *)wide_list;
    const ZoneDataView *zone;
    u32 worlds, world, total = 0, extra = 0, extra_first, i;
    s32 slot_floor = 0x7FFFFFFF, last_slot = 0;
    u32 extra_limit;

    if (!Platform_WideShown())
        return baked;
    zone = RAM_AT(ZoneDataView, g_pads[0]);
    worlds = (u32)*RAM_AT(s32, zone->data);
    if (worlds > WIDE_MAX_WORLDS)
        worlds = WIDE_MAX_WORLDS;
    d.far = (u32)far << 2;
    d.world_index = 0xFFFF;
    d.world = NULL;
    d.fog_near = d.fog_shift = 0;
    memset(wide_seen, 0, sizeof(wide_seen));
    for (i = 0; i < baked->count; i++)
        wide_seen[baked->polys[i] & (WIDE_MAX_POLYS - 1)] = 1;

    /* The baked polygons keep their order (a polygon is never linked behind
     * the previous one, so the order shapes the picture); their depth is
     * needed to place the extras among them. */
    for (i = 0; i < baked->count; i++) {
        u32 entry = baked->polys[i], xy[3], sz[3];
        const WorldEntry *we = RAM_AT(WorldEntry, SCRATCH_WORLDS) + (entry >> 12);
        u32 counts[2];
        s32 slot = last_slot;

        if ((entry >> 12) < worlds && wide_in_ram(we->wgeo) && wide_in_ram(we->polys)
            && wide_in_ram(we->vertices) && wide_in_ram(we->texinfos)) {
            counts[0] = (u32)RAM_AT(s32, we->wgeo)[4];
            counts[1] = (u32)RAM_AT(s32, we->wgeo)[5];
            World_Select(&d, entry >> 12, WORLD_PLAIN);
            if ((entry & 0xFFF) < (u32)RAM_AT(s32, we->wgeo)[3]
                && World_WideProject(&d, we, entry & 0xFFF, counts, xy, sz)) {
                slot = (s32)d.far - (s32)(((sz[0] + sz[1] + sz[2]) >> 5) << 2);
                /* Baked for a view that may turn, a polygon wholly outside
                 * the screen only takes room from the extras. */
                if (sz[0] >= WIDE_NEAR && sz[1] >= WIDE_NEAR && sz[2] >= WIDE_NEAR
                    && (World_OffScreen(xy) || World_BackFacing(&d, we, entry & 0xFFF, xy))) {
                    last_slot = slot;
                    continue;
                }
            }
        }
        last_slot = slot;
        if (slot < slot_floor)
            slot_floor = slot;
        wide_keys[total++] = ((u64)(u32)slot << 32) | entry;
    }
    extra_first = total;
    extra_limit = total >= WIDE_MAX_TOTAL ? 0 : WIDE_MAX_TOTAL - total;
    if (extra_limit > WIDE_MAX_EXTRA)
        extra_limit = WIDE_MAX_EXTRA;
    for (world = 0; world < worlds && extra < extra_limit; world++) {
        const WorldEntry *entry = RAM_AT(WorldEntry, SCRATCH_WORLDS) + world;
        u32 counts[2], polys;

        if (!wide_in_ram(entry->wgeo) || !wide_in_ram(entry->polys) || !wide_in_ram(entry->vertices)
            || !wide_in_ram(entry->texinfos))
            continue;
        polys = (u32)RAM_AT(s32, entry->wgeo)[3];
        counts[0] = (u32)RAM_AT(s32, entry->wgeo)[4];
        counts[1] = (u32)RAM_AT(s32, entry->wgeo)[5];
        if (polys > 0x1000)
            continue;
        World_Select(&d, world, WORLD_PLAIN);
        for (i = 0; i < polys && extra < extra_limit; i++) {
            const WorldPoly *poly = RAM_AT(WorldPoly, entry->polys) + i;
            u32 xy[3], sz[3], flags;
            s32 x_low = 0x7FFF, x_high = -0x7FFF, y_low = 0x7FFF, y_high = -0x7FFF, k, slot;
            s32 ax, ay, bx, by;

            if (wide_seen[world << 12 | i])
                continue;
            if (!World_WideProject(&d, entry, i, counts, xy, sz))
                continue;
            if (sz[0] < WIDE_NEAR || sz[1] < WIDE_NEAR || sz[2] < WIDE_NEAR)
                continue;
            for (k = 0; k < 3; k++) {
                s32 x = (s16)xy[k], y = (s16)(xy[k] >> 16);

                x_low = x < x_low ? x : x_low;
                x_high = x > x_high ? x : x_high;
                y_low = y < y_low ? y : y_low;
                y_high = y > y_high ? y : y_high;
            }
            if (x_high < -SCREEN_HALF_W || x_low > SCREEN_HALF_W || y_high < -SCREEN_HALF_H
                || y_low > SCREEN_HALF_H)
                continue;
            /* One-sided polygons facing away are not drawn (flag bit 3 marks
             * two-sided ones). */
            ax = (s16)xy[1] - (s16)xy[0];
            ay = (s16)(xy[1] >> 16) - (s16)(xy[0] >> 16);
            bx = (s16)xy[2] - (s16)xy[0];
            by = (s16)(xy[2] >> 16) - (s16)(xy[0] >> 16);
            flags = RAM_AT(u32, entry->texinfos)[(poly->word0 >> 8) & 0xFFF] >> 24;
            if (ax * by - ay * bx < 0 && !(flags & 0x08))
                continue;
            slot = (s32)d.far - (s32)(((sz[0] + sz[1] + sz[2]) >> 5) << 2);
            if (slot < 0 || slot < slot_floor)
                continue; /* beyond the far plane or the baked reach */
            wide_keys[total++] = ((u64)(u32)slot << 32) | (world << 12 | i);
            extra++;
        }
    }
    qsort(wide_keys + extra_first, total - extra_first, sizeof(wide_keys[0]), wide_key_order);
    {
        u32 out = 0, next = extra_first;

        for (i = 0; i < extra_first; i++) {
            while (next < total && (wide_keys[next] >> 32) <= (wide_keys[i] >> 32))
                list->polys[out++] = (u16)wide_keys[next++];
            list->polys[out++] = (u16)wide_keys[i];
        }
        while (next < total)
            list->polys[out++] = (u16)wide_keys[next++];
        list->count = out;
    }
    return list;
}

/* For world_transform_fx.c. */
void *World_VisibleListFor(void *baked, s32 far)
{
    return World_VisibleList(baked, far);
}

/* The shared polygon loop of the three variants. */
static void World_Transform(void *worlds, s32 ot, s32 far, s32 stamp, s32 *prims_tail, u8 *uvs,
                            WorldEffect effect)
{
    WorldPolyList *list = World_VisibleList(worlds, far);
    WorldDraw d;
    u32 i;

    *RAM_AT(s32, SCRATCH_STAMP) = stamp;
    d.ot = RAM_AT(u8, ot);
    d.far = (u32)far << 2;
    d.slot = 0x1FFC;
    d.prim = RAM_AT(u8, *prims_tail);
    d.stamp = (u32)stamp;
    d.uvs = uvs;
    d.world_index = 0xFFFF;
    d.world = NULL;
    d.fog_near = d.fog_shift = 0;

    /* The callers skip an empty list; the original loops at least once. */
    for (i = list->count; i != 0; i--) {
        u32 entry = list->polys[i - 1];
        const WorldPoly *poly;
        const u32 *texinfo;
        u32 color[3], flags, sum;

        if (entry >> 12 != d.world_index)
            World_Select(&d, entry >> 12, effect);
        poly = RAM_AT(WorldPoly, d.world->polys) + (entry & 0xFFF);
        World_Project(&d, poly, effect, color);
        texinfo = RAM_AT(u32, d.world->texinfos) + ((poly->word0 >> 8) & 0xFFF);
        flags = texinfo[0] >> 24;

        if (flags & 0x80) {
            WorldPolyGT3 *prim = (WorldPolyGT3 *)d.prim;
            gte_stsxy0(prim->xy0);
            gte_stsxy1(prim->xy1);
            gte_stsxy2(prim->xy2);
            sum = effect == WORLD_FOG ? World_Fog(&d, color) : 0;
            prim->rgb0 = (color[0] & 0xFFFFFF) | 0x34000000;
            if ((flags & 0x60) != 0x60)
                prim->rgb0 |= 0x02000000; /* semi-transparent */
            prim->rgb1 = color[1];
            prim->rgb2 = color[2];
            World_Texture(&d, poly, texinfo, flags, prim);
            if (effect != WORLD_FOG)
                sum = World_DepthSum();
            World_Link(&d, &prim->tag, sum, 9);
            d.prim += sizeof(WorldPolyGT3);
        } else {
            WorldPolyG3 *prim = (WorldPolyG3 *)d.prim;
            gte_stsxy0(prim->xy0);
            gte_stsxy1(prim->xy1);
            gte_stsxy2(prim->xy2);
            sum = effect == WORLD_FOG ? World_Fog(&d, color) : World_DepthSum();
            prim->rgb0 = (color[0] & 0xFFFFFF) | 0x30000000;
            prim->rgb1 = color[1];
            prim->rgb2 = color[2];
            World_Link(&d, &prim->tag, sum, 6);
            d.prim += sizeof(WorldPolyG3);
        }
    }
    *prims_tail = (s32)Ram_Addr(d.prim);
}

void Gte_RTransformWorlds(void *worlds, s32 ot, s32 far, s32 stamp, s32 *prims_tail, u8 *uvs)
{
    World_Transform(worlds, ot, far, stamp, prims_tail, uvs, WORLD_PLAIN);
}

void Gte_RTransformWorldsFog(void *worlds, s32 ot, s32 far, s32 stamp, s32 *prims_tail, u8 *uvs)
{
    World_Transform(worlds, ot, far, stamp, prims_tail, uvs, WORLD_FOG);
}

void Gte_RTransformWorldsRipple(void *worlds, s32 ot, s32 far, s32 stamp, s32 *prims_tail, u8 *uvs)
{
    World_Transform(worlds, ot, far, stamp, prims_tail, uvs, WORLD_RIPPLE);
}
