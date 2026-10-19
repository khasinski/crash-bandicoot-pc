#include "common.h"

s32 _status(void) {
    return *g_gpu_stat_reg;
}
