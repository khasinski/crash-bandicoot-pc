#include "common.h"

/* Set or clear the clipped rectangle in the 32-row grid. */

void Solid_PlotWall(s32 x, s32 y) {
    s32 i;
    s32 end;
    u32 *p;

    if ((x >= 0 ? x : -x) >= 32 || (y >= 0 ? y : -y) >= 32) {
        return;
    }
    if (x < 0) {
        p = &gp_solid_scratch_grid[(y + 32) << 1];
        if (*p & (1 << (x + 32))) {
            return;
        }
        *p |= 1 << (x + 32);
    } else {
        p = &gp_solid_scratch_grid[((y + 32) << 1) + 1];
        if (*p & (1 << x)) {
            return;
        }
        *p |= 1 << x;
    }
    if (y < 0) {
        i = -y;
        end = 32;
    } else {
        i = 0;
        end = 32 - y;
    }
    p = (u32 *)&gp_grid_rows[y >= 0 ? y : 0];
    if (x < 0) {
        x = -x;
        for (; i < end; i++, p++) {
            *p |= gp_solid_scratch_workspace[i] << x;
        }
    } else {
        for (; i < end; i++, p++) {
            *p |= gp_solid_scratch_workspace[i] >> x;
        }
    }
}

void Solid_PlotWallB(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4) {
    s32 *var_v1;
    s32 temp_a0;
    s32 temp_a2;
    s32 var_a1;
    s32 var_a3;
    s32 var_t0;

    temp_a2 = arg2 + 0x10;
    var_t0 = -1;
    if (temp_a2 < 0x20) {
        if (temp_a2 > 0) {
            var_t0 = -1 << (0x20 - temp_a2);
            goto block_3;
        }
    } else {
block_3:
        temp_a0 = arg0 + 0xF;
        if (temp_a0 < 0x20) {
            if (temp_a0 >= 0) {
                var_t0 &= ~((s32)0x80000000 >> temp_a0);
            }
            if (var_t0 != 0) {
                var_a1 = arg1 + 0x10;
                var_a3 = arg3 + 0x10;
                if (var_a1 < 0) {
                    var_a1 = 0;
                }
                if (var_a3 >= 0x21) {
                    var_a3 = 0x20;
                }
                if (var_a1 < 0x20) {
                    var_v1 = gp_grid_rows + var_a1;
                    if (arg4 != 0) {
                        if (var_a1 < var_a3) {
                            do {
                                var_a1 += 1;
                                *var_v1 |= var_t0;
                                var_v1++;
                            } while (var_a1 < var_a3);
                        }
                    } else {
                        var_t0 = ~var_t0;
                        if (var_a1 < var_a3) {
                            do {
                                var_a1 += 1;
                                *var_v1 &= var_t0;
                                var_v1++;
                            } while (var_a1 < var_a3);
                        }
                    }
                }
            }
        }
    }
}
