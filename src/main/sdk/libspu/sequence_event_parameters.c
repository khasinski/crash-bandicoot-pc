#include "common.h"
void _SsContRpn1(s16 sequence,s16 channel,u8 value) {
    SequenceState *state=g_ss_score_table[sequence]+channel;
    state->rpn_flag=value;
    state->nrpn_mode++;
    state->step_counter=_SsReadDeltaValue(sequence,channel);
}
void _SsContRpn2(s16 sequence,s16 channel,u8 value) {
    SequenceState *state=g_ss_score_table[sequence]+channel;
    state->nrpn_voice=value;
    state->nrpn_mode++;
    state->step_counter=_SsReadDeltaValue(sequence,channel);
}
