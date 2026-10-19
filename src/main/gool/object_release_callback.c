#include "gool.h"
#include "object_links.h"

typedef GoolRuntimeObject Object;
#define CRASH_EXT_GOOL_OBJECT_RELEASE_CALLBACK
#include "gool_externs.h"
#undef CRASH_EXT_GOOL_OBJECT_RELEASE_CALLBACK

/* Kill the object if it was spawned from the zone entry at context. */
s32 Gool_ZoneObjectTerminate(Node *node, void *context) {
    Object *arg0 = (Object *)node;
    s32 zone = (s32)Ram_Addr(context);
    s32 temp_s1;

    if (arg0->kind != 0) {
        temp_s1 = arg0->entry;
        if ((temp_s1 == zone) && !(arg0->flagsCC & 0x01000000) && !(arg0->flags120 & 0x40000) && ((Gool_ExecStateCode(0, (struct GoolObjView *)arg0, 0x1A00, 0, 0), (arg0->entry == temp_s1)) || (g_cur_zone_ptr == -1))) {
            return Gool_ObjectKill((struct GoolObj *)arg0, 0);
        }
    }
    return -0xFF;
}
