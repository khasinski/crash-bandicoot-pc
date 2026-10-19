#include "common.h"

s32 CdReset(s32 mode) {
    if (mode == 2) {
        CD_init();
        return 1;
    }
    if (CD_reset() != 0) {
        return 0;
    }
    if (mode == 1 && CD_initintr() != 0) {
        return 0;
    }
    return 1;
}
