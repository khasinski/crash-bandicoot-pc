#include "common.h"

              /* extern */

typedef void (*SequenceCallback)();

void _SsInit(void) {
    u16 *defaults;
    SequenceCallback *row;
    SequenceCallback *slot;
    s32 index;
    s32 register_index;
    s32 count;
    volatile u16 *registers;
    u16 *source;
    u16 value;
    u16 global_value;

    registers = (volatile u16 *)0x1F801C00;
    index = 0;
    defaults = g_ss_zero_reverb_attr;
    do {
        count = 0;
        source = defaults;
write_voice_register:
        value = *source;
        source += 1;
        count += 1;
        *registers = value;
        registers += 1;
        if (count < 8) {
            goto write_voice_register;
        }
        index += 1;
    } while (index < 0x18);
    registers = (volatile u16 *)0x1F801D80;
    register_index = 0;
    source = g_ss_default_common_attr;
    do {
        global_value = *source;
        source += 1;
        register_index += 1;
        *registers = global_value;
        registers += 1;
    } while (register_index < 0x10);
    SpuVmInit(0x18);
    count = 0;
    row = g_ss_mark_callbacks;
    do {
        index = 0xF;
        slot = row + 15;
clear_callback:
        *slot = 0;
        index -= 1;
        slot -= 1;
        if (index >= 0) {
            goto clear_callback;
        }
        count += 1;
        row += 16;
    } while (count < 0x20);
    g_spawns_base = 0x3C;
    g_ss_open_seq_mask = 0;
    g_ss_callback_busy = 0;
}
