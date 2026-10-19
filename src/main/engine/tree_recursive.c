#include "object_links.h"

s32 GOOL_TraverseObjectTreePostorder(Node *root, Visitor visitor, void *context)
{
    Node *child = root->children;
    Node *next;
    if(child != NULL) {
        do {
            next = child->next;
            if(GOOL_TraverseObjectTreePostorder(child, visitor, context) == -25)
                Gool_ObjectKill(root, 0);
            child = next;
        } while(child != NULL);
    }
    return visitor(root, context);
}

s32 Gool_ObjectSearchTree(Node *root, Visitor visitor, void *context)
{
    Node *child;
    Node *next;
    s32 result = 0;

    if (root == NULL) {
        return 0;
    }
    if (root->kind != 2) {
        result = visitor(root, context);
        if (result != 0) {
            return result;
        }
    }
    if (root->kind == 2) {
        child = root->word04;
    } else {
        child = root->children;
    }
    while (child != NULL && result == 0) {
        next = child->next;
        result = Gool_ObjectSearchTree(child, visitor, context);
        child = next;
    }
    return result;
}
