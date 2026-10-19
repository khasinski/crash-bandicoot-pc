#include "common.h"

void SsSetTableSize(void *table, s16 s_max, s16 t_max) {
    s32 i;
    s32 j;

    g_ss_seq_max = s_max;
    g_ss_sep_max = t_max;
    for (i = 0; i < s_max; i++) {
        g_ss_score_table[i] = (SequenceState *)table + i * t_max;
    }
    for (i = 0; i < g_ss_seq_max; i++) {
        for (j = 0; j < g_ss_sep_max; j++) {
            g_ss_score_table[i][j].flags = 0;
            g_ss_score_table[i][j].next_sep = 0xFF;
            g_ss_score_table[i][j].open_sep_no = 0;
            g_ss_score_table[i][j].crescendo_step = 0;
            g_ss_score_table[i][j].crescendo_target = 0;
            g_ss_score_table[i][j].crescendo_ticks = 0;
            g_ss_score_table[i][j].crescendo_cnt = 0;
            g_ss_score_table[i][j].tempo_step = 0;
            g_ss_score_table[i][j].tempo_ticks = 0;
            g_ss_score_table[i][j].tempo_cnt = 0;
            g_ss_score_table[i][j].tempo_accum = 0;
            g_ss_score_table[i][j].tempo_diff = 0;
            g_ss_score_table[i][j].voll = 0x7F;
            g_ss_score_table[i][j].volr = 0x7F;
            g_ss_score_table[i][j].start_voll = 0x7F;
            g_ss_score_table[i][j].start_volr = 0x7F;
        }
    }
}
