#include "object_links.h"
#define CRASH_EXT_ENGINE_LEVEL_START
#include "engine_externs.h"
#undef CRASH_EXT_ENGINE_LEVEL_START

void Level_InitMisc(s32 arg0) {
    Node *root;
    s32 var_a1;
    s32 var_a2;
    s32 temp_v0;

    temp_v0 = RAM_AT(s32, g_ns_page_pool)[1];
    switch (temp_v0) {
    case 23:
        if (arg0 != 0) {
            g_level_pause_menu_obj = GOOL_CreateObject((struct GoolObj *)&g_gool_obj_list_3, 0x27, 4, 0, 0, 1);
        }
        break;
    case 20:
    case 22:
        var_a1 = 0x17;
        if (arg0 != 0) {
            root = &g_gool_obj_list_3;
            var_a2 = 6;
            goto create_object;
        }
        break;
    case 5:
        var_a1 = 9;
        if (arg0 != 0) {
            root = &g_gool_obj_list_3;
            var_a2 = 4;
            goto create_object;
        }
        break;
    case 34:
    case 46:
        var_a1 = 0x35;
        if (arg0 != 0) {
            root = &g_gool_obj_list_3;
            var_a2 = 0xD;
create_object:
            GOOL_CreateObject((struct GoolObj *)root, var_a1, var_a2, 0, 0, 1);
        }
        break;
    case 14:
        break;
    case 40:
    case 42:
        Shader_ParamsUpdate(1);
        break;
    default:
        g_level_pause_menu_obj = 0;
        break;
    }
    g_level_first_spawn_flag[0] = 1;
    g_death_count[0] = 0;
    g_cam_goal_stop_flag = 0;
    g_level_count = 0;
    if (g_pbak_frame_step != 2) {
        g_pad_debug_override = 0;
    }
    g_key_count[0] = 0;
    Solid_TransSmoothStopAtSolid(0, 0, 0);
    g_gpu_max_frame_ticks = 0x20;
    g_level_misc_init_zero = 0;
    if (g_game_substate != 0x600) {
        g_cam_rot_xz = 0x120;
    }
}
