#include "gool.h"

/* Packed frame offsets are measured in bytes from object offset 0x60.
 * stack and frame hold PS1 addresses inside the object. */
typedef GoolRuntimeObject Object;

s32 GOOL_CreateObjectStackFrame(Object *object, s32 arg_words, s32 flags)
{
    u32 base = Ram_Addr(object);
    u32 sp;
    u32 old_fp;
    u32 new_sp;
    u32 new_fp;

    sp = object->stack;
    old_fp = object->frame;
    object->frame = sp;
    *RAM_AT(u32, object->stack) = flags;
    object->stack += 4;
    *RAM_AT(u32, object->stack) = object->pc;
    object->stack += 4;
    new_sp = sp - (arg_words + 24) * 4;
    new_fp = old_fp - 24 * 4;
    *RAM_AT(u32, object->stack) = (new_sp - base) | ((new_fp - base) << 16);
    object->stack += 4;
    return -0xFF;
}

s32 Gool_ObjectPopFrame(Object *object, s32 *flags) {
    u32 base = Ram_Addr(object);
    u32 packed;
    u32 frame_offset;
    u32 stack_offset;
    u32 *frame;
    u32 saved_flags;

    frame = RAM_AT(u32, object->frame);
    object->stack = object->frame + 8;
    packed = frame[2];
    frame_offset = packed >> 16;
    if (frame_offset == 0)
        return -25;
    object->pc = frame[1];
    object->stack = object->frame;
    saved_flags = frame[0];
    if (flags != 0)
        *flags = (*flags & 0xFFFF0000) | (saved_flags & 0xFFFF);
    stack_offset = (packed & 0xFFFF) + 0x60;
    object->stack = base + stack_offset;
    object->frame = base + frame_offset + 0x60;
    return -255;
}
