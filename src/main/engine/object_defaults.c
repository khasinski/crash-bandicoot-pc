#include "gool.h"
#include "zone.h"

typedef GoolDefaults Defaults;
typedef ResourceEntry Entry;
typedef GoolDefaultsObject Object;
/* Storage is owned by engine/state; retain the absolute view of the
 * resource selection word while sized gp_crash_obj uses the native gp load. */
#define CRASH_EXT_ENGINE_OBJECT_DEFAULTS
#include "engine_externs.h"
#undef CRASH_EXT_ENGINE_OBJECT_DEFAULTS
void Gool_ObjectColorByZone(Object *object,s32 mode) {
    Entry *entry;
    Defaults *defaults;
    entry=object->entry;
    if(entry==0) entry=level_position.zone;
    if(object==gp_crash_obj) defaults=(Defaults *)(entry->data+0x348);
    else defaults=(Defaults *)(entry->data+0x318);
    switch(mode) {
    case 0: object->defaults=*defaults;return;
    case 1:
        object->defaults.intensity_r=defaults->intensity_r;
        object->defaults.intensity_g=defaults->intensity_g;
        object->defaults.intensity_b=defaults->intensity_b;
        return;
    }
}
