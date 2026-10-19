#ifdef CRASH_EXT_GOOL_ALLOC
extern RAM_REF(void) g_player;
extern RAM_REF(s32) g_ns_page_pool;
#endif

#ifdef CRASH_EXT_GOOL_BROADCAST_0
extern RAM_REF(LevelDesc) g_ns_page_pool[];
#endif

#ifdef CRASH_EXT_GOOL_BROADCAST_1
extern s32 g_fade_counter[];
extern u8 g_pbak_header_byte_0[];
extern u32 g_pbak_frame_cursor[];
#endif

#ifdef CRASH_EXT_GOOL_CHANGE_STATE_0
extern struct {
    u8 reserved[0x10];
    RAM_REF(u8) data;
} *g_pads;
extern struct {
    s32 word_0;
    s32 type;
} *g_ns_page_pool;
extern RAM_REF(void) gp_crash_obj;
extern RAM_REF(void) g_savestate[2];
#endif

#ifdef CRASH_EXT_GOOL_CHANGE_STATE_1
extern struct {
    u8 reserved[0x7C];
    u32 stamp;
} *g_db_prev[2];
#endif

#ifdef CRASH_EXT_GOOL_CHANGE_STATE_2
extern GoolUpdateGlobals gool_update;
#endif

#ifdef CRASH_EXT_GOOL_CREATE_OBJECT_0
extern RAM_REF(NsdTable) g_ns_page_pool[];
extern RAM_REF(GoolObj) g_player[];
extern RAM_REF(ResourceEntry) g_pads[];
extern GoolObj g_gool_obj_list_3;
extern RAM_REF(GoolObj) gp_crash_obj;
extern GoolObj g_gool_obj_list_7;
#endif

#ifdef CRASH_EXT_GOOL_CREATE_OBJECT_1
extern RAM_REF(ZoneEntity) g_mdat_title_str_ptr[];
extern GoolObj g_gool_obj_list_4;
#endif

#ifdef CRASH_EXT_GOOL_ENTRIES_9D08
extern RAM_REF(u16) gp_level_entry;
extern struct {
    u8 reserved[0x10];
    RAM_REF(u8) data;
} *g_pads;
extern s32 g_view_proj;
extern struct {
    s32 word_0;
    s32 type;
} *g_ns_page_pool;
#endif

#ifdef CRASH_EXT_GOOL_ENTRIES_A2F4
extern struct {
    u8 reserved[0x10];
    RAM_REF(ZoneFogHeader) data;
} *g_pads;
extern struct {
    s32 word_0;
    s32 type;
} *g_ns_page_pool;
#endif

#ifdef CRASH_EXT_GOOL_ENTRIES_A3CC
extern RAM_REF(u16) gp_level_entry[];
extern RAM_REF(ZoneHeader) g_pads[];
extern s32 g_view_proj[];
extern RAM_REF(NsdHeader) g_ns_page_pool[];
#endif

#ifdef CRASH_EXT_GOOL_ENTRIES_A5E0
extern RAM_REF(u16) gp_level_entry[];
extern struct {
    u8 reserved[0x10];
    RAM_REF(u8) data;
} *g_pads[];
#endif

#ifdef CRASH_EXT_GOOL_ENTRIES_A8CC
extern RAM_REF(u16) gp_level_entry[];
extern RAM_REF(ZoneHeader) g_pads[];
extern s32 g_view_proj[];
extern RAM_REF(NsdHeader) g_ns_page_pool[];
#endif

#ifdef CRASH_EXT_GOOL_ENTRIES_AAE0_0
extern RAM_REF(u16) gp_level_entry[];
extern struct {
    u8 reserved[0x10];
    RAM_REF(u8) data;
} *g_pads[];
extern RAM_REF(void) gp_crash_obj;
#endif

#ifdef CRASH_EXT_GOOL_ENTRIES_AAE0_1
extern GoolAllocGlobals gool_alloc;
extern GoolObj * volatile g_player[];
extern s32 g_gool_obj_list_7[];
extern RAM_REF(void) g_savestate[];
#endif

#ifdef CRASH_EXT_GOOL_INTERPRET
extern RAM_REF(GoolObj) gp_crash_obj;
extern RAM_REF(ResourceEntry) g_pads[];
extern RAM_REF(ResourceEntry) g_cur_zone_ptr[];
extern RAM_REF(PadStateView) g_db_cur[];
extern RAM_REF(LevelStateView) g_ns_page_pool[];
extern GoolObj g_gool_obj_list_4;
#endif

#ifdef CRASH_EXT_GOOL_OBJECT_RELEASE_CALLBACK
extern s32 g_cur_zone_ptr;
#endif

#ifdef CRASH_EXT_GOOL_PATH_FOLLOW
extern GoolVec3 g_path_orient_zero_vec;
extern Unk4Bytes gp_path_orient_step;
#endif

#ifdef CRASH_EXT_GOOL_PHYSICS_0
extern RAM_REF(ResourceEntry) g_pads[];
#endif

#ifdef CRASH_EXT_GOOL_PHYSICS_1
extern RAM_REF(GoolObj) gp_crash_obj;
#endif

#ifdef CRASH_EXT_GOOL_PHYSICS_2
extern GoolBoundsGlobals g_obj_bounds;
#endif

#ifdef CRASH_EXT_GOOL_PHYSICS_3
extern GoolMoveParams g_accel_states[];
extern GoolDirEntry g_move_states[];
extern s32 g_cam_dir_rot_y[];
extern RAM_REF(GoolTimeState) g_db_cur[];
extern u8 g_solid_wall_plot_buf[];
#endif

#ifdef CRASH_EXT_GOOL_PROJECT
extern u8 g_m_wgeo_view[];
extern s32 g_cam_trans_x;
#endif

#ifdef CRASH_EXT_GOOL_QUERY_NEAREST
extern struct {
    u8 reserved[0x10];
    RAM_REF(u8) data;
} *g_pads;
extern struct {
    s32 word_0;
    s32 type;
} *g_ns_page_pool;
#endif

#ifdef CRASH_EXT_GOOL_RESOURCE
extern s32 g_mdat_title_str_ptr;
#endif

#ifdef CRASH_EXT_GOOL_SEND_EVENT_0
extern RAM_REF(GoolTimeState) g_db_cur[];
#endif

#ifdef CRASH_EXT_GOOL_SEND_EVENT_1
extern RAM_REF(GoolObjView) gp_crash_obj;
#endif

#ifdef CRASH_EXT_GOOL_UPDATE_OBJECT_0
extern RAM_REF(ResourceEntry) g_pads[];
extern RAM_REF(GoolObj) gp_crash_obj;
#endif

#ifdef CRASH_EXT_GOOL_UPDATE_OBJECT_1
extern RAM_REF(GoolObj) g_savestate[];
#endif

#ifdef CRASH_EXT_GOOL_UPDATE_OBJECT_2
extern RenderProjectionState g_view_proj;
extern RAM_REF(RenderBufferView) g_ot_cur[];
#endif

#ifdef CRASH_EXT_GOOL_VM_OPERANDS_0
extern RAM_REF(GoolTimeState) g_db_cur[];
#endif

#ifdef CRASH_EXT_GOOL_VM_OPERANDS_1
extern GoolPad g_gool_pads[];
extern GoolDirEntry g_move_states[];
#endif
