#include "common.h"

typedef void (*Callback)(void);

void trapIntrDMA(void) {
    s32 i;
    u32 pending;

    pending = (*g_dma_dicr_reg >> 24) & 0x7F;
    while (pending != 0) {
        for (i = 0; pending != 0; i++, pending >>= 1) {
            if (i >= 7) {
                break;
            }
            if (pending & 1) {
                *g_dma_dicr_reg &= (1 << (i + 24)) | 0xFFFFFF;
                if (g_dma_cb_slots[i] != NULL) {
                    g_dma_cb_slots[i]();
                }
            }
        }
        pending = (*g_dma_dicr_reg >> 24) & 0x7F;
    }
    if ((*g_dma_dicr_reg & 0xFF000000) == 0x80000000 || (*g_dma_dicr_reg & 0x8000)) {
        BIOS_Printf(g_str_dma_callback_dispatch_dma_bus_error_code, *g_dma_dicr_reg);
        for (i = 0; i < 7; i++) {
            BIOS_Printf(g_str_dma_callback_dispatch_madr_d_08x_n, i, g_dma_chcr_base_reg[i * 4]);
        }
    }
}

void * setIntrDMA(s32 index, Callback callback) {
    Callback previous = g_dma_cb_slots[index];

    if (callback != previous) {
        if (callback != NULL) {
            g_dma_cb_slots[index] = callback;
            *g_dma_dicr_reg = ((*g_dma_dicr_reg & 0xFFFFFF) | 0x800000) | (1 << (index + 16));
        } else {
            g_dma_cb_slots[index] = NULL;
            *g_dma_dicr_reg = ((*g_dma_dicr_reg & 0xFFFFFF) | 0x800000) & ~(1 << (index + 16));
        }
    }
    return previous;
}

void dma_memclr(Callback *table, s32 count) {
    while (--count != -1) {
        *table++ = NULL;
    }
}
