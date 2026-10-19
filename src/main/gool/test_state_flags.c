#include "gool.h"
typedef GoolRuntimeObject Object;
void * GOOL_TestObjectStateFlags(Object *object,u32 flags) {
 if (object->flags120 & flags) return object;
 return NULL;
}
