#include "common.h"

/* For entries matching the high-bit group ID, set bit 3 in the indexed state table. */

void GOOL_InitLevelSpawns(u32 group)
{
    u16 *entry = g_level_spawns;
    while (*entry) {
        if ((*entry >> 9) == group)
            g_spawns[*entry & 0x1FF] |= 8;
        entry++;
    }
}
