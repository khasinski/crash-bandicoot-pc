#include "common.h"

/*
 * Collision queries of the hand-written MIPS block (0x80037D50..0x8003902C):
 * collecting the leaves of the zone collision octrees around an object into
 * the query workspace, plotting the walls among them into the 32x32 wall
 * grid, and averaging the floor and ceiling heights under and above it.
 *
 * The workspace node list holds, per zone, a two-entry header (the zone size
 * and the octree depth limit per axis) followed by its leaves, and ends with
 * a 0xFFFF entry. A leaf keeps its octree value in the flags (bits 3..15, see
 * Solid_RZoneQueryOctree), its depth in bits 0..2 and its minimum corner
 * relative to the query box in units of 16.
 */

typedef struct ZqVec {
    s32 x, y, z;
} ZqVec;

typedef struct ZqBound {
    ZqVec p1;
    ZqVec p2;
} ZqBound;

typedef struct ZqZoneRect {
    s32 x, y, z;
    s32 w, h, d;
    u32 word_18;
    u16 root;
    s16 max_depth_x;
    s16 max_depth_y;
    s16 max_depth_z;
} ZqZoneRect;

typedef struct SubWsNode {
    u16 flags;
    u16 x;
    u16 y;
    u16 z;
} SubWsNode;

typedef struct SubWorkspace {
    SubWsNode nodes[0x200];
    s32 initialized;
    s32 count;
    ZqBound bound1;
    ZqBound bound2;
    s32 word_1038;
    s32 word_103c;
    s32 word_1040;
    s32 word_1044;
    s32 word_1048;
    s32 word_104c;
} SubWorkspace;

/* State of Solid_RFindFloorY's scan. The original keeps it in registers and
 * spills it to the scratchpad around the node callback, then continues with
 * what it reloads from there. */
typedef struct FloorScan {
    s32 max_y;
    u32 value;
    s32 floor_count;
    s32 floor_sum;
    u32 type2; /* node type * 2 */
    s32 wall_count;
    RAM_REF(const SubWsNode) node;
    s32 hi[3]; /* node maximum corner */
    s32 bound_x1, bound_y1;
} FloorScan;

/* The scratchpad as these routines use it. */
typedef struct SolidScratch {
    /* 0x000: the original saves the caller's s0-s7, t8, t9, k0, k1, gp, sp,
     * fp and ra here and reloads them on return; nothing else reads them, so
     * the C version leaves the area alone. */
    u8 reserved_00[0x70];
    /* 0x070: Solid_RFindFloorY's scan state across the node callback. */
    FloorScan floor_scan;
    u8 reserved_a0[0x40];
    /* 0x0E0: values kept across calls. */
    union {
        struct {
            s32 mode;
            s32 y_min;
        } plot;
        struct {
            s32 init;
            u32 node_fn;
            RAM_REF(s32) out;
            RAM_REF(struct SubObj) obj;
            s32 wall_sum;
        } floor;
    } keep;
    u8 reserved_f4[0x0C];
    /* 0x100: wall grid, one bit per cell, row by row (gp_grid_rows). */
    u32 rows[32];
    /* 0x180: cells already plotted, by row offset +32 and sign of x
     * (gp_solid_scratch_grid). */
    u32 plotted[64][2];
    /* 0x380: the shape stamped for one wall (gp_solid_scratch_workspace). */
    u32 wall_shape[32];
} SolidScratch;

_Static_assert(sizeof(SolidScratch) == RAM_SCRATCH_SIZE, "SolidScratch covers the scratchpad");

#define SCRATCH ((SolidScratch *)g_ps1_scratch)

/* Return address of Solid_RFindFloorY's node callback (jalr at 0x80038CEC). */
#define FLOOR_CALLBACK_RA 0x80038CF4u

/* Node flags of the list terminator and of a zone header entry. */
#define NODE_END 0xFFFF
#define NODE_HEADER 0

/* Size and octree depth limit of the zone whose leaves follow a header. */
typedef struct ZoneDims {
    u32 size[3];
    u32 max_depth[3];
} ZoneDims;

static void read_zone_header(ZoneDims *dims, const SubWsNode *header)
{
    dims->size[0] = (u32)header[0].x << 8;
    dims->size[1] = (u32)header[0].y << 8;
    dims->size[2] = (u32)header[0].z << 8;
    dims->max_depth[0] = header[1].x;
    dims->max_depth[1] = header[1].y;
    dims->max_depth[2] = header[1].z;
}

/* Octree value of a leaf: bits 1..13 of the original node word, odd. */
static u32 node_value(const SubWsNode *node)
{
    return ((u32)(node->flags & 0xFFF8) >> 2) | 1;
}

/* Collision type of a leaf, (value >> 1) & 7. */
static u32 node_type(const SubWsNode *node)
{
    return (node_value(node) & 0xE) >> 1;
}

/* Box of a leaf in world units: the minimum corner from the stored offset
 * against `origin`, the size from the zone size halved once per level, up to
 * the depth limit of each axis. */
static void node_box(const SubWsNode *node, const ZqVec *origin, const ZoneDims *dims, s32 *lo,
                     s32 *hi)
{
    u32 depth = node->flags & 7;
    s32 i;

    lo[0] = (s32)((u32)origin->x + (u32)((s16)node->x * 16));
    lo[1] = (s32)((u32)origin->y + (u32)((s16)node->y * 16));
    lo[2] = (s32)((u32)origin->z + (u32)((s16)node->z * 16));
    for (i = 0; i < 3; i++) {
        u32 level = dims->max_depth[i] < depth ? dims->max_depth[i] : depth;
        hi[i] = (s32)((u32)lo[i] + (dims->size[i] >> (level & 31)));
    }
}

/* Walk state of Solid_RZoneQueryOctree. Positions are relative to the
 * query's minimum corner. */
typedef struct OctreeWalk {
    const u8 *zone;
    u32 max_depth[3];
    s32 extent[3];
    s32 pos[3];
    u32 size[3];
    SubWsNode *out;
    s32 count;
} OctreeWalk;

/* Visits the octree node `word` at `depth`, whose box starts at w->pos with
 * size w->size. 0 is empty space, an odd word a leaf; an even word is the
 * offset from the zone of the node's child words. A node splits each axis
 * whose depth limit is not reached yet into a lower and an upper half; its
 * children are ordered with x as the most significant axis, and only those
 * overlapping the query are visited. */
static void octree_visit(OctreeWalk *w, u32 word, s32 depth)
{
    const u16 *children;
    s32 axes[3], lower[3], upper[3];
    s32 n = 0, i, child;

    if (word == 0)
        return;
    if (word & 1) {
        /* A leaf: append it while the workspace has room. */
        if (w->count < 0x200) {
            w->out->flags = (u16)(((word << 2) & 0xFFF8) | (u32)depth);
            w->out->x = (u16)((u32)w->pos[0] >> 4);
            w->out->y = (u16)((u32)w->pos[1] >> 4);
            w->out->z = (u16)((u32)w->pos[2] >> 4);
            w->out++;
            w->count++;
        }
        return;
    }
    children = (const u16 *)(w->zone + word);
    for (i = 0; i < 3; i++) {
        if ((u32)depth < w->max_depth[i])
            axes[n++] = i;
    }
    for (i = 0; i < n; i++) {
        s32 a = axes[i];
        s32 mid;

        w->size[a] >>= 1;
        mid = (s32)((u32)w->pos[a] + w->size[a]);
        lower[i] = mid >= 0;
        upper[i] = !(w->extent[a] < mid);
    }
    for (child = 0; child < 1 << n; child++) {
        s32 inside = 1;

        for (i = 0; i < n; i++) {
            s32 high = (child >> (n - 1 - i)) & 1;
            inside &= high ? upper[i] : lower[i];
        }
        if (!inside)
            continue;
        for (i = 0; i < n; i++) {
            if ((child >> (n - 1 - i)) & 1)
                w->pos[axes[i]] = (s32)((u32)w->pos[axes[i]] + w->size[axes[i]]);
        }
        octree_visit(w, children[child], depth + 1);
        for (i = 0; i < n; i++) {
            if ((child >> (n - 1 - i)) & 1)
                w->pos[axes[i]] = (s32)((u32)w->pos[axes[i]] - w->size[axes[i]]);
        }
    }
    for (i = 0; i < n; i++)
        w->size[axes[i]] <<= 1;
}

/* Appends to `nodes` the header of the zone (size, depth limits) and the
 * leaves of its collision octree that overlap the query box. `count` is the
 * workspace index of `nodes`; leaves are kept while it is below 0x200.
 * Returns the new count. */
s32 Solid_RZoneQueryOctree(struct ZqZoneRect *rect, struct ZqBound *query, struct SubWsNode *nodes,
                           s32 count)
{
    OctreeWalk w;

    nodes[0].flags = NODE_HEADER;
    nodes[0].x = (u16)rect->w;
    nodes[0].y = (u16)rect->h;
    nodes[0].z = (u16)rect->d;
    nodes[1].flags = NODE_HEADER;
    nodes[1].x = (u16)rect->max_depth_x;
    nodes[1].y = (u16)rect->max_depth_y;
    nodes[1].z = (u16)rect->max_depth_z;

    w.zone = (const u8 *)rect;
    w.max_depth[0] = (u16)rect->max_depth_x;
    w.max_depth[1] = (u16)rect->max_depth_y;
    w.max_depth[2] = (u16)rect->max_depth_z;
    w.size[0] = (u32)rect->w << 8;
    w.size[1] = (u32)rect->h << 8;
    w.size[2] = (u32)rect->d << 8;
    w.pos[0] = (s32)(((u32)rect->x << 8) - (u32)query->p1.x);
    w.pos[1] = (s32)(((u32)rect->y << 8) - (u32)query->p1.y);
    w.pos[2] = (s32)(((u32)rect->z << 8) - (u32)query->p1.z);
    w.extent[0] = (s32)((u32)query->p2.x - (u32)query->p1.x);
    w.extent[1] = (s32)((u32)query->p2.y - (u32)query->p1.y);
    w.extent[2] = (s32)((u32)query->p2.z - (u32)query->p1.z);
    w.out = nodes + 2;
    w.count = count + 2;
    octree_visit(&w, rect->root, 0);
    return w.count;
}

/* Marks the wall cell (x, z) of the grid, centred on the object, unless it
 * is already marked: ORs the wall shape, shifted by x, into the grid rows
 * from z on. Both coordinates are within -31..31. */
static void plot_wall(s32 x, s32 z)
{
    u32 *seen = &SCRATCH->plotted[z + 32][x >= 0];
    u32 bit = x < 0 ? 1u << (x + 32) : 1u << x;
    u32 *row;
    s32 i, end;

    if (*seen & bit)
        return;
    *seen |= bit;
    if (z >= 0) {
        i = 0;
        end = 32 - z;
    } else {
        i = -z;
        end = 32;
    }
    row = &SCRATCH->rows[z >= 0 ? z : 0];
    for (; i < end; i++, row++) {
        if (x < 0)
            *row |= SCRATCH->wall_shape[i] << -x;
        else
            *row |= SCRATCH->wall_shape[i] >> x;
    }
}

/* World coordinate to wall grid cell around `centre` (2048 units a cell). */
static s32 grid_cell(s32 v, s32 centre)
{
    return (s32)(((u32)v - (u32)centre) << 2) >> 13;
}

/* Plots every eighth cell along the outline of the cell box (x1, z1)-(x2,
 * z2), within the grid, walking each edge from one corner towards the next. */
static void plot_outline(s32 x1, s32 z1, s32 x2, s32 z2)
{
    s32 x, z;

    if (-32 < z1 && z1 < 32) {
        for (x = x1;; x += 8) {
            if (x <= -32)
                continue;
            if (x >= 32 || x >= x2)
                break;
            plot_wall(x, z1);
        }
    }
    if (-32 < x2 && x2 < 32) {
        for (z = z1;; z += 8) {
            if (z <= -32)
                continue;
            if (z >= 32 || z >= z2)
                break;
            plot_wall(x2, z);
        }
    }
    if (-32 < z2 && z2 < 32) {
        for (x = x2;; x -= 8) {
            if (x >= 32)
                continue;
            if (x <= -32 || x <= x1)
                break;
            plot_wall(x, z2);
        }
    }
    if (-32 < x1 && x1 < 32) {
        for (z = z2;; z -= 8) {
            if (z >= 32)
                continue;
            if (z <= -32 || z <= z1)
                break;
            plot_wall(x1, z);
        }
    }
}

/* Plots the outline of every wall leaf of the workspace into the wall grid
 * around (x, z). Types 3 and 4 are never walls. A type 1 leaf is a wall when
 * it spans past y_min; any other one when it spans past y_max, and leaves
 * whose kind (value bits 4..9) is 1..0x26 only when `mode` is set. In every
 * case the leaf must start below y_top. */
void Solid_RPlotQueryWalls(struct SubWorkspace *ws, struct ZqBound *query, s32 mode, s32 y_min,
                           s32 y_max, s32 y_top, s32 x, s32 z)
{
    const SubWsNode *node = ws->nodes;
    ZoneDims dims = {{0}, {0}};
    s32 lo[3], hi[3];

    while (node->flags != NODE_END) {
        u32 type, kind;

        if (node->flags == NODE_HEADER) {
            read_zone_header(&dims, node);
            node += 2;
            continue;
        }
        node_box(node, &query->p1, &dims, lo, hi);
        type = node_type(node);
        kind = (node_value(node) & 0x3F0) >> 4;
        if (type == 3 || type == 4) {
            node++;
            continue;
        }
        if (type == 1) {
            if (!(y_min < hi[1] && lo[1] < y_top)) {
                node++;
                continue;
            }
        } else if ((kind != 0 && kind < 0x27 && mode == 0) || !(y_max < hi[1] && lo[1] < y_top)) {
            node++;
            continue;
        }
        SCRATCH->keep.plot.mode = mode;
        SCRATCH->keep.plot.y_min = y_min;
        plot_outline(grid_cell(lo[0], x), grid_cell(lo[2], z), grid_cell(hi[0], x),
            grid_cell(hi[2], z));
        node++;
    }
}

/* Averages the tops of the leaves overlapping `bound` that are not above
 * max_y: out[1] for floors (type 0), out[2] for the others, `init` where
 * there are none. Leaves of type 3 and 4 and those whose kind (value bits
 * 4..9) is 1..0x26 are first offered to node_fn(obj, value), and count only
 * when it returns 0. */
void Solid_RFindFloorY(struct SubObj *obj, struct SubWorkspace *ws, struct ZqBound *query,
                       struct ZqBound *bound, s32 max_y, s32 *out, s32 init,
                       s32 (*node_fn)(struct SubObj *, u32))
{
    const SubWsNode *node = ws->nodes;
    ZoneDims dims = {{0}, {0}};
    ZqVec origin = query->p1;
    ZqBound box = *bound;
    FloorScan scan;
    s32 wall_sum = 0;
    s32 lo[3];

    SCRATCH->keep.floor.obj = Ram_Addr(obj);
    SCRATCH->keep.floor.init = init;
    SCRATCH->keep.floor.node_fn = Ram_FnAddr((const void *)node_fn);
    SCRATCH->keep.floor.out = Ram_Addr(out);
    scan.max_y = max_y;
    scan.floor_count = scan.floor_sum = scan.wall_count = 0;
    scan.bound_x1 = box.p1.x;
    scan.bound_y1 = box.p1.y;
    for (; node->flags != NODE_END; node++) {
        u32 kind;

        if (node->flags == NODE_HEADER) {
            read_zone_header(&dims, node);
            node++;
            continue;
        }
        node_box(node, &origin, &dims, lo, scan.hi);
        if (box.p2.x < lo[0] || box.p2.y < lo[1] || box.p2.z < lo[2])
            continue;
        if (scan.hi[0] < scan.bound_x1 || scan.hi[1] < scan.bound_y1 || scan.hi[2] < box.p1.z)
            continue;
        scan.value = node_value(node);
        scan.type2 = node_type(node) << 1;
        kind = (scan.value & 0x3F0) >> 4;
        if (scan.type2 == 3 << 1 || scan.type2 == 4 << 1 || (kind != 0 && kind < 0x27)) {
            s32 hit;

            /* The callback may run code that reuses this part of the
             * scratchpad; the scan goes on with what is there afterwards. */
            SCRATCH->keep.floor.wall_sum = wall_sum;
            scan.node = Ram_Addr(node);
            SCRATCH->floor_scan = scan;
            hit = RAM_FN(s32 (*)(struct SubObj *, u32), SCRATCH->keep.floor.node_fn)(
                RAM_AT(struct SubObj, SCRATCH->keep.floor.obj), scan.value);
            scan = SCRATCH->floor_scan;
            node = RAM_AT(const SubWsNode, scan.node);
            wall_sum = SCRATCH->keep.floor.wall_sum;
            /* The original keeps the origin's z in ra, which the call
             * overwrites with its return address. From here on leaves are
             * placed against that, which in practice rejects them all. */
            origin.z = (s32)FLOOR_CALLBACK_RA;
            if (hit != 0)
                continue;
        }
        if (scan.max_y < scan.hi[1])
            continue;
        if (scan.type2 == 0) {
            scan.floor_count++;
            scan.floor_sum = (s32)((u32)scan.floor_sum + (u32)scan.hi[1]);
        } else {
            scan.wall_count++;
            wall_sum = (s32)((u32)wall_sum + (u32)scan.hi[1]);
        }
    }
    out = RAM_AT(s32, SCRATCH->keep.floor.out);
    init = SCRATCH->keep.floor.init;
    out[1] = scan.floor_count != 0 ? scan.floor_sum / scan.floor_count : init;
    out[2] = scan.wall_count != 0 ? wall_sum / scan.wall_count : init;
}

/* Averages the bottoms of the leaves of type type_a - 1 or type_b - 1 that
 * overlap `bound`; returns `init` where there are none. */
s32 Solid_RFindCeilY(struct SubObj *obj, struct SubWorkspace *ws, struct ZqBound *query,
                     struct ZqBound *bound, s32 type_a, s32 type_b, s32 init)
{
    const SubWsNode *node = ws->nodes;
    ZoneDims dims = {{0}, {0}};
    u32 match_a = (u32)(type_a - 1) << 1, match_b = (u32)(type_b - 1) << 1;
    s32 sum = 0, count = 0;
    s32 lo[3], hi[3];

    (void)obj;
    for (; node->flags != NODE_END; node++) {
        u32 type2;

        if (node->flags == NODE_HEADER) {
            read_zone_header(&dims, node);
            node++;
            continue;
        }
        type2 = node_type(node) << 1;
        if (type2 != match_a && type2 != match_b)
            continue;
        node_box(node, &query->p1, &dims, lo, hi);
        if (bound->p2.x < lo[0] || bound->p2.y < lo[1] || bound->p2.z < lo[2])
            continue;
        if (hi[0] < bound->p1.x || hi[1] < bound->p1.y || hi[2] < bound->p1.z)
            continue;
        count++;
        sum = (s32)((u32)sum + (u32)lo[1]);
    }
    return count != 0 ? sum / count : init;
}
