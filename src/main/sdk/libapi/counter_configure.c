#include "common.h"
s32 SetRCnt(s32 counter,s32 target,s32 flags) {
    s32 index=counter&0xFFFF;
    s32 mode=0x48;
    volatile u16 *registers;
    if (index>=3) return 0;
    registers=g_rcnt_base_reg[index];
    registers[2]=0;
    registers[4]=target;
    if ((u32)index<2) {
        if (flags&0x10) mode=0x49;
        if (!(flags&1)) mode|=0x100;
    } else if (index==2) {
        if (!(flags&1)) mode=0x248;
    }
    if (flags&0x1000) mode|=0x10;
    g_rcnt_base_reg[index][2]=mode;
    return 1;
}

s32 GetRCnt(s32 counter)
{
    s32 index = counter & 0xFFFF;
    if (index >= 3) {
        return 0;
    }
    return g_rcnt_base_reg[index][0];
}

