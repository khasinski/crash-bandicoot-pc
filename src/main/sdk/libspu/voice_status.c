#include "common.h"
typedef struct {u16 registers[8];} VoiceRegisters;

s32 SpuRGetAllKeysStatus(s32 first,s32 last,u8 *output) {
    s32 voice;
    s32 end;
    u8 *cursor;
    u16 level;
    if(first<0)first=0;
    if(first>=24)return -3;
    if(last>=24)last=23;
    if(last<0 || last<first)return -3;
    voice=first;
    end=last+1;
    if(voice<end) {
        s32 on=1,silent=3,off=2;
        cursor=(u8 *)(voice+(u32)output);
        do {
            level=((volatile VoiceRegisters *)g_spu_RXX)[voice].registers[6];
            if((on<<voice)&g_spu_irq_callback) {
                if(level)*cursor=on;else *cursor=silent;
            } else if(level)*cursor=off;else *cursor=0;
            voice++;
            cursor++;
        } while(voice<end);
    }
    return 0;
}

void SpuGetAllKeysStatus(u8 *output) {
    s32 end=24;
    s32 voice=0;
    s32 on=1,silent=3,off=2;
    u8 *cursor=output;
    u16 level;
    do {
        level=((volatile VoiceRegisters *)g_spu_RXX)[voice].registers[6];
        if((on<<voice)&g_spu_irq_callback) {
            if(level)*cursor=on;else *cursor=silent;
        } else if(level)*cursor=off;else *cursor=0;
        voice++;
        cursor++;
    } while(voice<end);
}
