#ifndef CRASH_GPU_H
#define CRASH_GPU_H

#include <stdint.h>
#include <libgpu.h>

/*
 * The game builds GPU packets, ordering tables and environments in game
 * memory in their PS1 format: a packet starts with a tag word holding the
 * low 24 bits of the next packet's PS1 address and the packet's word count.
 * PsyZ's libgpu uses a host layout for the same types, so the game's GPU
 * calls go to src/platform/gpu.c, which reads the PS1 format and feeds the
 * commands to PsyZ.
 */

/* Word view used when inserting packets into a DMA chain. */
typedef struct GpuPacketTag {
    uint32_t next : 24;
    uint32_t length : 8;
} GpuPacketTag;

/* Little-endian DMA tag followed by the GPU colour/command word.
 * length counts the 32-bit words following the tag, excluding the tag itself.
 */
typedef struct GpuPacketHeader {
    uint8_t next[3];
    uint8_t length;
    uint8_t color[3];
    uint8_t command;
} GpuPacketHeader;

/* Layouts reconstructed from accesses in the retail environment initialisers. */
typedef struct GpuRect {
    int16_t x, y, w, h;
} GpuRect;

typedef struct GpuDrawEnv {
    GpuRect clip;
    int16_t offset_x, offset_y;
    GpuRect texture_window;
    uint16_t texture_page;
    uint8_t dither, draw_to_display, clear_background;
    uint8_t r, g, b;
} GpuDrawEnv;

typedef struct GpuDisplayEnv {
    GpuRect display;
    GpuRect screen;
    uint8_t interlaced, rgb24, video_mode, reserved;
} GpuDisplayEnv;

/* PutDrawEnv copies exactly 0x5C bytes, including the command packet. */
typedef struct GpuDrawing {
    GpuDrawEnv env;
    uint32_t packet[16];
} GpuDrawing;

typedef struct GpuTile {
    GpuPacketHeader header;
    GpuRect rect;
} GpuTile;

_Static_assert(sizeof(GpuDrawing) == 0x5C, "PS1 DRAWENV size");
_Static_assert(sizeof(GpuDisplayEnv) == sizeof(DISPENV), "PS1 DISPENV layout");

uint32_t get_tw(GpuRect *rect);

/* PS1-format implementations of the game's libgpu calls. */
GpuDrawEnv *Ps1Gpu_SetDefDrawEnv(GpuDrawEnv *env, int32_t x, int32_t y, int32_t w, int32_t h);
GpuDisplayEnv *Ps1Gpu_SetDefDispEnv(GpuDisplayEnv *env, int32_t x, int32_t y, int32_t w, int32_t h);
void Ps1Gpu_TermPrim(void *packet);
void Ps1Gpu_AddPrim(void *ordering_table, void *packet);
void Ps1Gpu_SetBlockFill(void *packet);
void Ps1Gpu_SetPolyF4(void *packet);
void Ps1Gpu_SetSprt16(void *packet);
void Ps1Gpu_SetSemiTrans(void *packet, int32_t abe);
void Ps1Gpu_SetDrawMode(void *packet, int32_t dfe, int32_t dtd, int32_t tpage, GpuRect *tw);
void Ps1Gpu_DrawPrim(void *packet);
void Ps1Gpu_DrawOTag(void *ordering_table);
GpuDrawing *Ps1Gpu_PutDrawEnv(GpuDrawing *drawing);
GpuDisplayEnv *Ps1Gpu_PutDispEnv(GpuDisplayEnv *env);
void Ps1Gpu_DrawOTagEnv(void *ordering_table, GpuDrawing *drawing);
void Ps1Gpu_SetDrawEnv(void *packet, GpuDrawEnv *env);
void Ps1Gpu_ClearOTagR(uint32_t *ordering_table, int32_t count);

#define SetDefDrawEnv(env, x, y, w, h) Ps1Gpu_SetDefDrawEnv(env, x, y, w, h)
#define SetDefDispEnv(env, x, y, w, h) Ps1Gpu_SetDefDispEnv(env, x, y, w, h)
#define TermPrim(p) Ps1Gpu_TermPrim(p)
#define AddPrim(ot, p) Ps1Gpu_AddPrim(ot, p)
#define SetBlockFill(p) Ps1Gpu_SetBlockFill(p)
#define SetPolyF4(p) Ps1Gpu_SetPolyF4(p)
#define SetSprt16(p) Ps1Gpu_SetSprt16(p)
#define SetSemiTrans(p, abe) Ps1Gpu_SetSemiTrans(p, abe)
#define SetDrawMode(p, dfe, dtd, tpage, tw) Ps1Gpu_SetDrawMode(p, dfe, dtd, tpage, tw)
#define DrawPrim(p) Ps1Gpu_DrawPrim(p)
#define DrawOTag(ot) Ps1Gpu_DrawOTag(ot)
#define PutDrawEnv(d) Ps1Gpu_PutDrawEnv(d)
#define PutDispEnv(d) Ps1Gpu_PutDispEnv(d)
#define DrawOTagEnv(ot, d) Ps1Gpu_DrawOTagEnv(ot, d)
#define SetDrawEnv(p, env) Ps1Gpu_SetDrawEnv(p, env)
#define ClearOTagR(ot, n) Ps1Gpu_ClearOTagR(ot, n)
/* The remaining calls share PsyZ's layouts; only the pointer types differ. */
#define LoadImage(rect, data) LoadImage((RECT *)(rect), (u_long *)(data))
#define StoreImage(rect, data) StoreImage((RECT *)(rect), (u_long *)(data))
#define LoadClut(clut, x, y) LoadClut((u_long *)(clut), x, y)

#endif
