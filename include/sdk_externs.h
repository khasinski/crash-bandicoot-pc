#ifdef CRASH_EXT_SDK_KERNEL_INTR_INIT_0
extern IntrState g_event_handlers;
#endif

#ifdef CRASH_EXT_SDK_KERNEL_INTR_INIT_1
extern RAM_REF(Dispatch) g_intr_ctrl_ptr;
#endif

#ifdef CRASH_EXT_SDK_LIBAPI_CALLBACK_DISPATCH
extern RAM_REF(Dispatch) g_intr_ctrl_ptr;
#endif

#ifdef CRASH_EXT_SDK_LIBAPI_RESUME
extern u16 g_event_handlers[];
#endif

#ifdef CRASH_EXT_SDK_LIBAPI_ROOT_INTERRUPT
extern IntrState g_event_handlers;
#endif

#ifdef CRASH_EXT_SDK_LIBCD_BIOS_COM
extern CdCmdTable g_cd_com_sent_counters;
extern volatile CdStatus g_cd_status;
#endif

#ifdef CRASH_EXT_SDK_LIBCD_BIOS_INIT_0
extern CdReadState g_cd_read_state;
#endif

#ifdef CRASH_EXT_SDK_LIBCD_BIOS_INIT_1
extern volatile CdStatus g_cd_status;
#endif

#ifdef CRASH_EXT_SDK_LIBCD_BIOS_SYNC_0
extern CdReadState g_cd_read_state;
#endif

#ifdef CRASH_EXT_SDK_LIBCD_BIOS_SYNC_1
extern volatile CdStatus g_cd_status;
#endif

#ifdef CRASH_EXT_SDK_LIBCD_CALLBACKS
extern s32 g_cd_read_state[10];
#endif

#ifdef CRASH_EXT_SDK_LIBCD_INTERRUPT_CALLBACKS
extern volatile u8 g_cd_status[];
extern volatile u8 g_cd_in_callback[];
#endif

#ifdef CRASH_EXT_SDK_LIBCD_INTERRUPT_DISPATCH
extern volatile u8 g_cd_status[];
extern volatile u8 g_cd_in_callback;
#endif

#ifdef CRASH_EXT_SDK_LIBCD_RESET
extern volatile s8 g_cd_status;
extern volatile u8 g_cd_in_callback[];
#endif

#ifdef CRASH_EXT_SDK_LIBGPU_API_DISPATCH
extern u8 g_gpu_debug_level;
#endif

#ifdef CRASH_EXT_SDK_LIBGPU_COMMAND_DISPATCH
extern u8 g_gpu_debug_level;
extern s16 g_gpu_vram_max_w;
extern s16 g_gpu_vram_max_h;
#endif

#ifdef CRASH_EXT_SDK_LIBGPU_CONTROL_DISPATCH
extern u8 g_gpu_mode;
extern u8 g_gpu_debug_level;
extern u8 g_gpu_reverse_flag;
#endif

#ifdef CRASH_EXT_SDK_LIBGPU_CONTROL_VALUE
extern u8 g_gpu_mode;
extern u8 g_gpu_debug_level;
extern u8 g_gpu_reverse_flag;
#endif

#ifdef CRASH_EXT_SDK_LIBGPU_DEBUG_MODE_0
extern u8 g_gpu_mode;
#endif

#ifdef CRASH_EXT_SDK_LIBGPU_DEBUG_MODE_1
extern GpuState g_gpu_debug_level;
#endif

#ifdef CRASH_EXT_SDK_LIBGPU_DEBUG_MODE_2
extern GpuReverseState g_gpu_reverse_flag;
#endif

#ifdef CRASH_EXT_SDK_LIBGPU_DEBUG_MODE_3
extern GpuModeState g_gpu_debug_level_2;
#endif

#ifdef CRASH_EXT_SDK_LIBGPU_DISPLAY_ENABLE
extern GpuState g_gpu_debug_level;
#endif

#ifdef CRASH_EXT_SDK_LIBGPU_DISPLAY_ENVIRONMENT_READ
extern u8 g_gpu_cur_dispenv[0x14];
#endif

#ifdef CRASH_EXT_SDK_LIBGPU_DRAW_SUBMIT
extern GpuState g_gpu_debug_level;
#endif

#ifdef CRASH_EXT_SDK_LIBGPU_GET_GRAPH_DEBUG
extern u8 g_gpu_debug_level;
#endif

#ifdef CRASH_EXT_SDK_LIBGPU_HARDWARE_VERSION
extern u8 g_gpu_mode;
#endif

#ifdef CRASH_EXT_SDK_LIBGPU_ORDERING_TABLE
extern u8 g_gpu_debug_level;
#endif

#ifdef CRASH_EXT_SDK_LIBGPU_ORDERING_TABLE_DMA
extern volatile s16 g_gpu_vram_max_w[];
extern volatile s16 g_gpu_vram_max_h[];
#endif

#ifdef CRASH_EXT_SDK_LIBGPU_PACKET_LINK
extern u8 g_gpu_debug_level;
#endif

#ifdef CRASH_EXT_SDK_LIBGPU_PUT_DISP_ENV_0
extern GpuState g_gpu_mode;
#endif

#ifdef CRASH_EXT_SDK_LIBGPU_PUT_DISP_ENV_1
extern volatile GpuDebugState g_gpu_debug_level;
#endif

#ifdef CRASH_EXT_SDK_LIBGPU_PUT_DISP_ENV_2
extern volatile GpuReverseState g_gpu_reverse_flag;
extern volatile GpuDisplayEnv g_gpu_cur_dispenv;
#endif

#ifdef CRASH_EXT_SDK_LIBGPU_QUEUE_0
extern GpuState g_gpu_mode;
#endif

#ifdef CRASH_EXT_SDK_LIBGPU_QUEUE_1
extern volatile GpuActiveEntry g_gpu_queue_enabled;
extern volatile GpuQueueEntry g_gpu_queue_entries[64];
#endif

#ifdef CRASH_EXT_SDK_LIBGPU_QUEUE_RESET
extern u8 g_gpu_queue_entries[0x1800];
#endif

#ifdef CRASH_EXT_SDK_LIBGPU_RESET
extern u8 g_gpu_mode[];
extern u8 g_gpu_debug_level_2;
extern u8 g_gpu_debug_level;
extern u16 g_gpu_vram_max_w;
extern u16 g_gpu_vram_max_h;
#endif

#ifdef CRASH_EXT_SDK_LIBGPU_TEXTURE_WINDOW_ENCODE
extern volatile u8 g_gpu_mode[];
extern u8 g_gpu_reverse_flag;
extern volatile s16 g_gpu_vram_max_w[];
extern volatile s16 g_gpu_vram_max_h[];
#endif

#ifdef CRASH_EXT_SDK_LIBGPU_TIMEOUT_INIT
extern volatile s32 g_gpu_queue_enabled[];
#endif

#ifdef CRASH_EXT_SDK_LIBSND_SVM_KON
extern SvmCur g_midi_sep;
extern VoiceU16 g_spu_vm_voice_envx[];
#endif

#ifdef CRASH_EXT_SDK_LIBSND_SVM_VOICE_ALLOC
extern SvmCur g_midi_sep;
extern VoiceStruct g_spu_vm_voice_table[];
extern VoiceU16 g_spu_vm_voice_age[];
extern VoiceU16 g_spu_vm_voice_envx[];
extern VoiceS16 g_spu_vm_voice_prior[];
extern VoiceU8 g_spu_vm_voice_auto_flag[];
#endif

#ifdef CRASH_EXT_SDK_LIBSPU_BANK_SELECT
extern RAM_REF(u8) g_vab_prog_tables[];
extern RAM_REF(u8) g_vab_master_vol_table[];
extern RAM_REF(u8) g_vab_cur_master_vol;
#endif

#ifdef CRASH_EXT_SDK_LIBSPU_BANK_TRANSFER
extern RAM_REF(VabHdr) g_vab_master_vol_table[];
extern RAM_REF(ProgAtr) g_vab_prog_tables[];
#endif

#ifdef CRASH_EXT_SDK_LIBSPU_NOISE_VOICE_0
extern RAM_REF(volatile u16) g_snd_tempo;
#endif

#ifdef CRASH_EXT_SDK_LIBSPU_NOISE_VOICE_1
extern VoicePitch g_spu_vm_voice_pitch[];
#endif

#ifdef CRASH_EXT_SDK_LIBSPU_NOISE_VOICE_2
extern VoiceFlags g_spu_vm_voice_auto_flag[];
#endif

#ifdef CRASH_EXT_SDK_LIBSPU_NOISE_VOICE_3
extern VoiceVol g_midi_seq_table[];
#endif

#ifdef CRASH_EXT_SDK_LIBSPU_REVERB_ATTR_SET
extern SpuReverbRegisters g_spu_reverb_reg_offsets[];
#endif

#ifdef CRASH_EXT_SDK_LIBSPU_REVERB_DEPTH_SET
extern ReverbAttributes g_spu_reverb_attr;
#endif

#ifdef CRASH_EXT_SDK_LIBSPU_SEQUENCE_PITCH_BEND
extern u8 g_spu_vm_voice_seq_sep[];
extern u8 g_spu_vm_voice_tone_prog[];
extern u8 g_spu_vm_voice_tone_idx[];
extern u8 g_spu_vm_voice_vab_id[];
#endif

#ifdef CRASH_EXT_SDK_LIBSPU_SEQUENCE_VOICE_STOP
extern s16 g_spu_vm_voice_seq_sep[][26];
#endif

#ifdef CRASH_EXT_SDK_LIBSPU_SEQUENCE_VOLUME_0
extern u8 g_spu_vm_voice_vol[];
extern u8 g_spu_vm_voice_seq_sep[];
extern u8 g_spu_vm_voice_tone_idx[];
extern u8 g_midi_seq_table[];
#endif

#ifdef CRASH_EXT_SDK_LIBSPU_SEQUENCE_VOLUME_1
extern ProgramVoice g_spu_vm_voice_tone_prog[];
extern PanVoice g_spu_vm_voice_pan[];
extern RAM_REF(Master) g_vab_cur_master_vol;
#endif

#ifdef CRASH_EXT_SDK_LIBSPU_VM_NOISE_OFF
extern RAM_REF(SpuPorts) g_snd_tempo;
extern u8 g_spu_vm_voice_pitch[];
extern u8 g_spu_vm_voice_auto_flag[];
#endif

#ifdef CRASH_EXT_SDK_LIBSPU_VOICE_ADSR_TABLE
extern VoiceStruct g_spu_vm_voice_table[];
extern RAM_REF(u16) g_snd_tempo;
extern u16 g_midi_seq_table[];
#endif

#ifdef CRASH_EXT_SDK_LIBSPU_VOICE_NOTE_0
extern u8 g_spu_vm_voice_table[];
extern u8 g_spu_vm_voice_age[];
extern u8 g_spu_vm_voice_vol[];
extern u8 g_spu_vm_voice_pan[];
extern u8 g_spu_vm_voice_seq_sep[];
extern u8 g_spu_vm_voice_tone_prog[];
extern u8 g_spu_vm_voice_tone_idx[];
extern u8 g_spu_vm_voice_vab_id[];
extern u8 g_spu_vm_voice_prior[];
extern u8 g_spu_vm_voice_auto_flag[];
extern RAM_REF(VabHeader) g_vab_cur_master_vol;
#endif

#ifdef CRASH_EXT_SDK_LIBSPU_VOICE_NOTE_1
extern u8 g_midi_sep[32];
#endif

#ifdef CRASH_EXT_SDK_LIBSPU_VOICE_RELEASE
extern VoiceWord g_spu_vm_voice_table[];
extern VoiceWord g_spu_vm_voice_pitch[];
extern VoiceByte g_spu_vm_voice_auto_flag[];
#endif

#ifdef CRASH_EXT_SDK_LIBSPU_VOICE_STATE_INIT
extern RAM_REF(volatile u8) g_snd_tempo;
extern s32 g_spu_reverb_attr;
extern u8 g_spu_vm_voice_table[];
extern u8 g_spu_vm_voice_age[];
extern u8 g_spu_vm_voice_pitch[];
extern u8 g_spu_vm_voice_envx[];
extern u8 g_spu_vm_voice_vol[];
extern u8 g_spu_vm_voice_pan[];
extern u8 g_spu_vm_voice_seq_sep[];
extern u8 g_spu_vm_voice_tone_prog[];
extern u8 g_spu_vm_voice_tone_idx[];
extern u8 g_spu_vm_voice_auto_flag[];
extern u16 g_midi_seq_table[];
#endif

#ifdef CRASH_EXT_SDK_LIBSPU_VOICE_TONE_LIST
extern u8 g_midi_sep[];
#endif

#ifdef CRASH_EXT_SDK_LIBSPU_VOICE_TRIGGER
extern u8 g_spu_vm_voice_pitch[];
extern u8 g_spu_vm_voice_auto_flag[];
extern RAM_REF(u8) g_vab_cur_master_vol;
#endif

#ifdef CRASH_EXT_SDK_LIBSPU_VOICE_UPDATE_0
extern VoiceWord g_spu_vm_voice_vol[];
extern VoiceWord g_spu_vm_voice_seq_sep[];
extern VoiceWord g_spu_vm_voice_tone_prog[];
extern VoiceWord g_spu_vm_voice_tone_idx[];
extern VoiceWord g_spu_vm_voice_vab_id[];
#endif

#ifdef CRASH_EXT_SDK_LIBSPU_VOICE_UPDATE_1
extern VoiceVol g_midi_seq_table[];
extern RAM_REF(u8) g_vab_cur_master_vol;
#endif
