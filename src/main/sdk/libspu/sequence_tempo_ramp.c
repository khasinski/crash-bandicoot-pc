#include "common.h"

void _SsSndSetRit(s16 arg0, s16 arg1) {
    s32 temp_t0;
    s32 var_v0_2;
    u32 temp_a0;
    u32 temp_a0_2;
    u32 temp_lo;
    u32 temp_v0;
    u32 temp_v1;
    u32 var_v0;
    SequenceState *temp_a3;

    temp_a3 = &g_ss_score_table[(s16)arg0][arg1];
    temp_t0 = temp_a3->tempo_diff;
    temp_v0 = temp_a3->tempo_cnt - 1;
    temp_a3->tempo_cnt = temp_v0;
    if (temp_t0 > 0) {
        if ((temp_v0 % (u32) temp_t0) == 0) {
            temp_v1 = temp_a3->cur_bpm;
            temp_a0 = temp_a3->tempo_ticks;
            if (temp_a0 < temp_v1) {
                var_v0 = temp_v1 - 1;
                goto update_tempo;
            }
            if (temp_v1 < temp_a0) {
                var_v0 = temp_v1 + 1;
update_tempo:
                temp_a3->cur_bpm = var_v0;
            }
            goto block_11;
        }
    } else {
        temp_v1 = temp_a3->cur_bpm;
        temp_a0_2 = temp_a3->tempo_ticks;
        if (temp_a0_2 < temp_v1) {
            temp_a3->cur_bpm = temp_t0 + temp_v1;
            var_v0_2 = temp_a3->cur_bpm < temp_a0_2;
            goto block_9;
        }
        if (temp_v1 < temp_a0_2) {
            temp_a3->cur_bpm = temp_v1 - temp_t0;
            temp_a0_2 = temp_a3->tempo_ticks;
            var_v0_2 = temp_a0_2 < temp_a3->cur_bpm;
block_9:
            if (var_v0_2 != 0) {
                temp_a3->cur_bpm = temp_a0_2;
            }
        }
block_11:
        temp_lo = (u32) (temp_a3->resolution * temp_a3->cur_bpm * 0xA) / (u32) (g_spawns_base * 0x3C);
        temp_a3->tick_rem = (s16) temp_lo;
        if ((s32)(temp_lo << 16) <= 0) {
            temp_a3->tick_rem = 1;
        }
        if ((temp_a3->tempo_cnt == 0) || (temp_a3->cur_bpm == temp_a3->tempo_ticks)) {
            g_ss_score_table[arg0][arg1].flags &= ~0x40;
            g_ss_score_table[arg0][arg1].flags &= ~0x80;
        }
    }
}
