#include "render.h"
typedef GpuDrawing Drawing;
typedef GpuTile Tile;

typedef struct RenderSystem {
    s32 count;
    RAM_REF(RenderBuffer) active[3];
    RenderBuffer buffers[2];
} RenderSystem;

#define CRASH_EXT_ENGINE_DISPLAY_CLEAR
#include "engine_externs.h"
#undef CRASH_EXT_ENGINE_DISPLAY_CLEAR

void Gpu_ClearVRAM(void) {
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
    tile.rect.x = RAM_AT(RenderBuffer, g_context.active[1])->drawing.env.clip.x;
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
            g_context.active[2] = Ram_Addr(buffer);
            g_context.active[0] = Ram_Addr(buffer);
        }
        if (i == 1) {
            g_context.active[1] = Ram_Addr(buffer);
        }
        *(s32 *)&buffer->meta[0] = frame_counter;
        *(s32 *)&buffer->meta[8] = 0x22;
    }
    PutDispEnv(&RAM_AT(RenderBuffer, g_context.active[0])->display);
    PutDrawEnv(&RAM_AT(RenderBuffer, g_context.active[1])->drawing);
    SetDispMask(1);
}

/* The primitive pools hold what a frame draws. The console had 0x16000 bytes
 * of heap each; the wide view draws more, so they live in the host's spare
 * RAM above the PS1's 2 MiB (ordering-table links reach 8 MiB). The heap
 * block is still taken, so the rest of the game's memory stays where it was. */
#define PRIM_POOL_BASE 0x80300000u
#define PRIM_POOL_SIZE 0x60000u
#define PRIM_POOL_MAX 2
static void *prim_heap_block[PRIM_POOL_MAX];

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
        prim_heap_block[i] = LIBC_Malloc(size);
        if (prim_heap_block[i] == NULL || i >= PRIM_POOL_MAX) {
            BIOS_Abs(-15);
        }
        buffer->word74 = (s32)(PRIM_POOL_BASE + (u32)i * PRIM_POOL_SIZE);
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
        (void)buffer;
        LIBC_Free(prim_heap_block[i]);
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

