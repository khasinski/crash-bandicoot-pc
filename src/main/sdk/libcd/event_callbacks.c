#include "common.h"
void def_cbsync(void) {BIOS_DeliverEvent(0xF0000003,0x20);}
void def_cbready(void) {BIOS_DeliverEvent(0xF0000003,0x40);}
void def_cbread(void) {BIOS_DeliverEvent(0xF0000003,0x40);}
