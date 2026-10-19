#include "common.h"

typedef void (*ContCallback)(s16, s16, u8, ToneAttributes, s16, u8);

void _SsSetControlChange(s16 sequence, s16 channel, u8 value) {
    ProgramAttributes prog_attr;
    ToneAttributes tone_attr;
    SequenceState *state = g_ss_score_table[sequence] + channel;
    s32 i;
    u8 index = state->channel;
    u16 var;
    s32 var2;
    s32 z;

    SsUtGetProgAtr(state->vab_id, state->programs[index], &prog_attr);
    if (state->rpn2 == 1 && state->nrpn1 == 0) {
        state->data_entry = value;
        state->nrpn1 = 1;
        state->step_counter = _SsReadDeltaValue(sequence, channel);
        return;
    }
    if (state->nrpn_mode == 2) {
        if (state->nrpn_voice == 0) {
            for (i = 0; i < prog_attr.tones; i++) {
                SsUtGetVagAtr(state->vab_id, state->programs[index], i, &tone_attr);
                switch (state->rpn_flag) {
                case 0:
                    /* Force GCC 2.5.7 loop/cse2 to hoist value * 3 into $s3/$fp before combine/jump2 elides block. */
                    z = tone_attr.vol & 1;
                    if ((z >> 1) != 0) {
                        var2 = value * 25;
                        var2 += value;
                        var2 += value;
                        tone_attr.vol = var2;
                    }
                    tone_attr.pbmax = value & 0x7F;
                    tone_attr.pbmin = value & 0x7F;
                    break;
                case 1:
                    var = (value > 0x40 && value < 0x80) ? (((value * 100) / 8192) << 13) : 0;
                    tone_attr.shift += var;
                    break;
                case 2:
                    if (value >= 0x40 && value < 0x80) {
                        var2 = value << 1;
                        var2 += value;
                        var2 <<= 3;
                        var2 += value;
                        var = var2 << 8;
                    } else {
                        var2 = 0;
                        var = 0;
                    }
                    tone_attr.center += var;
                    break;
                }
                SsUtSetVagAtr(state->vab_id, state->programs[index], i, &tone_attr);
            }
        }
        state->step_counter = _SsReadDeltaValue(sequence, channel);
        state->nrpn_mode = 0;
        return;
    }
    if (state->nrpn_count == 2) {
        if (state->rpn1 == 0x10) {
            for (i = 0; i < prog_attr.tones; i++) {
                g_ss_nrpn_attr_fn_00[state->nrpn2](state->vab_id, state->programs[index], i, tone_attr, state->nrpn2, value);
            }
        } else {
            g_ss_nrpn_attr_fn_00[state->nrpn2](state->vab_id, state->programs[index], state->rpn1, tone_attr, state->nrpn2, value);
        }
        state->step_counter = _SsReadDeltaValue(sequence, channel);
        state->nrpn_count = 0;
        return;
    }
    state->step_counter = _SsReadDeltaValue(sequence, channel);
}
