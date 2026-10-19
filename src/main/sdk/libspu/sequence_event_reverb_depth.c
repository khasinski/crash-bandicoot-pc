#include "common.h"
void _SsContResetAll(s16 sequence,s16 channel,u8 value) {
    SequenceState *state=g_ss_score_table[sequence]+channel;
    s16 depth=value;
    SsUtSetReverbDepth(depth,depth);
    state->step_counter=_SsReadDeltaValue(sequence,channel);
}
