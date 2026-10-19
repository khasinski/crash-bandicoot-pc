#ifdef CRASH_EXT_LEVEL_CHECKPOINT_0
extern RAM_REF(LevelHeader) g_ns_page_pool[];
#endif

#ifdef CRASH_EXT_LEVEL_CHECKPOINT_1
extern RAM_REF(SaveObj) gp_crash_obj[2];
extern SaveVec g_checkpoint_spawn_trans;
#endif

#ifdef CRASH_EXT_LEVEL_CHECKPOINT_2
extern s32 g_view_proj;
extern u32 g_fade_counter;
#endif

#ifdef CRASH_EXT_LEVEL_COLLISION_0
extern RAM_REF(LevelStateView) g_ns_page_pool[];
#endif

#ifdef CRASH_EXT_LEVEL_COLLISION_1
extern SubBoxList g_obj_bounds;
extern ZqVec gp_solid_prev_vel;
#endif

#ifdef CRASH_EXT_LEVEL_GRID_SEARCH
extern SubWorkspace g_solid_wall_plot_buf;
#endif

#ifdef CRASH_EXT_LEVEL_GRID_WORKSPACE
extern SubBoxList g_obj_bounds;
#endif

#ifdef CRASH_EXT_LEVEL_LIGHT
extern RAM_REF(LevelHeader) g_ns_page_pool[];
#endif

#ifdef CRASH_EXT_LEVEL_OCTREE_0
extern RAM_REF(SubObj) gp_crash_obj[];
#endif

#ifdef CRASH_EXT_LEVEL_OCTREE_1
extern SubBoxList g_obj_bounds;
#endif

#ifdef CRASH_EXT_LEVEL_UPDATE_0
extern LvlCamera lvl_camera;
extern LvlColorState lvl_color;
extern s32 g_mdat_title_str_ptr[];
#endif

#ifdef CRASH_EXT_LEVEL_UPDATE_1
extern RAM_REF(void) gp_level_entry;
#endif
