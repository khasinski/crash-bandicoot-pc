#include "common.h"

typedef struct {
    u32 mask;
    s32 mode;
    s16 depth_left;
    s16 depth_right;
    s32 delay;
    s32 feedback;
} SpuReverbAttr;

typedef struct {
    u32 mask;
    s16 values[32];
} SpuReverbRegisters;

#define CRASH_EXT_SDK_LIBSPU_REVERB_ATTR_SET
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_LIBSPU_REVERB_ATTR_SET

s32 SpuSetReverbModeParam(SpuReverbAttr *arg0)
{
    SpuReverbRegisters param;
    s32 clear_mode;
    u32 mask;
    s32 all;
    s32 set_mode;
    s32 set_delay;
    s32 set_feedback;
    s32 was_on;
    s32 mode;
    s32 delay_offset;
    u16 ctrl;

    was_on = 0;
    set_mode = 0;
    set_delay = 0;
    clear_mode = 0;
    mask = arg0->mask;
    set_feedback = 0;
    all = (mask == 0);
    param.mask = 0;
    if (all || (mask & 1)) {
        mode = arg0->mode;
        if (mode & 0x100) {
            mode &= ~0x100;
            clear_mode = 1;
        }
        if ((u32)mode >= 0xA || _SpuIsInAllocateArea_(g_spu_reverb_preset_params[mode]) != 0) {
            return -1;
        }
        set_mode = 1;
        g_spu_rev_param = mode;
        {
            u8 *dst = (u8 *)&param;
            s32 i = 0x43;
            s32 idx = *(volatile s32 *)&g_spu_rev_param;
            u8 *src;
            g_spu_reverb_param_room = g_spu_reverb_preset_params[idx];
            src = (u8 *)&g_spu_reverb_reg_offsets[idx];
            do {
                *dst++ = *src++;
            } while (--i != -1);
        }
        switch (g_spu_rev_param) {
        case 7:
            g_spu_reverb_mask_hi_cur = 0x7F;
            g_spu_reverb_mask_lo_cur = 0x7F;
            break;
        case 8:
            g_spu_reverb_mask_hi_cur = 0;
            g_spu_reverb_mask_lo_cur = 0x7F;
            break;
        default:
            g_spu_reverb_mask_hi_cur = 0;
            g_spu_reverb_mask_lo_cur = 0;
            break;
        }
    }
    if (all || (mask & 8)) {
        switch (g_spu_rev_param) {
        case 7:
        case 8:
            set_delay = 1;
            if (set_mode == 0) {
                u8 *dst = (u8 *)&param;
                s32 i = 0x43;
                u8 *src = (u8 *)&g_spu_reverb_reg_offsets[*(volatile s32 *)&g_spu_rev_param];
                do {
                    *dst++ = *src++;
                } while (--i != -1);
                param.mask = 0x0C011C00;
            }
            g_spu_reverb_mask_lo_cur = arg0->delay;
            param.values[10] = ((g_spu_reverb_mask_lo_cur << 13) / 127) - param.values[0];
            delay_offset = (g_spu_reverb_mask_lo_cur << 12) / 127;
            param.values[11] = delay_offset - param.values[1];
            param.values[12] = delay_offset + param.values[13];
            param.values[16] = delay_offset + param.values[17];
            param.values[26] = delay_offset + param.values[28];
            param.values[27] = delay_offset + param.values[29];
            break;
        default:
            g_spu_reverb_mask_lo_cur = 0;
            break;
        }
    }
    if (all || (mask & 0x10)) {
        switch (g_spu_rev_param) {
        case 7:
        case 8:
            set_feedback = 1;
            if (set_mode == 0) {
                if (set_delay == 0) {
                    u8 *dst = (u8 *)&param;
                    s32 i = 0x43;
                    u8 *src = (u8 *)&g_spu_reverb_reg_offsets[*(volatile s32 *)&g_spu_rev_param];
                    do {
                        *dst++ = *src++;
                    } while (--i != -1);
                    param.mask = 0x80;
                } else {
                    param.mask |= 0x80;
                }
            }
            g_spu_reverb_mask_hi_cur = arg0->feedback;
            param.values[7] = (g_spu_reverb_mask_hi_cur * 33024) / 127;
            break;
        default:
            g_spu_reverb_mask_hi_cur = 0;
            break;
        }
    }
    if (set_mode != 0) {
        was_on = (g_spu_RXX[0xD5] >> 7) & 1;
        if (was_on != 0) {
            ctrl = g_spu_RXX[0xD5];
            ctrl &= 0xFF7F;
            g_spu_RXX[0xD5] = ctrl;
        }
        goto clear_depth;
    }
    if (all || (mask & 2)) {
        g_spu_RXX[0xC2] = arg0->depth_left;
        g_spu_reverb_vl_cur = arg0->depth_left;
    }
    if (all || (mask & 4)) {
        g_spu_RXX[0xC3] = arg0->depth_right;
        g_spu_reverb_vr_cur = arg0->depth_right;
    }
    goto after_depth;
clear_depth:
    g_spu_RXX[0xC2] = 0;
    g_spu_RXX[0xC3] = 0;
    g_spu_reverb_vl_cur = 0;
    g_spu_reverb_vr_cur = 0;
after_depth:
    if (set_mode || set_delay || set_feedback) {
        _spu_setReverbAttr(&param);
    }
    if (clear_mode) {
        SpuClearReverbWorkArea(g_spu_rev_param);
    }
    if (set_mode) {
        _spu_FsetRXX(0xD1, g_spu_reverb_param_room, 0);
        if (was_on) {
            ctrl = g_spu_RXX[0xD5];
            ctrl |= 0x80;
            g_spu_RXX[0xD5] = ctrl;
        }
    }
    return 0;
}
