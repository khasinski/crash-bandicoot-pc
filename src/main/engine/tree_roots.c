#include "object_links.h"

void Gool_ForEachObjectHandle(Traversal traversal, Visitor visitor, void *context)
{
    s32 i = 0;
    u8 *root = g_free_objects;
    do {
        traversal((Node *)root, visitor, context);
        i++;
        root += 8;
    } while(i < 8);
}
