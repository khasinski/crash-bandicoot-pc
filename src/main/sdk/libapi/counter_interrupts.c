#include "common.h"

s32 StartRCnt(s32 counter) {
    volatile u32 *registers=g_rcnt_i_stat_reg;
    counter&=0xFFFF;
    registers[1]|=g_rcnt_intr_masks[counter];
    return counter<3;
}
s32 StopRCnt(s32 counter) {
    volatile u32 *registers=g_rcnt_i_stat_reg;
    counter&=0xFFFF;
    registers[1]&=~g_rcnt_intr_masks[counter];
    return 1;
}
s32 ResetRCnt(s32 counter) {
    s32 index=counter&0xFFFF;
    if (index>=3) return 0;
    g_rcnt_base_reg[index][0]=0;
    return 1;
}

