#include "vector.h"

typedef struct Matrix {s16 m[9];} Matrix;
void Gfx_Transform(Vec3 *arg0, Matrix *arg1, Vec3 *arg2) {
    arg2->x = (s32) ((s32) ((arg1->m[0] * arg0->x) + (arg1->m[1] * arg0->y) + (arg1->m[2] * arg0->z)) >> 0xC);
    arg2->y = (s32) ((s32) ((arg1->m[3] * arg0->x) + (arg1->m[4] * arg0->y) + (arg1->m[5] * arg0->z)) >> 0xC);
    arg2->z = (s32) ((s32) ((arg1->m[6] * arg0->x) + (arg1->m[7] * arg0->y) + (arg1->m[8] * arg0->z)) >> 0xC);
}
