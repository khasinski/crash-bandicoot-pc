#include "common.h"
#include "gte.h"
#include "vector.h"

/*
 * Object and sprite matrices (hand-written MIPS block, 0x8003913C..0x8003A768).
 * They build an object's rotation from its three 12-bit angles, combine it
 * with the camera rotation and the object scale, and leave the result in the
 * GTE rotation matrix for the transform that follows. Gte_RCalcObjectRotMatrix
 * also builds the light and light-colour matrices.
 *
 * The original forms most products of sines and cosines with GTE mvmva
 * multiplies (a diagonal matrix times V0 or IR), so the GTE register state
 * left behind depends on every intermediate step. The C issues the same GTE
 * writes and commands with the same values.
 *
 * The original also saves the caller's registers in scratchpad
 * 0x1F800000..0x1F80003F and restores them before returning; nothing else
 * reads that area, so the C does not write it.
 */

/* mvmva with sf=1, lm=0 and no translation: rotation (RT) or light (LLM)
 * matrix times V0 or times IR1..IR3. */
#define MVMVA_RT_V0 0x0486012u
#define MVMVA_RT_IR 0x049E012u
#define MVMVA_LLM_V0 0x04A6012u
#define MVMVA_LLM_IR 0x04BE012u

/* First GTE control register of the rotation, light and light-colour
 * matrices. */
#define GTE_RT 0
#define GTE_LLM 8
#define GTE_LCM 16
/* Background (ambient) colour, red, green, blue. */
#define GTE_BK 13

/* An object's position, rotation (12-bit angles) and scale (4.12). */
typedef struct ObjVectors {
    Vec3 trans;
    Vec3 rot;
    Vec3 scale;
} ObjVectors;

/* The model scale the object scale is multiplied with. */
typedef struct ModelScale {
    u32 word_00;
    Vec3 scale;
} ModelScale;

/* An object's lighting: three light directions, the gain of each ambient
 * channel, the colour of each light and the ambient colour. */
typedef struct ObjLights {
    s16 dir[3][3];
    s16 ambient_gain[3];
    s16 color[3][3];
    s16 ambient[3];
} ObjLights;

/* Camera position and angles, as passed to the sprite matrix. */
typedef struct SpriteCamera {
    Vec3 trans;
    Vec3 rot;
} SpriteCamera;

/* Low word of the 64-bit product (mult/mflo). */
static s32 mul_lo(s32 a, s32 b)
{
    return (s32)(u32)((s64)a * b);
}

static u32 pack(s32 lo, s32 hi)
{
    return (u16)lo | ((u32)(u16)hi << 16);
}

/* Loads a 3x3 matrix into the five control registers at `base`. */
static void gte_load_matrix(u32 base, s32 m[3][3])
{
    gte_ctc2(pack(m[0][0], m[0][1]), base + 0);
    gte_ctc2(pack(m[0][2], m[1][0]), base + 1);
    gte_ctc2(pack(m[1][1], m[1][2]), base + 2);
    gte_ctc2(pack(m[2][0], m[2][1]), base + 3);
    gte_ctc2((u16)m[2][2], base + 4);
}

/* Sets the diagonal of the matrix at `base`; the off-diagonal registers are
 * cleared separately. */
static void gte_load_diagonal(u32 base, s32 d0, s32 d1, s32 d2)
{
    gte_ctc2((u16)d0, base + 0);
    gte_ctc2((u16)d1, base + 2);
    gte_ctc2((u16)d2, base + 4);
}

static void gte_clear_off_diagonal(u32 base)
{
    gte_ctc2(0, base + 1);
    gte_ctc2(0, base + 3);
}

static void gte_load_v0(s32 x, s32 y, s32 z)
{
    gte_mtc2((u16)z, 1);
    gte_mtc2(pack(x, y), 0);
}

static void gte_read_ir(s32 v[3])
{
    v[0] = (s32)Psyz_GteDataRead(9);
    v[1] = (s32)Psyz_GteDataRead(10);
    v[2] = (s32)Psyz_GteDataRead(11);
}

/* v = (matrix selected by `cmd`) * v, through IR1..IR3. */
static void gte_multiply_ir(u32 cmd, s32 v[3])
{
    gte_mtc2(v[0], 9);
    gte_mtc2(v[1], 10);
    gte_mtc2(v[2], 11);
    Psyz_GteCommand(cmd);
    gte_read_ir(v);
}

/* Rotation matrix of an object from angles a, b, c, built with the matrix
 * at `base` (rotation or light), whose multiply command is `cmd`. Leaves V0
 * = (sin a, cos c, sin b) and a diagonal matrix in those registers. */
static void euler_matrix(u32 base, u32 cmd, s32 a, s32 b, s32 c, s32 e[3][3])
{
    s32 sa = Math_Sin(a), sb = Math_Sin(b), sc = Math_Sin(c);
    s32 ca = Math_Cos(a), cb = Math_Cos(b), cc = Math_Cos(c);
    s32 i[3], j[3], k[3];
    s32 p = mul_lo(ca, sc) >> 12;
    s32 q, r, u;

    gte_clear_off_diagonal(base);
    gte_load_v0(sa, cc, sb);
    /* i = (cos b sin a, cos a cos c, sin a sin b) */
    gte_load_diagonal(base, cb, ca, sa);
    Psyz_GteCommand(cmd);
    gte_read_ir(i);
    q = mul_lo(p, cb) >> 12;
    /* j = (cos c sin a, sin b cos c, sin c sin b) */
    gte_load_diagonal(base, cc, sb, sc);
    Psyz_GteCommand(cmd);
    gte_read_ir(j);
    r = mul_lo(i[0], sc) >> 12;
    /* k = (sin c sin a, i0 cos c, cos a sin b) */
    gte_load_diagonal(base, sc, i[0], ca);
    Psyz_GteCommand(cmd);
    gte_read_ir(k);
    u = mul_lo(i[1], cb) >> 12;

    e[0][0] = i[1] - r;
    e[0][1] = i[2];
    e[0][2] = p + k[1];
    e[1][0] = j[2];
    e[1][1] = cb;
    e[1][2] = -j[1];
    e[2][0] = -(j[0] + q);
    e[2][1] = k[2];
    e[2][2] = u - k[0];
}

/* Scales the row `row` of m by the diagonal of the rotation matrix. */
static void scale_row(s32 m[3][3], s32 row)
{
    s32 v[3];

    v[0] = m[row][0];
    v[1] = m[row][1];
    v[2] = m[row][2];
    gte_multiply_ir(MVMVA_RT_IR, v);
    m[row][0] = v[0];
    m[row][1] = v[1];
    m[row][2] = v[2];
}

/* Screen-space rows: y scaled by -5/8 (aspect), z negated. */
static s32 flip_y(s32 v)
{
    return -((v * 5) >> 3);
}

/* Loads RT = flip(camera * E * scale), E the object's rotation from its
 * angles and scale the product of the object and model scales. */
static void load_object_rotation(const MATRIX *camera, const ModelScale *model,
                                 const ObjVectors *vectors)
{
    const Vec3 *rot = &vectors->rot;
    s32 e[3][3], m[3][3], v[3];
    s32 scale[3];
    s32 col;

    euler_matrix(GTE_RT, MVMVA_RT_V0, rot->z & 0xFFF, rot->x & 0xFFF,
        (rot->y - rot->z) & 0xFFF, e);

    /* Each column of E through the camera rotation; m is stored by row. */
    gte_ldrotmatrix(camera);
    for (col = 0; col < 3; col++) {
        v[0] = e[0][col];
        v[1] = e[1][col];
        v[2] = e[2][col];
        gte_multiply_ir(MVMVA_RT_IR, v);
        m[0][col] = v[0];
        m[1][col] = v[1];
        m[2][col] = v[2];
    }
    scale[0] = mul_lo(vectors->scale.x, model->scale.x) >> 12;
    scale[1] = mul_lo(vectors->scale.y, model->scale.y) >> 12;
    scale[2] = mul_lo(vectors->scale.z, model->scale.z) >> 12;

    /* Scale the columns, one row at a time (y, z, x), with a diagonal RT. */
    gte_ctc2((u16)scale[0], 0);
    gte_ctc2(0, 1);
    gte_ctc2((u16)scale[1], 2);
    gte_ctc2(0, 3);
    gte_ctc2((u16)scale[2], 4);
    scale_row(m, 1);
    scale_row(m, 2);
    scale_row(m, 0);
    for (col = 0; col < 3; col++) {
        m[1][col] = flip_y(m[1][col]);
        m[2][col] = -m[2][col];
    }
    gte_load_matrix(GTE_RT, m);
}

/* Builds the object's rotation matrix in the GTE (see load_object_rotation). */
void Gte_RCalcObjectMatrices(void *matrix, void *model, void *vectors, void *light)
{
    (void)light;
    load_object_rotation(matrix, model, vectors);
}

/* Builds the object's rotation matrix, and the light matrix (the light
 * directions in object space, mirrored in x for a negative x scale), the
 * ambient colour and the light-colour matrix. */
void Gte_RCalcObjectRotMatrix(void *matrix, void *model, void *vectors, void *light)
{
    const ObjVectors *vec = vectors;
    const ObjLights *lights = light;
    s32 mirror = vec->scale.x < 0;
    s32 e[3][3], d[3][3], color[3][3];
    s32 a, l, ch;

    load_object_rotation(matrix, model, vec);

    /* The inverse of the object's rotation, in the light matrix. */
    a = mirror ? vec->rot.z + vec->rot.y : vec->rot.z - vec->rot.y;
    euler_matrix(GTE_LLM, MVMVA_LLM_V0, a & 0xFFF, -vec->rot.x & 0xFFF,
        -vec->rot.z & 0xFFF, e);
    gte_load_matrix(GTE_LLM, e);

    /* Rotate each light direction into object space. */
    for (l = 0; l < 3; l++) {
        d[l][0] = mirror ? -lights->dir[l][0] : lights->dir[l][0];
        d[l][1] = lights->dir[l][1];
        d[l][2] = lights->dir[l][2];
        gte_multiply_ir(MVMVA_LLM_IR, d[l]);
        gte_ctc2(mul_lo(lights->ambient[l], lights->ambient_gain[l]) >> 8, GTE_BK + l);
    }
    gte_load_matrix(GTE_LLM, d);

    /* Light-colour matrix: one column per light. */
    for (l = 0; l < 3; l++)
        for (ch = 0; ch < 3; ch++)
            color[ch][l] = lights->color[l][ch];
    gte_load_matrix(GTE_LCM, color);
}

/* 11- and 12-bit signed fields of the camera angles. */
static s32 sign_extend(s32 value, s32 bits)
{
    return (s32)((u32)value << (32 - bits)) >> (32 - bits);
}

static s32 clamp_tilt(s32 value)
{
    if (value > 0xAA)
        return 0xAA;
    if (value < -0xAA)
        return -0xAA;
    return value;
}

/* Sets the GTE rotation and translation for a sprite object. With `flag`
 * clear the object is placed in view space with the camera (cam_rot,
 * cam_trans) and culled when its depth is not in (proj, min(0x2EE0, far)],
 * far = 0 meaning no far limit; the sprite then faces the camera, turned by
 * the camera's roll and by a small part of its pitch and yaw. With `flag`
 * set the position is in screen space at depth `proj`. Rows: x, y (scaled
 * by -5/8), z = 0, scaled by the object scale >> shift. Returns 0 when
 * culled. `flip` is not used. */
s32 Gte_RCalcSpriteRotMatrix(void *vectors, void *cam_trans, s32 flag, s32 shift, void *cam_rot,
                             s32 proj, s32 far, s32 flip)
{
    const ObjVectors *vec = vectors;
    const SpriteCamera *cam = cam_trans;
    s32 rx, ry, rz;
    s32 sg, ss, sf, cg, cs, cf;
    s32 g, t8, s7, at;
    s32 i[3], j[3], k[3], xrow[3], yrow[3];
    s32 col;

    (void)flip;
    gte_ldrotmatrix(cam_rot);
    if (flag != 0) {
        gte_ctc2(vec->trans.x >> 8, 5);
        gte_ctc2(-vec->trans.y >> 8, 6);
        gte_ctc2(proj, 7);
    } else {
        s32 v[3];
        v[0] = (vec->trans.x - cam->trans.x) >> 8;
        v[1] = (vec->trans.y - cam->trans.y) >> 8;
        v[2] = (vec->trans.z - cam->trans.z) >> 8;
        gte_multiply_ir(MVMVA_RT_IR, v);
        gte_ctc2(v[0], 5);
        gte_ctc2(v[1], 6);
        gte_ctc2(v[2], 7);
        if (!(proj < v[2]) || v[2] > 0x2EE0 || (far != 0 && v[2] > far))
            return 0;
    }

    rx = vec->rot.x;
    ry = vec->rot.y;
    rz = vec->rot.z;
    if (flag == 0) {
        rx += clamp_tilt(-sign_extend(cam->rot.x, 11));
        ry += clamp_tilt(-sign_extend(cam->rot.y, 11));
        rz -= sign_extend(cam->rot.z, 12);
    }
    sg = Math_Sin(rz);
    ss = Math_Sin(rx);
    sf = Math_Sin(ry);
    cg = Math_Cos(rz);
    cs = Math_Cos(rx);
    cf = Math_Cos(ry);

    gte_clear_off_diagonal(GTE_RT);
    gte_load_v0(sf, cf, cg);
    g = mul_lo(ss, sf) >> 12;
    /* i = (0, sin x cos y, cos x cos z) */
    gte_load_diagonal(GTE_RT, 0, ss, cs);
    Psyz_GteCommand(MVMVA_RT_V0);
    s7 = mul_lo(sg, cs) >> 12;
    gte_read_ir(i);
    /* j = (cos z sin y, cos z cos y, g cos z) */
    gte_load_diagonal(GTE_RT, cg, cg, g);
    Psyz_GteCommand(MVMVA_RT_V0);
    t8 = mul_lo(sg, g) >> 12;
    gte_read_ir(j);
    /* k = (sin z sin y, sin z cos y, i1 cos z) */
    gte_load_diagonal(GTE_RT, sg, sg, i[1]);
    Psyz_GteCommand(MVMVA_RT_V0);
    gte_read_ir(k);
    /* The original multiplies by sin z masked to 16 bits, which adds
     * 16 * i1 when sin z is negative. */
    at = mul_lo((u16)sg, i[1]) >> 12;

    xrow[0] = j[1] - t8;
    xrow[1] = -s7;
    xrow[2] = j[0] + at;
    yrow[0] = k[1] + j[2];
    yrow[1] = i[2];
    yrow[2] = k[0] - k[2];

    /* Diagonal scale. The words are written whole, so the upper halves of
     * the x and y scale land in R12 and R23. */
    gte_ctc2(vec->scale.x >> (shift & 31), 0);
    gte_ctc2(0, 1);
    gte_ctc2(vec->scale.y >> (shift & 31), 2);
    gte_ctc2(0, 3);
    gte_ctc2(vec->scale.z >> (shift & 31), 4);
    gte_multiply_ir(MVMVA_RT_IR, yrow);
    gte_multiply_ir(MVMVA_RT_IR, xrow);
    for (col = 0; col < 3; col++)
        yrow[col] = flip_y(yrow[col]);

    gte_ctc2(pack(xrow[0], xrow[1]), 0);
    gte_ctc2(pack(xrow[2], yrow[0]), 1);
    gte_ctc2(pack(yrow[1], yrow[2]), 2);
    gte_ctc2(0, 3);
    gte_ctc2(0, 4);
    return 1;
}
