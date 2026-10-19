#include "gpu.h"

/* Print the draw environment and hardware-specific texture-page fields. */
void DumpDrawEnv(GpuDrawEnv *arg0) {
    u16 temp_v0;
    u16 temp_v0_2;

    g_gpu_ot_status(g_str_environment_print_clip_3d_3d_d, arg0->clip.x, arg0->clip.y, arg0->clip.w, (s32) arg0->clip.h);
    g_gpu_ot_status(g_str_environment_print_ofs_3d_3d_n, arg0->offset_x, arg0->offset_y);
    g_gpu_ot_status(g_str_environment_print_tw_d_d_d, arg0->texture_window.x, arg0->texture_window.y, arg0->texture_window.w, (s32) arg0->texture_window.h);
    g_gpu_ot_status(g_str_environment_print_dtd_d_n, (s16) arg0->dither);
    g_gpu_ot_status(g_str_environment_print_dfe_d_n, (s16) arg0->draw_to_display);
    if ((GetGraphType() == 1) || (GetGraphType() == 2)) {
        temp_v0 = arg0->texture_page;
        g_gpu_ot_status(g_str_environment_print_tpage_d_d_d, (temp_v0 >> 9) & 3, (temp_v0 >> 7) & 3, (temp_v0 << 6) & 0x7C0, (temp_v0 * 8) & 0x300);
        return;
    }
    temp_v0_2 = arg0->texture_page;
    g_gpu_ot_status(g_str_environment_print_tpage_d_d_d, (temp_v0_2 >> 7) & 3, (temp_v0_2 >> 5) & 3, (temp_v0_2 << 6) & 0x7C0, ((temp_v0_2 * 0x10) & 0x100) + ((temp_v0_2 >> 2) & 0x200));
}

/* Print the display and screen rectangles and display flags. */
void DumpDispEnv(GpuDisplayEnv *env)
{
    g_gpu_ot_status(g_str_environment_print_disp_3d_3d_d, env->display.x, env->display.y, env->display.w, env->display.h);
    g_gpu_ot_status(g_str_environment_print_screen_3d_3d_d, env->screen.x, env->screen.y, env->screen.w, env->screen.h);
    g_gpu_ot_status(g_str_environment_print_isinter_d_n, env->interlaced);
    g_gpu_ot_status(g_str_environment_print_isrgb24_d_n, env->rgb24);
}
