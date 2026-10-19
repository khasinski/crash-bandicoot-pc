#include "common.h"
#include "ns.h"
/* External word view: this unit uses an absolute reference to the flag. */

typedef struct PadSlot {
    u32 word00;
    u32 word04;
    u32 word08;
    u32 word0C;
    u32 word10;
} PadSlot;

typedef struct PadState {
    s32 count;
    PadSlot slots[2];
} PadState;

#define CRASH_EXT_ENGINE_SHUTDOWN_0
#include "engine_externs.h"
#undef CRASH_EXT_ENGINE_SHUTDOWN_0

/* OpenEvent takes a long (*)() handler; RIncTickCount returns nothing. */
static long core_rcnt2_handler(void)
{
    RIncTickCount();
    return 0;
}

s32 Core_Init(void)
{
    s32 i;

    for (i = 0; i < 0x400; i++) {
        RAM_AT(u8, RAM_SCRATCH_BASE)[i] = 0;
    }
    SetVideoMode(0);
    InitGeom();
    if (gp_pause_B_pressed != 0) {
        ResetCallback();
        CdInit();
    }
    Pad_Init(2);
    Card_Init();
    EnterCriticalSection();
    StopRCnt(0xF2000002);
    gp_rcnt2_event = OpenEvent(0xF2000002, 2, 0x1000, core_rcnt2_handler);
    EnableEvent(gp_rcnt2_event);
    SetRCnt(0xF2000002, 0x1000, 0x1000);
    StartRCnt(0xF2000002);
    ExitCriticalSection();
    Gpu_Init();
    Math_Sranda2();
    g_ns.draw_skip = 0;
    for (i = 0; i < 4; i++) {
        g_ns_recent_page_eids[i] = 0x6396347F;
    }
    for (i = 0; i < 16; i++) {
        g_slst_buf0[i].word18 = 0x6396347F;
    }
    if (gp_pause_B_pressed != 0) {
        Cd_ReadFileSys();
    }
    return -0xFF;
}

s32 Core_Kill(void)
{
    if (gp_pause_B_pressed != 0) {
        CdControl(8, NULL, NULL);
        StopCallback();
    }
    Card_Kill();
    PadStop();
    ResetGraph(3);
    EnterCriticalSection();
    StopRCnt(0xF2000002);
    DisableEvent(gp_rcnt2_event);
    CloseEvent(gp_rcnt2_event);
    ExitCriticalSection();
    return -0xFF;
}

void Pad_Init(s32 count)
{
    PadState *state = &g_pad_driver_state;
    s32 i;

    state->count = count;
    for (i = 0; i < count; i++) {
        state->slots[i].word10 = 0;
        state->slots[i].word08 = 0;
        state->slots[i].word04 = 0;
        g_pad_driver_state.slots[i].word00 = 0;
        PadInit(i);
    }
}

typedef struct DemoInput {
    u32 frame;
    u32 buttons;
} DemoInput;

typedef struct DemoRecording {
    s32 count;
    u8 reserved_04[0x524];
    s32 seed;
    DemoInput inputs[1];
} DemoRecording;

typedef struct DemoBufferView {
    u8 reserved_00[0x84];
    s32 word84;
} DemoBufferView;

typedef struct PlaybackState {
    s32 player;
    u8 reserved_04[0x70];
    s32 playback_mode;
} PlaybackState;

#define CRASH_EXT_ENGINE_SHUTDOWN_1
#include "engine_externs.h"
#undef CRASH_EXT_ENGINE_SHUTDOWN_1

void Pad_Update(void)
{
    s32 i = 0;
    s32 arg[4];

    if (g_pad_driver_state.count > 0) {
        do {
            PadSlot *slot = &g_pad_driver_state.slots[i];
            u32 raw;
            u32 buttons;

            slot->word10 = slot->word08;
            slot->word08 = slot->word04;
            slot->word0C = slot->word00;
            raw = PadRead(i);
            if (i == 0) {
                buttons = raw & 0xFFFF;
            } else {
                buttons = raw >> 16;
            }
            slot->word04 = buttons;
            if (slot->word04 & 0x1000) {
                slot->word04 &= 0xBFFF;
            }
            if (slot->word04 & 0x8000) {
                slot->word04 &= ~0x2000;
            }
            if (i == 0) {
                if (g_pad_debug_override.playback_mode != 2) {
                    if (g_pad_debug_override.playback_mode == 3) {
                        slot->word04 = 0;
                    }
                } else {
                    u32 pressed = slot->word04;
                    s32 player;
                    DemoRecording *frame = RAM_AT(DemoRecording, gp_cur_pbak_frame);

                    if (gp_cur_pbak_header == Ram_Addr(frame->inputs)) {
                        RAM_AT(DemoBufferView, g_db_cur[0])->word84 = frame->seed;
                    }
                    slot->word04 = RAM_AT(DemoInput, gp_cur_pbak_header)->buttons;
                    gp_cur_pbak_header += sizeof(DemoInput);
                    if ((pressed & 0x9F0) || RAM_AT(DemoInput, gp_cur_pbak_header) >= &frame->inputs[frame->count]) {
                        arg[0] = 0;
                        if ((player = g_pad_debug_override.player) != 0) {
                            Gool_ExecStateCode(NULL, RAM_AT(struct GoolObjView, player), 0xE00, 1, arg);
                            g_pad_debug_override.playback_mode = 3;
                        } else {
                            g_pad_debug_override.playback_mode = 0;
                        }
                    }
                }
            }
            slot->word00 = ~slot->word08 & slot->word04 & 0xF9FF;
            i++;
        } while (i < g_pad_driver_state.count);
    }
}

