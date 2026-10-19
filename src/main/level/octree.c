#include "common.h"
#include "zone.h"

typedef struct ZqVec {
    s32 x, y, z;
} ZqVec;

typedef struct ZqBound {
    ZqVec p1;
    ZqVec p2;
} ZqBound;

typedef struct ZqRect {
    s32 x, y, z;
    s32 w, h, d;
} ZqRect;

typedef struct ZqZoneRect {
    s32 x, y, z;
    s32 w, h, d;
    u32 word_18;
    u16 root;
    s16 max_depth_x;
    s16 max_depth_y;
    s16 max_depth_z;
} ZqZoneRect;

typedef struct LevelStateView {
    u8 reserved_00[4];
    s32 level_id;
} LevelStateView;

typedef struct SubLight {
    s16 m1[3][3];
    s16 v1[3];
    s16 m2[3][3];
    s16 v2[3];
} SubLight;

typedef struct SubObj {
    u8 reserved_00[8];
    ZqBound bound;
    void *word_20;
    u8 reserved_24[4];
    ResourceEntry *entry;
    u8 reserved_2c[4];
    SubLight light;
    u8 reserved_60[4];
    struct SubObj *parent;
    u8 reserved_68[0x78 - 0x68];
    struct SubObj *collider;
    u8 reserved_7c[4];
    ZqVec trans;
    u8 reserved_8c[0xA8 - 0x8C];
    s32 vel_y;
    u8 reserved_ac[0xC8 - 0xAC];
    u32 status_a;
    u32 status_b;
    u32 status_c;
    u8 reserved_d4[0xFC - 0xD4];
    u32 arg1;
    u8 reserved_100[0x110 - 0x100];
    u16 *word_110;
    u8 reserved_114[0x120 - 0x114];
    u32 flags120;
    u8 reserved_124[4];
    s32 anim_state;
    u8 reserved_12c[4];
    u32 ground_time;
    s32 impact_vel_y;
    s32 offset138;
    s32 event;
    u8 reserved_140[0x148 - 0x140];
    s32 head_turn;
    u8 reserved_14c[0x158 - 0x14C];
    s32 word_158;
} SubObj;

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

#define CRASH_EXT_LEVEL_OCTREE_0
#include "level_externs.h"
#undef CRASH_EXT_LEVEL_OCTREE_0

typedef struct SubBox {
    ZqBound bound;
    SubObj *obj;
} SubBox;

typedef struct SubBoxList {
    SubBox boxes[96];
    s32 count;
} SubBoxList;

#define CRASH_EXT_LEVEL_OCTREE_1
#include "level_externs.h"
#undef CRASH_EXT_LEVEL_OCTREE_1

static __inline__ s32 zone_rect_overlaps(ZqBound *b, ZqRect *r)
{
    if ((r->x << 8) < b->p2.x && (r->y << 8) < b->p2.y && (r->z << 8) < b->p2.z
        && ((r->x + r->w) << 8) >= b->p1.x && ((r->y + r->h) << 8) >= b->p1.y
        && ((r->z + r->d) << 8) >= b->p1.z) {
        return 1;
    }
    return 0;
}

static __inline__ s32 Level_TestBoundIntersection_inline(ZqBound *a, ZqBound *b)
{
    if (a->p1.y <= b->p2.y && a->p2.y > b->p1.y && a->p2.x > b->p1.x
        && a->p2.z > b->p1.z && a->p1.x <= b->p2.x && a->p1.z <= b->p2.z) {
        return 1;
    }
    return 0;
}

static __inline__ s32 Level_TestBoundInBound(ZqBound *a, ZqBound *b)
{
    if (a->p1.x >= b->p1.x && a->p1.y >= b->p1.y && a->p1.z >= b->p1.z
        && a->p2.x < b->p2.x && a->p2.y < b->p2.y && a->p2.z < b->p2.z) {
        return 1;
    }
    return 0;
}

static __inline__ void Solid_PlotWallB(s32 x1, s32 y1, s32 x2, s32 y2, s32 set)
{
    s32 *p;
    u32 mask;

    x2 += 16;
    mask = -1;
    if (x2 < 32) {
        if (x2 <= 0) {
            return;
        }
        mask = -1 << (32 - x2);
    }
    x1 += 15;
    if (x1 >= 32) {
        return;
    }
    if (x1 >= 0) {
        mask &= ~((s32)0x80000000 >> x1);
    }
    if (mask == 0) {
        return;
    }
    y1 += 16;
    y2 += 16;
    if (y1 < 0) {
        y1 = 0;
    }
    if (y2 > 32) {
        y2 = 32;
    }
    if (y1 >= 32) {
        return;
    }
    p = &gp_grid_rows[y1];
    if (set) {
        for (; y1 < y2; y1++, p++) {
            *p |= mask;
        }
    } else {
        mask = ~mask;
        for (; y1 < y2; y1++, p++) {
            *p &= mask;
        }
    }
}

static __inline__ void clamp_light(s16 *dst_f, s16 *src_f, s32 step)
{
    if (step != 0) {
        if (*src_f - *dst_f > step) {
            *src_f = *dst_f + step;
        } else if (*src_f - *dst_f < -step) {
            *src_f = *dst_f - step;
        }
    }
    *dst_f = *src_f;
}

void Zone_ColorsScaleSeek(void *defaults, SubObj *obj, s32 idx, s32 flag)
{
    SubLight buf;
    SubLight *dst;
    s32 step, i;

    dst = &obj->light;
    if (obj == gp_crash_obj[0] && (obj->flags120 & 0x20)) {
        idx = 0x37;
    }
    Zone_ColorsScaleByNode(idx, defaults, &buf);
    step = flag ? 0x15E : 0;
    clamp_light(&dst->v1[0], &buf.v1[0], step);
    clamp_light(&dst->v1[1], &buf.v1[1], step);
    clamp_light(&dst->v1[2], &buf.v1[2], step);
    clamp_light(&dst->v2[0], &buf.v2[0], step);
    clamp_light(&dst->v2[1], &buf.v2[1], step);
    clamp_light(&dst->v2[2], &buf.v2[2], step);
    for (i = 0; i < 3; i++) {
        clamp_light(&dst->m1[i][0], &buf.m1[i][0], step);
        clamp_light(&dst->m1[i][1], &buf.m1[i][1], step);
        clamp_light(&dst->m1[i][2], &buf.m1[i][2], step);
        clamp_light(&dst->m2[i][0], &buf.m2[i][0], step);
        clamp_light(&dst->m2[i][1], &buf.m2[i][1], step);
        clamp_light(&dst->m2[i][2], &buf.m2[i][2], step);
    }
}

u32 Zone_FindNearestNode(ZqZoneRect *zone, u16 node, ZqRect *rect, ZqVec *va, s32 level, s32 flags)
{
    u16 *children;
    s32 i, j, k, idx;

    if (!(node & 1) && node) {
        children = (u16 *)((u32)zone + node);
    } else {
        children = (u16 *)(u32)node;
    }
    if ((node & 1) || !node) {
        if (children
            && (!((flags & 8) && (children ? ((((u32)children & 0xE) >> 1) + 1) : 0) == 4))
            && (((u32)children & 0x3F0) >> 4) != 0xB
            && (children ? ((((u32)children & 0xE) >> 1) + 1) : 0) != 5) {
            if (flags & 1) {
                va->y = rect->y + rect->h;
            } else if (flags & 2) {
                va->z = rect->z + rect->d;
            }
            return (u32)children;
        } else {
            if (flags & 1) {
                va->y = rect->y - 1;
            } else if (flags & 2) {
                va->z = rect->z - 1;
            }
            return 0;
        }
    } else {
        ZqRect child;
        s32 split_x, split_y, split_z;
        s32 tx, ty, tz;

        child.x = rect->x;
        child.y = rect->y;
        child.z = rect->z;
        tx = 0;
        ty = 0;
        tz = 0;
        split_x = level < zone->max_depth_x;
        if (split_x) {
            child.w = rect->w / 2;
            if (va->x >= child.x + child.w) {
                child.x += child.w;
                tx++;
            }
        } else {
            child.w = rect->w;
        }
        split_y = level < zone->max_depth_y;
        if (split_y) {
            child.h = rect->h / 2;
            if (va->y >= child.y + child.h) {
                child.y += child.h;
                ty++;
            }
        } else {
            child.h = rect->h;
        }
        split_z = level < zone->max_depth_z;
        if (split_z) {
            child.d = rect->d / 2;
            if (va->z >= child.z + child.d) {
                child.z += child.d;
                tz++;
            }
        } else {
            child.d = rect->d;
        }
        idx = 0;
        for (i = 0; i < (split_x ? 2 : 1); i++) {
            for (j = 0; j < (split_y ? 2 : 1); j++) {
                for (k = 0; k < (split_z ? 2 : 1); k++, idx++) {
                    if (i == tx && j == ty && k == tz) {
                        return Zone_FindNearestNode(zone, children[idx], &child, va, ++level, flags);
                    }
                }
            }
        }
    }
}

static __inline__ s32 zone_rect_contains(ZqZoneRect *rect, ZqVec *v)
{
    if (v->x >= (rect->x << 8) && v->y >= (rect->y << 8) && v->z >= (rect->z << 8)
        && v->x < ((rect->x + rect->w) << 8) && v->y < ((rect->y + rect->h) << 8)
        && v->z < ((rect->z + rect->d) << 8)) {
        return 1;
    }
    return 0;
}

void * Zone_FindNearestObjectNode(SubObj *obj, ZqVec *va)
{
    ZqVec orig;
    ZqVec copy;
    ZqRect rect;
    s32 best_y, i, cand_y;
    SubObj *res, *best_obj, *cand_obj;
    ZoneHeader *header;
    ZoneEntry *neighbor;
    ZqZoneRect *zone_rect;
    SubBox *box;

    best_y = -0x3B9AC9FF;
    res = NULL;
    best_obj = NULL;
    header = (ZoneHeader *)((ZoneEntry *)level_position.zone)->items[0];
    orig = *va;
    do {
        copy = *va;
        for (i = 0; i < header->neighbor_count; i++) {
            neighbor = Ns_Lookup(&header->neighbors[i]);
            if (zone_rect_contains((ZqZoneRect *)neighbor->items[1], va)) {
                zone_rect = (ZqZoneRect *)neighbor->items[1];
                rect.x = zone_rect->x << 8;
                rect.y = zone_rect->y << 8;
                rect.z = zone_rect->z << 8;
                rect.w = zone_rect->w << 8;
                rect.h = zone_rect->h << 8;
                rect.d = zone_rect->d << 8;
                res = (SubObj *)Zone_FindNearestNode(zone_rect, zone_rect->root, &rect, va, 0, 9);
                break;
            }
        }
        if (i == header->neighbor_count) {
            res = NULL;
            break;
        }
    } while (!res);
    if (g_obj_bounds.count > 0) {
        box = g_obj_bounds.boxes;
        for (i = 0; i < g_obj_bounds.count; i++, box++) {
            cand_obj = box->obj;
            if (cand_obj->status_b & 0x20000) {
                cand_y = box->bound.p2.y;
                if (orig.x >= box->bound.p1.x - 0x4E20
                    && orig.x <= box->bound.p2.x + 0x4E20
                    && orig.z >= box->bound.p1.z - 0x4E20
                    && orig.z <= box->bound.p2.z + 0x4E20) {
                    if (va->y <= cand_y && va->y >= box->bound.p1.y) {
                        va->y = cand_y;
                        res = cand_obj;
                        goto done;
                    }
                    if (cand_y > best_y && cand_y <= orig.y) {
                        best_obj = cand_obj;
                        best_y = cand_y;
                    }
                }
            }
        }
        if (best_obj != NULL && (res == NULL || best_y >= va->y)) {
            va->y = best_y;
            res = best_obj;
        }
    }
done:
    return res;
}

static __inline__ s32 get_offset_base(SubObj *obj)
{
    return !(obj->parent->flags120 & 0x10) ? 0x18 : 0;
}

u32 Zone_FindNearestObjectNode2(SubObj *obj, ZqVec *va)
{
    ZqVec orig;
    ZqVec copy;
    ZqRect rect;
    s32 best_y, i, cand_y, hit_box, offset, dy, idx;
    u32 res;
    SubObj *best_obj, *cand_obj;
    ZoneHeader *header;
    ZoneEntry *neighbor;
    ZqZoneRect *zone_rect;
    SubBox *box;

    best_y = -0x3B9AC9FF;
    res = 0;
    best_obj = NULL;
    header = (ZoneHeader *)((ZoneEntry *)level_position.zone)->items[0];
    hit_box = 0;
    if (!(obj->status_b & 0x4000000)) {
        return 0;
    found_y:
        va->y = cand_y;
        res = (u32)cand_obj;
        goto set_hit_box;
    }
    orig = *va;
    do {
        copy = *va;
        for (i = 0; i < header->neighbor_count; i++) {
            neighbor = Ns_Lookup(&header->neighbors[i]);
            if (zone_rect_contains((ZqZoneRect *)neighbor->items[1], va)) {
                zone_rect = (ZqZoneRect *)neighbor->items[1];
                rect.x = zone_rect->x << 8;
                rect.y = zone_rect->y << 8;
                rect.z = zone_rect->z << 8;
                rect.w = zone_rect->w << 8;
                rect.h = zone_rect->h << 8;
                rect.d = zone_rect->d << 8;
                res = Zone_FindNearestNode(zone_rect, zone_rect->root, &rect, va, 0, 1);
                break;
            }
        }
        if (i == header->neighbor_count) {
            res = 0;
            break;
        }
    } while (!res);
    if (g_obj_bounds.count > 0) {
        box = g_obj_bounds.boxes;
        for (i = 0; i < g_obj_bounds.count; i++, box++) {
            cand_obj = box->obj;
            if ((cand_obj->status_b & 0x40020000) == 0x20000) {
                cand_y = box->bound.p2.y;
                if (orig.x >= box->bound.p1.x - 0x88B8
                    && orig.x <= box->bound.p2.x + 0x88B8
                    && orig.z >= box->bound.p1.z - 0x88B8
                    && orig.z <= box->bound.p2.z + 0x88B8) {
                    if (va->y <= cand_y && va->y >= box->bound.p1.y) {
                        goto found_y;
                    }
                    if (cand_y > best_y && cand_y <= orig.y) {
                        best_obj = cand_obj;
                        best_y = cand_y;
                    }
                }
            }
        }
        if (best_obj != NULL && (res == 0 || best_y >= va->y)) {
            va->y = best_y;
            res = (u32)best_obj;
        set_hit_box:
            hit_box = 1;
        }
    }
    if (res != 0) {
        if (hit_box) {
            offset = ((SubObj *)res)->offset138;
            dy = obj->trans.y - g_cam_trans_y;
            if (dy > 0) {
                offset -= dy >> 12;
            }
            obj->parent->offset138 = get_offset_base(obj) + offset;
        } else {
            idx = (res & 0x3C00) >> 10;
            obj->parent->offset138 = get_offset_base(obj) + (idx >= 0 ? g_cam_zoom_presets[idx] : -1);
        }
    } else {
        obj->parent->offset138 = get_offset_base(obj) + g_cam_zoom_presets[0];
    }
    return res;
}

u32 Zone_FindNearestObjectNode3(SubObj *obj, ZqVec *in_va, s32 flags, s32 arg3)
{
    ZqVec local_va;
    ZqVec orig;
    ZqVec copy;
    ZqRect rect;
    ZqVec *va;
    s32 best_v, hit_box, i, cand_v, idx;
    u32 res;
    SubObj *best_obj, *cand_obj;
    ZoneHeader *header;
    ZoneEntry *neighbor;
    ZqZoneRect *zone_rect;
    SubBox *box;
    void *defaults;

    best_v = -0x3B9AC9FF;
    res = 0;
    best_obj = NULL;
    hit_box = 0;
    header = (ZoneHeader *)((ZoneEntry *)level_position.zone)->items[0];
    va = &local_va;
    if (!(obj->status_b & 0x4000000) || ((flags & 4) && !(obj->parent->status_b & 0x4000000))) {
        return 0;
    }
    *va = *in_va;
    va->y += 0x61A8;
    orig = *va;
    do {
        copy = *va;
        for (i = 0; i < header->neighbor_count; i++) {
            neighbor = Ns_Lookup(&header->neighbors[i]);
            if (zone_rect_contains((ZqZoneRect *)neighbor->items[1], va)) {
                zone_rect = (ZqZoneRect *)neighbor->items[1];
                rect.x = zone_rect->x << 8;
                rect.y = zone_rect->y << 8;
                rect.z = zone_rect->z << 8;
                rect.w = zone_rect->w << 8;
                rect.h = zone_rect->h << 8;
                rect.d = zone_rect->d << 8;
                res = Zone_FindNearestNode(zone_rect, zone_rect->root, &rect, va, 0, flags & 3);
                break;
            }
        }
        if (i == header->neighbor_count) {
            res = 0;
            break;
        }
    } while (!res);
    if (g_obj_bounds.count > 0) {
        box = g_obj_bounds.boxes;
        for (i = 0; i < g_obj_bounds.count; i++, box++) {
            cand_obj = box->obj;
            if (cand_obj != obj && cand_obj->word_158 != 0xFFFF) {
                if (flags & 1) {
                    cand_v = box->bound.p2.y;
                    if (orig.x >= box->bound.p1.x && orig.x <= box->bound.p2.x
                        && orig.z >= box->bound.p1.z && orig.z <= box->bound.p2.z) {
                        if (va->y <= cand_v && va->y >= box->bound.p1.y) {
                            va->y = box->bound.p1.y;
                            res = (u32)cand_obj;
                            hit_box = 1;
                            goto done_boxes;
                        }
                        if (cand_v > best_v && cand_v <= orig.y) {
                            best_obj = cand_obj;
                            best_v = cand_v;
                        }
                    }
                } else if (flags & 2) {
                    cand_v = box->bound.p2.z;
                    if (orig.x >= box->bound.p1.x && orig.x <= box->bound.p2.x
                        && orig.y >= box->bound.p1.y && orig.y <= box->bound.p2.y) {
                        if (va->z <= cand_v && va->z >= box->bound.p1.z) {
                            va->z = box->bound.p1.z;
                            res = (u32)cand_obj;
                            hit_box = 1;
                            goto done_boxes;
                        }
                        if (cand_v > best_v && cand_v <= orig.z) {
                            best_obj = cand_obj;
                            best_v = cand_v;
                        }
                    }
                }
            }
        }
        if (best_obj != NULL) {
            if (flags & 1) {
                if (res == 0 || best_v >= va->y) {
                    va->y = best_v;
                    res = (u32)best_obj;
                    hit_box = 1;
                }
            } else if (flags & 2) {
                if (res == 0 || best_v >= va->z) {
                    va->z = best_v;
                    res = (u32)best_obj;
                    hit_box = 1;
                }
            }
        }
    }
done_boxes:
    if (flags & 4) {
        if (obj == gp_crash_obj[0] || obj->parent == gp_crash_obj[0]) {
            defaults = obj->parent->entry->data + 0x348;
        } else {
            defaults = obj->parent->entry->data + 0x318;
        }
    } else {
        if (obj == gp_crash_obj[0] || obj->parent == gp_crash_obj[0]) {
            defaults = obj->entry->data + 0x348;
        } else {
            defaults = obj->entry->data + 0x318;
        }
    }
    idx = -1;
    if (res != 0) {
        if (hit_box) {
            idx = ((SubObj *)res)->word_158;
            if (idx < 0) {
                idx = -idx;
                arg3 = 0;
            }
        } else {
            idx = (res & 0x3F0) >> 4;
            if (idx < 0x27) {
                idx = -1;
            }
        }
    }
    if (flags & 4) {
        Zone_ColorsScaleSeek(defaults, obj->parent, idx, arg3);
    }
    return res;
}

void Zone_ColorsScaleSeekByEntityNode(SubObj *obj)
{
    void *defaults;
    u16 *ptr;
    u32 val;
    s32 idx;

    if (obj == gp_crash_obj[0] || obj->parent == gp_crash_obj[0]) {
        defaults = obj->entry->data + 0x348;
    } else {
        defaults = obj->entry->data + 0x318;
    }
    ptr = obj->word_110;
    if (ptr != NULL) {
        val = ptr[2] >> 3;
        if (val != 0xFFFF) {
            idx = (val & 0x3F0) >> 4;
            if (idx < 0x27) {
                idx = -1;
            }
            Zone_ColorsScaleSeek(defaults, obj, idx, 0);
        }
    }
}

s32 Zone_QueryOctrees(ZqVec *v, SubObj *obj, SubWorkspace *ws)
{
    u8 unused[0x18];
    ZoneHeader *header;
    ZqZoneRect *rect;
    s32 i;

    ws->count = 0;
    ws->initialized = 1;
    ws->bound1.p1.x = v->x - 0x12C00;
    ws->bound1.p1.y = v->y - 0x10B80;
    ws->bound1.p1.z = v->z - 0x12C00;
    ws->bound1.p2.x = v->x + 0x12C00;
    ws->bound1.p2.y = v->y + 0x3A480;
    ws->bound1.p2.z = v->z + 0x12C00;
    header = (ZoneHeader *)((ZoneEntry *)level_position.zone)->items[0];
    for (i = 0; i < header->neighbor_count; i++) {
        rect = (ZqZoneRect *)Ns_Lookup(&header->neighbors[i])->items[1];
        if (zone_rect_overlaps(&ws->bound1, (ZqRect *)rect)) {
            ws->count = Solid_RZoneQueryOctree(rect, &ws->bound1, &ws->nodes[ws->count], ws->count);
        }
    }
    ws->nodes[ws->count].flags = 0xFFFF;
    return ws->count;
}
