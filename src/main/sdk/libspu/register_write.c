#include "common.h"

void _spu_FsetRXX(s32 reg,u32 address,s32 shift) {
    if (shift == 0) g_spu_RXX[reg] = address;
    else g_spu_RXX[reg] = address >> g_spu_trans_mode;
}
