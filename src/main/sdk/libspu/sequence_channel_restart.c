#include "common.h"
void _SsSndNextSep(s16 sequence, s16 channel) {
    SequenceState *state;
    state = &g_ss_score_table[sequence][channel];
    state->loop_max = 1;
    state->loop_count = 0;
    g_ss_score_table[sequence][channel].flags &= ~0x100;
    g_ss_score_table[sequence][channel].flags &= ~8;
    g_ss_score_table[sequence][channel].flags &= ~2;
    g_ss_score_table[sequence][channel].flags &= ~4;
    g_ss_score_table[sequence][channel].flags &= ~0x200;
    state->play_mode = 1;
    state->read_pos = state->start_pos;
    g_ss_score_table[sequence][channel].flags |= 1;
}
