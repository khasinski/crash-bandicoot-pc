#include "gpu.h"

typedef struct {u8 next[3];u8 length;u32 word04,word08;} Packet;
                        /* extern */
                        /* extern */

void SetDrawArea(Packet *arg0, GpuRect *arg1) {
    arg0->length = 2;
    arg0->word04 = get_cs(arg1->x, arg1->y);
    arg0->word08 = get_ce((s16) (((u16) arg1->x + arg1->w) - 1), (s16) (((u16) arg1->y + arg1->h) - 1));
}
