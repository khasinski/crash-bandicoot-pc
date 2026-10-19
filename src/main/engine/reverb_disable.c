#include "common.h"
typedef struct ReverbAttributes {
    u32 mask;
    s32 mode;
    s16 depth_left,depth_right;
    s32 delay,feedback;
} ReverbAttributes;
void Audio_Kill(void) {
    ReverbAttributes attributes;
    attributes.mask = 7;
    attributes.mode = 0;
    attributes.depth_left = 0;
    attributes.depth_right = 0;
    SpuSetReverbModeParam(&attributes);
    SpuSetReverb(0);
}
