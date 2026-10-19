#include "common.h"
#include "voice_attributes.h"

void SpuGetVoiceAttr(VoiceAttributes *arg0) {
    s32 temp_v0_3;
    u32 var_a0;
    s16 var_a1;
    u16 adsr1;
    u16 adsr2;
    s32 word_index;
    u16 left_bias;
    u16 right_bias;
    s32 temp_s2;
    s32 var_s1;
    s32 var_v1;
    u16 *temp_v0_2;
    u16 var_a2;
    u16 var_a3;
    u32 temp_v1_4;
    u32 temp_v1_5;
    volatile u16 *temp_v0;
    volatile u16 *temp_v1;
    volatile u16 *temp_v1_6;

    var_s1 = -1;
    for (var_v1 = 0; var_v1 < 24; var_v1++) {
        if ((1 << var_v1) & arg0->voice) {
            var_s1 = var_v1;
            break;
        }
    }
    var_a1 = 0;
    if (var_s1 != -1) {
        temp_v1 = g_spu_RXX + (var_s1 * 8);
        var_a2 = temp_v1[0];
        var_a3 = temp_v1[1];
        temp_s2 = var_s1 * 8;
        word_index = var_s1 * 2;
        if (var_a2 & 0x8000) {
            switch (var_a2 & 0xF000) {
            case 0x8000: var_a1 = 1; break;
            case 0x9000: var_a1 = 2; break;
            case 0xA000: var_a1 = 3; break;
            case 0xB000: var_a1 = 4; break;
            case 0xC000: var_a1 = 5; break;
            case 0xD000: var_a1 = 6; break;
            case 0xE000: var_a1 = 7; break;
            case 0xF000: var_a1 = 7; break;
            }
            var_a2 &= 0xFFF;
        }
        var_a0 = 0;
        if (var_a3 & 0x8000) {
            switch (var_a3 & 0xF000) {
            case 0x8000: var_a0 = 1; break;
            case 0x9000: var_a0 = 2; break;
            case 0xA000: var_a0 = 3; break;
            case 0xB000: var_a0 = 4; break;
            case 0xC000: var_a0 = 5; break;
            case 0xD000: var_a0 = 6; break;
            case 0xE000: var_a0 = 7; break;
            case 0xF000: var_a0 = 7; break;
            }
            var_a3 &= 0xFFF;
        }
        temp_v1_4 = var_a2 & 0xFFFF;
        if (temp_v1_4 < 0x4000U) {
            arg0->vol_left = var_a2;
        } else {
            left_bias = 0x8000;
            arg0->vol_left = (u16) (temp_v1_4 - (u16)left_bias);
        }
        temp_v1_5 = var_a3 & 0xFFFF;
        if (temp_v1_5 < 0x4000U) {
            arg0->vol_right = var_a3;
        } else {
            right_bias = 0x8000;
            arg0->vol_right = (u16) (temp_v1_5 - (u16)right_bias);
        }
        arg0->volmode_right = var_a0;
        arg0->volmode_left = var_a1;
        temp_v0 = g_spu_RXX + word_index;
        arg0->volx_left = temp_v0[256];
        arg0->volx_right = temp_v0[257];
        arg0->pitch = g_spu_RXX[temp_s2 + 2];
        temp_v0_2 = g_spu_pitch_scale_tab + var_s1;
        temp_v0_3 = _spu_pitch2note((u16) *temp_v0_2 >> 8, *(u8 *)temp_v0_2, arg0->pitch);
        if (temp_v0_3 >= 0) {
            arg0->note = temp_v0_3;
        } else {
            arg0->note = 0;
        }
        arg0->sample_note = (u16) g_spu_pitch_scale_tab[var_s1];
        temp_v1_6 = g_spu_RXX + temp_s2;
        arg0->envx = temp_v1_6[6];
        arg0->addr = (s32) (temp_v1_6[3] << g_spu_trans_mode);
        arg0->loop_addr = (s32) (temp_v1_6[7] << g_spu_trans_mode);
        adsr1 = temp_v1_6[4];
        adsr2 = temp_v1_6[5];
        arg0->a_mode = (adsr1 & 0x80) ? 5 : 1;
        arg0->s_mode = (adsr2 & 0x300) ? 7 : ((adsr2 & 0x200) ? 5 : ((adsr2 & 0x100) ? 3 : 1));
        arg0->r_mode = (adsr2 & 0x20) ? 7 : 3;
        arg0->ar = (s16) (adsr1 >> 8);
        arg0->dr = (s16) ((u32) (adsr1 & 0xF0) >> 4);
        arg0->sr = (s16) ((adsr2 >> 6) & 0xFCFF);
        arg0->rr = (s16) (adsr2 & 0x1F);
        arg0->sl = (s16) (adsr1 & 0xF);
        arg0->adsr1 = adsr1;
        arg0->adsr2 = adsr2;
    }
}
