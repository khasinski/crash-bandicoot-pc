#include "object_links.h"
s32 Gool_SendToColliders(Node *arg0, s32 arg1, s32 arg2, s32 arg3, s32 *arg4)
{
    struct { RAM_REF(Node) sender; s32 word04, word08, word0C, word10; s32 *word14; } message;
    void *context = &message;
    s32 index = 0;
    u8 *root = g_free_objects;
    message.sender = Ram_Addr(arg0);
    message.word04 = arg1;
    message.word08 = arg3;
    message.word0C = arg2;
    message.word10 = 0;
    message.word14 = arg4;
    do {
        Gool_ObjectHandleTraverseTreePostorder((Node *)root, &Gool_SendIfColliding, context);
        index++;
        root += 8;
    } while (index < 8);
    return -0xFF;
}
