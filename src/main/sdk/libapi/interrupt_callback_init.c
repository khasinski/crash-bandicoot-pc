#include "common.h"

typedef void (*Callback)(void);
typedef Callback (*Registrar)(s32,Callback);
void * startIntrDMA(void) {
    dma_memclr(g_dma_cb_slots, 8);
    *g_dma_dicr_reg = 0;
    InterruptCallback(3, &trapIntrDMA);
    return &setIntrDMA;
}
