#include "common.h"

/* Export the real small-common state object. */
extern s16 gp_midi_vol;
void Midi_Kill(void) {
    Midi_Close(gp_midi_vol,1);
    SsQuit();
}
