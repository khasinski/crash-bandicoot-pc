#include "common.h"

/* Initialise SPU state and its 24 cached voice values. */

/* Partial memory view: element zero precedes the 24 cached voice values. */

void _SpuInit(s32 arg0) {
    u32 level;
    u16 *voice;
    s32 remaining;

    ResetCallback();
    _spu_init(arg0);
    level = 0xC000;
    if (arg0 == 0) {
        remaining = 0x17;
        voice = &g_spu_reverb_master_flag[24];
        do {
            *voice = level;
            remaining -= 1;
            voice--;
        } while (remaining >= 0);
    }
    SpuStart(level);
    g_spu_reverb_work_reserved = 0;
    g_spu_reverb_mode_type = 0;
    g_spu_rev_param = 0;
    g_spu_reverb_vl_cur = 0;
    g_spu_reverb_vr_cur = 0;
    g_spu_reverb_mask_lo_cur = 0;
    g_spu_reverb_mask_hi_cur = 0;
    g_spu_reverb_param_room = g_spu_reverb_preset_params[0];
    _spu_FsetRXX(0xD1, g_spu_reverb_preset_params[0], 0);
    g_spu_transfer_mode_cur = 0;
    g_spu_reverb_depth_l = 0;
    g_spu_irq_callback = 0;
}
