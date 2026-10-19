#include "common.h"

void _spu_r_(s32 arg0, s16 arg1, s32 arg2) {
    /* Retail stores and reloads both delay words on the stack. */
    volatile s32 delay_count;
    volatile u32 delay_value;

    g_spu_RXX[0xD3] = arg1;
    delay_value = 0xD;
    delay_count = 0;
    if (delay_count < 0xF0) {
        do {
            delay_value *= 3;
            delay_count++;
        } while (delay_count < 0xF0);
    }
    delay_value = 13;
    for (delay_count = 0; delay_count < 240; delay_count++) {
        delay_value *= 3;
    }
    g_spu_RXX[0xD5] = (u16) (g_spu_RXX[0xD5] | 0x30);
    delay_value = 13;
    for (delay_count = 0; delay_count < 240; delay_count++) {
        delay_value *= 3;
    }
    delay_value = 13;
    for (delay_count = 0; delay_count < 240; delay_count++) {
        delay_value *= 3;
    }
    *g_spu_reverb_type = (*g_spu_reverb_type & 0xF0FFFFFF) | 0x22000000;
    *g_spu_trans_dir = arg0;
    *g_spu_in_transfer = (arg2 << 0x10) | 0x10;
    g_spu_reverb_work_sizes = 1;
    *g_spu_irq_enabled = 0x01000200;
}
