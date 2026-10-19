#include "common.h"
typedef void (*ControllerCallback)(s16,s16,u8);
typedef void (*SeqResetCallback)(s16,s16);
typedef void (*EventCallback)();
void _SsGetSeqData(sequence,channel,controller)
s16 sequence,channel;
u8 controller;
{
    s32 channel_index=(s16)channel;
    s32 sequence_index=(s16)sequence;
    SequenceState *state=g_ss_score_table[sequence_index]+channel_index;
    u8 *cursor=state->read_pos;
    u8 value;
    state->read_pos=cursor+1;
    value=*cursor;
    switch(controller) {
    case 0: state->vab_id=value;state->step_counter=_SsReadDeltaValue(sequence_index,channel_index);return;
    case 6: g_ss_cc_data_entry(sequence_index,channel_index,value);goto done;
    case 7: g_ss_cc_main_vol(sequence_index,channel_index,value);goto done;
    case 10: g_ss_cc_panpot(sequence_index,channel_index,value);goto done;
    /* Controller 11 uses slot 9: 0x80058360 + 9 * 4 = 0x80058384. */
    case 11: ((ControllerCallback)g_Snd_FnTable[9])(sequence_index,channel_index,value);goto done;
    case 64: g_ss_cc_damper(sequence_index,channel_index,value);goto done;
    case 91: g_ss_cc_reverb(sequence_index,channel_index,value);goto done;
    case 98: g_ss_cc_nrpn_lsb(sequence_index,channel_index,value);goto done;
    case 99: g_ss_cc_nrpn_msb(sequence_index,channel_index,value);goto done;
    case 100: g_ss_cc_rpn_lsb(sequence_index,channel_index,value);goto done;
    case 101: g_ss_cc_rpn_msb(sequence_index,channel_index,value);goto done;
    case 121: goto reset;
    default: goto read_delta;
    }
reset:
    g_ss_cc_reset_all(sequence_index,channel_index);
    goto done;
read_delta:
    state->step_counter=_SsReadDeltaValue(sequence,channel);
done:
    return;
}
