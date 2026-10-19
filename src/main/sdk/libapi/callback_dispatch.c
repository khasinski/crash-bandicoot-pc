#include "common.h"

/* Dispatch through slots at the offsets observed in the retail table.
 * Results describe the word left in v0; argument meanings remain unidentified.
 */
typedef s32 (*Callback)(void);
typedef void (*IrqHandler)(void);
typedef IrqHandler (*RegisterInterrupt)(s32, IrqHandler);
typedef s32 (*Exchange)(s32, s32);
typedef u16 *(*StateCallback)(void);

typedef struct {
    u32 word00;
    Exchange slot04;
    RegisterInterrupt slot08;
    Callback slot0C;
    StateCallback slot10;
    Exchange slot14;
    StateCallback slot18;
} Dispatch;

#define CRASH_EXT_SDK_LIBAPI_CALLBACK_DISPATCH
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_LIBAPI_CALLBACK_DISPATCH

s32 ResetCallback(void)
{
    return g_intr_ctrl_ptr->slot0C();
}

void * InterruptCallback(s32 index, IrqHandler callback)
{
    return g_intr_ctrl_ptr->slot08(index, callback);
}

s32 DMACallback(s32 arg0, s32 arg1)
{
    return g_intr_ctrl_ptr->slot04(arg0, arg1);
}

s32 VSyncCallback(s32 arg0)
{
    return g_intr_ctrl_ptr->slot14(0, arg0);
}

s32 VSyncCallbacks(s32 arg0, s32 arg1)
{
    return g_intr_ctrl_ptr->slot14(arg0, arg1);
}

u16 *StopCallback(void)
{
    return g_intr_ctrl_ptr->slot10();
}

u16 *RestartCallback(void)
{
    return g_intr_ctrl_ptr->slot18();
}
