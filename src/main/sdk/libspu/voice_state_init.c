#include "common.h"
/* Retail passes 1; this callee does not consume incoming arguments. */
#define CRASH_EXT_SDK_LIBSPU_VOICE_STATE_INIT
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_LIBSPU_VOICE_STATE_INIT
void SpuVmInit(s32 arg0) {
    u32 index;
    u16 register_offset;
    s32 temp_v0;
    u32 temp_a0;
    u16 *temp_a1;

    SsSetMono(0);
    g_spu_vm_damper_mask = 0;
    SpuInitMalloc(0x20, &g_spu_vm_malloc_buf);
    index = 0;
    do {
        g_midi_seq_table[(index & 0xFFFF)] = 0;
        index += 1;
    } while ((u32) (index & 0xFFFF) < 0xC0U);
    index = 0;
    do {
        g_card_ev_table[(index & 0xFFFF)] = 0;
        index += 1;
    } while ((u32) (index & 0xFFFF) < 0x18U);
    g_vab_open_count = 0;
    index = 0;
    do {
        g_midi_tracks[(index & 0xFFFF)] = 0;
        index += 1;
    } while ((u32) (index & 0xFFFF) < 0x10U);
    temp_a0 = arg0 & 0xFF;
    if (temp_a0 >= 0x18U) {
        gp_spawn_count = 0x18;
    } else {
        gp_spawn_count = temp_a0;
    }
    index = 0;
    if (gp_spawn_count != 0) {
        do {
            register_offset = (index & 0xFFFF) * 8;
            temp_v0 = (index & 0xFFFF) * 0x34;
            *(s16 *)(g_spu_vm_voice_age + temp_v0) = 0x18;
            *(s16 *)(g_spu_vm_voice_table + temp_v0) = 0xFF;
            g_spu_vm_voice_auto_flag[temp_v0] = 0;
            *(s16 *)(g_spu_vm_voice_pitch + temp_v0) = 0;
            *(s16 *)(g_spu_vm_voice_envx + temp_v0) = 0;
            *(s16 *)(g_spu_vm_voice_seq_sep + temp_v0) = -1;
            *(s16 *)(g_spu_vm_voice_prog + temp_v0) = 0;
            *(s16 *)(g_spu_vm_voice_tone_prog + temp_v0) = 0;
            *(s16 *)(g_spu_vm_voice_tone_idx + temp_v0) = 0xFF;
            *(s16 *)(g_spu_vm_voice_vol + temp_v0) = 0;
            g_spu_vm_voice_pan[temp_v0] = 0x40;
            *(s16 *)(g_spu_vm_voice_auto_vol_cnt + temp_v0) = 0;
            *(s16 *)(g_spu_vm_voice_auto_vol_step + temp_v0) = 0;
            *(s16 *)(g_spu_vm_voice_auto_vol_cur + temp_v0) = 0;
            *(s16 *)(g_spu_vm_voice_auto_vol_dst + temp_v0) = 0;
            *(s16 *)(g_spu_vm_voice_auto_pan_step + temp_v0) = 0;
            *(s16 *)(g_spu_vm_voice_auto_pan_cur + temp_v0) = 0;
            *(s16 *)(g_spu_vm_voice_auto_pan_dst + temp_v0) = 0;
            *(s16 *)(g_spu_vm_voice_auto_pan_start + temp_v0) = 0;
            *(s16 *)(g_spu_vm_voice_auto_vol_start + temp_v0) = 0;
            *(s16 *)(g_spu_vm_voice_auto_pan_cnt + temp_v0) = 0;
            temp_a1 = (u16 *)(((register_offset & 0xFFFF) * 2) + (u32)g_snd_tempo);
            temp_a1[0] = 0;
            temp_a1[1] = 0;
            temp_a1[3] = 0x200;
            temp_a1[2] = 0x1000;
            temp_a1[4] = 0x80FF;
            temp_a1[5] = 0x4000;
            g_spu_vm_last_voice[0] = index;
            SpuVmKeyOffNow(1);
            index += 1;
            } while ((u32) (index & 0xFFFF) < (u8) gp_spawn_count);
    }
    g_spu_vm_key_on_lo = 0;
    g_spu_vm_key_on_hi = 0;
    g_spu_vm_active_mask_lo = 0;
    g_spu_vm_noise_mask_lo = 0;
    g_spu_vm_noise_mask_hi = 0;
    g_spu_reverb_attr = 0;
    g_spu_reverb_depth_left = 0x3FFF;
    g_spu_reverb_depth_right = 0x3FFF;
    g_spu_reverb_mode_cur = 0;
    g_spu_vm_flush_cond = 0;
    g_spu_vm_mono_flag = 0;
    g_vab_cur_prog_count = 0x80;
    SpuVmFlush();
}
