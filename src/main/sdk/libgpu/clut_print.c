#include "common.h"

/* Decode CLUT coordinates for the diagnostic callback. */

void DumpClut(s32 arg0) {
    g_gpu_ot_status(g_str_clut_print_clut_d_d_n, (arg0 & 0x3F) * 0x10, (u32) (arg0 & 0xFFFF) >> 6);
}
