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

/* How strongly the overlay darkens the screen (0 is not at all). */
u32 Gpu_FadeAlpha(u32 fade) {
    FadeTable table = g_tile_fade_alpha_table;
    return fade != 0 ? table.bytes[fade >> 4] : 0;
}

void Gpu_DrawOverlayAlpha(u32 alpha) {
    void *mode_packet;
    GpuPolyF4 *poly;
    RenderBuffer *buf = RAM_AT(RenderBuffer, g_db_cur[2]);

    mode_packet = RAM_AT(void, buf->word78);
    buf->word78 = buf->word78 + 0xC;
    poly = RAM_AT(GpuPolyF4, buf->word78);
    buf->word78 = Ram_Addr(poly + 1);
    if (alpha != 0) {
        poly->x0 = -256;
        poly->y0 = -120;
        poly->x1 = 256;
        poly->y1 = -120;
        poly->x2 = -256;
        poly->y2 = 120;
        poly->x3 = 256;
        poly->y3 = 120;
        poly->header.color[0] = alpha;
        poly->header.color[1] = alpha;
        poly->header.color[2] = alpha;
        SetPolyF4(poly);
        SetSemiTrans(poly, 1);
        AddPrim(&buf->ordering_table[0x7FF], poly);
        SetDrawMode(mode_packet, 1, 0, GetTPage(0, 2, 0, 0), 0);
        AddPrim(&buf->ordering_table[0x7FF], mode_packet);
    }
}

void Gpu_DrawOverlay(u32 fade) {
    Gpu_DrawOverlayAlpha(Gpu_FadeAlpha(fade));
}
