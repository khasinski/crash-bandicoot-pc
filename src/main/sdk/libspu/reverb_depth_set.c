#include "common.h"
typedef struct {u32 mask;s32 mode;s16 depth_left,depth_right;s32 delay,feedback;} ReverbAttributes;
#define CRASH_EXT_SDK_LIBSPU_REVERB_DEPTH_SET
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_LIBSPU_REVERB_DEPTH_SET
void SsUtSetReverbDepth(s16 left, s16 right) {
    s32 depth;
    depth = (left * 0x7FFF) / 127;
    g_spu_reverb_attr.mask = 6;
    g_spu_reverb_attr.depth_left = depth;
    g_spu_reverb_attr.depth_right = (right * 0x7FFF) / 127;
    SpuSetReverbModeParam(&g_spu_reverb_attr);
}
