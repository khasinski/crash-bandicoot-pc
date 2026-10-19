#include "common.h"

/* Forward the CD controller request with its original arguments. */

void CdFlush(void) {
    CD_initvol();
}
