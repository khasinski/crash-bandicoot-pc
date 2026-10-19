#include "gool.h"
#include "resource.h"
/* Only the cache word at +0x7C is established by this routine. */
typedef struct LevelCacheView {
    u8 reserved_00[0x7C];
    u32 word7C;
} LevelCacheView;

#define CRASH_EXT_ENGINE_LEVEL_RESOURCE_INITIALIZE
#include "engine_externs.h"
#undef CRASH_EXT_ENGINE_LEVEL_RESOURCE_INITIALIZE

void Pbak_Start(void)
{
    GoolActorView *object;

    g_pbak_frame_step = 2;
    gp_cur_pbak_header = Ram_Addr(RAM_AT(LevelResourceView, gp_cur_pbak_frame)->bytes52C);
    Math_Sranda(RAM_AT(LevelResourceView, gp_cur_pbak_frame)->word04);
    object = RAM_AT(GoolActorView, gp_crash_obj);
    g_draw_stamp[0] = RAM_AT(LevelResourceView, gp_cur_pbak_frame)->word0C;
    __builtin_memcpy(&RAM_AT(LevelCacheView, g_db_prev)->word7C,
                     &RAM_AT(LevelResourceView, gp_cur_pbak_frame)->word50C, sizeof(u32));
    __builtin_memcpy(object->bounds, RAM_AT(LevelResourceView, gp_cur_pbak_frame)->bounds,
                     sizeof(object->bounds));
    g_checkpoint_id_2[0] = -1;
    Level_Restart((struct LevelSave *)RAM_AT(LevelResourceView, gp_cur_pbak_frame)->bytes10);
}

void Pbak_Play(s32 *resource_id)
{
    s32 arguments[2];
    ResourceEntry *entry;

    g_ns_load_done_flag[0] = 0;
    entry = RAM_AT(ResourceEntry, Ns_Open((u32 *)resource_id, 1, 1));
    *resource_id = 0x6396347F;
    gp_cur_pbak_frame = entry->data;
    arguments[0] = 0x8E7;
    arguments[1] = 0x4E19;
    GOOL_CreateObject((struct GoolObj *)g_handles, 4, 8, 2, arguments, 1);
    Pbak_Start();
}
