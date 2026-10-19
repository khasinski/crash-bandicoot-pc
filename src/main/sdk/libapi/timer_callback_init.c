#include "common.h"

typedef void (*Callback)(void);
typedef Callback (*Registrar)(s32,Callback);
void * startIntrVSync(void) {
    *g_vsync_rcnt1_mode_reg = 0x107;
    g_vsync_cb_active_cnt = 0;
    rcnt_memclr(g_vsync_cb_slots, 8);
    InterruptCallback(0, &trapIntrVSync);
    return &setIntrVSync;
}
