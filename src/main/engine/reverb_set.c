#include "common.h"
s32 Audio_SetReverbAttr(s32 mode,s16 left,s16 right,s32 delay,s32 feedback)
{
    SpuReverbAttr attributes;
    attributes.mask=7;
    attributes.mode=mode|0x100;
    attributes.depth.left=left;
    attributes.depth.right=right;
    if((u32)(mode-7)<2) {
        attributes.delay=delay;
        attributes.feedback=feedback;
    }
    SpuSetReverbModeParam(&attributes);
    SpuSetReverbDepth(&attributes);
    SpuSetReverb(mode!=0);
    return 0;
}
