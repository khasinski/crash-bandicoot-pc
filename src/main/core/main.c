#include "common.h"
#include "render.h"
#include "resource.h"

/* Initialise the runtime, run the game loop for initial level 0x19, then shut down. */

s32 CORE_Main(void)
{
    __main();
    gp_pause_B_pressed = 1;
    Core_Init();
    CORE_Loop(0x19);
    Core_Kill();
    return 0;
}

typedef struct ZoneHeaderView {
    s32 word00;
    u8 reserved_04[0x2F8];
    u32 flags;
} ZoneHeaderView;

typedef struct LevelDataView {
    u8 reserved_00[4];
    s32 level_id;
    u8 reserved_08[0xC];
    s32 eid14;
    u8 reserved_18[8];
    s32 eid20;
    s32 eid24;
    s32 eid28;
    u8 reserved_2c[0x5C];
    s32 eid88;
    u8 reserved_8c[0x10];
    s32 eid9C;
    u8 reserved_a0[0x44];
    s32 eidE4;
} LevelDataView;

typedef struct NsState {
    u8 reserved_00[0x14];
    LevelDataView *level;
    u8 reserved_18[0xC];
    s32 draw_skip;
} NsState;

/* Object storage from the pause object's parent at 0x80060DF0 through the
 * playback mode at 0x80061A30. The loop addresses all three through one base. */

typedef struct DrawStampView {
    u8 reserved_00[0x7C];
    u32 stamp;
} DrawStampView;

#define CRASH_EXT_CORE_MAIN_0
#include "core_externs.h"
#undef CRASH_EXT_CORE_MAIN_0

/* Spawn default HUD objects and load the level's initial resource entries. */
void Core_ObjectsCreate(void)
{
    Pad_Update();
    g_title_tile_load_cnt[0] = 0;
    switch (gp_pbak_mode) {
    case 0x38:
    case 0x39:
        break;
    case 0x19:
        Ns_Open(&g_ns.level->eid24, 0, 1);
        Ns_Open(&g_ns.level->eidE4, 0, 1);
        break;
    case 0x2D:
        Ns_Open(&g_ns.level->eid88, 0, 1);
        Ns_Open(&g_ns.level->eid9C, 0, 1);
        Ns_Open(&g_ns.level->eid20, 0, 1);
        break;
    default:
        g_hud_fruit_obj = GOOL_CreateObject(&g_handles, 4, 0, 0, 0, 0);
        g_hud_life_obj = GOOL_CreateObject(&g_handles, 4, 1, 0, 0, 0);
        g_hud_pickup_obj = GOOL_CreateObject(&g_handles, 4, 5, 0, 0, 0);
        Ns_Open(&g_ns.level->eid14, 0, 1);
        Ns_Open(&g_ns.level->eid28, 0, 1);
        Ns_Open(&g_ns.level->eid88, 0, 1);
        if (gp_pbak_mode != 0x2C) {
            Ns_Open(&g_ns.level->eid9C, 0, 1);
        }
        Ns_Open(&g_ns.level->eid20, 0, 1);
        Ns_Open(&g_ns.level->eid24, 0, 1);
        break;
    }
    Level_InitMisc(1);
}
#define CRASH_EXT_CORE_MAIN_1
#include "core_externs.h"
#undef CRASH_EXT_CORE_MAIN_1

/* .sdata owned by this unit (gp-addressed only here). */
s32 gp_cur_lid = 0;
s32 gp_next_lid = 0;
s32 gp_paused = 0x1000;
s32 gp_ram_size_bytes = 0x200000;
s32 gp_use_cd = 0;

/* .sbss owned by this unit. */
asm(".globl gp_rcnt_event, gp_core_init_done");
u32 gp_rcnt_event;
u32 gp_core_init_done;

/* Run levels until the exit flag is set: pause toggling, demo playback,
 * level transitions, per-frame update and zone-specific drawing. */
void CORE_Loop(s32 level)
{
    char debug_text[0xA8]; /* reserved in the retail frame, never accessed */
    s32 arg;
    s32 restore;
    s32 id;
    u32 flags;
    ZoneHeaderView *zone;

    Level_InitGlobals();
    Ns_Init(&g_ns, level);
    Core_ObjectsCreate();
    do {
        /* Start toggles pause outside the title, map and intro levels. */
        if ((g_gool_pads & 0x800) && g_pause_state.playback_mode == 0
            && (((id = g_ns.level->level_id) != 0x19 && id != 0x2D && id != 0x38
                 && g_pause_state.pause_override != -1) || g_key_count[0] > 0)) {
            if ((gp_next_lid = 1 - gp_next_lid) != 0) {
                if (g_pause_state.pause_object == 0) {
                    g_pause_state.pause_object = GOOL_CreateObject(&g_pause_state, 4, 4, 0, 0, 0);
                    if (g_pause_state.pause_object >= -0xFE) {
                        gp_next_lid = 0;
                        g_pause_state.pause_object = 0;
                        goto not_paused;
                    }
                    gp_use_cd = 1;
                    gp_rcnt_event = engine_counter[0];
                    gp_core_init_done = g_db_prev->stamp;
                }
            } else if (g_pause_state.pause_object != 0) {
                arg = 0;
                Gool_ExecStateCode(0, g_pause_state.pause_object, 0xC00, 1, &arg);
                g_pause_state.pause_object = 0;
                gp_use_cd = -1;
                engine_counter[0] = gp_rcnt_event;
                g_db_prev->stamp = gp_core_init_done;
            }
        } else {
not_paused:
            gp_use_cd = 0;
        }

        /* 0x6396347F is the null resource ID. */
        if (gp_crash_obj != 0 && gp_next_level_ldat_eid != 0x6396347F) {
            Pbak_Play(&gp_next_level_ldat_eid);
        }

        /* Game over and continue states return to the map (level 0x19). */
        if (gp_pbak_frame_idx == -1 && g_ns.level->level_id != 0x19
            && (g_game_substate == 0x200 || g_game_substate == 0x300 || g_game_substate == 0x400)) {
            gp_pbak_frame_idx = 0x19;
        }
        /* Level change; -2 restores the saved level after a bonus round. */
        if (gp_pbak_frame_idx != -1) {
            restore = 0;
            level = gp_pbak_frame_idx;
            Gool_SendToColliders(0, 0x2900, 0, 0, 0);
            if (gp_pbak_frame_idx == -2) {
                level = g_pbak_target_level_id;
                restore = 1;
                gp_cur_zone = 1;
            } else {
                gp_cur_zone = 0;
            }
            g_ns.draw_skip = 2;
            Ns_Kill(&g_ns);
            gp_next_lid = 0;
            if (level == 0x19) {
                g_level_restart_fade_cnt[0] = 0;
                g_level_frame_accum = 0;
                g_checkpoint_id = 0;
                g_level_load_timer = 0;
                g_mono = 0;
                g_checkpoint_id_2[0] = -1;
            }
            Ns_Init(&g_ns, level);
            Core_ObjectsCreate();
            if (restore) {
                gp_pbak_frame_idx = -2;
                Level_SpawnObjects2();
                gp_pbak_frame_idx = -1;
                Level_Restart(g_checkpoint_save_state);
            }
            gp_cur_zone = 0;
        }

        Ns_Update(-1);
        Level_SpawnObjects2();
        if (gp_next_lid == 0) {
            if (((ZoneHeaderView *)g_pads->data)->flags & 0x600) {
                Shader_ParamsUpdate(0);
            }
            if (gp_next_lid == 0) {
                if (((ZoneHeaderView *)g_pads->data)->flags & 0x100) {
                    Shader_ParamsUpdateRipple(0);
                }
                if (gp_next_lid == 0) {
                    Cam_Update();
                }
            }
        }
        Gfx_UpdateMatrices();

        if (g_cur_lid_ro[0] & 1) {
            zone = (ZoneHeaderView *)g_pads->data;
            if (zone->word00 != 0 && gp_cur_lid == 0) {
                flags = zone->flags;
                if (flags & 0x400) {
                    Gfx_TransformWorldsDark2(g_ot_cur->ordering_table);
                } else if ((flags & 0x210) == 0x210) {
                    Gfx_TransformWorldsDark(g_ot_cur->ordering_table);
                } else if (flags & 0x10) {
                    Gfx_TransformWorldsFog(g_ot_cur->ordering_table);
                } else if (flags & 0x100) {
                    Gfx_TransformWorldsRipple(g_ot_cur->ordering_table);
                } else if (flags & 0x200) {
                    Gfx_TransformWorldsLightning(g_ot_cur->ordering_table);
                } else {
                    Gfx_TransformWorlds(g_ot_cur->ordering_table);
                }
            }
        }
        Gool_UpdateObjects(gp_next_lid == 0);
        if (g_ns.level->level_id == 0x19) {
            Title_Update(g_ot_cur->ordering_table);
        }
        Gpu_Update();
    } while (gp_bonus_return == 0);

    g_cur_lid_ro[0] = 0;
    Gpu_Update();
    Gpu_Update();
    Ns_Kill(&g_ns);
}
