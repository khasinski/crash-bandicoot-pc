#include "common.h"
#include "gpu.h"

/* --- startIntr --- */

typedef void (*Callback)(void);

typedef struct {
    u16 initialized;
    u16 in_interrupt;
    Callback callbacks[11];
    u16 mask;
    u16 saved_imask;
    s32 saved_dpcr;
    u32 context[12];
    u32 stack[0x400];
} IntrState;

#define CRASH_EXT_SDK_KERNEL_INTR_INIT_0
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_KERNEL_INTR_INIT_0

typedef struct {
    u32 word00;
    s32 slot04;
    s32 slot08;
    s32 slot0C;
    s32 slot10;
    s32 slot14;
    s32 slot18;
} Dispatch;

#define CRASH_EXT_SDK_KERNEL_INTR_INIT_1
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_KERNEL_INTR_INIT_1

void *startIntr(void) {
    void *result;

    if (g_event_handlers.initialized == 0) {
        *g_intr_i_stat_reg = *g_intr_i_mask_reg = 0;
        *g_intr_dpcr_reg = 0x33333333;
        intr_memclr(&g_event_handlers, sizeof(IntrState) / 4);
        if (setjmp(g_event_handlers.context) != 0) {
            trapIntr();
        }
        g_event_handlers.context[1] = (u32)&g_event_handlers.stack[0x3EC];
        HookEntryInt(g_event_handlers.context);
        g_event_handlers.initialized = 1;
        g_intr_ctrl_ptr->slot14 = startIntrVSync();
        g_intr_ctrl_ptr->slot04 = startIntrDMA();
        _96_remove(g_intr_ctrl_ptr);
        ExitCriticalSection();
        result = &g_event_handlers;
    } else {
        result = NULL;
    }
    return result;
}

