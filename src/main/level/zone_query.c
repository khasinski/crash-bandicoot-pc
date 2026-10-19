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

/* Zone dimensions item: bounds, then the octree header. */
typedef struct ZqZoneRect {
    s32 x, y, z;
    s32 w, h, d;
    u32 word_18;
    u16 root;
    s16 max_depth_x;
    s16 max_depth_y;
    s16 max_depth_z;
} ZqZoneRect;

/* Whether v lies inside the zone's 8-bit shifted bounding box. */
static __inline__ s32 zone_rect_contains(ZqZoneRect *rect, ZqVec *v)
{
    return v->x >= (rect->x << 8) && v->y >= (rect->y << 8) && v->z >= (rect->z << 8)
        && v->x < ((rect->x + rect->w) << 8) && v->y < ((rect->y + rect->h) << 8)
        && v->z < ((rect->z + rect->d) << 8);
}

/* Whether point v lies inside the bounding box [b->p1, b->p2). */
s32 Level_TestPointInBound(ZqVec *v, ZqBound *b)
{
    if (v->x >= b->p1.x && v->y >= b->p1.y && v->z >= b->p1.z
        && v->x < b->p2.x && v->y < b->p2.y && v->z < b->p2.z) {
        return 1;
    }
    return 0;
}

/* Whether bounding box b overlaps the 8-bit shifted zone rectangle r. */
s32 Level_TestRectIntersectsBound(ZqBound *b, ZqRect *r)
{
    if ((r->x << 8) < b->p2.x && (r->y << 8) < b->p2.y && (r->z << 8) < b->p2.z
        && ((r->x + r->w) << 8) >= b->p1.x && ((r->y + r->h) << 8) >= b->p1.y
        && ((r->z + r->d) << 8) >= b->p1.z) {
        return 1;
    }
    return 0;
}

/* Whether bounding boxes a and b overlap. */
s32 Level_TestBoundIntersection(ZqBound *a, ZqBound *b)
{
    if (a->p1.y <= b->p2.y && a->p2.y > b->p1.y && a->p2.x > b->p1.x
        && a->p2.z > b->p1.z && a->p1.x <= b->p2.x && a->p1.z <= b->p2.z) {
        return 1;
    }
    return 0;
}

/* Whether bounding box a is contained in bounding box b. */
s32 Level_TestBoundInBound(ZqBound *a, ZqBound *b)
{
    if (a->p1.x >= b->p1.x && a->p1.y >= b->p1.y && a->p1.z >= b->p1.z
        && a->p2.x < b->p2.x && a->p2.y < b->p2.y && a->p2.z < b->p2.z) {
        return 1;
    }
    return 0;
}

/* ZoneFindNeighbor: find the neighbour of zone (or of the current level zone)
 * whose bounding box contains v, or return -0x17 if none does. */
void * Zone_FindNeighbor(ZoneEntry *zone, ZqVec *v)
{
    ZoneHeader *header;
    ZoneEntry *neighbor;
    s32 i;

    header = (ZoneHeader *)zone->items[0];
    for (i = header->neighbor_count - 1; i >= 0; i--) {
        neighbor = Ns_Lookup(&header->neighbors[i]);
        if (zone_rect_contains((ZqZoneRect *)neighbor->items[1], v)) {
            return neighbor;
        }
    }
    if (zone != (ZoneEntry *)level_position.zone) {
        header = (ZoneHeader *)((ZoneEntry *)level_position.zone)->items[0];
        for (i = header->neighbor_count - 1; i >= 0; i--) {
            neighbor = Ns_Lookup(&header->neighbors[i]);
            if (zone_rect_contains((ZqZoneRect *)neighbor->items[1], v)) {
                return neighbor;
            }
        }
    }
    return (ZoneEntry *)-0x17;
}

/* ZoneFindNode: follow the ray va + t*vb through the zone octree below node,
 * which covers rect. A leaf returns its node and moves va onto the nearest
 * face it crossed (reversing that component of vb); an empty node advances
 * va just past its far side. Recurses into the children that contain va. */
s32 Zone_FindNode(ZqZoneRect *zone, u16 node, ZqRect *rect, ZqVec *va, ZqVec *vb, s32 level)
{
    u16 *children;
    s32 t, tn, axis, i, j, k, idx, res;

    /* Non-leaf nodes are offsets of their child arrays in the zone item;
     * leaves (odd) and empty nodes (0) are returned as they are. */
    if (!(node & 1) && node) {
        children = (u16 *)((u32)zone + node);
    } else {
        children = (u16 *)(u32)node;
    }
    if ((node & 1) || !node) {
        ZqVec corner;

        t = 0x7FFFFFFF;
        if (!children) {
            if (vb->x > 0) {
                corner.x = rect->x + rect->w;
            } else {
                corner.x = rect->x;
            }
            if (vb->y > 0) {
                corner.y = rect->y + rect->h;
            } else {
                corner.y = rect->y;
            }
            if (vb->z > 0) {
                corner.z = rect->z + rect->d;
            } else {
                corner.z = rect->z;
            }
            if (vb->x) {
                tn = ((corner.x - va->x) << 8) / vb->x;
                if (tn < t) {
                    t = tn;
                }
            }
            if (vb->y) {
                tn = ((corner.y - va->y) << 8) / vb->y;
                if (tn < t) {
                    t = tn;
                }
            }
            if (vb->z) {
                tn = ((corner.z - va->z) << 8) / vb->z;
                if (tn < t) {
                    t = tn;
                }
            }
            t += 0x100;
            va->x += (t * vb->x) >> 8;
            va->y += (t * vb->y) >> 8;
            va->z += (t * vb->z) >> 8;
        } else {
            if (vb->x < 0) {
                corner.x = rect->x + rect->w;
            } else {
                corner.x = rect->x;
            }
            if (vb->y < 0) {
                corner.y = rect->y + rect->h;
            } else {
                corner.y = rect->y;
            }
            if (vb->z < 0) {
                corner.z = rect->z + rect->d;
            } else {
                corner.z = rect->z;
            }
            t = 0x80000000;
            if (vb->x) {
                tn = ((corner.x - (va->x - vb->x)) << 8) / vb->x;
                if (tn > t) {
                    t = tn;
                    axis = 0;
                }
            }
            if (vb->z) {
                tn = ((corner.z - (va->z - vb->z)) << 8) / vb->z;
                if (tn > t || __builtin_abs(vb->x) < __builtin_abs(vb->z)) {
                    t = tn;
                    axis = 2;
                }
            }
            if (vb->y) {
                tn = ((corner.y - (va->y - vb->y)) << 8) / vb->y;
                if (tn > t || (axis == 0 && __builtin_abs(vb->x) < __builtin_abs(vb->y))
                    || (axis == 2 && __builtin_abs(vb->z) < __builtin_abs(vb->y))) {
                    t = tn;
                    axis = 1;
                }
            }
            switch (axis) {
            case 0:
                vb->x = -vb->x;
                va->x = corner.x;
                break;
            case 1:
                vb->y = -vb->y;
                va->y = corner.y;
                break;
            case 2:
                vb->z = -vb->z;
                va->z = corner.z;
                break;
            }
        }
        return (s32)children;
    } else {
        s32 split[3];
        ZqRect child;

        split[0] = level < zone->max_depth_x;
        if (split[0]) {
            child.w = rect->w / 2;
        } else {
            child.w = rect->w;
        }
        split[1] = level < zone->max_depth_y;
        if (split[1]) {
            child.h = rect->h / 2;
        } else {
            child.h = rect->h;
        }
        split[2] = level < zone->max_depth_z;
        if (split[2]) {
            child.d = rect->d / 2;
        } else {
            child.d = rect->d;
        }
        idx = 0;
        for (i = 0; i < (split[0] ? 2 : 1); i++) {
            for (j = 0; j < (split[1] ? 2 : 1); j++) {
                for (k = 0; k < (split[2] ? 2 : 1);) {
                    child.x = rect->x + (i ? child.w : 0);
                    child.y = rect->y + (j ? child.h : 0);
                    child.z = rect->z + (k ? child.d : 0);
                    k++;
                    if (child.x < va->x && child.y < va->y && child.z < va->z
                        && child.x + child.w >= va->x && child.y + child.h >= va->y
                        && child.z + child.d >= va->z) {
                        res = Zone_FindNode(zone, children[idx], &child, va, vb, level + 1);
                        if (res) {
                            return res;
                        }
                    }
                    idx++;
                }
            }
        }
        /* No return here: retail leaves whatever is in v0 (0 on every
         * path that gets here). */
    }
}

/* Step va along vb across the current zone's neighbours until ZoneFindNode
 * hits a leaf (returned) or va leaves every neighbour's bounds (returns 1). */
s32 Zone_ReboundVector(ZqVec *va, ZqVec *vb)
{
    ZqVec copy;
    ZqRect rect;
    ZoneHeader *header;
    ZoneEntry *neighbor;
    ZqZoneRect *zone_rect;
    s32 i, res;

    header = (ZoneHeader *)((ZoneEntry *)level_position.zone)->items[0];
    if (vb->x == 0 && vb->y == 0 && vb->z == 0) {
        return 0;
    }
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
                res = Zone_FindNode(zone_rect, zone_rect->root, &rect, va, vb, 0);
                break;
            }
        }
        if (i == header->neighbor_count) {
            return 1;
        }
    } while (!res);
    return res;
}
