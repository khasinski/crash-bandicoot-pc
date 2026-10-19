#include "common.h"

/* Align a byte address, convert to SPU units, optionally write a register. */

u32 _spu_FsetRXXa(s32 reg,u32 address) {
    u32 units;
    if (g_spu_voice_bit_lo && address % g_spu_reverb_master_on) {
        address += g_spu_reverb_master_on;
        address &= ~g_spu_key_stat_mask;
    }
    units = address >> g_spu_trans_mode;
    switch (reg) {
    case -1: return units & 0xffff;
    case -2: return address;
    default: g_spu_RXX[reg] = units; return address;
    }
}

u32 _spu_FgetRXXa(s32 reg, s32 mode) {
    u16 units = g_spu_RXX[reg];
    switch (mode) {
    case -1:
        return units;
    default:
        return units << g_spu_trans_mode;
    }
}

s32 SpuSetReverb(s32 mode) {
    u16 ctrl;
    switch (mode) {
    case 0:
        g_spu_reverb_work_reserved = 0;
        ctrl = g_spu_RXX[0xD5];
        ctrl &= ~0x80;
        g_spu_RXX[0xD5] = ctrl;
        break;
    case 1:
        if (g_spu_reverb_mode_type != 1 && _SpuIsInAllocateArea_(g_spu_reverb_param_room)) {
            g_spu_reverb_work_reserved = 0;
            ctrl = g_spu_RXX[0xD5];
            ctrl &= ~0x80;
            g_spu_RXX[0xD5] = ctrl;
        } else {
            g_spu_reverb_work_reserved = mode;
            ctrl = g_spu_RXX[0xD5];
            ctrl |= 0x80;
            g_spu_RXX[0xD5] = ctrl;
        }
        break;
    }
    return g_spu_reverb_work_reserved;
}
