#include "common.h"

typedef struct { s32 x, y, z; } Point3;
typedef struct { s32 x, y, z, width, height, depth; } Bounds3;
typedef struct { u8 reserved_00[0x14]; Bounds3 *bounds; } BoundsView;

s32 Level_TestPointInRect(Bounds3 *arg0, Point3 *arg1)
{
    if (arg1->x >= (arg0->x << 8)
        && arg1->y >= (arg0->y << 8)
        && arg1->z >= (arg0->z << 8)
        && arg1->x < ((arg0->x + arg0->width) << 8)
        && arg1->y < ((arg0->y + arg0->height) << 8)
        && arg1->z < ((arg0->z + arg0->depth) << 8)) {
        return 1;
    }
    return 0;
}

s32 Level_SaveStateFull(BoundsView *arg0, Point3 *arg1) {
    s32 temp_a0;
    s32 temp_a1;
    s32 temp_a2;
    s32 temp_a3;
    s32 temp_t0;
    s32 temp_t1;
    s32 var_t2;
    Bounds3 *temp_v1;

    temp_v1 = arg0->bounds;
    temp_a0 = temp_v1->x;
    temp_t0 = arg1->x;
    var_t2 = 0;
    if (temp_t0 >= (temp_a0 << 8)) {
        temp_a2 = temp_v1->y;
        temp_t1 = arg1->y;
        if (temp_t1 >= (temp_a2 << 8)) {
            temp_a3 = temp_v1->z;
            temp_a1 = arg1->z;
            if ((temp_a1 >= (temp_a3 << 8)) && (temp_t0 < ((temp_a0 + temp_v1->width) << 8)) && (temp_t1 < ((temp_a2 + temp_v1->height) << 8))) {
                var_t2 = temp_a1 < ((temp_a3 + temp_v1->depth) << 8);
            }
        }
    }
    return var_t2;
}
