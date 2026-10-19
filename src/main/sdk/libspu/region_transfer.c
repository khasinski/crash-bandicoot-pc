#include "common.h"

/* Wait for this block, then account for the transfer quantum. */
static __inline__ u32 finish_chunk(u32 remaining, s32 event) {
    _spu_crwa_cb(event);
    return remaining - 0x400;
}

s32 SpuClearReverbWorkArea(u32 region) {
    volatile s32 saved_transfer_flag;
    s32 *region_entry;
    s32 *table_base;
    s32 region_address;
    s32 previous_mode;
    s32 transfer_address;
    s32 more_blocks;
    s32 restore_mode;
    s32 last_block;
    u32 block_size;
    u32 remaining;

    saved_transfer_flag = 0;
    restore_mode = 0;
    if ((region >= 0xAU) || (table_base = g_spu_reverb_preset_params, region_entry = table_base + region, (_SpuIsInAllocateArea_(*region_entry) != 0))) {
        return -1;
    }
    if (region == 0) {
        remaining = 0x10 << g_spu_trans_mode;
        transfer_address = 0xFFF0 << g_spu_trans_mode;
    } else {
        region_address = *region_entry;
        remaining = (0x10000 - region_address) << g_spu_trans_mode;
        transfer_address = region_address << g_spu_trans_mode;
    }
    previous_mode = g_spu_reverb_depth_l;
    if (previous_mode == 1) {
        g_spu_reverb_depth_l = 0;
        restore_mode = 1;
    }
    more_blocks = 1;
    if (g_spu_dma_done_cb != 0) {
        saved_transfer_flag = g_spu_dma_done_cb;
        g_spu_dma_done_cb = 0;
    }
    last_block = remaining < 0x401U;
    do {
        block_size = remaining;
        if (last_block == 0) {
            block_size = 0x400;
        } else {
            more_blocks = 0;
        }
        _spu_t(2, transfer_address);
        _spu_t(1);
        _spu_t(3, g_spu_reverb_param_studio_b, block_size);
        remaining = finish_chunk(remaining, g_spu_dma_done_callback);
        transfer_address += 0x400;
        last_block = remaining < 0x401U;
    } while (more_blocks != 0);
    if (restore_mode != 0) {
        g_spu_reverb_depth_l = previous_mode;
    }
    if (saved_transfer_flag != 0) {
        g_spu_dma_done_cb = saved_transfer_flag;
    }
    return 0;
}
