#include "common.h"

/* Preserve the driver reset write after enabling SPU control. */

s32 _spu_init(s32 arg0) {
    /* The hardware delay repeatedly accesses these two stack words. */
    volatile s32 delay_count;
    volatile u32 delay_value;
    s32 poll_count;
    s32 voice;
    volatile u16 *registers;

    *g_spu_reverb_reserved |= 0xB0000;
    g_spu_RXX[0xC0] = 0;
    g_spu_RXX[0xC1] = 0;
    g_spu_RXX[0xD5] = 0;
    delay_value = 0xD;
    delay_count = 0;
    g_spu_reverb_depth_l = 0;
    g_spu_reverb_delay = 0;
    g_spu_adsr_dummy_1 = 0;
    if (delay_count < 0xF0) {
        do {
            delay_value *= 3;
            delay_count++;
        } while (delay_count < 0xF0);
    }
    g_spu_RXX[0xC0] = 0;
    g_spu_RXX[0xC1] = 0;
    g_spu_attr_zero_3 = 0;
    if (g_spu_RXX[0xD7] & 0x7FF) {
loop_3:
        poll_count = g_spu_attr_zero_3 + 1;
        g_spu_attr_zero_3 = poll_count;
        if (poll_count >= 0x1389) {
            BIOS_Printf(g_str_cd_ack_err, g_str_hardware_init_wait_reset);
        } else if (g_spu_RXX[0xD7] & 0x7FF) {
            goto loop_3;
        }
    }
    g_spu_voice_bit_lo = 2;
    g_spu_trans_mode = 3;
    g_spu_reverb_master_on = 8;
    g_spu_key_stat_mask = 7;
    g_spu_RXX[0xD6] = 4;
    g_spu_RXX[0xC2] = 0;
    g_spu_RXX[0xC3] = 0;
    g_spu_RXX[0xC6] = 0xFFFFU;
    g_spu_RXX[0xC7] = 0xFFFFU;
    g_spu_RXX[0xCC] = 0;
    g_spu_RXX[0xCD] = 0;
    if (arg0 == 0) {
        g_spu_RXX[0xC8] = 0;
        g_spu_RXX[0xC9] = 0;
        g_spu_RXX[0xCA] = 0;
        g_spu_RXX[0xCB] = 0;
        g_spu_RXX[0xD8] = 0;
        g_spu_RXX[0xD9] = 0;
        g_spu_RXX[0xDA] = 0;
        g_spu_RXX[0xDB] = 0;
        g_spu_adsr_dummy_1 = 0x200;
        _spu_writeByIO(g_spu_zero_block, 0x10);
        registers = g_spu_RXX;
        for (voice=0; voice<24; voice++) {
            registers[voice * 8 + 0] = 0;
            registers[voice * 8 + 1] = 0;
            registers[voice * 8 + 2] = 0x3FFF;
            registers[voice * 8 + 3] = 0x200;
            registers[voice * 8 + 4] = 0;
            registers[voice * 8 + 5] = 0;
        }
        g_spu_RXX[0xC4] |= 0xFFFFU;
        g_spu_RXX[0xC5] = (u16) (g_spu_RXX[0xC5] | 0xFF);
        delay_value = 13;
        for (delay_count = 0; delay_count < 240; delay_count++) {
            delay_value *= 3;
        }
        delay_value = 13;
        for (delay_count = 0; delay_count < 240; delay_count++) {
            delay_value *= 3;
        }
        delay_value = 13;
        for (delay_count = 0; delay_count < 240; delay_count++) {
            delay_value *= 3;
        }
        delay_value = 13;
        for (delay_count = 0; delay_count < 240; delay_count++) {
            delay_value *= 3;
        }
        g_spu_RXX[0xC6] |= 0xFFFFU;
        g_spu_RXX[0xC7] = (u16) (g_spu_RXX[0xC7] | 0xFF);
        delay_value = 13;
        for (delay_count = 0; delay_count < 240; delay_count++) {
            delay_value *= 3;
        }
        delay_value = 13;
        for (delay_count = 0; delay_count < 240; delay_count++) {
            delay_value *= 3;
        }
        delay_value = 13;
        for (delay_count = 0; delay_count < 240; delay_count++) {
            delay_value *= 3;
        }
        delay_value = 13;
        for (delay_count = 0; delay_count < 240; delay_count++) {
            delay_value *= 3;
        }
    }
    g_spu_trans_start_addr = 1;
    g_spu_RXX[0xD5] = 0xC000;
    g_spu_dma_done_cb = 0;
    g_spu_irq_cb = 0;
    return 0;
}
