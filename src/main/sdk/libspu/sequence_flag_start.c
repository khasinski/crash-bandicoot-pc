#include "common.h"
void _SsSndSetReplayMode(s16 sequence,s16 channel) {
    SequenceState *state = &g_ss_score_table[sequence][channel];
    u32 word = state->flags;
    if (!(word&0x204)) {
        if (!(word&0x100)) {
            state->flags = word&~2;
            g_ss_score_table[sequence][channel].flags |= 8;
            g_ss_score_table[sequence][channel].flags |= 1;
        }
    }
}
