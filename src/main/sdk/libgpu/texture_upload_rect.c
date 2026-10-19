#include "gpu.h"

                /* extern */
                        /* extern */

s32 LoadTPage(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6) {
    GpuRect rect;
    s32 var_v0;

    rect.x = arg3;
    rect.h = (s16) arg6;
    rect.y = (s16) arg4;
    switch (arg1) {                                 /* irregular */
    case 0:
        var_v0 = arg5;
        if (arg5 < 0) {
            var_v0 = arg5 + 3;
        }
        rect.w = (s16) (var_v0 >> 2);
        break;
    case 1:
        rect.w = (s16) (arg5 / 2);
        break;
    case 2:
        rect.w = (s16) arg5;
        break;
    }
    LoadImage(&rect, arg0);
    return GetTPage(arg1, arg2, arg3, arg4) & 0xFFFF;
}
