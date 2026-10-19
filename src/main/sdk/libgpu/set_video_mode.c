#include "common.h"

s32 SetVideoMode(s32 arg0) {
    s32 temp_v0;

    temp_v0 = g_gpu_video_mode;
    g_gpu_video_mode = arg0;
    return temp_v0;
}
