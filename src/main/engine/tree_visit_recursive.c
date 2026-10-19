#include "object_links.h"

#define NODE_CHILDREN(n) ((n)->kind == 2 ? (n)->word04 : (n)->children)
#define NODE_CHILDREN_PTR(n) ((n)->kind == 2 ? &(n)->word04 : &(n)->children)

s32 Gool_RemoveLevelSpawn(Node *node, Node *new_parent)
{
    Node *parent = RAM_AT(Node, node->parent);
    Node *prev;

    if (parent != NULL) {
        prev = RAM_AT(Node, NODE_CHILDREN(parent));
        if (prev == node) {
            *NODE_CHILDREN_PTR(parent) = node->next;
        } else if (prev != NULL) {
            while (RAM_AT(Node, prev->next) != node) {
                if (prev == NULL) {
                    return -0x18;
                }
                prev = RAM_AT(Node, prev->next);
                if (prev == NULL) {
                    return -0x18;
                }
            }
            prev->next = node->next;
        } else {
            return -0x18;
        }
    }
    node->parent = Ram_Addr(new_parent);
    node->next = NODE_CHILDREN(new_parent);
    *NODE_CHILDREN_PTR(new_parent) = Ram_Addr(node);
    /* The original falls off the end with v0 = 2 from the kind test. */
    return 2;
}

s32 GOOL_TraverseObjectTreePreorder(Node *root, Visitor visitor, void *context)
{
    Node *child, *next;
    s32 result = visitor(root, context);
    if(result == -25) {
        Gool_ObjectKill((struct GoolObj *)root, 0);
        return -25;
    }
    child = RAM_AT(Node, root->children);
    while(child != NULL && child->kind != 0) {
        next = RAM_AT(Node, child->next);
        GOOL_TraverseObjectTreePreorder(child, visitor, context);
        child = next;
    }
    return result;
}
