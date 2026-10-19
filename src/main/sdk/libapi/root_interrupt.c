#include "common.h"

typedef void (*Callback)(void);

typedef struct {
    u16 initialized;
    u16 in_interrupt;
    Callback callbacks[11];
    u16 mask;
    u16 saved_imask;
    s32 saved_dpcr;
    u32 context[12];
    u32 stack[0x3F0];
} IntrState;

#define CRASH_EXT_SDK_LIBAPI_ROOT_INTERRUPT
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_LIBAPI_ROOT_INTERRUPT

void trapIntr(void)
{
    u16 pending;
    s32 i;

    if (g_event_handlers.initialized == 0) {
        BIOS_Printf(g_str_root_interrupt_unexpected_interrupt_04x_n, *g_intr_i_stat_reg);
        ReturnFromException();
    }
    g_event_handlers.in_interrupt = 1;
    pending = (g_event_handlers.mask & *g_intr_i_stat_reg) & *g_intr_i_mask_reg;
    while (pending != 0) {
        for (i = 0; pending != 0; i++, pending >>= 1) {
            if (i >= 11) {
                break;
            }
            if (pending & 1) {
                *g_intr_i_stat_reg = ~(1 << i);
                if (g_event_handlers.callbacks[i] != NULL) {
                    g_event_handlers.callbacks[i]();
                }
            }
        }
        pending = (g_event_handlers.mask & *g_intr_i_stat_reg) & *g_intr_i_mask_reg;
    }
    if ((*g_intr_i_stat_reg & *g_intr_i_mask_reg) != 0) {
        if (g_intr_hook_installed++ > 0x800) {
            BIOS_Printf(g_str_root_interrupt_intr_timeout_04x_04x, *g_intr_i_stat_reg, *g_intr_i_mask_reg);
            g_intr_hook_installed = 0;
            *g_intr_i_stat_reg = 0;
        }
    } else {
        g_intr_hook_installed = 0;
    }
    g_event_handlers.in_interrupt = 0;
    ReturnFromException();
}

Callback setIntr(s32 index, Callback callback)
{
    Callback previous = g_event_handlers.callbacks[index];
    s32 imask;

    if (callback != previous && g_event_handlers.initialized != 0) {
        imask = *g_intr_i_mask_reg;
        *g_intr_i_mask_reg = 0;
        if (callback != NULL) {
            imask |= 1 << index;
            g_event_handlers.callbacks[index] = callback;
            g_event_handlers.mask |= 1 << index;
        } else {
            imask &= ~(1 << index);
            g_event_handlers.callbacks[index] = NULL;
            g_event_handlers.mask &= ~(1 << index);
        }
        if (index == 0) {
            ChangeClearPAD(callback == NULL);
            ChangeClearRCnt(3, callback == NULL);
        }
        if (index == 4) {
            ChangeClearRCnt(0, callback == NULL);
        }
        if (index == 5) {
            ChangeClearRCnt(1, callback == NULL);
        }
        if (index == 6) {
            ChangeClearRCnt(2, callback == NULL);
        }
        *g_intr_i_mask_reg = imask;
    }
    return previous;
}

void *stopIntr(void)
{
    void *result;

    if (g_event_handlers.initialized != 0) {
        EnterCriticalSection();
        g_event_handlers.saved_imask = *g_intr_i_mask_reg;
        g_event_handlers.saved_dpcr = *g_intr_dpcr_reg;
        *g_intr_i_stat_reg = *g_intr_i_mask_reg = 0;
        *g_intr_dpcr_reg &= 0x77777777;
        ResetEntryInt();
        g_event_handlers.initialized = 0;
        result = &g_event_handlers;
    } else {
        result = NULL;
    }
    return result;
}
