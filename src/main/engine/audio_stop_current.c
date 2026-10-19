#include "common.h"

/* Export the real small-common state object. */
extern s16 gp_midi_vol;
void Midi_Kill(void) {
    Midi_Close(gp_midi_vol,1);
    /* The retail routine the decompilation labels SsQuit is libsnd's SsEnd:
     * it stops the sequencer tick and restores the VSync callback. */
    SsEnd();
}
