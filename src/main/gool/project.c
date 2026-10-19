#include "common.h"
#include "ns.h"
#include "vector.h"

typedef struct Matrix {
    s16 m[3][3];
    s32 t[3];
} Matrix;

typedef struct SVector {
    s16 x, y, z, reserved;
} SVector;

typedef struct LVector {
    s32 x, y, z, reserved;
} LVector;

#define CRASH_EXT_GOOL_PROJECT
#include "gool_externs.h"
#undef CRASH_EXT_GOOL_PROJECT

void Gool_Transform(Vec3 *arg0, Vec3 *arg1, Vec3 *arg2, Vec3 *arg3, Vec3 *arg4)
{
    Matrix m1;
    Matrix m2;
    LVector v_out;
    LVector v_in;
    s16 s1, c1;
    s16 s2, c2;
    s16 s3, c3;

    s1 = Math_Sin(arg2->z & 0xFFF);
    c1 = Math_Cos(arg2->z & 0xFFF);
    m1.m[0][0] = c1;
    m1.m[0][1] = 0;
    m1.m[0][2] = s1;
    m1.m[1][0] = 0;
    m1.m[1][1] = 0x1000;
    m1.m[1][2] = 0;
    m1.m[2][0] = -s1;
    m1.m[2][1] = 0;
    m1.m[2][2] = c1;

    s2 = Math_Sin(arg2->x);
    c2 = Math_Cos(arg2->x);
    m2.m[0][0] = 0x1000;
    m2.m[0][1] = 0;
    m2.m[0][2] = 0;
    m2.m[1][0] = 0;
    m2.m[1][1] = c2;
    m2.m[1][2] = -s2;
    m2.m[2][0] = 0;
    m2.m[2][1] = s2;
    m2.m[2][2] = c2;
    MulMatrix((MATRIX *)&m1, (MATRIX *)&m2);

    s3 = Math_Sin(-((arg2->z - arg2->y) & 0xFFF));
    c3 = Math_Cos(-((arg2->z - arg2->y) & 0xFFF));
    m2.m[0][0] = c3;
    m2.m[0][1] = 0;
    m2.m[0][2] = s3;
    m2.m[1][0] = 0;
    m2.m[1][1] = 0x1000;
    m2.m[1][2] = 0;
    m2.m[2][0] = -s3;
    m2.m[2][1] = 0;
    m2.m[2][2] = c3;
    MulMatrix((MATRIX *)&m1, (MATRIX *)&m2);

    if (arg3 != NULL) {
        ScaleMatrix((MATRIX *)&m1, (VECTOR *)arg3);
    }

    v_in.x = arg0->x / 16;
    v_in.y = arg0->y / 16;
    v_in.z = arg0->z / 16;
    Gfx_Transform((Vec3 *)&v_in, &m1, (Vec3 *)&v_out);

    v_out.x <<= 4;
    v_out.y <<= 4;
    v_out.z <<= 4;

    arg4->x = v_out.x + arg1->x;
    arg4->y = v_out.y + arg1->y;
    arg4->z = v_out.z + arg1->z;
}

s32 Gool_Transform2(Vec3 *arg0, Vec3 *arg1, s32 arg2)
{
    SVector sv;
    LVector v;
    s32 flag;

    v.x = arg0->x;
    v.y = arg0->y;
    v.z = arg0->z;
    if (arg2 != 0) {
        SetRotMatrix((MATRIX *)g_m_wgeo_view);
    } else {
        SetRotMatrix((MATRIX *)g_m_view);
    }
    SetTransMatrix((MATRIX *)g_cams);
    if (arg2 != 0) {
        sv.x = (v.x - g_cam_eye_x[0]) >> 8;
        sv.y = (v.y - g_cam_eye_y[0]) >> 8;
        sv.z = (v.z - g_cam_eye_z[0]) >> 8;
    } else {
        sv.x = (v.x - g_cam_trans_x) >> 8;
        sv.y = (v.y - g_cam_trans_y) >> 8;
        sv.z = (v.z - g_cam_trans_z) >> 8;
    }
    RotTrans((SVECTOR *)&sv, (VECTOR *)arg1, &flag);
    arg1->x <<= 8;
    arg1->y = -(arg1->y << 8);
    arg1->z <<= 8;
    return -0xFF;
}

s32 Gool_Project(Vec3 *arg0, Vec3 *arg1)
{
    SVector sv;
    LVector v;
    s32 sxy;
    s32 p;
    s32 flag;
    s32 otz1;
    s32 otz2;
    s32 otz3;

    v.x = arg0->x;
    v.y = arg0->y;
    v.z = arg0->z;
    SetRotMatrix((MATRIX *)g_m_view);
    SetTransMatrix((MATRIX *)g_cams);
    sv.x = (v.x - g_cam_trans_x) >> 8;
    sv.y = (v.y - g_cam_trans_y) >> 8;
    sv.z = (v.z - g_cam_trans_z) >> 8;
    RotTransPers((SVECTOR *)&sv, &sxy, &p, &flag);
    arg1->x = (s16)sxy << 8;
    arg1->y = -(sxy >> 16) << 8;
    ReadSZfifo3(&otz1, &otz2, &otz3);
    arg1->z = otz3 << 8;
    return -0xFF;
}

typedef struct CollideObj {
    u8 reserved_00[0x78];
    RAM_REF(struct CollideObj) other;
    u8 reserved_7c[4];
    Vec3 pos;
    u8 reserved_8c[0x3C];
    u32 flagsC8;
    u8 reserved_cc[0x54];
    u32 flags120;
    u8 reserved_124[0x24];
    s32 radius;
} CollideObj;

s32 Gool_Collide(CollideObj *arg0, s32 *arg1, CollideObj *arg2, s32 *arg3)
{
    s32 box1[6];
    s32 box2[6];
    CollideObj *other;
    s32 dist;

    other = RAM_AT(CollideObj, arg0->other);
    if (other != NULL && other != arg2) {
        if (arg2->flags120 & 0x800) {
            arg2->other = Ram_Addr(arg0);
            return -0x1C;
        }
        dist = Math_ApxDist(&arg0->pos, &other->pos);
        if (Math_ApxDist(&arg0->pos, &arg2->pos) >= dist && !(other->flags120 & 0x800)) {
            return -0x1C;
        }
    }
    arg0->other = Ram_Addr(arg2);
    arg2->other = Ram_Addr(arg0);
    if (arg0->radius != 0) {
        box1[0] = arg1[0] + arg0->radius;
        box1[1] = arg1[1];
        box1[2] = arg1[2] + arg0->radius;
        box1[3] = arg1[3] - arg0->radius;
        box1[4] = arg1[4];
        box1[5] = arg1[5] - arg0->radius;
        if (Level_TestBoundIntersection((struct ZqBound *)box1, (struct ZqBound *)arg3) != 0) {
            arg0->flagsC8 |= 0x1000;
        }
    }
    if (arg2->radius != 0) {
        box2[0] = arg3[0] + arg2->radius;
        box2[1] = arg3[1];
        box2[2] = arg3[2] + arg2->radius;
        box2[3] = arg3[3] - arg2->radius;
        box2[4] = arg3[4];
        box2[5] = arg3[5] - arg2->radius;
        if (Level_TestBoundIntersection((struct ZqBound *)box2, (struct ZqBound *)arg1) != 0) {
            arg2->flagsC8 |= 0x1000;
        }
    }
    return -0xFF;
}
