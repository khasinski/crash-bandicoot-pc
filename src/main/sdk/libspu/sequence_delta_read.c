#include "common.h"
s32 _SsReadDeltaValue(s16 sequence, s16 channel) {
    s32 ticks;
    s32 scaled;
    u8 *cursor;
    u8 *next;
    u8 byte;
    s32 value;
    SequenceState *state;
    state = g_ss_score_table[sequence] + channel;
    cursor = state->read_pos;
    state->read_pos = cursor + 1;
    value = *cursor;
    if (value == 0) return 0;
    if (value & 0x80) {
        value &= 0x7F;
        do {
            next = state->read_pos;
            state->read_pos = next + 1;
            byte = *next;
            value = (value << 7) + (byte & 0x7F);
        } while (byte & 0x80);
    }
    scaled = value * 4;
    ticks = (scaled + value) * 2;
    state->delta_value = ticks + state->delta_value;
    return ticks;
}
