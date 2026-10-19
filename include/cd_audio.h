#ifndef CD_AUDIO_H
#define CD_AUDIO_H

#include "common.h"
extern RAM_REF(volatile u8) g_cd_reg0;
extern RAM_REF(volatile u8) g_cd_reg1_ptr;
extern RAM_REF(volatile u8) g_cd_reg2_ptr;
extern RAM_REF(volatile u8) g_cd_reg3_ptr;

static inline s32 CD_WriteVolumeRegisters(u8 *arg0) {
    *g_cd_reg0 = 2;
    *g_cd_reg2_ptr = arg0[0];
    *g_cd_reg3_ptr = arg0[1];
    *g_cd_reg0 = 3;
    *g_cd_reg1_ptr = arg0[2];
    *g_cd_reg2_ptr = arg0[3];
    *g_cd_reg3_ptr = 0x20;
    return 0;
}

#endif
