#include "common.h"

/* Mark a resource slot free and place the -0x12 sentinel in its ID-indexed table entry. */

#include "resource.h"

void Ns_PageDelete(ResourceSlot *slot)
{
    slot->state = 1;
    g_ns_page_status_map[slot->id >> 1] = -0x12;
}
