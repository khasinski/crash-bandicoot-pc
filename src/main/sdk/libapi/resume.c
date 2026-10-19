#include "common.h"

/* Restore the saved interrupt context and hardware masks if callbacks are paused.
 * The partial state view begins at its flag and includes context bytes at 0x38.
 */
#define CRASH_EXT_SDK_LIBAPI_RESUME
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_LIBAPI_RESUME

u16 *restartIntr(void)
{
    u16 *result;

    if (*g_event_handlers == 0) {
        HookEntryInt(&g_event_handlers[0x1C]);
        *g_event_handlers = 1;
        *g_intr_i_mask_reg = g_intr_saved_mask;
        *g_intr_dpcr_reg = g_intr_dma_saved_dpcr;
        ExitCriticalSection();
        result = g_event_handlers;
    } else {
        result = NULL;
    }
    return result;
}

void intr_memclr(s32 *table, s32 count)
{
    while (--count != -1) {
        *table++ = 0;
    }
}

