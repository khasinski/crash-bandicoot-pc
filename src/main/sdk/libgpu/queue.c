#include "common.h"
#include "gpu.h"

typedef struct {
    u8 version;
    volatile u8 mode;
    u8 debug;
    u8 reverse;
    s16 w;
    s16 h;
    volatile s32 active;
    void (* volatile callback)(void);
} GpuState;

#define CRASH_EXT_SDK_LIBGPU_QUEUE_0
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_LIBGPU_QUEUE_0

/* --- _addque2 & _exeque --- */

typedef struct {
    void (*func)(void *, s32);
    void *buf;
    s32 arg;
    u32 data[21];
} GpuQueueEntry;

typedef struct {
    void (*func)(void *, s32);
    void *buf;
    s32 arg;
} GpuActiveEntry;

#define CRASH_EXT_SDK_LIBGPU_QUEUE_1
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_LIBGPU_QUEUE_1

s32 _addque2(void (*func)(void *, s32), u32 *buf, s32 size, s32 arg)
{
    s32 i;
    u32 *src;

    set_alarm();
    while (((g_gpu_q_write + 1) & 0x3F) == g_gpu_q_read) {
        if (get_alarm() != 0) {
            return -1;
        }
        _exeque();
    }

    g_gpu_queue_busy[0] = SetIntrMask(0);
    g_gpu_mode.active = 1;
    if (g_gpu_mode.mode == 0 ||
        (g_gpu_q_write == g_gpu_q_read && !(*g_gpu_dma2_chcr_reg & 0x01000000) && g_gpu_mode.callback == 0)) {
        while (!(*g_gpu_stat_reg & 0x04000000)) {
        }
        func(buf, arg);
        g_gpu_queue_enabled.func = func;
        g_gpu_queue_enabled.buf = buf;
        g_gpu_queue_enabled.arg = arg;
        SetIntrMask(g_gpu_queue_busy[0]);
        return 0;
    }

    DMACallback(2, _exeque);
    if (size != 0) {
        for (i = 0; i < size / 4; i++) {
            g_gpu_queue_entries[g_gpu_q_write].data[i] = buf[i];
        }
        g_gpu_queue_entries[g_gpu_q_write].buf = (void *)g_gpu_queue_entries[g_gpu_q_write].data;
    } else {
        g_gpu_queue_entries[g_gpu_q_write].buf = buf;
    }
    g_gpu_queue_entries[g_gpu_q_write].arg = arg;
    g_gpu_queue_entries[g_gpu_q_write].func = func;
    g_gpu_q_write = (g_gpu_q_write + 1) & 0x3F;
    SetIntrMask(g_gpu_queue_busy[0]);
    _exeque();
    return (g_gpu_q_write - g_gpu_q_read) & 0x3F;
}

s32 _exeque(void)
{
    if (*g_gpu_dma2_chcr_reg & 0x01000000) {
        return 1;
    }
    g_gpu_queue_in_exec[0] = SetIntrMask(0);
    while (g_gpu_q_write != g_gpu_q_read && !(*g_gpu_dma2_chcr_reg & 0x01000000)) {
        if (((g_gpu_q_read + 1) & 0x3F) == g_gpu_q_write && g_gpu_mode.callback == 0) {
            DMACallback(2, 0);
        }
        while (!(*g_gpu_stat_reg & 0x04000000)) {
        }
        g_gpu_queue_entries[g_gpu_q_read].func(g_gpu_queue_entries[g_gpu_q_read].buf, g_gpu_queue_entries[g_gpu_q_read].arg);
        g_gpu_queue_enabled.func = g_gpu_queue_entries[g_gpu_q_read].func;
        g_gpu_queue_enabled.buf = g_gpu_queue_entries[g_gpu_q_read].buf;
        g_gpu_queue_enabled.arg = g_gpu_queue_entries[g_gpu_q_read].arg;
        g_gpu_q_read = (g_gpu_q_read + 1) & 0x3F;
    }
    SetIntrMask(g_gpu_queue_in_exec[0]);
    if (g_gpu_q_write == g_gpu_q_read && !(*g_gpu_dma2_chcr_reg & 0x01000000) && g_gpu_mode.active && g_gpu_mode.callback) {
        g_gpu_mode.active = 0;
        g_gpu_mode.callback();
    }
    return (g_gpu_q_write - g_gpu_q_read) & 0x3F;
}

