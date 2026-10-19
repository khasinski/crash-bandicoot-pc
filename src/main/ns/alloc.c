#include "common.h"

/* Allocate a 64 KiB resource buffer. Returns its PS1 address, which is
 * negative as s32, or -0xF; the original signed pointer/error test remains. */

s32 Ns_MallocPage(void)
{
    s32 ptr = (s32)Ram_Addr(LIBC_Malloc(0x10000));
    if (ptr == 0 || ptr >= -0xFE)
        return -0xF;
    return ptr;
}
