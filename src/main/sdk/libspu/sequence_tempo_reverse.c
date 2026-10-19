#include "common.h"
void _SsSndSetDecres(s16 sequence,s16 channel,s16 value,u32 duration) {
    _SsSndSetVolData(sequence,channel,value,duration);
    g_ss_score_table[sequence][channel].flags |= 0x20;
    g_ss_score_table[sequence][channel].flags &= ~0x10;
}
