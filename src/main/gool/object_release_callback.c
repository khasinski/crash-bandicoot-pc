#include "gool.h"

typedef GoolRuntimeObject Object;
#define CRASH_EXT_GOOL_OBJECT_RELEASE_CALLBACK
#include "gool_externs.h"
#undef CRASH_EXT_GOOL_OBJECT_RELEASE_CALLBACK
s32 Gool_ZoneObjectTerminate(Object *arg0, s32 arg1) {
    s32 temp_s1;

    if (arg0->kind != 0) {
        temp_s1 = arg0->entry;
        if ((temp_s1 == arg1) && !(arg0->flagsCC & 0x01000000) && !(arg0->flags120 & 0x40000) && ((Gool_ExecStateCode(0, arg0, 0x1A00, 0, 0), (arg0->entry == temp_s1)) || (g_cur_zone_ptr == -1))) {
            return Gool_ObjectKill(arg0, 0);
        }
        /* Duplicate return node #8. Try simplifying control flow for better match */
        return -0xFF;
    }
    return -0xFF;
}
