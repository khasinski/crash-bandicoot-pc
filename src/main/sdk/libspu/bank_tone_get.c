#include "common.h"
                          /* extern */

s32 SsUtGetVagAtr(s16 bank, s16 program, s32 tone, ToneAttributes *result) {
    s32 temp_v1;
    ToneAttributes *temp_v1_2;

    if (g_midi_tracks[bank] == 1) {
        SpuVmVSetUp(bank, program);
        temp_v1 = (s32) (((u32)tone + (g_spu_vm_cur_prog_tones * 0x10)) << 0x10) >> 0xB;
        result->prior = (u8) ((ToneAttributes *)(temp_v1 + (u32)g_ns_vpage_pool))->prior;
        result->mode = (u8) ((ToneAttributes *)(temp_v1 + (u32)g_ns_vpage_pool))->mode;
        result->vol = (u8) ((ToneAttributes *)(temp_v1 + (u32)g_ns_vpage_pool))->vol;
        result->pan = (u8) ((ToneAttributes *)(temp_v1 + (u32)g_ns_vpage_pool))->pan;
        result->center = (u8) ((ToneAttributes *)(temp_v1 + (u32)g_ns_vpage_pool))->center;
        result->shift = (u8) ((ToneAttributes *)(temp_v1 + (u32)g_ns_vpage_pool))->shift;
        result->max = (u8) ((ToneAttributes *)(temp_v1 + (u32)g_ns_vpage_pool))->max;
        result->min = (u8) ((ToneAttributes *)(temp_v1 + (u32)g_ns_vpage_pool))->min;
        result->vibW = (u8) ((ToneAttributes *)(temp_v1 + (u32)g_ns_vpage_pool))->vibW;
        result->vibT = (u8) ((ToneAttributes *)(temp_v1 + (u32)g_ns_vpage_pool))->vibT;
        result->porW = (u8) ((ToneAttributes *)(temp_v1 + (u32)g_ns_vpage_pool))->porW;
        result->porT = (u8) ((ToneAttributes *)(temp_v1 + (u32)g_ns_vpage_pool))->porT;
        result->pbmin = (u8) ((ToneAttributes *)(temp_v1 + (u32)g_ns_vpage_pool))->pbmin;
        result->pbmax = (u8) ((ToneAttributes *)(temp_v1 + (u32)g_ns_vpage_pool))->pbmax;
        temp_v1_2 = (ToneAttributes *)(temp_v1 + (u32)g_ns_vpage_pool);
        result->adsr1 = (u16) temp_v1_2->adsr1;
        result->adsr2 = (u16) temp_v1_2->adsr2;
        result->prog = (u16) temp_v1_2->prog;
        result->vag = (u16) temp_v1_2->vag;
        return 0;
    }
    return -1;
}
