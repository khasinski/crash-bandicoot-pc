#include "gool.h"
typedef GoolActorView Actor;
typedef GoolMessage Message;

/* Traversal callback: deliver the message in context to node when the
 * selector's collision test passes. */
s32 Gool_SendIfColliding(struct Node *node, void *context) {
    Actor *arg0 = (Actor *)node;
    Message *arg1 = context;
    s32 first[6],second[6];
    s32 temp_a0;
    s32 type;
    Actor *source;

    if (arg1->selector == 0) goto send_message;
    switch (arg1->selector) {
    case 4:
        type = RAM_AT(GoolMessageEntry, RAM_AT(GoolMessageResource, arg0->resource)->entry)->type;
        if ((type == 0x300) || (type == 0x400)) {
        case 1:
            first[0] = arg0->bounds[0] + arg0->position[0];
            first[1] = arg0->bounds[1] + arg0->position[1];
            first[2] = arg0->bounds[2] + arg0->position[2];
            first[3] = arg0->bounds[3] + arg0->position[0];
            first[4] = arg0->bounds[4] + arg0->position[1];
            first[5] = arg0->bounds[5] + arg0->position[2];
            source = RAM_AT(Actor, arg1->source);
            second[0] = source->bounds[0] + source->position[0];
            second[1] = source->bounds[1] + source->position[1];
            second[2] = source->bounds[2] + source->position[2];
            second[3] = source->bounds[3] + source->position[0];
            second[4] = source->bounds[4] + source->position[1];
            second[5] = source->bounds[5] + source->position[2];
            if (Level_TestBoundIntersection((struct ZqBound *)first, (struct ZqBound *)second) == 0) {
                return -0xFF;
            }
            goto send_message;
        }
        return -0xFF;
    case 3:
        type = RAM_AT(GoolMessageEntry, RAM_AT(GoolMessageResource, arg0->resource)->entry)->type;
        if ((type == 0x300) || (type == 0x400)) {
        case 2:
            source = RAM_AT(Actor, arg1->source);
            second[0] = source->bounds[0] + source->position[0];
            second[1] = source->bounds[1] + source->position[1];
            second[2] = source->bounds[2] + source->position[2];
            second[3] = source->bounds[3] + source->position[0];
            second[4] = source->bounds[4] + source->position[1];
            second[5] = source->bounds[5] + source->position[2];
            if (Level_TestPointInBound((struct ZqVec *)arg0->position, (struct ZqBound *)second) == 0) {
                return -0xFF;
            }
            goto send_message;
        }
        return -0xFF;
    default: goto send_message;
    case 5:
        type = RAM_AT(GoolMessageEntry, RAM_AT(GoolMessageResource, arg0->resource)->entry)->type;
        if ((type == 0x300) || (type == 0x400)) {
            first[0] = arg0->bounds[0] + arg0->position[0];
            first[1] = arg0->bounds[1] + arg0->position[1];
            first[2] = arg0->bounds[2] + arg0->position[2];
            first[3] = arg0->bounds[3] + arg0->position[0];
            first[4] = arg0->bounds[4] + arg0->position[1];
            first[5] = arg0->bounds[5] + arg0->position[2];
            source = RAM_AT(Actor, arg1->source);
            second[0] = source->position[0] - source->radius[0];
            second[1] = source->position[1] - source->radius[1];
            second[2] = source->position[2] - source->radius[2];
            second[3] = source->radius[0] + source->position[0];
            second[4] = source->radius[1] + source->position[1];
            second[5] = source->radius[2] + source->position[2];
            if (Level_TestBoundIntersection((struct ZqBound *)first, (struct ZqBound *)second) != 0) {
                temp_a0 = arg1->count;
                if ((temp_a0 >= 3) && (temp_a0 != ((arg1->count / 5) * 5))) {
                    arg1->count = (s32) (temp_a0 + 1);
                } else {
                    goto send_message;
                }
            }
        }
        return -0xFF;
    }
    return -0xFF;
send_message:
    arg1->count++;
    Gool_ExecStateCode(RAM_AT(struct GoolObjView, arg1->source), (struct GoolObjView *)arg0,
                       arg1->word04, arg1->word08, RAM_AT(s32, arg1->argv));
    return -0xFF;
}
