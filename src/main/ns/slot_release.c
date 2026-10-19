#include "resource.h"
typedef ResourceSlotStateView Slot;

s32 Ns_TexturePageFree(s32 arg0) {
    s16 temp_v1;

    temp_v1 = g_slst_buf1[arg0].state;
    switch (temp_v1) {                              /* irregular */
    case 20:
        *g_slst_buf1[arg0].id_pointer = g_slst_buf1[arg0].id;
        g_ns_page_status_map[g_slst_buf1[arg0].id >> 1] = -0x12;
        /* fallthrough */
    case 21:
        g_slst_buf1[arg0].state = 1;
        break;
    }
    return -0xE;
}

s32 Ns_AudioPageAllocate(s32 arg0, s32 arg1) {
    s32 i;
    ResourceSlot *slot;

    for (i = 7; i >= 0; i--) {
        slot = &g_spu_slots[i];
        if ((s16)slot->state == 1 && g_spu_slots[i].word06 == arg0) {
            if (arg1 == -1 || arg1 == g_spu_slots[i].word08) {
                return i;
            }
        }
    }
    return -12;
}
