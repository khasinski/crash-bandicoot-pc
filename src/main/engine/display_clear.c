#include "render.h"
typedef GpuDrawing Drawing;
typedef GpuTile Tile;

typedef struct {
    s32 count;
    RenderBuffer *active[3];
    RenderBuffer buffers[2];
} RenderSystem;

#define CRASH_EXT_ENGINE_DISPLAY_CLEAR
#include "engine_externs.h"
#undef CRASH_EXT_ENGINE_DISPLAY_CLEAR

__inline__ void Gpu_ClearVRAM(void) {
    Drawing drawing;
    Tile tile;
    SetDefDrawEnv(&drawing.env,0,0,1024,512);
    drawing.env.draw_to_display = 1;
    PutDrawEnv(&drawing);
    TermPrim((u32 *)&tile);
    SetBlockFill(&tile.header);
    tile.rect.w = 1023;
    tile.rect.h = 511;
    tile.header.color[0] = 0;
    tile.header.color[1] = 0;
    tile.header.color[2] = 0;
    tile.rect.x = 0;
    tile.rect.y = 0;
    DrawPrim(&tile.header);
}

void Gpu_FillDisplay(void) {
    Drawing drawing;
    Tile tile;
    TermPrim((u32 *)&tile);
    SetBlockFill(&tile.header);
    tile.rect.w = 512;
    tile.rect.h = 240;
    tile.header.color[0] = 0;
    tile.header.color[1] = 0;
    tile.header.color[2] = 0;
    tile.rect.x = g_context.active[1]->drawing.env.clip.x;
    tile.rect.y = 0;
    DrawPrim(&tile.header);
    DrawSync(0);
}

void Gpu_Init(void) {
    s32 i;
    RenderBuffer *buffer;

    g_draw_stamp[0] = 0;
    ResetGraph(0);
    SetGraphDebug(0);
    Gpu_ClearVRAM();
    g_context.count = 2;
    for (i = 0; i < g_context.count; i++) {
        buffer = &g_context.buffers[i];
        buffer->word00 = i;
        *(s32 *)&buffer->meta[0] = 0;
        SetDefDispEnv(&buffer->display, i << 9, 0, 512, 240);
        SetDefDrawEnv(&buffer->drawing.env, i << 9, 12, 512, 216);
        buffer->drawing.env.offset_x = (i << 9) + 256;
        buffer->drawing.env.offset_y = 120;
        buffer->drawing.env.dither = 1;
        if (i == 0) {
            g_context.active[2] = buffer;
            g_context.active[0] = buffer;
        }
        if (i == 1) {
            g_context.active[1] = buffer;
        }
        *(s32 *)&buffer->meta[0] = frame_counter;
        *(s32 *)&buffer->meta[8] = 0x22;
    }
    PutDispEnv(&g_context.active[0]->display);
    PutDrawEnv(&g_context.active[1]->drawing);
    SetDispMask(1);
}

s32 Gpu_SetupPrims(void) {
    s32 i;
    RenderBuffer *buffer;
    s32 size;

    switch (gp_pbak_mode) {
    case 0x19:
    case 0x24:
    case 0x25:
    case 0x2D:
    case 0x33:
    case 0x34:
        Card_Alloc();
        break;
    }
    for (i = 0; i < g_context.count; i++) {
        buffer = &g_context.buffers[i];
        switch (gp_pbak_mode) {
        case 0x19:
            size = 0x1E000;
            break;
        case 0x38:
            size = 0x1CC00;
            break;
        default:
            size = 0x16000;
            break;
        }
        buffer->word74 = (s32)LIBC_Malloc(size);
        if (!buffer->word74) {
            BIOS_Abs(-15);
        }
        Gpu_ResetPrims(buffer);
    }
    return -0xFF;
}

s32 Gpu_Kill(void) {
    s32 i;
    RenderBuffer *buffer;

    DrawSync(0);
    for (i = 0; i < g_context.count; i++) {
        buffer = &g_context.buffers[i];
        LIBC_Free((void *)buffer->word74);
    }
    switch (gp_pbak_mode) {
    case 0x19:
    case 0x24:
    case 0x25:
    case 0x2D:
    case 0x33:
    case 0x34:
        Card_Free();
        break;
    }
    return -0xFF;
}

