#include "common.h"
#include "render.h"

typedef RenderTransformWords TransformWords;
void Gfx_ScreenProj(TransformWords *arg0, TransformWords *arg1) {
    arg0->word00 = (u16) arg1->word00;
    arg0->word02 = (u16) arg1->word02;
    arg0->word04 = (u16) arg1->word04;
    arg0->word06 = (s16) ((s32) -(arg1->word06 * 5) >> 3);
    arg0->word08 = (s16) ((s32) -(arg1->word08 * 5) >> 3);
    arg0->word0A = (s16) ((s32) -(arg1->word0A * 5) >> 3);
    arg0->word0C = (s16) -(s16) arg1->word0C;
    arg0->word0E = (s16) -(s16) arg1->word0E;
    arg0->word10 = (s16) -(s16) arg1->word10;
}
