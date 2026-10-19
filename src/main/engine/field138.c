#include "gool.h"
typedef GoolRuntimeObject Object;

void Cam_SetObjectZoom(Object *object,s32 index,s32 fallback) {
    s32 bias = (object->flags120 & 0x10) ? 0 : 24;
    if (index >= 0) object->offset138 = bias + g_cam_zoom_presets[index];
    else object->offset138 = bias + fallback;
}
