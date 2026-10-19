#include "common.h"

/* Free the two allocation tables and update the fixed-point value from the current resource. */

#define CRASH_EXT_GOOL_ALLOC
#include "gool_externs.h"
#undef CRASH_EXT_GOOL_ALLOC

void GOOL_FreeAllocTable(void)
{
    LIBC_Free(Ram_Ptr(g_objects));
    LIBC_Free(Ram_Ptr(g_player));
}

s32 Gool_InitLid(void)
{
    s32 *entry = RAM_AT(s32, g_ns_page_pool);
    g_cur_display_flags[0] = entry[1] << 8;
    if (!entry[0])
        return -0xE;
    return -0xFF;
}
