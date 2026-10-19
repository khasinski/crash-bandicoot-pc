#include "common.h"

void _spu_writeByIO(u16 *arg0, u32 arg1)
{
    volatile s32 delay_count;
    volatile s32 delay_value;
    s32 count;
    u32 total;
    u16 *ptr;
    s32 i;
    u16 ctrl;
    u16 stat;

    total = arg1;
    stat = g_spu_RXX[0xD7] & 0x7FF;
    ptr = arg0;
    g_spu_RXX[0xD3] = g_spu_adsr_dummy_1;
    delay_value = 13;
    for (delay_count = 0; delay_count < 240; delay_count++) {
        delay_value *= 3;
    }
    while (total != 0) {
        count = total;
        if (total > 0x40) {
            count = 0x40;
        }
        for (i = 0; i < count; i += 2) {
            g_spu_RXX[0xD4] = *ptr++;
        }
        ctrl = g_spu_RXX[0xD5];
        ctrl = (ctrl & 0xFFCF) | 0x10;
        g_spu_RXX[0xD5] = ctrl;
        delay_value = 13;
        for (delay_count = 0; delay_count < 240; delay_count++) {
            delay_value *= 3;
        }
        g_spu_attr_zero_3 = 0;
        while (g_spu_RXX[0xD7] & 0x400) {
            if (++g_spu_attr_zero_3 > 0x1388) {
                BIOS_Printf(g_str_cd_ack_err, g_str_fifo_write_wait_wrdy_h_l);
                break;
            }
        }
        delay_value = 13;
        for (delay_count = 0; delay_count < 240; delay_count++) {
            delay_value *= 3;
        }
        delay_value = 13;
        for (delay_count = 0; delay_count < 240; delay_count++) {
            delay_value *= 3;
        }
        total -= count;
    }
    ctrl = g_spu_RXX[0xD5];
    ctrl &= 0xFFCF;
    g_spu_RXX[0xD5] = ctrl;
    g_spu_attr_zero_3 = 0;
    while ((g_spu_RXX[0xD7] & 0x7FF) != stat) {
        if (++g_spu_attr_zero_3 > 0x1388) {
            BIOS_Printf(g_str_cd_ack_err, g_str_fifo_write_wait_dmaf_clear_w);
            break;
        }
    }
}
