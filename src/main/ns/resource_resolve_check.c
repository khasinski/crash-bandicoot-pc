#include "common.h"
s32 Ns_ProbeAndOpen(s32 *handle) {
    s32 initial=handle[0];
    s32 resolved;
    if (!(initial&1)) return initial;
    Ns_PagePhysical(initial,handle[1]);
    resolved=handle[0];
    if (!(resolved&1)) return resolved;
    return -10;
}
