#include "common.h"

void SsInit(void) {
    ResetCallback();
    SpuInit();
    _SsInit();
}
