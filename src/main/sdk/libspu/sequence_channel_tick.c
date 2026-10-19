#include "common.h"

void _SsSeqPlay(s16 arg0, s16 arg1) {
    s16 temp_v0;
    s16 temp_v1_2;
    s16 temp_v1_3;
    s32 temp_a1;
    s32 temp_v0_2;
    s32 temp_v1;
    s32 var_s0;
    s32 var_v0;
    SequenceState *temp_s1;

    temp_s1 = &g_ss_score_table[arg0][arg1];
    temp_v0 = temp_s1->tick_rem;
    temp_v1 = temp_s1->step_counter;
    temp_a1 = temp_v1 - temp_v0;
    if (temp_a1 > 0) {
        temp_v1_2 = temp_s1->delta_step;
        if (temp_v1_2 > 0) {
            temp_s1->delta_step = (s16) (temp_v1_2 - 1);
            return;
        }
        if (temp_v1_2 == 0) {
            temp_s1->delta_step = temp_v0;
            var_v0 = temp_s1->step_counter - 1;
            goto block_10;
        }
        temp_s1->step_counter = temp_a1;
        return;
    }
    if (temp_v0 >= temp_v1) {
        var_s0 = temp_v1;
        do {
loop_8:
            _SsSndTempo(arg0, arg1);
            temp_v0_2 = temp_s1->step_counter;
            if (temp_v0_2 == 0) {
                goto loop_8;
            }
            temp_v1_3 = temp_s1->tick_rem;
            var_s0 += temp_v0_2;
        } while (var_s0 < temp_v1_3);
        var_v0 = var_s0 - temp_v1_3;
block_10:
        temp_s1->step_counter = var_v0;
    }
}

typedef void (*EventCallback)();

void _SsSndTempo(s16 arg0, s16 arg1) {
    s32 channel_index = arg1;
    s32 sequence_index = arg0;
    SequenceState *state = (SequenceState *)g_ss_score_table[sequence_index] + channel_index;
    s32 status;
    s32 vel;

    status = *state->read_pos++;
    if (status & 0x80) {
        state->channel = status & 0xF;
        switch (status & 0xF0) {
        case 0x90:
            state->last_status = 0x90;
            status = *state->read_pos++;
            vel = *state->read_pos++;
            state->step_counter = _SsReadDeltaValue(sequence_index, channel_index);
            g_Snd_FnTable[0](sequence_index, channel_index, status, vel);
            break;
        case 0xB0:
            state->last_status = 0xB0;
            status = *state->read_pos++;
            status &= 0xFF;
            g_ss_fn_control_change(sequence_index, channel_index, status);
            break;
        case 0xC0:
            state->last_status = 0xC0;
            status = *state->read_pos++;
            status &= 0xFF;
            g_ss_fn_note_off(sequence_index, channel_index, status);
            break;
        case 0xE0:
            state->last_status = 0xE0;
            state->read_pos++;
            goto pitch_bend;
        case 0xF0:
            state->last_status = 0xFF;
            state->channel = status & 0xF;
            status = *state->read_pos++;
            g_ss_fn_meta_event(sequence_index, channel_index, status);
            break;
        }
    } else {
        switch (state->last_status) {
        case 0x90:
            vel = *state->read_pos++;
            state->step_counter = _SsReadDeltaValue(sequence_index, channel_index);
            g_Snd_FnTable[0](sequence_index, channel_index, status, vel);
            break;
        case 0xB0:
            g_ss_fn_control_change(sequence_index, channel_index, status);
            break;
        case 0xC0:
            g_ss_fn_note_off(sequence_index, channel_index, status);
            break;
        case 0xE0:
        pitch_bend:
            g_ss_fn_pitch_bend(sequence_index, channel_index);
            break;
        case 0xFF:
            g_ss_fn_meta_event(sequence_index, channel_index, status);
            break;
        }
    }
}

