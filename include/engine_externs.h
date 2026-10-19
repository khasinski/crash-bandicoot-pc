#ifdef CRASH_EXT_ENGINE_ALLOCATE_BUFFERS
extern RAM_REF(void) gp_level_entry;
#endif

#ifdef CRASH_EXT_ENGINE_AUDIO_MODE_UPDATE
extern AudioChannel g_midi_seq_slot_b[];
#endif

#ifdef CRASH_EXT_ENGINE_AUDIO_RESET
extern VoiceEntry g_voices[];
extern VoiceEntry g_audio_vol_scale;
#endif

#ifdef CRASH_EXT_ENGINE_AUDIO_VOICE_0
extern RAM_REF(LevelHeaderInfo) g_ns_page_pool;
#endif

#ifdef CRASH_EXT_ENGINE_AUDIO_VOICE_1
extern VoiceEntry g_voices[24];
extern VoiceParams g_audio_voices_end;
extern u8 g_midi_seq_slot_b[];
#endif

#ifdef CRASH_EXT_ENGINE_AUDIO_VOICE_2
extern RAM_REF(ZonePtr57914) g_pads;
#endif

#ifdef CRASH_EXT_ENGINE_BIOS_CTRL
extern EngineByteRecord gp_str_card_invalid_tag[];
extern EngineByteRecord g_card_filename[];
#endif

#ifdef CRASH_EXT_ENGINE_CARD_0
extern EngineByteRecord g_card_filename;
#endif

#ifdef CRASH_EXT_ENGINE_CARD_1
extern ByteRecord gp_str_card_invalid_tag;
extern ByteRecord g_card_filename;
#endif

#ifdef CRASH_EXT_ENGINE_CD_DIR
extern NsFileMapBits g_file_map[];
#endif

#ifdef CRASH_EXT_ENGINE_DISPLAY_CLEAR
extern RenderSystem g_context;
#endif

#ifdef CRASH_EXT_ENGINE_FORMAT
extern FormatSpec g_sprintf_thousands_table;
#endif

#ifdef CRASH_EXT_ENGINE_LEVEL_EFFECTS_0
extern RAM_REF(LevelHeader) g_ns_page_pool[];
#endif

#ifdef CRASH_EXT_ENGINE_LEVEL_EFFECTS_1
extern RAM_REF(SaveObj) gp_crash_obj;
extern RAM_REF(SaveObj) g_shader_obj_color_scale;
#endif

#ifdef CRASH_EXT_ENGINE_LEVEL_RESOURCE_CHOOSE
extern s32 g_ns[];
#endif

#ifdef CRASH_EXT_ENGINE_LEVEL_RESOURCE_INITIALIZE
extern RAM_REF(u8) gp_cur_pbak_header;
extern RAM_REF(GoolActorView) gp_crash_obj;
extern RAM_REF(LevelResourceView) gp_cur_pbak_frame;
extern RAM_REF(LevelCacheView) g_db_prev;
#endif

#ifdef CRASH_EXT_ENGINE_LEVEL_START
extern RAM_REF(s32) g_ns_page_pool;
extern Node g_gool_obj_list_3;
extern s32 g_pad_debug_override;
extern s32 g_cam_rot_xz;
#endif

#ifdef CRASH_EXT_ENGINE_OBJECT_DEFAULTS
extern RAM_REF(Object) gp_crash_obj;
#endif

#ifdef CRASH_EXT_ENGINE_RESET
extern s32 gp_cur_pbak_frame;
#endif

#ifdef CRASH_EXT_ENGINE_SETTINGS_RESET
extern s32 g_island_cam_state;
#endif

#ifdef CRASH_EXT_ENGINE_SHUTDOWN_0
extern PadState g_pad_driver_state;
#endif

#ifdef CRASH_EXT_ENGINE_SHUTDOWN_1
extern RAM_REF(DemoInput) gp_cur_pbak_header;
extern RAM_REF(DemoRecording) gp_cur_pbak_frame;
extern RAM_REF(DemoBufferView) g_db_cur[];
extern PlaybackState g_pad_debug_override;
#endif

#ifdef CRASH_EXT_ENGINE_STATE_0
extern volatile s32 g_island_cam_state;
#endif

#ifdef CRASH_EXT_ENGINE_STATE_1
    extern s32 g_fade_counter[];
#endif

#ifdef CRASH_EXT_ENGINE_STATE_2
    extern s32 g_fade_counter[];
#endif

#ifdef CRASH_EXT_ENGINE_STATE_3
    extern s32 g_fade_counter[];
#endif

#ifdef CRASH_EXT_ENGINE_STATE_4
extern RAM_REF(RenderBuffer) g_db_prev;
#endif

#ifdef CRASH_EXT_ENGINE_STATE_5
extern s32 g_cam_rot_xz;
#endif

#ifdef CRASH_EXT_ENGINE_STATE_6
        extern s32 g_fade_counter[];
#endif

#ifdef CRASH_EXT_ENGINE_STATE_7
                extern s32 g_fade_counter[1];
#endif

#ifdef CRASH_EXT_ENGINE_STATE_WORD_COPY
extern BufferToken g_pbak_seed_val;
extern BufferToken g_pbak_frame_cursor[];
#endif

#ifdef CRASH_EXT_ENGINE_TAG_LIST
extern RAM_REF(Root) g_pads;
extern RAM_REF(Context) g_ns_page_pool;
#endif
