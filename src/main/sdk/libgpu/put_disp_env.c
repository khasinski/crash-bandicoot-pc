#include "common.h"
#include "gpu.h"

/* --- PutDispEnv --- */

typedef struct {
    u8 version;
    volatile u8 mode;
    u8 debug;
    u8 reverse;
    s16 w;
    s16 h;
    volatile s32 active;
    void (* volatile callback)(void);
} GpuState;

#define CRASH_EXT_SDK_LIBGPU_PUT_DISP_ENV_0
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_LIBGPU_PUT_DISP_ENV_0

typedef struct {
    u8 debug;
    u8 reserved[15];
} GpuDebugState;
#define CRASH_EXT_SDK_LIBGPU_PUT_DISP_ENV_1
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_LIBGPU_PUT_DISP_ENV_1

typedef struct {
    u8 reverse;
    u8 reserved[15];
} GpuReverseState;
#define CRASH_EXT_SDK_LIBGPU_PUT_DISP_ENV_2
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_LIBGPU_PUT_DISP_ENV_2

#define CLAMP(val, min, max) ((val) < (min) ? (min) : ((val) > (max) ? (max) : (val)))

void * PutDispEnv(GpuDisplayEnv *env)
{
    u32 mode;
    s32 hx1, hx2, vy1, vy2;

    mode = 0x08000000;
    if (g_gpu_debug_level.debug >= 2) {
        g_gpu_ot_status(g_str_put_disp_env_putdispenv_08x_n, env);
    }

    g_gpu_ctl->ctl10(((u32)(g_gpu_mode.version - 1) < 2)
        ? ((((env->display.y & 0xFFF) << 12) | (get_dx(&env->display) & 0xFFF)) | 0x05000000)
        : ((((env->display.y & 0x3FF) << 10) | (env->display.x & 0x3FF)) | 0x05000000));

    if (g_gpu_cur_dispenv.screen.x != env->screen.x ||
        g_gpu_cur_dispenv.screen.y != env->screen.y ||
        g_gpu_cur_dispenv.screen.w != env->screen.w ||
        g_gpu_cur_dispenv.screen.h != env->screen.h) {
        env->video_mode = GetVideoMode();
        hx1 = 0x260 + env->screen.x * 10;
        vy1 = env->screen.y + (env->video_mode ? 0x13 : 0x10);
        hx2 = hx1 + (env->screen.w ? env->screen.w * 10 : 0xA00);
        vy2 = vy1 + (env->screen.h ? env->screen.h : 0xF0);

        hx1 = CLAMP(hx1, 0x1F4, 0xCDA);
        hx2 = CLAMP(hx2, hx1 + 0x50, 0xCDA);
        vy1 = CLAMP(vy1, 0x10, env->video_mode ? 0x136 : 0x100);
        vy2 = CLAMP(vy2, vy1 + 2, env->video_mode ? 0x138 : 0x102);

        g_gpu_ctl->ctl10(0x06000000 | ((hx2 & 0xFFF) << 12) | (hx1 & 0xFFF));
        g_gpu_ctl->ctl10(0x07000000 | ((vy2 & 0x3FF) << 10) | (vy1 & 0x3FF));
    }

    if (*(s32 *)&g_gpu_cur_dispenv.interlaced != *(s32 *)&env->interlaced ||
        g_gpu_cur_dispenv.display.x != env->display.x ||
        g_gpu_cur_dispenv.display.y != env->display.y ||
        g_gpu_cur_dispenv.display.w != env->display.w ||
        g_gpu_cur_dispenv.display.h != env->display.h) {
        s32 small_h;

        env->video_mode = GetVideoMode();
        if (env->video_mode == 1) {
            mode |= 0x08;
        }
        if (env->rgb24) {
            mode |= 0x10;
        }
        if (env->interlaced) {
            mode |= 0x20;
        }
        if (g_gpu_reverse_flag.reverse) {
            mode |= 0x80;
        }
        if (env->display.w >= 0x119) {
            if (env->display.w < 0x161) {
                mode |= 0x01;
            } else if (env->display.w < 0x191) {
                mode |= 0x40;
            } else if (env->display.w < 0x231) {
                mode |= 0x02;
            } else {
                mode |= 0x03;
            }
        }
        small_h = env->display.h < (env->video_mode ? 0x121 : 0x101);
        if (!small_h) {
            mode |= 0x24;
        }
        g_gpu_ctl->ctl10(mode);
    }

    BIOS_MemCpy(&g_gpu_cur_dispenv, env, 0x14);
    return env;
}

