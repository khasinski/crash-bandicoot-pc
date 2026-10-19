#include "common.h"

/* Query transfer completion, or wait for the event in blocking mode. */

s32 SpuIsTransferCompleted(s32 blocking) {
    s32 completed;

    if ((g_spu_transfer_mode_cur == 1) || (g_spu_trans_start_addr == 1)) {
        return 1;
    }
    completed = TestEvent(g_spu_dma_done_callback);
    if (blocking == 1) {
        if (completed == 0) {
            do {
            } while (TestEvent(g_spu_dma_done_callback) == 0);
        }
        completed = 1;
        goto record_completion;
    }
    if (completed == 1) {
record_completion:
        g_spu_trans_start_addr = completed;
    }
    return completed;
}
