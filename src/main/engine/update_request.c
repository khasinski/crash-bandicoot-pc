#include "common.h"

/* Storage is owned by engine/state. */

void Midi_SetStateStopped(void) {
    if (gp_card_chan == 0) gp_card_chan = 1;
}
