#include "common.h"

/* Configure the CD DMA registers and poll the volatile hardware completion bit. */

s32 CD_getsector(s32 arg0, s32 arg1) {
    *g_cd_reg0 = 0;
    *g_cd_reg3_ptr = 0x80;
    *g_cd_dma3_delay_reg = 0x20943;
    *g_cd_com_delay_reg = 0x1323;
    *g_spu_dma_ctrl_reg |= 0x8000;
    *g_spu_common_reg_base = arg0;
    *g_spu_pitch_zero = arg1 | 0x10000;
    *g_spu_attr_zero_1 = 0x11000000;
    if (*g_spu_attr_zero_1 & 0x01000000) {
        do {

        } while (*g_spu_attr_zero_1 & 0x01000000);
    }
    *g_cd_com_delay_reg = 0x1325;
    return 0;
}
