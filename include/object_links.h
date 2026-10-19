#ifndef CRASH_OBJECT_LINKS_H
#define CRASH_OBJECT_LINKS_H

#include "common.h"
typedef struct Node {
    s32 kind;
    RAM_REF(struct Node) word04;
    u8 reserved_08[0x5C];
    RAM_REF(struct Node) parent;
    RAM_REF(struct Node) next;
    RAM_REF(struct Node) children;
} Node;
typedef s32 (*Visitor)(Node *, void *);
typedef s32 (*Traversal)(Node *, Visitor, void *);

s32 Gool_ObjectHandleTraverseTreePostorder();
void Gool_ForEachObjectHandle(Traversal traversal, Visitor visitor, void *context);
s32 Gool_ObjectTerminate();

extern RAM_REF(Node) g_level_pause_menu_obj;
#endif
