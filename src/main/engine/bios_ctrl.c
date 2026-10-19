#include "common.h"

#include "engine_state.h"

#define CRASH_EXT_ENGINE_BIOS_CTRL
#include "engine_externs.h"
#undef CRASH_EXT_ENGINE_BIOS_CTRL

static __inline__ s32 Card_ClearFlag20_inline(void)
{
    RAM_AT(EngineState, gp_card)->word04 &= ~0x20;
    return 0;
}

static __inline__ s32 Card_StartWritePart_inline(s32 value)
{
    EngineState *state = RAM_AT(EngineState, gp_card);
    s32 result = 0;

    if (state->word04 & 13) {
        engine_flag_shadow[0] = state->word04 |= 2;
        result = 1;
    } else {
        state->word00 = 6;
        engine_flag_shadow[0] = state->word04 |= 1;
        state->word14 = 0;
        state->word10 = 32;
        state->word0C = value;
    }
    return result;
}

static __inline__ s32 Card_StartReadPart_inline(s32 value)
{
    EngineState *state = RAM_AT(EngineState, gp_card);
    s32 result = 0;

    if (state->word04 & 13) {
        engine_flag_shadow[0] = state->word04 |= 2;
        result = 1;
    } else {
        state->word00 = 2;
        engine_flag_shadow[0] = state->word04 |= 1;
        state->word14 = 0;
        state->word10 = 32;
        state->word0C = value;
    }
    return result;
}

static __inline__ s32 Card_InvalidateFilename_inline(void)
{
    g_card_filename[0] = gp_str_card_invalid_tag[0];
    return 0;
}

static __inline__ s32 Card_RequestCheck_inline(void)
{
    s32 result;
    if (gp_card) {
        Card_ResetStateAndFlags((RAM_AT(EngineState, gp_card)->word04 & 0x10) | 4);
        result = 0;
    } else {
        result = 1;
    }
    return result;
}

s32 Card_Control(s32 op, s32 value)
{
    switch (op) {
    case 0:
    case 1:
        return 1;
    case 2:
        return Card_ClearFlag20_inline();
    case 3:
        return Card_StartWritePart_inline(value);
    case 4:
        return Card_StartReadPart_inline(value);
    case 5:
        return Card_StartFormat();
    case 6:
        return Card_StartWriteNamedPart();
    case 7:
        return Card_CheckNewDevice();
    case 8:
        return Card_ProbeTimeout();
    case 9:
        return Card_InvalidateFilename_inline();
    case 10:
        return Card_RequestCheck_inline();
    default:
        return 1;
    }
}
