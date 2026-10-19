#include "vector.h"

typedef struct ObjectFlagsView {u8 reserved_00[0xC8];s32 flags;} ObjectFlagsView;
s32 Math_OutOfRange(ObjectFlagsView *object, Vec3 *a, Vec3 *b, s32 max_x, s32 max_y, s32 max_z) {
    s32 min_x, min_y, min_z;
    s32 dx;
    s32 dy;
    s32 dz;

    dx = a->x;
    dx -= b->x;
    min_x = -max_x;
    min_y = -max_y;
    min_z = -max_z;
    if ((max_x < dx) || (dx < min_x) ||
        (dy = a->y - b->y, ((max_y < dy) != 0)) || (dy < min_y) ||
        (dz = a->z - b->z, ((max_z < dz) != 0)) || (dz < min_z)) {
        if (object != NULL) {
            object->flags = (s32) (object->flags | 0x8000);
        }
        return 1;
    }
    return 0;
}
