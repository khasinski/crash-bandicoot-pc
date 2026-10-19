#include "common.h"
#include "gte.h"

void SetRotMatrix(void *m)
{
    gte_ldrotmatrix(m);
}

void SetLightMatrix(void *m)
{
    gte_ldlightmatrix(m);
}

void SetColorMatrix(void *m)
{
    gte_ldcolormatrix(m);
}

void SetTransMatrix(void *m)
{
    gte_ldtransmatrix(m);
}

void ReadSZfifo3(s32 *sz0, s32 *sz1, s32 *sz2)
{
    gte_stsz3(sz0, sz1, sz2);
}

gte_obj_align(1)

void SetBackColor(s32 r, s32 g, s32 b)
{
    r <<= 4;
    g <<= 4;
    b <<= 4;
    gte_ctc2(r, 21);
    gte_ctc2(g, 22);
    gte_ctc2(b, 23);
}

void SetGeomOffset(s32 ofx, s32 ofy)
{
    ofx <<= 16;
    ofy <<= 16;
    gte_ctc2(ofx, 24);
    gte_ctc2(ofy, 25);
}

gte_obj_align(2)

void SetGeomScreen(s32 h)
{
    gte_ctc2(h, 26);
}

gte_obj_align(3)

s32 RotTransPers(u32 *v0, u32 *sxy, u32 *p, u32 *flag)
{
    u32 f;
    s32 sz;

    gte_rtps_v0(v0);
    gte_stsxy2(*sxy);
    gte_swc2(*p, 8);
    gte_getflag(f);
    gte_getsz3(sz);
    *flag = f;
    return sz >> 2;
}

gte_obj_align(4)

void RotTrans(u32 *v0, u32 *v1, u32 *flag)
{
    u32 f;

    gte_mvmva_v0(v0);
    gte_stlvnl(v1);
    gte_getflag(f);
    *flag = f;
}

gte_obj_align(5)
