#include "common.h"
#include "gte.h"

/*
 * Model and sprite transformation of the hand-written MIPS block
 * (0x8003460C..0x80035434 and 0x8003A76C): vertex-animated models (SVTX
 * frames lit by normals), vertex-coloured models, the screen bound of an
 * object's box and textured sprites. Each visible triangle or sprite becomes
 * a GPU packet in the primitive buffer, linked into the ordering table by
 * depth.
 */

/* A model frame (SVTX/CVTX item): the origin of the frame's vertices and the
 * vertices themselves, 8-bit coordinates biased by 0x80. The three bytes
 * after the position are the normal (lit models) or the colour
 * (vertex-coloured models). */
typedef struct ModelVertex {
    u8 x, y, z;
    u8 extra[3];
} ModelVertex;

typedef struct ModelFrame {
    u32 word_00;
    u32 model_eid;
    s32 x, y, z;
    u8 reserved_14[0x38 - 0x14];
    ModelVertex vertices[1];
} ModelFrame;

/* Model header (TGEO item 0): the triangle count and the texture/colour
 * infos the triangles index by word. */
typedef struct ModelHeader {
    u32 poly_count;
    s32 scale_x, scale_y, scale_z;
    u32 word_10;
    u32 infos[1];
} ModelHeader;

/* A triangle (TGEO item 1): byte offsets of its three vertices in the
 * frame's vertex array and the word index of its info; bit 15 of `info`
 * lights the triangle with the info colour alone. */
typedef struct ModelPoly {
    u16 v0, v1, v2;
    u16 info;
} ModelPoly;

#define POLY_INFO_INDEX 0x7FFF
#define POLY_INFO_FLAT 0x8000

/* High byte of an info's colour word (and of a sprite frame's first word). */
#define INFO_TEXTURED 0x80
#define INFO_BLEND_MODE 0x60
#define INFO_DOUBLE_SIDED 0x10
#define INFO_CLUT_X 0x0F

/* A textured info: the colour word, a reference to the texture page word and
 * the layout word (colour mode, page and CLUT position, UV origin and the
 * index of the UV set). */
typedef struct ModelTexInfo {
    u32 color;
    RAM_REF(u32) page;
    u32 layout;
} ModelTexInfo;

/* An entry of the UV table: three (triangles) or four (sprites) UV pairs. */
typedef struct UvSet {
    u32 uv01;
    u16 uv2;
    u16 uv3;
} UvSet;

/* GPU packets in their PS1 format. The UV words keep the CLUT or texture page
 * in their high half; the low half is the sum of a template UV and the UV
 * origin, which the original also stores as one word. */
typedef struct PolyGT3 {
    u32 tag;
    u32 color0;
    u32 xy0;
    u32 uv0_clut;
    u32 color1;
    u32 xy1;
    u32 uv1_tpage;
    u32 color2;
    u32 xy2;
    u16 uv2;
    u16 reserved_26;
} PolyGT3;

typedef struct PolyG3 {
    u32 tag;
    u32 color0;
    u32 xy0;
    u32 color1;
    u32 xy1;
    u32 color2;
    u32 xy2;
} PolyG3;

typedef struct PolyFT4 {
    u32 tag;
    u32 color;
    u32 xy0;
    u32 uv0_clut;
    u32 xy1;
    u32 uv1_tpage;
    u32 xy2;
    u16 uv2;
    u16 reserved_1e;
    u32 xy3;
    u16 uv3;
    u16 reserved_26;
} PolyFT4;

_Static_assert(sizeof(PolyGT3) == 0x28, "POLY_GT3 size");
_Static_assert(sizeof(PolyG3) == 0x1C, "POLY_G3 size");
_Static_assert(sizeof(PolyFT4) == 0x28, "POLY_FT4 size");

#define CODE_POLY_G3 0x30000000u
#define CODE_POLY_GT3 0x34000000u
#define CODE_POLY_FT4 0x2C000000u
#define CODE_SEMI_TRANS 0x02000000u

/* Ordering-table entries are 0x800 words; the index is a byte offset. */
#define OT_INDEX_MASK 0x1FFC

/* GTE command words, as the original issues them. */
#define GTE_CMD_NCCT 0x118043F
#define GTE_CMD_DPCS 0x0780010

/*
 * The original also saves the caller's registers in scratchpad words 0..15
 * (Gte_RProjectBound only ra, in word 0) and reloads them on return. Nothing
 * else reads that area, so the C versions leave it alone.
 */

/* Gte_RTransformCvtx keeps the far colour it overrides, the shade shift and
 * the per-channel mask of that shift at scratchpad 0xE0. */
typedef struct CvtxScratch {
    u32 far_color[3];
    s32 shade;
    u32 shade_mask;
} CvtxScratch;

#define CVTX_SCRATCH ((CvtxScratch *)(g_ps1_scratch + 0xE0))

/* Gte_RProjectBound leaves the screen rectangle at scratchpad 0x48. */
typedef struct BoundScratch {
    s32 min_x, min_y, max_x, max_y;
} BoundScratch;

#define BOUND_SCRATCH ((BoundScratch *)(g_ps1_scratch + 0x48))

static s32 gte_flag(void)
{
    return (s32)Psyz_GteCtrlRead(31);
}

/* Loads the three vertices of `poly` into V0..V2: frame origin plus the
 * biased 8-bit coordinates, scaled by 4. */
static void load_poly_vertices(const ModelFrame *frame, const ModelPoly *poly, const ModelVertex **v,
                               u32 ox, u32 oy, u32 oz)
{
    const u8 *base = (const u8 *)frame->vertices;
    int i;

    v[0] = (const ModelVertex *)(base + poly->v0);
    v[1] = (const ModelVertex *)(base + poly->v1);
    v[2] = (const ModelVertex *)(base + poly->v2);
    for (i = 0; i < 3; i++) {
        u32 x = ((v[i]->x + ox) << 2) & 0xFFFF;
        u32 y = (v[i]->y + oy) << 18;
        gte_mtc2(y | x, 2 * i);
        gte_mtc2((v[i]->z + oz) << 2, 2 * i + 1);
    }
}

/* Backface test after rtpt: rejects degenerate triangles and those whose
 * winding sign differs from `facing` (negative for mirrored models). */
static int poly_faces_viewer(s32 facing)
{
    s32 area;

    gte_nclip();
    gte_mfc2(area, 24);
    return area != 0 && (area ^ facing) >= 0;
}

/* Texture fields shared by model triangles and sprites, decoded from the
 * info flags, the layout word and the texture page word. */
typedef struct TexFields {
    u32 uv_origin;
    u32 clut;
    u32 tpage;
    const UvSet *uvs;
} TexFields;

static TexFields decode_texture(u32 flags, u32 layout, u32 page, const u8 *uv_table)
{
    TexFields tex;
    u32 color_mode = (layout >> 20) & 3;
    u32 u = ((layout >> 10) & 0xF8) >> color_mode;
    u32 v = ((layout & 0x1F) << 2) | (page & 0x80);

    tex.uv_origin = u | (v << 8);
    tex.tpage = (color_mode << 7) | (page & 0x1C) | ((layout >> 18) & 3) | (flags & INFO_BLEND_MODE);
    tex.clut = (flags & INFO_CLUT_X) | (layout & 0x1FC0) | ((page >> 4) & 0xFFF0);
    tex.uvs = (const UvSet *)(uv_table + (layout >> 22) * 8);
    return tex;
}

/* Packet colour word: colour, command code and, unless the info selects
 * blend mode 3 (none), the semi-transparency bit. */
static u32 textured_color(u32 color, u32 flags, u32 code)
{
    color = (color & 0xFFFFFF) | code;
    if ((flags & INFO_BLEND_MODE) != INFO_BLEND_MODE)
        color |= CODE_SEMI_TRANS;
    return color;
}

/* Fills the UVs, CLUT and page of a textured triangle. */
static void set_poly_texture(PolyGT3 *p, const TexFields *tex)
{
    p->uv0_clut = (tex->clut << 16) | ((tex->uvs->uv01 & 0xFFFF) + tex->uv_origin);
    p->uv1_tpage = (tex->tpage << 16) | ((tex->uvs->uv01 >> 16) + tex->uv_origin);
    p->uv2 = (u16)(tex->uvs->uv2 + tex->uv_origin);
}

/* Inserts the packet at `prim` (`words` long) at byte offset `index` of the
 * ordering table. */
static void link_prim(u32 *ot, u32 index, void *prim, u32 words)
{
    u32 *entry = (u32 *)((u8 *)ot + index);
    u32 next = *entry;

    *entry = Ram_Addr(prim) & 0xFFFFFF;
    *(u32 *)prim = next | (words << 24);
}

/* Ordering-table offset of a textured triangle: depth = SZ sum / 8. */
static u32 textured_ot_index(u32 far4, u32 depth)
{
    return (far4 - ((depth >> 5) << 2)) & OT_INDEX_MASK;
}

/* Same for a flat triangle, clamped to the first entry when it lies beyond
 * the far plane. */
static u32 flat_ot_index(u32 far4, u32 depth)
{
    s32 index = (s32)(far4 - depth);
    return index < 0 ? 0 : (u32)index & OT_INDEX_MASK;
}

/* SZ1 + SZ2 + SZ3, or with `extremes` 1.5 * (min + max) of them. */
static u32 poly_depth(s32 extremes)
{
    s32 z0, z1, z2, hi, lo;
    u32 sum;

    gte_mfc2(z0, 17);
    gte_mfc2(z1, 18);
    gte_mfc2(z2, 19);
    if (extremes == 0)
        return (u32)z0 + (u32)z1 + (u32)z2;
    hi = z0 < z1 ? z1 : z0;
    if (hi < z2)
        hi = z2;
    lo = z1 < z0 ? z1 : z0;
    if (z2 < lo)
        lo = z2;
    sum = (u32)lo + (u32)hi;
    return sum + (sum >> 1);
}

/*
 * Transforms and lights a vertex-animated model: for each visible triangle of
 * `polys` (`model` holds the count and the infos) it emits a POLY_GT3 or
 * POLY_G3 lit by the vertex normals (NCCT) or flat in the info colour.
 * `facing` is the model's x scale, whose sign selects the front winding;
 * `far` (shifted to an OT byte offset) is the depth of the far plane. With
 * `extremes` the depth comes from the nearest and farthest vertex instead of
 * the sum. `prims_tail` holds the PS1 address of the next free packet.
 */
void Gte_RTransformSvtx(u8 *frame_data, u32 *ot, void *polys, void *model, s32 facing, s32 far,
                        s32 *prims_tail, u8 *uv_table, s32 extremes)
{
    const ModelFrame *frame = (const ModelFrame *)frame_data;
    const ModelHeader *header = model;
    u8 *prim = Ram_Ptr((u32)*prims_tail);
    u32 far4 = (u32)far << 2;
    u32 ox = (u32)frame->x - 0x80, oy = (u32)frame->y - 0x80, oz = (u32)frame->z - 0x80;
    u32 count = header->poly_count, n = 0;

    do {
        const ModelPoly *poly = (const ModelPoly *)polys + n;
        const ModelVertex *v[3];
        const u32 *info = &header->infos[poly->info & POLY_INFO_INDEX];
        u32 color = info[0];
        u32 flags = color >> 24;
        s32 flag;
        int i;

        load_poly_vertices(frame, poly, v, ox, oy, oz);
        gte_rtpt();
        flag = gte_flag();
        gte_ldrgb(color);
        if (flag < 0)
            continue;
        if (!(flags & INFO_DOUBLE_SIDED) && !poly_faces_viewer(facing))
            continue;

        if (poly->info & POLY_INFO_FLAT) {
            gte_mtc2(color, 20);
            gte_mtc2(color, 21);
            gte_mtc2(color, 22);
        } else {
            /* Normals into V0..V2, then light them with NCCT. */
            for (i = 0; i < 3; i++) {
                gte_mtc2((v[i]->extra[0] << 8) | ((u32)v[i]->extra[1] << 24), 2 * i);
                gte_mtc2(v[i]->extra[2] << 8, 2 * i + 1);
            }
            Psyz_GteCommand(GTE_CMD_NCCT);
        }

        if (flags & INFO_TEXTURED) {
            PolyGT3 *p = (PolyGT3 *)prim;
            const ModelTexInfo *ti = (const ModelTexInfo *)info;
            TexFields tex = decode_texture(flags, ti->layout, *RAM_AT(u32, ti->page), uv_table);

            gte_swc2(p->xy0, 12);
            gte_swc2(p->xy1, 13);
            gte_swc2(p->xy2, 14);
            p->color0 = textured_color(Psyz_GteDataRead(20), flags, CODE_POLY_GT3);
            gte_swc2(p->color1, 21);
            gte_swc2(p->color2, 22);
            set_poly_texture(p, &tex);
            link_prim(ot, textured_ot_index(far4, poly_depth(extremes)), p, 9);
            prim += sizeof(PolyGT3);
        } else {
            PolyG3 *p = (PolyG3 *)prim;

            gte_swc2(p->xy0, 12);
            gte_swc2(p->xy1, 13);
            gte_swc2(p->xy2, 14);
            p->color0 = (Psyz_GteDataRead(20) & 0xFFFFFF) | CODE_POLY_G3;
            gte_swc2(p->color1, 21);
            gte_swc2(p->color2, 22);
            link_prim(ot, flat_ot_index(far4, poly_depth(extremes) >> 3), p, 6);
            prim += sizeof(PolyG3);
        }
    } while (++n != count);
    *prims_tail = (s32)Ram_Addr(prim);
}

/* Depth-cues one colour channel of the three vertices: the info's channel
 * level (0x80 = unchanged) fades the vertex colours toward black or white
 * with DPCS. Returns the three results, one per byte. */
static u32 cue_vertex_channel(u32 level, u32 channels)
{
    u32 fade, ir0;
    int i;

    if (level < 0x80) {
        fade = 0;
        ir0 = 0x7F - level;
    } else {
        fade = 0xFF0;
        ir0 = level - 0x80;
    }
    for (i = 21; i <= 23; i++)
        gte_ctc2(fade, i);
    gte_mtc2(ir0 << 5, 8);
    gte_mtc2(channels, 6);
    Psyz_GteCommand(GTE_CMD_DPCS);
    return Psyz_GteDataRead(22);
}

/*
 * Transforms a vertex-coloured model. Like Gte_RTransformSvtx, but the three
 * bytes after each vertex position are its colour, tinted by the info colour
 * through depth cueing and darkened by `shade` (a right shift). Back faces of
 * double-sided triangles are kept, without blending and pushed back in the
 * ordering table.
 */
void Gte_RTransformCvtx(u8 *frame_data, u32 *ot, void *polys, void *model, s32 facing, s32 far,
                        s32 *prims_tail, u8 *uv_table, s32 shade)
{
    const ModelFrame *frame = (const ModelFrame *)frame_data;
    const ModelHeader *header = model;
    CvtxScratch *state = CVTX_SCRATCH;
    u8 *prim = Ram_Ptr((u32)*prims_tail);
    u32 far4 = (u32)far << 2;
    u32 ox = (u32)frame->x - 0x80, oy = (u32)frame->y - 0x80, oz = (u32)frame->z - 0x80;
    u32 count = header->poly_count, n = 0;
    u32 shift = (u32)shade & 31;
    u32 mask = 0xFFu >> shift;
    int i;

    state->shade = shade;
    state->shade_mask = mask | (mask << 8) | (mask << 16);
    for (i = 0; i < 3; i++)
        state->far_color[i] = Psyz_GteCtrlRead(21 + i);

    do {
        const ModelPoly *poly = (const ModelPoly *)polys + n;
        const ModelVertex *v[3];
        const u32 *info = &header->infos[poly->info & POLY_INFO_INDEX];
        u32 color = info[0];
        u32 flags = color >> 24;
        u32 bias = 0;
        u32 vc[3] = {0, 0, 0};
        int c;

        load_poly_vertices(frame, poly, v, ox, oy, oz);
        gte_rtpt();
        if (gte_flag() < 0)
            continue;
        if (!poly_faces_viewer(facing)) {
            if (!(flags & INFO_DOUBLE_SIDED))
                continue;
            /* A back face: blend mode 1 or 3 and a little farther away. */
            flags &= ~0x20u;
            bias = 0x30;
        }

        /* Channel by channel, all three vertices at once. */
        for (c = 0; c < 3; c++) {
            u32 channels = v[0]->extra[c] | (v[1]->extra[c] << 8) | ((u32)v[2]->extra[c] << 16);
            u32 out = cue_vertex_channel((color >> (8 * c)) & 0xFF, channels);
            for (i = 0; i < 3; i++)
                vc[i] |= ((out >> (8 * i)) & 0xFF) << (8 * c);
        }
        if (shade != 0) {
            for (i = 0; i < 3; i++)
                vc[i] = (vc[i] >> shift) & state->shade_mask;
        }

        if (flags & INFO_TEXTURED) {
            PolyGT3 *p = (PolyGT3 *)prim;
            const ModelTexInfo *ti = (const ModelTexInfo *)info;
            TexFields tex;

            gte_swc2(p->xy0, 12);
            gte_swc2(p->xy1, 13);
            gte_swc2(p->xy2, 14);
            p->color0 = textured_color(vc[0], flags, CODE_POLY_GT3);
            p->color1 = vc[1];
            p->color2 = vc[2];
            tex = decode_texture(flags, ti->layout, *RAM_AT(u32, ti->page), uv_table);
            set_poly_texture(p, &tex);
            link_prim(ot, (far4 - (((poly_depth(0) >> 5) << 2) + bias)) & OT_INDEX_MASK, p, 9);
            prim += sizeof(PolyGT3);
        } else {
            PolyG3 *p = (PolyG3 *)prim;

            gte_swc2(p->xy0, 12);
            gte_swc2(p->xy1, 13);
            gte_swc2(p->xy2, 14);
            p->color0 = vc[0] | CODE_POLY_G3;
            p->color1 = vc[1];
            p->color2 = vc[2];
            link_prim(ot, flat_ot_index(far4, (poly_depth(0) >> 3) + bias), p, 6);
            prim += sizeof(PolyG3);
        }
    } while (++n != count);

    for (i = 0; i < 3; i++)
        gte_ctc2(state->far_color[i], 21 + i);
    *prims_tail = (s32)Ram_Addr(prim);
}

/* Grows the screen rectangle by the screen point `sxy`. */
static void extend_bound(BoundScratch *r, u32 sxy)
{
    s32 x = (s16)(sxy & 0xFFFF);
    s32 y = (s32)sxy >> 16;

    if (x < r->min_x)
        r->min_x = x;
    if (r->max_x < x)
        r->max_x = x;
    if (y < r->min_y)
        r->min_y = y;
    if (r->max_y < y)
        r->max_y = y;
}

/* Projects the three vertices loaded in V0..V2 and grows the rectangle by
 * the last `points` of them. */
static void project_corners(BoundScratch *r, int points)
{
    int i;

    gte_rtpt();
    for (i = 3 - points; i < 3; i++)
        extend_bound(r, Psyz_GteDataRead(12 + i));
}

/*
 * Projects the eight corners of an object's bounding box (`bound`: min and
 * max, relative to the object position `trans`, moved by `offset - view`)
 * and leaves the screen rectangle at scratchpad 0x48 (min x, min y, max x,
 * max y).
 */
void Gte_RProjectBound(s32 *bound, void *offset, void *trans, void *view)
{
    const s32 *o = offset, *t = trans, *e = view;
    BoundScratch r;
    u32 x0, y0, z0, x1, y1, z1;

    x0 = (u32)(((s32)((u32)bound[0] + t[0] + ((u32)o[0] - e[0]))) >> 8) & 0xFFFF;
    y0 = (u32)(((s32)((u32)bound[1] + t[1] + ((u32)o[1] - e[1]))) >> 8) << 16;
    z0 = (u32)(((s32)((u32)bound[2] + t[2] + ((u32)o[2] - e[2]))) >> 8) & 0xFFFF;
    x1 = (u32)(((s32)((u32)bound[3] + t[0] + ((u32)o[0] - e[0]))) >> 8) & 0xFFFF;
    y1 = (u32)(((s32)((u32)bound[4] + t[1] + ((u32)o[1] - e[1]))) >> 8) << 16;
    z1 = (u32)(((s32)((u32)bound[5] + t[2] + ((u32)o[2] - e[2]))) >> 8) & 0xFFFF;

    r.min_x = r.min_y = 0x7FFFFFFF;
    r.max_x = r.max_y = (s32)0x80000000u;

    /* Corners (x0,y0,z0), (x0,y0,z1), (x0,y1,z0). */
    gte_ldv0(x0 | y0, z0);
    gte_ldv1(x0 | y0, z1);
    gte_ldv2(x0 | y1, z0);
    project_corners(&r, 3);
    /* (x0,y1,z1), (x1,y0,z0), (x1,y0,z1). */
    gte_ldv0(x0 | y1, z1);
    gte_ldv1(x1 | y0, z0);
    gte_ldv2(x1 | y0, z1);
    project_corners(&r, 3);
    /* (x1,y1,z0), (x1,y1,z1); V0 is projected again but not used. */
    gte_ldv1(x1 | y1, z0);
    gte_ldv2(x1 | y1, z1);
    project_corners(&r, 2);

    *BOUND_SCRATCH = r;
}

/*
 * Emits a textured sprite (POLY_FT4) of half-size `size` centred on the
 * current GTE transformation. `frame` holds the colour/flags word and the
 * texture layout word, `page` the texture page word. Nothing is emitted when a
 * corner fails to project; the packet may then be partly written.
 */
void Gte_RTransformSprite(void *ot, void *frame, u32 page, s32 size, s32 far, s32 *prims_tail,
                          u8 *uv_table)
{
    const u32 *words = frame;
    PolyFT4 *p = Ram_Ptr((u32)*prims_tail);
    u32 color = words[0];
    u32 flags = color >> 24;
    u32 pos = (u32)size & 0xFFFF, neg = (u32)-size & 0xFFFF;
    u32 depth, index;
    s32 sorted;
    TexFields tex;

    gte_ldv0(neg | (pos << 16), 0);
    gte_ldv1(pos | (pos << 16), 0);
    gte_ldv2(neg | (neg << 16), 0);
    gte_rtpt();
    /* The fourth corner is loaded before the error check. */
    gte_mtc2(pos | (neg << 16), 0);
    if (gte_flag() < 0)
        return;
    gte_mtc2(0, 1);
    gte_swc2(p->xy0, 12);
    gte_swc2(p->xy1, 13);
    gte_swc2(p->xy2, 14);
    depth = (poly_depth(0) >> 5) << 2;
    gte_rtps();
    if (gte_flag() < 0)
        return;
    gte_swc2(p->xy3, 14);

    p->color = textured_color(color, flags, CODE_POLY_FT4);
    tex = decode_texture(flags, words[1], page, uv_table);
    p->uv0_clut = (tex.clut << 16) | ((tex.uvs->uv01 & 0xFFFF) + tex.uv_origin);
    p->uv1_tpage = (tex.tpage << 16) | ((tex.uvs->uv01 >> 16) + tex.uv_origin);
    p->uv2 = (u16)(tex.uvs->uv2 + tex.uv_origin);
    p->uv3 = (u16)(tex.uvs->uv3 + tex.uv_origin);

    /* Sprites beyond the last entry go to the last entry. */
    sorted = (s32)(((u32)far << 2) - depth);
    index = (u32)sorted & OT_INDEX_MASK;
    if ((s32)index < sorted)
        index = OT_INDEX_MASK;
    link_prim(ot, index, p, 9);
    *prims_tail = (s32)Ram_Addr(p + 1);
}
