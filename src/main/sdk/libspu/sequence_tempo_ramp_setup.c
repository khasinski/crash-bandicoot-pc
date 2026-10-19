#include "common.h"
void _SsSndSetVolData(s32 arg0, s16 arg1, s16 arg2, u32 arg3) {
    s32 var_v1;
    s32 temp_v1;
    SequenceState *temp_a0;

    temp_a0 = &g_ss_score_table[(s16)arg0][arg1];
    temp_v1 = temp_a0->flags;
    if (temp_v1 & 4) return;
    if (temp_v1 & 0x100) return;
    if (arg2 == 0) return;
    {
        temp_a0->crescendo_step = arg2;
        temp_a0->crescendo_ticks = arg3;
        temp_a0->crescendo_target = arg2;
        var_v1 = arg2 < 0 ? -(s32)arg2 : (s32)arg2;
        temp_a0->crescendo_cnt = arg3;
        if (arg3 >= (u32) var_v1) {
            temp_a0->tempo_step = (s16) (arg3 / (u32) var_v1);
            return;
        }
        temp_a0->tempo_step = (s16) -(s16) ((u32) var_v1 / arg3);
    }
}
