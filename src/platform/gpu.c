/*
 * PS1-format GPU calls for the game. Packets, ordering tables and drawing
 * environments live in game memory in their PS1 layout; this module walks
 * them and hands the GP0 command words to PsyZ.
 */
#include <string.h>
#include <psyz.h>
#include <libgpu.h>
#include "ram.h"
#include "gpu.h"

#define TAG_END 0xFFFFFFu
/* PsyZ's GP0 queue holds 0x4000 words; flush well before it fills. */
#define QUEUE_FLUSH_WORDS 0x3000

static int queued_words;

static void gp0_packet(const uint32_t *words, int count)
{
    int i;
    if (queued_words + count > QUEUE_FLUSH_WORDS) {
        Psyz_GpuExeque();
        queued_words = 0;
    }
    for (i = 0; i < count; i++)
        Psyz_GpuWriteGP0(words[i]);
    queued_words += count;
}

static void gp0_flush(void)
{
    Psyz_GpuExeque();
    queued_words = 0;
}

static uint32_t tag_target(const void *packet)
{
    return Ram_Addr(packet) & TAG_END;
}

static uint32_t *tag_pointer(uint32_t next)
{
    return (uint32_t *)Ram_Ptr(RAM_PS1_BASE | next);
}

static void set_tag(void *packet, uint32_t length, uint32_t next)
{
    *(uint32_t *)packet = (length << 24) | (next & TAG_END);
}

static void set_length_code(void *packet, uint32_t length, uint8_t code)
{
    uint8_t *bytes = packet;
    bytes[3] = (uint8_t)length;
    bytes[7] = code;
}

uint32_t Ps1Gpu_GetTw(GpuRect *rect)
{
    uint32_t mask_x, mask_y, off_x, off_y;
    if (rect == NULL)
        return 0;
    off_x = (rect->x & 0xFF) >> 3;
    off_y = (rect->y & 0xFF) >> 3;
    mask_x = ((~(rect->w - 1)) & 0xFF) >> 3;
    mask_y = ((~(rect->h - 1)) & 0xFF) >> 3;
    return 0xE2000000u | (off_y << 15) | (off_x << 10) | (mask_y << 5) | mask_x;
}

GpuDrawEnv *Ps1Gpu_SetDefDrawEnv(GpuDrawEnv *env, int32_t x, int32_t y, int32_t w, int32_t h)
{
    env->clip.x = (int16_t)x;
    env->clip.y = (int16_t)y;
    env->clip.w = (int16_t)w;
    env->clip.h = (int16_t)h;
    env->offset_x = (int16_t)x;
    env->offset_y = (int16_t)y;
    env->texture_window.x = env->texture_window.y = 0;
    env->texture_window.w = env->texture_window.h = 0;
    env->texture_page = 0x0A;
    env->dither = 1;
    env->draw_to_display = h < 289 ? 0 : 1;
    env->clear_background = 0;
    env->r = env->g = env->b = 0;
    return env;
}

GpuDisplayEnv *Ps1Gpu_SetDefDispEnv(GpuDisplayEnv *env, int32_t x, int32_t y, int32_t w, int32_t h)
{
    env->display.x = (int16_t)x;
    env->display.y = (int16_t)y;
    env->display.w = (int16_t)w;
    env->display.h = (int16_t)h;
    env->screen.x = env->screen.y = env->screen.w = env->screen.h = 0;
    env->interlaced = 0;
    env->rgb24 = 0;
    env->video_mode = 0;
    env->reserved = 0;
    return env;
}

void Ps1Gpu_TermPrim(void *packet)
{
    uint32_t *tag = packet;
    *tag |= TAG_END;
}

void Ps1Gpu_AddPrim(void *ordering_table, void *packet)
{
    uint32_t *ot = ordering_table;
    uint32_t *tag = packet;
    *tag = (*tag & 0xFF000000u) | (*ot & TAG_END);
    *ot = (*ot & 0xFF000000u) | tag_target(packet);
}

void Ps1Gpu_SetBlockFill(void *packet)
{
    set_length_code(packet, 3, 0x02);
}

void Ps1Gpu_SetPolyF4(void *packet)
{
    set_length_code(packet, 5, 0x28);
}

void Ps1Gpu_SetSprt16(void *packet)
{
    set_length_code(packet, 3, 0x7C);
}

void Ps1Gpu_SetSemiTrans(void *packet, int32_t abe)
{
    uint8_t *bytes = packet;
    if (abe)
        bytes[7] |= 0x02;
    else
        bytes[7] &= (uint8_t)~0x02;
}

void Ps1Gpu_SetDrawMode(void *packet, int32_t dfe, int32_t dtd, int32_t tpage, GpuRect *tw)
{
    uint32_t *words = packet;
    words[0] = (words[0] & TAG_END) | (2u << 24);
    words[1] = 0xE1000000u | (dtd ? 0x200 : 0) | (dfe ? 0x400 : 0) | (tpage & 0x9FF);
    words[2] = Ps1Gpu_GetTw(tw);
}

void Ps1Gpu_DrawPrim(void *packet)
{
    const uint32_t *words = packet;
    gp0_packet(words + 1, (int)(words[0] >> 24));
    gp0_flush();
}

void Ps1Gpu_DrawOTag(void *ordering_table)
{
    const uint32_t *packet = ordering_table;
    for (;;) {
        uint32_t tag = packet[0];
        uint32_t length = tag >> 24;
        if (length)
            gp0_packet(packet + 1, (int)length);
        if ((tag & TAG_END) == TAG_END)
            break;
        packet = tag_pointer(tag & TAG_END);
    }
    gp0_flush();
}

/* The environment command words of a PS1 DR_ENV packet. */
static int draw_env_words(const GpuDrawEnv *env, uint32_t *words)
{
    int n = 0;
    int32_t x0 = env->clip.x, y0 = env->clip.y;
    int32_t x1 = env->clip.x + env->clip.w - 1, y1 = env->clip.y + env->clip.h - 1;
    GpuRect tw = env->texture_window;

    if (x1 < x0) x1 = x0;
    if (y1 < y0) y1 = y0;
    words[n++] = 0xE3000000u | ((y0 & 0x3FF) << 10) | (x0 & 0x3FF);
    words[n++] = 0xE4000000u | ((y1 & 0x3FF) << 10) | (x1 & 0x3FF);
    words[n++] = 0xE5000000u | ((env->offset_y & 0x7FF) << 11) | (env->offset_x & 0x7FF);
    words[n++] = 0xE1000000u | (env->dither ? 0x200 : 0) | (env->draw_to_display ? 0x400 : 0)
                 | (env->texture_page & 0x9FF);
    words[n++] = Ps1Gpu_GetTw(&tw);
    if (env->clear_background) {
        words[n++] = 0x02000000u | ((uint32_t)env->b << 16) | ((uint32_t)env->g << 8) | env->r;
        words[n++] = ((uint32_t)(uint16_t)env->clip.y << 16) | (uint16_t)env->clip.x;
        words[n++] = ((uint32_t)(uint16_t)env->clip.h << 16) | (uint16_t)env->clip.w;
    }
    return n;
}

void Ps1Gpu_SetDrawEnv(void *packet, GpuDrawEnv *env)
{
    uint32_t *words = packet;
    int n = draw_env_words(env, words + 1);
    words[0] = (words[0] & TAG_END) | ((uint32_t)n << 24);
}

GpuDrawing *Ps1Gpu_PutDrawEnv(GpuDrawing *drawing)
{
    uint32_t words[16];
    int n = draw_env_words(&drawing->env, words);
    Ps1Gpu_SetDrawEnv(drawing->packet, &drawing->env);
    Ps1Gpu_TermPrim(drawing->packet);
    gp0_packet(words, n);
    gp0_flush();
    return drawing;
}

GpuDisplayEnv *Ps1Gpu_PutDispEnv(GpuDisplayEnv *env)
{
    /* DISPENV has no pointers, so PsyZ shares the PS1 layout. */
    (PutDispEnv)((DISPENV *)env); /* PsyZ, not the macro */
    return env;
}

void Ps1Gpu_DrawOTagEnv(void *ordering_table, GpuDrawing *drawing)
{
    Ps1Gpu_PutDrawEnv(drawing);
    Ps1Gpu_DrawOTag(ordering_table);
}

void Ps1Gpu_ClearOTagR(uint32_t *ordering_table, int32_t count)
{
    int32_t i;
    if (count <= 0)
        return;
    for (i = count - 1; i > 0; i--)
        set_tag(&ordering_table[i], 0, tag_target(&ordering_table[i - 1]));
    set_tag(&ordering_table[0], 0, TAG_END);
}
