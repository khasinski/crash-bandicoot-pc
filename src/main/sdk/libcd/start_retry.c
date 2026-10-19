#include "common.h"
typedef void (*Callback)(void);

s32 CdInit(void)
{
    s32 retries = 4;
    while (CdReset(1) != 1) {
        if (--retries == -1) {
            BIOS_Printf(g_str_start_retry_cdinit_init_failed_n);
            return 0;
        }
    }
    CdSyncCallback(def_cbsync);
    CdReadyCallback(def_cbready);
    CdDataCallback(def_cbread);
    return 1;
}
