#include "common.h"

/* Attribute fields read at offsets 0x00, 0x08 and 0x0A. */
typedef struct {
    u32 mask;
    u8 reserved_04[4];
    u16 volume08, volume0A;
} SpuVolumeAttrs;

s32 SpuSetReverbDepth(SpuVolumeAttrs *arg0) {
    s32 temp_a1;
    s32 temp_a2;

    temp_a1 = arg0->mask;
    temp_a2 = temp_a1 == 0;
    if ((temp_a2 != 0) || (temp_a1 & 2)) {
        g_spu_RXX[0xC2] = (u16) arg0->volume08;
        g_spu_reverb_vl_cur = arg0->volume08;
    }
    if ((temp_a2 != 0) || (temp_a1 & 4)) {
        g_spu_RXX[0xC3] = (u16) arg0->volume0A;
        g_spu_reverb_vr_cur = arg0->volume0A;
    }
    return 0;
}
