#include "common.h"
typedef struct {
    u8 reserved_00[0xD8];
    s32 pid_flags;
} GoolObj;
void *Gool_ObjectHasPidFlags(GoolObj *object,s32 state) {
    if (object->pid_flags==state) return object;
    return 0;
}
