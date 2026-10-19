#include "common.h"
void Audio_Kill(void) {
    SpuReverbAttr attributes;
    attributes.mask = 7;
    attributes.mode = 0;
    attributes.depth.left = 0;
    attributes.depth.right = 0;
    SpuSetReverbModeParam(&attributes);
    SpuSetReverb(0);
}
