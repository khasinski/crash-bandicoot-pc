#include "common.h"

typedef void (*Callback)(void);
/* The interrupt tick counter can be read while callbacks are running. */
void trapIntrVSync(void) {
    s32 i = 0;
    Callback *slot = g_vsync_cb_slots;
    g_vsync_cb_active_cnt++;
    do {
        Callback callback = *slot++;
        if (callback != NULL) callback();
        i++;
    } while (i < 8);
}
