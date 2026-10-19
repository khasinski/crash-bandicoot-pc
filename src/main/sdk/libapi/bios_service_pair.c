#include "common.h"
s32 _card_clear(s32 argument) {
    _card_read(argument);
    return _new_card(argument,0x3F,0);
}
