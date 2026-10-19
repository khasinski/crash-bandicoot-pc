#include "object_links.h"
#include "zone.h"

/* Call the object cleanup routine with its recursive/event flag enabled. */

s32 Gool_ObjectTerminate(Node *obj, void *context)
{
    return Gool_ObjectKill(obj, 1);
}

/* Apply the callback to each of eight object roots. */

typedef struct ObjectRoot {
    s32 kind;
    RAM_REF(Node) word04;
} ObjectRoot;

void Gool_ZoneObjectsTerminate(void *object)
{
    s32 index = 0;
    u8 *root = (u8 *)((ObjectRoot *)g_free_objects);

    do {
        Gool_ObjectHandleTraverseTreePostorder((Node *)root, Gool_ZoneObjectTerminate, object);
        index++;
        root += 8;
    } while (index < 8);
}

void Gool_ZoneObjectTerminateSingle(void)
{
    ZoneHeader *zone;
    s32 i;
    void *entry;
    s32 index;

    if (*(ResourceEntry *volatile *)&g_pads != NULL) {
        zone = (ZoneHeader *)(*(ResourceEntry *volatile *)&g_pads)->data;
        for (i = 0; i < zone->neighbor_count; i++) {
            entry = Ns_Lookup(&zone->neighbors[i]);
            index = 0;
            do {
                Gool_ObjectHandleTraverseTreePostorder((Node *)&((ObjectRoot *)g_free_objects)[index], Gool_ZoneObjectTerminate, entry);
                index++;
            } while (index < 8);
        }
    }
}

