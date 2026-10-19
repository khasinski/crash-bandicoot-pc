#include "common.h"

u16 GetIntrMask(void) {
    return *g_intr_i_mask_reg;
}
