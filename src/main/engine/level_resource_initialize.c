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

__inline__ void Pbak_Start(void)
{
    GoolActorView *object;

    g_pbak_frame_step = 2;
    gp_cur_pbak_header = gp_cur_pbak_frame->bytes52C;
    Math_Sranda(gp_cur_pbak_frame->word04);
    object = gp_crash_obj;
    g_draw_stamp[0] = gp_cur_pbak_frame->word0C;
    __builtin_memcpy(&g_db_prev->word7C, &gp_cur_pbak_frame->word50C,
                     sizeof(g_db_prev->word7C));
    __builtin_memcpy(object->bounds, gp_cur_pbak_frame->bounds, sizeof(object->bounds));
    g_checkpoint_id_2[0] = -1;
    Level_Restart(gp_cur_pbak_frame->bytes10);
}

void Pbak_Play(s32 *resource_id)
{
    s32 arguments[2];
    ResourceEntry *entry;

    g_ns_load_done_flag[0] = 0;
    entry = Ns_Open(resource_id, 1, 1);
    *resource_id = 0x6396347F;
    gp_cur_pbak_frame = (LevelResourceView *)entry->data;
    arguments[0] = 0x8E7;
    arguments[1] = 0x4E19;
    GOOL_CreateObject(g_handles, 4, 8, 2, arguments, 1);
    Pbak_Start();
}
