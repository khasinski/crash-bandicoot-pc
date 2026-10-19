#include "common.h"

/* Forward the CD controller request with its original arguments. */

s32 CdReady(s32 mode,u8 *result) {
    return CD_ready(mode,result);
}
