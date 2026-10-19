#include "gpu.h"

s32 _addque(GpuRect *rect, void *data, s32 flags)
{
    return _addque2(rect, data, 0, flags);
}
