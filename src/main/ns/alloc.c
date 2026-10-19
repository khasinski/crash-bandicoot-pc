#include "common.h"

/* Allocate a 64 KiB resource buffer; retain the original signed pointer/error test. */

s32 Ns_MallocPage(void)
{
    s32 ptr = (s32) LIBC_Malloc(0x10000);
    if (ptr == 0 || ptr >= -0xFE)
        return -0xF;
    return ptr;
}
