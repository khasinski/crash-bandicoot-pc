#include "common.h"

s32 SpuInitMalloc(s32 capacity, SpuBlock *entries)
{
    s32 result = capacity;
    if (result > 0) {
        entries->address = 0x40001010;
        g_spu_memList = entries;
        g_spu_alloc_num = 0;
        g_spu_tsa_shadow = result;
        entries->size = (s32)(((u32)0x10000 << g_spu_trans_mode) - 0x1010);
    } else {
        return 0;
    }
    return result;
}
