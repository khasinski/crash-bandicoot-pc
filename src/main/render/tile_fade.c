#include "common.h"
#include "render.h"

typedef struct GpuPolyF4 {
    GpuPacketHeader header;
    s16 x0, y0;
    s16 x1, y1;
    s16 x2, y2;
    s16 x3, y3;
} GpuPolyF4;

typedef struct FadeTable {
    u8 bytes[19];
} FadeTable;

#define CRASH_EXT_RENDER_TILE_FADE
#include "render_externs.h"
#undef CRASH_EXT_RENDER_TILE_FADE

void Gpu_DrawRect(s32 x, s32 y, s32 w, s32 h, u8 r, u8 g, u8 b) {
    RenderBuffer *buf = RAM_AT(RenderBuffer, g_db_cur[2]);
    GpuTile *tile = RAM_AT(GpuTile, buf->word78);

    buf->word78 = Ram_Addr(tile + 1);
    SetBlockFill(&tile->header);
    tile->rect.w = w;
    tile->rect.h = h;
    tile->header.color[0] = r;
    tile->header.color[1] = g;
    tile->header.color[2] = b;
    tile->rect.x = x + buf->drawing.env.clip.x;
    tile->rect.y = y + buf->drawing.env.clip.y;
    AddPrim(buf->ordering_table, tile);
}

void Gpu_DrawOverlay(u32 fade) {
    void *mode_packet;
    GpuPolyF4 *poly;
    FadeTable table;
    RenderBuffer *buf = RAM_AT(RenderBuffer, g_db_cur[2]);

    mode_packet = RAM_AT(void, buf->word78);
    buf->word78 = buf->word78 + 0xC;
    poly = RAM_AT(GpuPolyF4, buf->word78);
    buf->word78 = Ram_Addr(poly + 1);
    table = g_tile_fade_alpha_table;
    if (fade != 0) {
        fade = table.bytes[fade >> 4];
        poly->x0 = -256;
        poly->y0 = -120;
        poly->x1 = 256;
        poly->y1 = -120;
        poly->x2 = -256;
        poly->y2 = 120;
        poly->x3 = 256;
        poly->y3 = 120;
        poly->header.color[0] = fade;
        poly->header.color[1] = fade;
        poly->header.color[2] = fade;
        SetPolyF4(poly);
        SetSemiTrans(poly, 1);
        AddPrim(&buf->ordering_table[0x7FF], poly);
        SetDrawMode(mode_packet, 1, 0, GetTPage(0, 2, 0, 0), 0);
        AddPrim(&buf->ordering_table[0x7FF], mode_packet);
    }
}
