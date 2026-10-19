#include "common.h"

s32 _spu_reset(void) {
    /* Retail stores and reloads both delay words on the stack. */
    volatile s32 delay_count;
    volatile u32 delay_value;
    u16 temp_a0;

    temp_a0 = g_spu_RXX[0xD5];
    g_spu_RXX[0xD5] = (u16) (temp_a0 & 0x7FCF);
    delay_value = 13;
    for (delay_count = 0; delay_count < 240; delay_count++) {
        delay_value *= 3;
    }
    temp_a0 &= 0xFFCF;
    g_spu_RXX[0xD5] = temp_a0;
    return 0;
}
