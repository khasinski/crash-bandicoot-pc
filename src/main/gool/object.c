#include "object_links.h"
#include "zone.h"

/* Call the object cleanup routine with its recursive/event flag enabled. */

s32 Gool_ObjectTerminate(Node *obj, void *context)
{
    return Gool_ObjectKill((struct GoolObj *)obj, 1);
}

/* Apply the callback to each of eight object roots. */

typedef struct ObjectRoot {
    s32 kind;
    RAM_REF(Node) word04;
} ObjectRoot;

void Gool_ZoneObjectsTerminate(void *object)
{
    s32 index = 0;
    u8 *root = g_free_objects;

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
    s32 entry;
    s32 index;

    if (*(volatile u32 *)&g_pads != 0) {
        zone = RAM_AT(ZoneHeader, RAM_AT(ResourceEntry, *(volatile u32 *)&g_pads)->data);
        for (i = 0; i < zone->neighbor_count; i++) {
            entry = Ns_Lookup(&zone->neighbors[i]);
            /* An error code matches no object's zone entry. */
            if (entry >= -0xFE)
                continue;
            index = 0;
            do {
                Gool_ObjectHandleTraverseTreePostorder((Node *)&((ObjectRoot *)g_free_objects)[index], Gool_ZoneObjectTerminate, RAM_AT(void, entry));
                index++;
            } while (index < 8);
        }
    }
}
