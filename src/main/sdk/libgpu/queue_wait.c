#include "common.h"

s32 _sync(s32 poll) {
    s32 pending;

    if (poll == 0) {
        set_alarm();
        while (g_gpu_q_write != g_gpu_q_read) {
            _exeque();
            if (get_alarm() != 0) return -1;
        }
        while ((*g_gpu_dma2_chcr_reg & 0x01000000) || !(*g_gpu_stat_reg & 0x04000000)) {
            if (get_alarm() != 0) return -1;
        }
        return 0;
    }
    pending = (g_gpu_q_write - g_gpu_q_read) & 0x3F;
    if (pending != 0) {
        _exeque();
    }
    if ((*g_gpu_dma2_chcr_reg & 0x01000000) || !(*g_gpu_stat_reg & 0x04000000)) {
        if (pending == 0) return 1;
        return pending;
    }
    return pending;
}
