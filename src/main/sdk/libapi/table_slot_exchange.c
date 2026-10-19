#include "common.h"
u32 setIntrVSync(s32 index,u32 value) {
    u32 *table=g_vsync_cb_slots;
    u32 *slot=table+index;
    u32 previous=*slot;
    if (value!=previous) *slot=value;
    return previous;
}
