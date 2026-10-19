#include "common.h"
#include "zone.h"

typedef struct CamSample {
    s32 x, y, z;
    s32 rx, ry, rz;
    s32 sx, sy, sz;
} CamSample;

typedef struct ZoneBase {
    s32 x, y, z;
} ZoneBase;

#define PATH_POINT(p, i) (((ZonePath *)((i) * sizeof(ZonePoint) + (u32)(p)))->points[0])

u16 Zone_PathProgressToLoc(ZonePath *path, s32 progress, CamSample *out)
{
    CamSample next;
    s32 pt;
    s32 frac;
    s32 use_same_path;
    ZoneBase *base;
    ZoneBase *n_base;
    u32 i;
    ZoneEntry *n_zone;
    ZonePath *n_path;
    s32 n_pt;

    progress = __builtin_abs(progress);
    pt = progress >> 8;
    base = (ZoneBase *)((ZoneEntry *)path->zone)->items[1];
    out->x = (base->x << 8) + (PATH_POINT(path, pt).x << 8);
    out->y = (base->y << 8) + (PATH_POINT(path, pt).y << 8);
    out->z = (base->z << 8) + (PATH_POINT(path, pt).z << 8);
    out->rx = PATH_POINT(path, pt).rot_x;
    out->ry = PATH_POINT(path, pt).rot_y;
    use_same_path = 1;
    out->rz = PATH_POINT(path, pt).rot_z;
    frac = progress & 0xFF;
    if (pt == path->length - 1 && frac != 0) {
        for (i = 0; i < path->neighbor_count; i++) {
            if (path->neighbors[i].relation & 2) {
                n_zone = Ns_Lookup(&((ZoneHeader *)((ZoneEntry *)path->zone)->items[0])->neighbors[path->neighbors[i].zone]);
                n_path = (ZonePath *)n_zone->items[path->neighbors[i].path + ((ZoneHeader *)n_zone->items[0])->path_index];
                n_base = (ZoneBase *)n_zone->items[1];
                if (n_path->cam_mode != 1) {
                    n_pt = 0;
                    if (path->neighbors[i].goal & 2) {
                        n_pt = n_path->length - 1;
                    }
                    next.x = (n_base->x << 8) + (PATH_POINT(n_path, n_pt).x << 8);
                    next.y = (n_base->y << 8) + (PATH_POINT(n_path, n_pt).y << 8);
                    next.z = (n_base->z << 8) + (PATH_POINT(n_path, n_pt).z << 8);
                    next.rx = PATH_POINT(n_path, n_pt).rot_x;
                    next.ry = PATH_POINT(n_path, n_pt).rot_y;
                    use_same_path = 0;
                    next.rz = PATH_POINT(n_path, n_pt).rot_z;
                }
                break;
            }
        }
    }
    n_pt = pt + 1;
    if (use_same_path) {
        if (n_pt > path->length - 1) {
            n_pt = path->length - 1;
        }
        next.x = (base->x << 8) + (PATH_POINT(path, n_pt).x << 8);
        next.y = (base->y << 8) + (PATH_POINT(path, n_pt).y << 8);
        next.z = (base->z << 8) + (PATH_POINT(path, n_pt).z << 8);
        next.rx = PATH_POINT(path, n_pt).rot_x;
        next.ry = PATH_POINT(path, n_pt).rot_y;
        next.rz = PATH_POINT(path, n_pt).rot_z;
    }
    out->x += ((next.x - out->x) * frac) >> 8;
    out->y += ((next.y - out->y) * frac) >> 8;
    out->z += ((next.z - out->z) * frac) >> 8;
    out->rx += ((next.rx - out->rx) * frac) >> 8;
    out->ry += (Gool_AngDiff(out->ry, next.ry) * frac) >> 8;
    out->rz += ((next.rz - out->rz) * frac) >> 8;
    return path->length;
}
