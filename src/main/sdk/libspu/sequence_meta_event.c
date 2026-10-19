#include "common.h"

static __inline__ u32 clock_divisor(u32 ticks) {
    return ticks << 2;
}

void _SsGetMetaEvent(s16 arg0, s16 arg1, u8 arg2) {
    s32 fifteen_ticks;
    u32 b0, b1;
    s32 val;
    s32 bpm;
    s32 prod;
    u8 *bytes;
    u32 divisor;
    SequenceState *state = g_ss_score_table[arg0] + arg1;

    switch (arg2) {
    case 0x51:
        bytes = state->read_pos;
        state->read_pos = bytes + 1;
        b0 = bytes[0];
        state->read_pos = bytes + 2;
        b1 = bytes[1];
        state->read_pos = bytes + 3;
        val = (b0 << 16) | (b1 << 8) | bytes[2];
        bpm = 60000000 / val;
        prod = state->resolution * bpm;
        fifteen_ticks = g_spawns_base << 4;
        fifteen_ticks -= g_spawns_base;
        divisor = clock_divisor(fifteen_ticks);
        state->cur_bpm = bpm;
        if ((u32)(prod * 10) < divisor) {
            u32 tmp = (u32)(g_spawns_base * 600) / (u32)prod;
            state->delta_step = tmp;
            state->tick_rem = tmp;
            {
                s32 z = (s32)state & 1;
                if ((z >> 1) == 0) {
                    goto L_end;
                }
                if (divisor == 0) goto L_else;
                state->step_counter = 0;
                if ((s32)divisor < 0) goto L_else;
                state->step_counter = 0;
                if ((s32)divisor > 0) goto L_else;
                state->step_counter = 0;
                if ((s32)divisor <= 0) goto L_else;
                state->step_counter = 0;
            }
        } else {
            u32 tmp;
        L_else:
            tmp = (u32)(state->resolution * state->cur_bpm * 10) / divisor;
            state->delta_step = -1;
            state->tick_rem = tmp;
            if ((u32)(fifteen_ticks * 2) < (u32)((u32)(state->resolution * state->cur_bpm * 10) % divisor)) {
                state->tick_rem = tmp + 1;
            }
        }
    L_end:
        state->step_counter = _SsReadDeltaValue(arg0, arg1);
        break;
    case 0x2F:
        state->loop_count++;
        if (state->loop_max == 0) {
            state->delta_value = 0;
            state->rpn2 = 0;
            state->step_counter = 0;
            state->read_pos = state->start_pos;
        } else if ((s16)state->loop_count < state->loop_max) {
            state->delta_value = 0;
            state->rpn2 = 0;
            state->step_counter = 0;
            state->read_pos = state->start_pos;
            state->loop_pos = state->start_pos;
        } else {
            (g_ss_score_table[arg0] + arg1)->flags &= ~1;
            (g_ss_score_table[arg0] + arg1)->flags &= ~8;
            (g_ss_score_table[arg0] + arg1)->flags &= ~2;
            (g_ss_score_table[arg0] + arg1)->flags |= 0x200;
            (g_ss_score_table[arg0] + arg1)->flags |= 4;
            state->play_mode = 0;
            state->loop_pos = state->start_pos;
            if (state->next_sep != 0xFF) {
                state->play_mode = 0;
                _SsSndNextSep(state->next_sep, state->open_sep_no);
                SpuVmSeqKeyOff((arg1 << 8) | arg0);
            }
            SpuVmSeqKeyOff((arg1 << 8) | arg0);
            state->step_counter = state->tick_rem;
        }
        break;
    }
}
