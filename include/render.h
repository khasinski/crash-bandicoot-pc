#ifndef CRASH_RENDER_H
#define CRASH_RENDER_H

#include "common.h"
#include "gpu.h"

/* Layout established by buffer swap (0x04, 0x18) and cursor/table access.
 * Keep unidentified fields named by offset until their meaning is confirmed.
 */
typedef struct RenderBuffer {
    u32 word00;
    GpuDisplayEnv display;
    GpuDrawing drawing;
    s32 word74;
    s32 word78;
    u8 meta[0xC];
    u32 ordering_table[0x800];
} RenderBuffer;

typedef struct RenderTransformWords {
    u16 word00, word02, word04;
    s16 word06, word08, word0A;
    u16 word0C, word0E, word10;
} RenderTransformWords;

/* Shared context word selected by mode update and read by Gfx_InitMatrices. */
typedef struct RenderContextView {
    u8 reserved_00[0x114];
    s32 word114;
} RenderContextView;

/* Distance plus the nine halfwords loaded into GTE controls 16..20.
 * The target's four-byte alignment provides the final halfword of padding
 * read by the setter's last 32-bit load. */
typedef struct RenderProjectionState {
    s32 distance;
    s16 matrix[9];
} RenderProjectionState;

/* Projection lies 0x6C bytes after the transform reset by Gfx_ResetCam.
 * Other words in that interval are accessed through existing named aliases. */
typedef struct RenderCameraPrefix {
    s32 transform[9];
    u8 reserved_24[0x48];
    RenderProjectionState projection;
} RenderCameraPrefix;

extern RAM_REF(RenderBuffer) g_db_cur[3];
extern RAM_REF(RenderBuffer) g_ot_cur;
extern RAM_REF(RenderContextView) g_ns_page_pool;
extern RenderProjectionState g_view_proj;

void Gpu_ResetPrims(RenderBuffer *buffer);
void Gpu_GetPrimsTail(s32 amount);
s32 *Gpu_GetCurPrimsTail(void);
void Gpu_SwapBuffers(void);
void Gfx_ScreenProj(RenderTransformWords *, RenderTransformWords *);
void Gfx_InitMatrices(void);
void Gfx_ResetCam(s32 *transform);

#endif
