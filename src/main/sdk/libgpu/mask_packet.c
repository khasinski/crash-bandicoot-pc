#include "common.h"

/* GPU mask command followed by a zero padding word. */
typedef struct {
    u8 link[3];
    u8 length;
    u32 word04, word08;
} MaskPacket;

void SetPriority(MaskPacket *arg0, s32 arg1, s32 arg2)
{
    s32 mode;

    arg0->length = 2;
    mode = 0xE6000000;
    if (arg1 != 0) {
        mode = 0xE6000002;
    }
    arg0->word04 = mode | (arg2 != 0);
    arg0->word08 = 0;
}
