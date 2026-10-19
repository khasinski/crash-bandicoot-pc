#include "common.h"

                          /* extern */
                                 /* extern */
                                  /* extern */

void PadInit(s32 arg0) {
    g_pad_init_mode = arg0;
    g_pad_mode_state = -1;
    ResetCallback();
    PAD_init(0x20000001, &g_pad_mode_state);
    ChangeClearPAD(0);
}
