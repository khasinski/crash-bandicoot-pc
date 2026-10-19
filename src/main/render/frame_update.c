#include "common.h"
#include "gpu.h"

/* =========================================================================
 * 1. Gpu_Update (asm/USA/main/text_765C.s)
 * ========================================================================= */

typedef struct RenderBufferFull {
    u32 word00;
    GpuDisplayEnv display;
    GpuDrawing drawing;
    s32 word74;
    s32 word78;
    u32 word7C;
    u32 word80;
    s32 word84;
    u32 ordering_table[0x800];
} RenderBufferFull;

typedef struct BufferToken {
    u8 bytes[4];
} BufferToken;

#define CRASH_EXT_RENDER_FRAME_UPDATE_0
#include "render_externs.h"
#undef CRASH_EXT_RENDER_FRAME_UPDATE_0

extern s32 gp_rcnt_stopped;
#define CRASH_EXT_RENDER_FRAME_UPDATE_1
#include "render_externs.h"
#undef CRASH_EXT_RENDER_FRAME_UPDATE_1

typedef struct ZoneHeader8214 {
    u8 reserved_00[0x2E0];
    s32 word2E0;
    u8 reserved_2e4[0x18];
    u32 flags2FC;
    u8 reserved_300[0x10];
    u8 r310;
    u8 g311;
    u8 b312;
    u8 reserved_313;
    u8 r314;
    u8 g315;
    u8 b316;
} ZoneHeader8214;

typedef struct ZoneEntry8214 {
    u8 reserved_00[0x10];
    RAM_REF(ZoneHeader8214) header;
} ZoneEntry8214;

#define CRASH_EXT_RENDER_FRAME_UPDATE_2
#include "render_externs.h"
#undef CRASH_EXT_RENDER_FRAME_UPDATE_2

static __inline__ BufferToken *Gpu_RoundTicks_inline(void)
{
    g_pbak_header_byte_0[0] = g_pbak_seed_val;
    (void)frame_counter;
    return &g_pbak_header_byte_0[0];
}

void Gpu_Update(void)
{
    BufferToken *token;
    RenderBufferFull *swap;

    token = Gpu_RoundTicks_inline();
    if (gp_next_lid == 0 && (g_cur_lid_ro[0] & 0x1000)) {
        g_draw_stamp[0]++;
    }
    if (g_cam_rot_xz[0] != 0) {
        if (g_cam_rot_xz[0] > 0) {
            g_cam_rot_xz[0] -= g_gpu_max_frame_ticks;
            Gpu_DrawOverlay(g_cam_rot_xz[0]);
        } else if (g_cam_rot_xz[0] >= -2) {
            Gpu_DrawOverlay(0x100);
            g_cam_rot_xz[0] = -1;
        } else {
            g_cam_rot_xz[0] += g_gpu_max_frame_ticks;
            Gpu_DrawOverlay(g_cam_rot_xz[0] + 0x100);
            if (g_cam_rot_xz[0] == 0 && !(g_cur_lid_ro[0] & 0x200000)) {
                g_cam_rot_xz[0] = -2;
            }
        }
    }
    if (!(g_cur_lid_ro[0] & 0x80000)) {
        if (g_cur_lid_ro[0] & 0x2000) {
            Gpu_DrawRect(0, 0, 0x200, g_pads[0]->header->word2E0, token->bytes[0], token->bytes[1], token->bytes[2]);
            Gpu_DrawRect(0, g_pads[0]->header->word2E0, 0x200, 0xD8 - g_pads[0]->header->word2E0, g_pads[0]->header->r310, g_pads[0]->header->g311, g_pads[0]->header->b312);
        } else {
            Gpu_DrawRect(0, 0, 0x200, 0xD8, 0, 0, 0);
        }
    }
    Audio_Update();
    g_db_prev->word80 = frame_counter;
    DrawSync(0);
    if (g_pbak_frame_step == 0) {
        g_box_count = frame_counter - g_db_prev->word7C;
    } else {
        g_box_count = 0x11;
    }
    if (g_ns_load_done_flag[0] == 0) {
        swap = g_db_prev;
        g_db_cur.buf = swap;
        g_db_prev = g_ot_cur;
        g_ot_cur = swap;
    }
    VSync(0);
    if ((u32)(frame_counter - g_db_cur.buf->word7C) < 0x19) {
        VSync(0);
    }
    if (g_cur_lid_ro[0] & 0x100000) {
        if (!gp_rcnt_stopped) {
            EnterCriticalSection();
            StopRCnt(0xF2000002);
            ExitCriticalSection();
        }
        gp_rcnt_stopped = 1;
        Card_Update();
        frame_counter += 0x22;
    } else {
        if (gp_rcnt_stopped) {
            EnterCriticalSection();
            StartRCnt(0xF2000002);
            ExitCriticalSection();
        }
        gp_rcnt_stopped = 0;
    }
    EnterCriticalSection();
    if (g_pbak_frame_step == 2) {
        frame_counter = *gp_cur_pbak_header;
    }
    g_db_prev->word7C = frame_counter;
    ExitCriticalSection();
    g_db_cur.buf->word84 = Gfx_TgeoGetTexinfoArray(g_db_prev->word7C - g_db_cur.buf->word7C);
    PutDispEnv(&g_db_cur.buf->display);
    PutDrawEnv(&g_db_prev->drawing);
    if (g_ns_load_done_flag[0] > 0) {
        g_ns_load_done_flag[0]--;
    }
    if (g_ns_load_done_flag[0] == 0) {
        DrawOTag(g_db_prev->ordering_table);
    }
    g_ot_cur->word78 = g_ot_cur->word74;
    Gpu_RResetOT(g_ot_cur->ordering_table, 0x800);
    g_cur_lid_ro[0] = g_fade_counter;
}

