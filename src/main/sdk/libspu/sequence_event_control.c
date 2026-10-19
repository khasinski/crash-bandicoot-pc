#include "common.h"
typedef void (*SequenceCallback)(s16,s16,u8);
void _SsContNrpn1(sequence,channel,value)
s16 sequence,channel;
u8 value;
{
    SequenceState *state=g_ss_score_table[(s16)sequence]+(s16)channel;
    SequenceCallback callback;
    if (state->rpn2==1 && state->nrpn1==0) {
        state->data_entry=value;
        state->nrpn1=1;
    } else {
        if(state->rpn1==30) goto check_callback;
        if(state->rpn1==20) goto check_callback;
        state->nrpn2=value;
        state->nrpn_count++;
    }
check_callback:
    if (state->rpn1==40) {
        callback=g_ss_mark_callbacks[(s16)sequence][(s16)channel];
        if(callback) callback(sequence,channel,value);
    }
    state->step_counter=_SsReadDeltaValue(sequence,channel);
}
