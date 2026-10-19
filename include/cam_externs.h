#ifdef CRASH_EXT_CAM_CAMERA_0
extern CamGlobals cam_globals;
#endif

#ifdef CRASH_EXT_CAM_CAMERA_1
extern RAM_REF(void) gp_crash_obj;    /* the player object */
extern s32 g_gool_pads;      /* pad buttons pressed this frame */
extern s32 g_cam_trans_x;
extern RAM_REF(CamLevelView) g_ns_page_pool;
#endif
