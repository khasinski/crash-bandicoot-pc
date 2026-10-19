#include "gpu.h"
#define CRASH_EXT_SDK_LIBGPU_ORDERING_TABLE
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_LIBGPU_ORDERING_TABLE

u32 *ClearOTag(u32 *ot,s32 count) {
    if (g_gpu_debug_level >= 2) g_gpu_ot_status(g_str_ordering_table_clearotag_08x_d_n,ot,count);
    count--;
    if (count) {
        u32 address_mask = 0x00ffffff;
        u32 length_mask = 0xff000000;
        do {
            u32 *next = ot + 1;
            u32 tag;
            count--;
            ((u8 *)ot)[3] = 0;
            tag = *ot;
            tag &= length_mask;
            *ot = tag | ((u32)next & address_mask);
            ot = next;
        } while (count);
    }
    *ot = (u32)&g_gpu_ot_clear_step & 0x00ffffff;
    return ot;
}
