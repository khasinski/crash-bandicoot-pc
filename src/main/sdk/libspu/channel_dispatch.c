#include "common.h"

void _SsSndSetVol(s16 arg0, s32 arg1, s16 arg2, s16 arg3) {
    SpuVmSetSeqVol(((s32)(arg1 << 16) >> 8) | arg0, arg2, arg3, 1);
}

void SsSeqSetVol(s16 arg0, s16 arg1, s16 arg2) {
    _SsSndSetVol(arg0, 0, arg1, arg2);
}

void SsSepSetVol(s16 arg0, s16 arg1, s16 arg2, s16 arg3) {
    _SsSndSetVol(arg0, arg1, arg2, arg3);
}

void SsSeqGetVol(s16 arg0, s32 arg1, u16 *arg2, u16 *arg3) {
    SpuVmGetSeqVol(((s32)(arg1 << 16) >> 8) | arg0, arg2, arg3);
}
