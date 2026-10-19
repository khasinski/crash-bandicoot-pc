#include "cd_audio.h"

/* Initialise SPU CD mixing and route CD stereo output. */

s32 CD_initintr(void) {
    u8 volume[4];
    if (g_cd_spu_base_reg[0x1B8/2] == 0) {
        if (g_cd_spu_base_reg[0x1BA/2] == 0) {
            g_cd_spu_base_reg[0x180/2] = 0x3FFF;
            g_cd_spu_base_reg[0x182/2] = 0x3FFF;
        }
    }
    g_cd_spu_base_reg[0x1B0/2] = 0x3FFF;
    g_cd_spu_base_reg[0x1B2/2] = 0x3FFF;
    g_cd_spu_base_reg[0x1AA/2] = 0xC001;
    volume[2] = 0x80;
    volume[0] = 0x80;
    volume[3] = 0;
    volume[1] = 0;
    return CD_WriteVolumeRegisters(volume);
}
