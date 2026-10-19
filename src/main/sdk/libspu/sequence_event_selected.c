#include "common.h"
void _SsNoteOff(s16 sequence,s16 channel,u8 value) {
    SequenceState *state=g_ss_score_table[sequence]+channel;
    state->programs[state->channel]=value;
    state->step_counter=_SsReadDeltaValue(sequence,channel);
}
