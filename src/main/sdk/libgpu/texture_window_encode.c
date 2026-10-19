#include "common.h"
#include "gpu.h"

#define CRASH_EXT_SDK_LIBGPU_TEXTURE_WINDOW_ENCODE
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_LIBGPU_TEXTURE_WINDOW_ENCODE

void SetDrawEnv(u32 *packet, GpuDrawing *drawing)
{
    s32 n;
    GpuDrawEnv *env = &drawing->env;
    u32 *p = packet;

    p[1] = get_cs(env->clip.x, env->clip.y);
    p[2] = get_ce(env->clip.w + env->clip.x - 1, env->clip.y + env->clip.h - 1);
    p[3] = get_ofs(env->offset_x, env->offset_y);
    p[4] = get_mode(env->draw_to_display, env->dither, env->texture_page);
    p[5] = get_tw(&env->texture_window);
    p[6] = 0xE6000000;
    n = 7;

    if (env->clear_background) {
        GpuRect rect;

        rect.x = env->clip.x;
        rect.y = env->clip.y;
        rect.w = env->clip.w;
        rect.h = env->clip.h;
        rect.w = (rect.w < 0) ? 0 : ({ s32 max_x = g_gpu_vram_max_w[0] - 1; (rect.w > max_x) ? max_x : rect.w; });
        rect.h = (rect.h < 0) ? 0 : ({ s32 max_y = g_gpu_vram_max_h[0] - 1; (rect.h > max_y) ? max_y : rect.h; });

        if ((rect.x & 0x3F) || (rect.w & 0x3F)) {
            rect.x -= env->offset_x;
            rect.y -= env->offset_y;
            p[n++] = 0x60000000 | (env->b << 16) | (env->g << 8) | env->r;
            p[n++] = *(u32 *)&rect.x;
            p[n++] = *(u32 *)&rect.w;
            rect.x += env->offset_x;
            rect.y += env->offset_y;
        } else {
            p[n++] = 0x02000000 | (env->b << 16) | (env->g << 8) | env->r;
            p[n++] = *(u32 *)&rect.x;
            p[n++] = *(u32 *)&rect.w;
        }
    }

    ((u8 *)p)[3] = n - 1;
}

u32 get_mode(s32 dfe, s32 dtd, s32 tpage)
{
    u32 cmd, val;

    if ((u32)(g_gpu_mode[0] - 1) < 2) {
        cmd = dtd ? 0xE1000800 : 0xE1000000;
        val = tpage & 0x27FF;
        if (dfe) {
            val |= 0x1000;
        }
        val = ({ cmd | val; });
    } else {
        cmd = dtd ? 0xE1000200 : 0xE1000000;
        val = tpage & 0x9FF;
        if (dfe) {
            val |= 0x400;
        }
        val = ({ cmd | val; });
    }
    return val;
}

u32 get_cs(s16 x, s16 y)
{
    x = (x < 0) ? 0 : ({ s32 max_x = g_gpu_vram_max_w[0] - 1; (x > max_x) ? max_x : x; });
    y = (y < 0) ? 0 : ({ s32 max_y = g_gpu_vram_max_h[0] - 1; (y > max_y) ? max_y : y; });

    if ((u32)(g_gpu_mode[0] - 1) < 2) {
        return 0xE3000000 | ((y & 0xFFF) << 12) | (x & 0xFFF);
    }
    return 0xE3000000 | ((y & 0x3FF) << 10) | (x & 0x3FF);
}

u32 get_ce(s16 x, s16 y)
{
    x = (x < 0) ? 0 : ({ s32 max_x = g_gpu_vram_max_w[0] - 1; (x > max_x) ? max_x : x; });
    y = (y < 0) ? 0 : ({ s32 max_y = g_gpu_vram_max_h[0] - 1; (y > max_y) ? max_y : y; });

    if ((u32)(g_gpu_mode[0] - 1) < 2) {
        return 0xE4000000 | ((y & 0xFFF) << 12) | (x & 0xFFF);
    }
    return 0xE4000000 | ((y & 0x3FF) << 10) | (x & 0x3FF);
}

u32 get_ofs(s16 x, s16 y)
{
    if ((u32)(g_gpu_mode[0] - 1) < 2) {
        return 0xE5000000 | ((y & 0xFFF) << 12) | (x & 0xFFF);
    }
    return 0xE5000000 | ((y & 0x7FF) << 11) | (x & 0x7FF);
}

/* Four encoded fields form the GPU texture-window command. */

u32 get_tw(GpuRect *rect)
{
    u32 fields[4];
    if (rect != NULL) {
        fields[0] = (rect->x & 255) >> 3;
        fields[2] = (-rect->w & 255) >> 3;
        fields[1] = (rect->y & 255) >> 3;
        fields[3] = (-rect->h & 255) >> 3;
        return 0xE2000000 | (fields[1] << 15) | (fields[0] << 10) |
               (fields[3] << 5) | fields[2];
    }
    return 0;
}

s32 get_dx(GpuRect *rect)
{
    switch (g_gpu_mode[0]) {
    case 1:
        if (g_gpu_reverse_flag) {
            return 0x400 - rect->x - rect->w;
        }
        return rect->x;
    case 2:
        return g_gpu_reverse_flag ? 0x400 - rect->x - rect->w / 2 : rect->x / 2;
    default:
        return rect->x;
    }
}
