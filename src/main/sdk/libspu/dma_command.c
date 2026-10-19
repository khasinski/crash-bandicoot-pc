#include "common.h"

typedef void *va_list;
#define va_start(ap, last) ((ap) = (void *)(&(last) + 1))
#define va_arg(ap, type) ((ap) += 4, ((type *)(ap))[-1])

s32 _spu_t(s32 mode, ...)
{
    va_list ap;
    s32 i;
    u16 ctrl;
    u32 val;
    u32 cmd;

    va_start(ap, mode);
    switch (mode) {
    case 2:
        val = va_arg(ap, u32);
        g_spu_RXX[0xD3] = g_spu_adsr_dummy_1 = val >> g_spu_trans_mode;
        break;
    case 1:
        g_spu_reverb_work_sizes = 0;
        i = 0;
        while (g_spu_RXX[0xD3] != g_spu_adsr_dummy_1) {
            if (++i > 0xF00) {
                return -2;
            }
        }
        ctrl = g_spu_RXX[0xD5];
        ctrl = (ctrl & 0xFFCF) | 0x20;
        g_spu_RXX[0xD5] = ctrl;
        break;
    case 0:
        g_spu_reverb_work_sizes = 1;
        i = 0;
        while (g_spu_RXX[0xD3] != g_spu_adsr_dummy_1) {
            if (++i > 0xF00) {
                return -2;
            }
        }
        ctrl = g_spu_RXX[0xD5];
        ctrl |= 0x30;
        g_spu_RXX[0xD5] = ctrl;
        break;
    case 3:
        ctrl = (g_spu_reverb_work_sizes == 1) ? 0x30 : 0x20;
        i = 0;
        while ((g_spu_RXX[0xD5] & 0x30) != ctrl) {
            if (++i > 0xF00) {
                return -2;
            }
        }
        if (g_spu_reverb_work_sizes == 1) {
            *g_spu_reverb_type = (*g_spu_reverb_type & 0xF0FFFFFF) | 0x22000000;
        } else {
            *g_spu_reverb_type = (*g_spu_reverb_type & 0xF0FFFFFF) | 0x20000000;
        }
        val = va_arg(ap, u32);
        g_spu_dma_mode_val = val;
        val = va_arg(ap, u32);
        g_spu_dma_dir_val = (val / 64) + ((val & 0x3F) != 0);
        *g_spu_trans_dir = g_spu_dma_mode_val;
        *g_spu_in_transfer = (g_spu_dma_dir_val << 16) | 0x10;
        cmd = (g_spu_reverb_work_sizes == 1) ? 0x01000200 : 0x01000201;
        *g_spu_irq_enabled = cmd;
        break;
    }
    return 0;
}
