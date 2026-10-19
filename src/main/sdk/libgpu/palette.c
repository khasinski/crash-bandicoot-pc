#include "common.h"
#include "gpu.h"

/* Upload a 256- or 16-colour palette and return its CLUT ID. */

u16 LoadClut(void *image, s32 x, s32 y)
{
    GpuRect rect;
    rect.w = 0x100;
    rect.x = x;
    rect.y = y;
    rect.h = 1;
    LoadImage(&rect,image);
    return GetClut(x,y) & 0xFFFF;
}

u16 LoadClut2(void *image, s32 x, s32 y)
{
    GpuRect rect;
    rect.w = 0x10;
    rect.x = x;
    rect.y = y;
    rect.h = 1;
    LoadImage(&rect,image);
    return GetClut(x,y) & 0xFFFF;
}
