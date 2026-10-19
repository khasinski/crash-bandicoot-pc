#include "object_links.h"

 /* extern */
                         /* extern */

s32 Gool_ObjectHandleTraverseTreePreorder(Node *arg0, s32 (*arg1)(Node *, void *), void * arg2) {
    Node *temp_s0;
    Node *temp_s1;
    Node *var_a0;
    Node *var_s0;

    if (arg0->kind == 2) {
        var_s0 = arg0->word04;
    } else {
        var_s0 = arg0->children;
    }
    if (var_s0 != NULL) {
        do {
            temp_s1 = var_s0->next;
            if (arg1(var_s0, arg2) == -0x19) {
                Gool_ObjectKill(var_s0, 0);
                var_s0 = temp_s1;
            } else {
                var_a0 = var_s0->children;
                var_s0 = temp_s1;
                if (var_a0 != NULL) {
loop_8:
                    if (var_a0->kind != 0) {
                        temp_s0 = var_a0->next;
                        GOOL_TraverseObjectTreePreorder(var_a0, arg1, arg2);
                        var_a0 = temp_s0;
                        if (var_a0 != NULL) {
                            goto loop_8;
                        }
                    }
                    var_s0 = temp_s1;
                }
            }
        } while (var_s0 != NULL);
    }
    return -0xFF;
}
