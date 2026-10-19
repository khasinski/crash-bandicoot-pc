#include "common.h"

/* Pack the palette coordinates into the GPU CLUT field. */

u16 GetClut(s32 x, s32 y)
{
    return ((y << 6) | ((x >> 4) & 0x3F)) & 0xFFFF;
}
