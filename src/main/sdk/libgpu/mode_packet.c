#include "common.h"

/* Build a one-word draw-mode packet and return the word held in v0 on exit. */

typedef struct {
    u8 link[3];
    u8 length;
    u32 mode;
} GpuModePacket;

u32 SetDrawTPage(GpuModePacket *arg0, s32 arg1, s32 arg2, s32 arg3) {
    s32 var_a3;
    s32 var_v0;
    s32 var_v0_2;
    s32 var_v0_3;
    s32 var_v1;

    arg0->length = 1;
    if ((GetGraphType() == 1) || (GetGraphType() == 2)) {
        var_v1 = 0xE1000000;
        if (arg2 != 0) {
            var_v1 = 0xE1000800;
        }
        var_v0 = arg3 & 0x27FF;
        if (arg1 != 0) {
            var_v0 |= 0x1000;
        }
        var_v0_2 = var_v1 | var_v0;
    } else {
        var_a3 = 0xE1000000;
        if (arg2 != 0) {
            var_a3 = 0xE1000200;
        }
        var_v0_3 = arg3 & 0x9FF;
        if (arg1 != 0) {
            var_v0_3 |= 0x400;
        }
        var_v0_2 = var_a3 | var_v0_3;
    }
    arg0->mode = var_v0_2;
    return var_v0_2;
}
