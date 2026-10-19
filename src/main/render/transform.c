#include "common.h"

/* Clear the first six transform words and initialise the three scales to 1.0 (12-bit fixed point). */

void Gfx_ResetCam(s32 *transform)
{
    transform[3] = 0;
    transform[4] = 0;
    transform[5] = 0;
    transform[0] = 0;
    transform[1] = 0;
    transform[2] = 0;
    transform[6] = 0x1000;
    transform[7] = 0x1000;
    transform[8] = 0x1000;
}
