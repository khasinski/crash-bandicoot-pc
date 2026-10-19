#include "common.h"
void _SsContNrpn2(sequence,channel,value)
s16 sequence,channel;
u8 value;
{
    SequenceState *state=g_ss_score_table[sequence]+channel;
    switch(value) {
    case 20:
        state->rpn1=value;
        state->rpn2=1;
        state->step_counter=_SsReadDeltaValue(sequence,channel);
        state->loop_pos=state->read_pos;
        break;
    case 30:
        state->rpn1=value;
        if(state->data_entry==0) {
            state->nrpn1=0;
            state->step_counter=_SsReadDeltaValue(sequence,channel);
        } else if(state->data_entry<127) {
            state->data_entry--;
            state->step_counter=_SsReadDeltaValue(sequence,channel);
            if(state->data_entry!=0) {
                state->read_pos=state->loop_pos;
            } else state->nrpn1=0;
        } else {
            _SsReadDeltaValue(sequence,channel);
            state->step_counter=0;
            state->read_pos=state->loop_pos;
        }
        break;
    default:
        state->rpn1=value;
        state->nrpn_count++;
        state->step_counter=_SsReadDeltaValue(sequence,channel);
        break;
    }
}
