#include "common.h"
s32 SpuVmSeKeyOff(s16 bank,s16 program,u16 note) {
    return SpuVmKeyOff(0x21,bank,program,note);
}
