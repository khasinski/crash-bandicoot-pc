#include "common.h"
void _SsSndReplay(s16 sequence,s16 channel) {
    SequenceState *state = &g_ss_score_table[sequence][channel];
    state->play_mode = 1;
    g_ss_score_table[sequence][channel].flags &= ~8;
}
