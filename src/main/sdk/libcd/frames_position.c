#include "common.h"

u8 *CdIntToPos(s32 frames,u8 *position) {
    s32 seconds,minutes;
    s32 frame_bcd,second_tens,minute_tens;
    frames+=150;
    seconds=frames/75;
    frames%=75;
    frame_bcd=(frames/10)*16+frames%10;
    minutes=seconds/60;
    seconds%=60;
    second_tens=seconds/10;
    minute_tens=minutes/10;
    position[1]=second_tens*16+seconds%10;
    position[2]=frame_bcd;
    position[0]=minute_tens*16+minutes%10;
    return position;
}
