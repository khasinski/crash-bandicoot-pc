#ifndef CRASH_GOOL_H
#define CRASH_GOOL_H

#include "common.h"

/* Partial view for routines accessing the resource pointer at offset 0x10.
 * This is not yet a reconstruction of the full object/resource layout.
 */
typedef struct ObjectView {
    u8 reserved_00[0x10];
    RAM_REF(u8) resource;
} ObjectView;

/* Runtime object view assembled from release, frame restore and flag access.
 * Unknown bytes remain padding; this is a prefix, not the full object size.
 * entry is a tagged word, not an unconditionally dereferenceable pointer. */
typedef struct GoolRuntimeObject {
    s32 kind;
    RAM_REF(struct GoolRuntimeObject) word04;
    u8 reserved_08[0x20];
    s32 entry;
    u8 reserved_2c[0x3C];
    RAM_REF(struct GoolRuntimeObject) next;
    RAM_REF(struct GoolRuntimeObject) children;
    u8 reserved_70[0x5C];
    u32 flagsCC;
    u8 reserved_d0[0xC];
    RAM_REF(u32) stack;
    u32 pc;
    RAM_REF(u32) frame;
    u8 reserved_e8[0x38];
    u32 flags120;
    u8 reserved_124[0x14];
    s32 offset138;
} GoolRuntimeObject;

/* Independent view of the resource reached through ObjectView.resource. */
typedef struct GoolResourceState {
    u8 reserved_00[0x2DC];
    u32 flags2DC;
} GoolResourceState;

/* Default block copied by Gool_ObjectColorByZone; halfword updates at 0x2A..0x2E. */
typedef struct GoolDefaults {
    u8 bytes[0x2A];
    u16 intensity_r, intensity_g, intensity_b;
} GoolDefaults;

struct ResourceEntry;
typedef struct GoolDefaultsObject {
    u8 reserved_00[0x28];
    RAM_REF(struct ResourceEntry) entry;
    u32 word2C;
    GoolDefaults defaults;
} GoolDefaultsObject;

/* Message/collision view from Gool_SendIfColliding. Its resource points to a
 * two-word entry prefix whose type is at +4, unlike ResourceEntry.kind (+8).
 * Keep that distinct format explicit rather than merging by similar names. */
typedef struct GoolMessageEntry {
    s32 word00, type;
} GoolMessageEntry;

typedef struct GoolMessageResource {
    u8 reserved_00[0x10];
    RAM_REF(GoolMessageEntry) entry;
} GoolMessageResource;

typedef struct GoolActorView {
    u8 reserved_00[8];
    s32 bounds[6];
    RAM_REF(GoolMessageResource) resource;
    u8 reserved_24[0x5C];
    s32 position[3];
    u8 reserved_8c[0x24];
    s32 radius[3];
} GoolActorView;

/* Built on the host stack by the senders; argv points at host locals. */
typedef struct GoolMessage {
    RAM_REF(GoolActorView) source;
    s32 word04, word08, selector, count;
    s32 *argv;
} GoolMessage;

s32 Gool_ObjectPopFrame(GoolRuntimeObject *object, s32 *flags);
void *GOOL_TestObjectStateFlags(GoolRuntimeObject *object, u32 flags);
void Gool_ObjectColorByZone(GoolDefaultsObject *object, s32 mode);
void Level_MdatOnLoad2(ObjectView *object);
void Zone_TerminateObjects(ObjectView *object);
void Cam_SetObjectZoom(GoolRuntimeObject *object, s32 index, s32 fallback);

#endif
