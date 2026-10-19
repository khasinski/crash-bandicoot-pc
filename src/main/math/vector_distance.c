#include "common.h"

/* Compute XYZ or XZ distance through the integer root helper, using 8-bit scaled deltas. */

#include "vector.h"

s32 Math_EucDist(Vec3 *a, Vec3 *b)
{
    Vec3 delta;

    delta.x = (a->x - b->x) >> 8;
    delta.y = (a->y - b->y) >> 8;
    delta.z = (a->z - b->z) >> 8;
    return SquareRoot0(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z) << 8;
}

s32 Math_EucDistXZ(Vec3 *a, Vec3 *b)
{
    Vec3 delta;

    delta.x = (a->x - b->x) >> 8;
    delta.z = (a->z - b->z) >> 8;
    return SquareRoot0(delta.x * delta.x + delta.z * delta.z) << 8;
}
