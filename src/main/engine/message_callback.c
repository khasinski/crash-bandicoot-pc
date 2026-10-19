#include "gool.h"
typedef GoolActorView Actor;
typedef GoolMessage Message;
     /* extern */
                   /* extern */
                    /* extern */

s32 Gool_SendIfColliding(Actor *arg0, Message *arg1) {
    s32 first[6],second[6];
    s32 temp_a0;
    s32 temp_v0;
    s32 temp_v1;
    s32 temp_v1_15;
    s32 temp_v1_8;
    Actor *temp_v1_10;
    Actor *temp_v1_11;
    Actor *temp_v1_12;
    Actor *temp_v1_13;
    Actor *temp_v1_14;
    Actor *temp_v1_16;
    Actor *temp_v1_17;
    Actor *temp_v1_18;
    Actor *temp_v1_19;
    Actor *temp_v1_20;
    Actor *temp_v1_21;
    Actor *temp_v1_2;
    Actor *temp_v1_3;
    Actor *temp_v1_4;
    Actor *temp_v1_5;
    Actor *temp_v1_6;
    Actor *temp_v1_7;
    Actor *temp_v1_9;

    temp_v0 = arg1->selector;
    if (temp_v0 == 0) goto send_message;
    switch (temp_v0) {
    case 4:
        temp_v1 = arg0->resource->entry->type;
        if ((temp_v1 == 0x300) || (temp_v1 == 0x400)) {
        case 1:
            first[0] = arg0->bounds[0] + arg0->position[0];
            first[1] = arg0->bounds[1] + arg0->position[1];
            first[2] = arg0->bounds[2] + arg0->position[2];
            first[3] = arg0->bounds[3] + arg0->position[0];
            first[4] = arg0->bounds[4] + arg0->position[1];
            first[5] = arg0->bounds[5] + arg0->position[2];
            temp_v1_2 = arg1->source;
            second[0] = temp_v1_2->bounds[0] + temp_v1_2->position[0];
            temp_v1_3 = arg1->source;
            second[1] = temp_v1_3->bounds[1] + temp_v1_3->position[1];
            temp_v1_4 = arg1->source;
            second[2] = temp_v1_4->bounds[2] + temp_v1_4->position[2];
            temp_v1_5 = arg1->source;
            second[3] = temp_v1_5->bounds[3] + temp_v1_5->position[0];
            temp_v1_6 = arg1->source;
            second[4] = temp_v1_6->bounds[4] + temp_v1_6->position[1];
            temp_v1_7 = arg1->source;
            second[5] = temp_v1_7->bounds[5] + temp_v1_7->position[2];
            if (Level_TestBoundIntersection(&first[0], &second[0]) == 0) {
                return -0xFF;
            }
            goto send_message;
        }
        return -0xFF;
    case 3:
        temp_v1_8 = arg0->resource->entry->type;
        if ((temp_v1_8 == 0x300) || (temp_v1_8 == 0x400)) {
        case 2:
            temp_v1_9 = arg1->source;
            second[0] = temp_v1_9->bounds[0] + temp_v1_9->position[0];
            temp_v1_10 = arg1->source;
            second[1] = temp_v1_10->bounds[1] + temp_v1_10->position[1];
            temp_v1_11 = arg1->source;
            second[2] = temp_v1_11->bounds[2] + temp_v1_11->position[2];
            temp_v1_12 = arg1->source;
            second[3] = temp_v1_12->bounds[3] + temp_v1_12->position[0];
            temp_v1_13 = arg1->source;
            second[4] = temp_v1_13->bounds[4] + temp_v1_13->position[1];
            temp_v1_14 = arg1->source;
            second[5] = temp_v1_14->bounds[5] + temp_v1_14->position[2];
            if (Level_TestPointInBound(arg0->position, &second[0]) == 0) {
                return -0xFF;
            }
            goto send_message;
        }
        return -0xFF;
    default: goto send_message;
    case 5:
        temp_v1_15 = arg0->resource->entry->type;
        if ((temp_v1_15 == 0x300) || (temp_v1_15 == 0x400)) {
            first[0] = arg0->bounds[0] + arg0->position[0];
            first[1] = arg0->bounds[1] + arg0->position[1];
            first[2] = arg0->bounds[2] + arg0->position[2];
            first[3] = arg0->bounds[3] + arg0->position[0];
            first[4] = arg0->bounds[4] + arg0->position[1];
            first[5] = arg0->bounds[5] + arg0->position[2];
            temp_v1_16 = arg1->source;
            second[0] = temp_v1_16->position[0] - temp_v1_16->radius[0];
            temp_v1_17 = arg1->source;
            second[1] = temp_v1_17->position[1] - temp_v1_17->radius[1];
            temp_v1_18 = arg1->source;
            second[2] = temp_v1_18->position[2] - temp_v1_18->radius[2];
            temp_v1_19 = arg1->source;
            second[3] = temp_v1_19->radius[0] + temp_v1_19->position[0];
            temp_v1_20 = arg1->source;
            second[4] = temp_v1_20->radius[1] + temp_v1_20->position[1];
            temp_v1_21 = arg1->source;
            second[5] = temp_v1_21->radius[2] + temp_v1_21->position[2];
            if (Level_TestBoundIntersection(&first[0], &second[0]) != 0) {
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
    Gool_ExecStateCode(arg1->source, arg0, arg1->word04, arg1->word08, arg1->word14);
    return -0xFF;
}
