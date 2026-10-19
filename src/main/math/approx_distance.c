#include "vector.h"

s32 Math_ApxDist(Vec3 *a, Vec3 *b) {
    Vec3 delta;
    s32 az;
    s32 bz;
    s32 bx;
    s32 by;
    s32 ax;
    s32 ay;
    s32 dy;
    s32 dx;
    s32 sum;
    s32 dz;

    ax = a->x;
    bx = b->x;
    dx = ax - bx;
    if (dx < 0) {
        dx = bx - ax;
    }
    delta.x = dx;
    ay = a->y;
    by = b->y;
    dy = ay - by;
    if (dy < 0) {
        dy = by - ay;
    }
    delta.y = dy;
    az = a->z;
    bz = b->z;
    dz = az - bz;
    if (dz < 0) {
        dz = bz - az;
    }
    delta.z = dz;
    if ((dx >= dy) && (dx >= dz)) {
        return ((s32) (dy + dz) / 4) + dx;
    }
    if ((delta.y >= delta.x) && (delta.y >= delta.z)) {
        return ((s32) (delta.x + delta.z) / 4) + delta.y;
    }
    sum = delta.y + delta.x;
    return sum / 4 + delta.z;
}
