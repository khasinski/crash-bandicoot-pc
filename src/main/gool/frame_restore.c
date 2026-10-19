#include "gool.h"

/* Packed frame offsets are measured in bytes from object offset 0x60. */
typedef GoolRuntimeObject Object;

s32 GOOL_CreateObjectStackFrame(Object *object, s32 arg_words, s32 flags)
{
    u32 *sp;
    u32 *old_fp;
    u32 *new_sp;
    u32 *new_fp;

    sp = object->stack;
    old_fp = object->frame;
    object->frame = sp;
    *object->stack++ = flags;
    *object->stack++ = object->pc;
    *object->stack++ = ((u32)(new_sp = sp - arg_words - 24) - (u32)object) | (((u32)(new_fp = old_fp - 24) - (u32)object) << 16);
    return -0xFF;
}

s32 Gool_ObjectPopFrame(Object *object,s32 *flags) {
    u32 packed;
    u32 frame_offset;
    u32 stack_offset;
    u32 restored_frame;
    u32 *frame;
    u32 *stack;
    u32 saved_flags;
    frame=object->frame;
    object->stack=frame+2;
    packed=frame[2];
    frame_offset=packed>>16;
    if(frame_offset==0) return -25;
    object->stack=frame+1;
    object->pc=frame[1];
    stack=object->stack;
    object->stack=stack-1;
    saved_flags = *(stack - 1);
    if(flags!=0) *flags=(*flags&0xFFFF0000)|(saved_flags&0xFFFF);
    stack_offset=(packed&0xFFFF)+0x60;
    object->stack=(u32 *)(stack_offset+(u32)object);
    restored_frame=frame_offset+0x60;
    object->frame=(u32 *)(restored_frame+(u32)object);
    return -255;
}
