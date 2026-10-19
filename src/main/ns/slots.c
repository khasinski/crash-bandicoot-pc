#include "common.h"

/* Clear the halfword at offset 0x30 for a slot with stride 0x2C. */

void Ns_KillPage(u8 *table, s32 index)
{
    s16 *slot_state = (s16 *)(table + index * 0x2C + 0x30);
    if (*slot_state)
        *slot_state = 0;
}
