#include "common.h"

s32 Solid_TestWallCell(s32 x, s32 y)
{
    if ((u32)x < 32 && (u32)y < 32 && (RAM_AT(s32, gp_grid_rows)[y] & (0x80000000U >> x)) == 0) {
        return 1;
    }
    return 0;
}
