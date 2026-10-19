#include "common.h"
#define CRASH_EXT_ENGINE_ALLOCATE_BUFFERS
#include "engine_externs.h"
#undef CRASH_EXT_ENGINE_ALLOCATE_BUFFERS

typedef struct DrawBuffer {
    s16 word_0;
    s16 count;
} DrawBuffer;

s32 Slst_Init(void)
{
    s32 allocation_size;
    s32 capacity;
    DrawBuffer *first;
    DrawBuffer *second;

    if (gp_pbak_mode == 0x19 || gp_pbak_mode == 0x38) {
        capacity = 0xA32;
    } else {
        capacity = 0x5B4;
    }
    allocation_size = (capacity * 2) + 4;
    first = LIBC_Malloc(allocation_size);
    gp_level_entry = Ram_Addr(first);
    gp_slst_poly_list_a = Ram_Addr(first);
    if (first != NULL) {
        first->count = 0;
        second = LIBC_Malloc(allocation_size);
        gp_slst_poly_list_b = Ram_Addr(second);
        if (second != NULL) {
            second->count = 0;
            return -0xFF;
        }
    }
    return -0xF;
}
