#ifndef CRASH_RESOURCE_H
#define CRASH_RESOURCE_H

#include "common.h"

/* Eight-byte handles cache a tagged entry word and its resource identifier. */
typedef struct ResourceHandle {
    u32 entry;
    u32 id;
} ResourceHandle;

typedef struct ResourceEntry {
    u32 word00;
    u32 id;
    s32 kind;
    s32 word0C;
    RAM_REF(u8) data;
} ResourceEntry;

/* Item-pointer view used by Title_LoadScreen on a resolved resource entry.
 * The first item is at +0x10; subsequent items follow at four-byte intervals.
 * Keep separate from ResourceHeader, whose page-header fields differ. */
typedef struct ResourceItemListView {
    u8 reserved_00[0x10];
    RAM_REF(u8) items[1];
} ResourceItemListView;

/* The first item's word at +0x204 selects another item in that list.
 * The remainder of this item's layout and its complete size are unknown. */
typedef struct ResourceItemSelection {
    u8 reserved_00[0x204];
    u32 word204;
} ResourceItemSelection;

/* First item of the map resource consumed by Title_LoadState. */
typedef struct ResourceMapMetadata {
    s32 word00, word04;
    u32 word08;
    s32 count;
} ResourceMapMetadata;

/* Partial level-data view used by Pbak_Play. Six words at +0x510
 * are copied to GoolActorView.bounds. The data beginning at +0x52C
 * has an unknown length; sizeof this view is not the resource size. */
typedef struct LevelResourceView {
    u8 reserved_00[4];
    s32 word04;
    u8 reserved_08[4];
    s32 word0C;
    u8 bytes10[0x4FC];
    u32 word50C;
    s32 bounds[6];
    u8 reserved_528[4];
    u8 bytes52C[1];
} LevelResourceView;

/* The native filter loads +0x18 with lh, then compares its sign-extended
 * value as unsigned. Do not replace this with a word or an unsigned halfword. */
typedef struct ResourceMapItem {
    u8 reserved_00[0x18];
    s16 word18;
} ResourceMapItem;

typedef struct ResourceHeader {
    u16 word00;
    u16 kind;
    u32 id;
    u32 count;
    u32 word0C;
    RAM_REF(ResourceEntry) entries[1];
} ResourceHeader;

/* The resource arrays advance by 0x2C bytes. Offsets and signedness are
 * recovered from processing, releasing and decrementing slot references.
 * Fields whose purpose remains unknown retain their offset names.
 */
typedef struct ResourceSlot {
    RAM_REF(ResourceHeader) header;
    u16 state;
    s16 word06;
    s16 word08;
    s16 references;
    u32 word0C;
    u32 word10;
    s32 id;
    s32 word18;
    RAM_REF(u8) base;
    RAM_REF(u32) mapping;
    RAM_REF(struct ResourceSlot) slot;
    u32 word28;
} ResourceSlot;

/* Hash lookup advances by eight bytes, not sizeof(ResourceEntry).
 * entry may contain a tagged/encoded address, so retain an integer word. */
typedef ResourceHandle ResourceBucket;

typedef struct ResourceHashMetadata {
    u8 reserved_00[0x404];
    s32 count;
} ResourceHashMetadata;

typedef struct ResourcePool {
    s32 count;
    ResourceSlot slots[38];
    s32 active_count;
} ResourcePool;

/* View based at slot + 4, used by the g_slst_buf1 alias in Ns_TexturePageFree.
 * The array still advances by 0x2C. Do not confuse this base with a full slot. */
typedef struct ResourceSlotStateView {
    s16 state;
    u8 reserved_02[0xE];
    s32 id;
    u8 reserved_14[8];
    RAM_REF(s32) id_pointer;
    u8 reserved_20[0xC];
} ResourceSlotStateView;

typedef struct ZoneLoadInfo {
    u8 reserved_00[0x234];
    s32 count;
    u32 word238;
    u32 refs[1];
} ZoneLoadInfo;

typedef struct ResourceHandler {
    RAM_REF(void) callback; /* s32 (*)(ResourceEntry *) */
    RAM_REF(void) release; /* s32 (*)(ResourceEntry *) */
    u32 words[5];
} ResourceHandler;

/* Shared owners confirmed by the lookup, allocation and unref units.
 * Offset aliases and the ambiguously typed g_ns_page_status_map table stay local. */
extern RAM_REF(ResourceEntry) g_pads;
extern ResourceSlot g_slst_buf0[];
extern RAM_REF(ResourceBucket *) g_ns_hash_buckets;
extern RAM_REF(ResourceBucket) g_ns_entry_table;
extern RAM_REF(ResourceHashMetadata) g_ns_nsd_header;
extern ResourcePool g_ns_virtual_page_pool;

s32 Ns_EIDType(ResourceHandle *handle);
s32 Ns_StringToEID(u8 *name);
s32 Ns_TexturePageAllocate(void);
s32 Ns_TexturePageFree(s32 index);
void Ns_PageDecRef(ResourceSlot *slot);
void Ns_PageDelete(ResourceSlot *slot);
ResourceBucket *Ns_Probe(s32 id);
s32 Ns_ProbeSafe(u32 id);

extern ResourceSlot g_spu_slots[8];
extern ResourceSlot g_ns_physical_pages[];
extern ResourceHandler g_ns_page_free_sentinel[];
extern ResourceSlotStateView g_slst_buf1[];
#endif
