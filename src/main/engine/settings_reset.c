#include "common.h"

#define CRASH_EXT_ENGINE_SETTINGS_RESET
#include "engine_externs.h"
#undef CRASH_EXT_ENGINE_SETTINGS_RESET

void Level_InitGlobals(void) {
    g_saved_level_id[0] = 0x63;
    g_pbak_state = -1;
    g_island_cam_state = 7;
    g_cur_map_level[0] = 0xFF;
    g_levels_unlocked[0] = 0xFF;
    g_new_gem_flag[0] = 0x400;
    g_life_count = 0;
    g_saved_item_pool2_hi[0] = 0;
    g_stereo_disabled[0] = 0;
    g_item_pool2[0] = 1;
    g_level_globals_init_zero = 0;
    g_level_unused_vec = 0;
    g_brio_round_done = 0;
    g_game_substate = 0;
    g_bonus_round_active = 0;
    g_cortex_round_done = 0;
    g_first_boot_flag = 1;
    Level_ResetGlobals(1);
}

void Level_ResetGlobals(s32 arg0) {
    if (arg0 != 0) {
        g_level_frame_accum = 0;
        g_level_restart_fade_cnt[0] = 0;
        g_brio_count = 0;
        g_tawna_count = 0;
        g_checkpoint_id = 0;
        g_level_load_timer = 0;
        g_mono = 0;
        g_next_level_id[0] = 1;
        g_saved_gem_pool_lo[0] = 0;
        g_checkpoint_id_2[0] = -1;
        g_audio_mode_flags[0] = 0;
        g_level_first_spawn_flag[0] = 1;
        g_saved_level_id[0] = 0x63;
        g_item_pool2[0] = 1;
        g_bonus_round_active = 0;
        g_cortex_round_done = 0;
        g_first_boot_flag = 1;
        g_cortex_count = g_new_gem_flag[0];
        BIOS_MemSet(g_level_spawns, 0, 0x200);
        Card_InvalidateFilename();
    }
}

