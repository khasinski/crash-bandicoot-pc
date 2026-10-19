#include "common.h"
#include <libgte.h>
#include "render.h"

/* Preserve unknown projection modes; reset the GTE matrix and camera words. */
void Gfx_InitMatrices(void)
{
    s32 mode;
    s32 distance;
    RenderProjectionState *projection;
    s16 *matrix;
    g_gfx_init_zero_2=0;g_gfx_init_zero_1=0;g_gfx_init_zero_0=0;
    mode=RAM_AT(RenderContextView, g_ns_page_pool)->word114;
    switch(mode) {
    case 30: distance=960;break;
    case 37: distance=800;break;
    case 55: distance=500;break;
    case 60: distance=460;break;
    case 90: distance=288;break;
    default:goto preserve_distance;
    }
    g_view_proj.distance=distance;
preserve_distance:
    projection=(RenderProjectionState *)&g_view_proj;
    SetGeomScreen(projection->distance);
    SetGeomOffset(0,0);
    g_m_ident_11=4096;g_m_ident_01=0;g_m_ident_02=0;
    g_m_ident_t_x=0;g_m_ident_t_y=0;g_m_ident_t_z=0;
    matrix=projection->matrix;
    *matrix=512;
    g_m_default_light_10=512;g_m_default_light_20=512;g_m_default_light_01=512;
    g_m_default_light_11=512;g_m_default_light_21=512;g_m_default_light_02=512;
    g_m_default_light_12=512;g_m_default_light_22=512;
    SetBackColor(0,0,0);
    SetColorMatrix((MATRIX *)matrix);
    Gfx_ResetCam(((RenderCameraPrefix *)((u8 *)projection
        - CRASH_OFFSETOF(RenderCameraPrefix, projection)))->transform);
    ((RenderCameraPrefix *)((u8 *)projection
        - CRASH_OFFSETOF(RenderCameraPrefix, projection)))->transform[0]=0;
    g_cam_trans_y=0;g_cam_trans_z=0x1F400;
    g_cam_rot_x=0;g_cam_rot_y=0;g_cam_rot_z=0;
    g_cam_eye_x[0]=0;g_cam_eye_y[0]=0xE1000;g_cam_eye_z[0]=0x5DC000;
    g_gfx_mat_init_zero=0;g_gfx_debug_flag[0]=0;g_item_pool1[0]=0;
}
