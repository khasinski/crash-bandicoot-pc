#include "common.h"

typedef struct {
    u8 minute;
    u8 second;
    u8 frame;
} DiscPositionView;
s32 CdPosToInt(DiscPositionView *arg0) {
    u8 temp_a1;
    u8 temp_a2;
    u8 temp_v1;

    temp_v1 = arg0->minute;
    temp_a2 = arg0->second;
    temp_a1 = arg0->frame;
    return (((((((temp_v1 >> 4) * 0xA) + (temp_v1 & 0xF)) * 0x3C) + (((temp_a2 >> 4) * 0xA) + (temp_a2 & 0xF))) * 0x4B) + (((temp_a1 >> 4) * 0xA) + (temp_a1 & 0xF))) - 0x96;
}
