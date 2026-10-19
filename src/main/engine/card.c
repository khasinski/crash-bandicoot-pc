#include "common.h"
#include "engine_ticks.h"
#include "engine_state.h"
#include "resource.h"
#include "gool.h"
#include "object_links.h"

typedef EngineFileRequest FileRequest;
/* Both cached words are compared before dispatching the current card mode. */

void Card_Update(void)
{
    u32 mode;

    if (gp_prev_card_flags != Card_GetFlags()) {
        gp_prev_card_flags = Card_GetFlags();
        Card_Noop2();
    }
    if (gp_prev_card_state != Card_GetState()) {
        gp_prev_card_state = Card_GetState();
        Card_Noop3();
    }
    mode = Card_GetState();
    switch (mode) {
    case 0: Card_PollEvents(); break;
    case 1: Card_Load(); break;
    case 5: Card_PartReadImages(); break;
    case 6: case 7: case 8: Card_PartWrite(Card_GetState()); break;
    case 2: Card_PartReadGlobals(); break;
    case 3: Card_Format(); break;
    case 4: Card_Check(); break;
    }
    engine_flag_shadow[0] = Card_GetFlags();
    g_card_prev_state = gp_card->word08;
}

void Card_PollEvents(void)
{
    s32 result;

    if (gp_card->word18) {
        gp_card->word18 = 0;
        _card_info(0);
    }
    result = Card_PollSoftEvent();
    switch (result) {
    case 1:
        Card_SetState(0);
        Card_SetFlags(2);
        return;
    case 0:
        if (!(Card_GetFlags() & 4)) {
            Card_SetState(0);
            return;
        }
    case 3:
        Card_ResetHardEvent();
        _card_clear(0);
        if (!Card_WaitHardEvent()) {
            Card_ClearFlags(6);
            Card_SetFlags(0x38);
            Card_SetState(4);
            Card_ResetRetryCount(150);
            goto reset_list;
        }
    case 2:
        Card_SetState(0);
        Card_SetFlags(4);
        Card_ClearFlags(8);
reset_list:
        Card_ResetParts(0);
        return;
    }
}

s32 Card_Check(void) {
    s32 result;
    gp_card->word18 = 0;
    if (Card_TestRead(0) == 1) {
        Card_ClearFlags(10);
        Card_SetState(0);
        Card_BeginLoadIfIdle();
        result = 1;
    } else {
        if (Card_IncRetryCount()) {
            Card_ResetParts(0);
            Card_ClearFlags(6);
            Card_SetFlags(8);
            Card_SetState(0);
            result = 1;
        } else {
            result = 0;
        }
    }
    return result;
}

s32 Card_Load(void)
{
    s32 result, count;

    if (gp_card->word18) {
        gp_card->word18 = 0;
        Card_ResetSoftEvent();
        _card_load(0);
    }
    result = Card_PollSoftEvent();
    switch (result) {
    case 0:
        count = Card_CountFiles(gp_str_card_dev_bu00, ((EngineDirectoryState *)gp_card)->records);
        Card_SortEntries(((EngineDirectoryState *)gp_card)->records, count);
        if (count > 0 || ++((EngineDirectoryState *)gp_card)->empty_scan_count >= 11) {
            Card_ClearFlags(2);
            Card_InitEmptyStrings();
            Card_SetState(5);
            Card_ResetRetryCount(32);
            gp_card->word08 = 0;
            ((EngineDirectoryState *)gp_card)->record_count = count;
            ((EngineDirectoryState *)gp_card)->record_cursor = 0;
            return 1;
        }
        goto retry;
    case 1:
        goto check_card;
    case 2:
check_card:
        if (!Card_IncRetryCount()) {
retry:
            Card_SetState(1);
            return 0;
        }
    case 3:
        Card_ResetParts(4);
        Card_SetFlags(2);
        Card_SetState(0);
        return 1;
    default:
        return 0;
    }
}

s32 Card_ClearUnused(s32 count,s32 limit,void (*callback)(void *),void *arg) {
    s32 result;
    if (count < limit) result = 0;
    else {callback(arg);result = 1;}
    return result;
}

void Card_OnReadImageError(s32 **resource) {
    if (Card_IncRetryCount()) {
        gp_card->word08 = 0;
        Card_SetFlags(6);
        Card_SetState(0);
    }
    Card_Close(*resource);
}

void Card_OnWriteError(FileRequest *request) {
    if (Card_IncRetryCount()) {
        Card_SetFlags(2);
        Card_SetState(0);
    }
    Card_Close(request->resource);
    if (*request->pending) {
        s32 retries;
        s32 status;
        VSync(60);
        retries = 0;
        do {
            status = erase(request->path);
            VSync(20);
            if (status == 1) break;
            retries++;
        } while (retries < 9);
        if (retries < 3) *request->pending = 0;
    }
}

void Card_OnReadGlobalsError(FileRequest *request) {
    if (Card_IncRetryCount()) {
        Card_SetFlags(2);
        Card_SetState(0);
    }
    Card_Close(request->resource);
}

typedef struct {
    s32 word00;
    s32 word04;
    s32 record_index;
    char title[64];
} EngineDirectorySlotView;

typedef struct {
    EngineState prefix;
    s32 empty_scan_count, record_count, record_cursor;
    EngineDirectorySlotView slots[15];
    EngineDirectoryRecord records[15];
} EngineDirectoryStateView;

typedef struct { s32 v; } WordView;
typedef struct { u32 v; } UWordView;

#define ENGINE_CARD_SCAN_RETRY(io, request) ({ \
    s32 retries = 0, failed; \
    do { \
        if (io) break; \
        retries++; \
    } while (retries < 3); \
    if (retries >= 3) { \
        Card_OnReadImageError((s32 **)(request)); \
        failed = 1; \
    } else { \
        failed = 0; \
    } \
    failed; \
})

void Card_PartReadImages(void)
{
    char path[64];
    u8 images[3][0x80];
    EngineCardBlock block;
    EngineFileRequest request;
    s32 descriptor;
    s32 invalid = 0;
    s32 slot_idx;
    s32 cur_rec;
    s32 blocks;
    s32 i, j;

    slot_idx = gp_card->word08;
    request.resource = &descriptor;
    descriptor = -1;
    if (((EngineDirectoryStateView *)gp_card)->record_cursor >= ((EngineDirectoryStateView *)gp_card)->record_count) {
        Card_ClearFlags(2);
        Card_SetState(0);
        return;
    }
    cur_rec = ((EngineDirectoryStateView *)gp_card)->record_cursor;
    sprintf(path, gp_str_card_concat_fmt, gp_str_card_dev_bu00,
        ((EngineDirectoryStateView *)gp_card)->records[cur_rec].bytes);
    if (ENGINE_CARD_SCAN_RETRY((descriptor = open(path, 1)) >= 0, &request))
        return;
    if (ENGINE_CARD_SCAN_RETRY(BIOS_Read(descriptor, &block, 128) == 128, &request))
        return;
    if (Card_EntryNameIs949((char *)((EngineDirectoryStateView *)gp_card)->records[cur_rec].bytes)) {
        ((EngineDirectoryStateView *)gp_card)->slots[slot_idx].word00 = 1;
    } else {
        ((EngineDirectoryStateView *)gp_card)->slots[slot_idx].word00 = 0;
    }
    ((EngineDirectoryStateView *)gp_card)->slots[slot_idx].word04 = block.header.icons - 0x10;
    if (((EngineDirectoryStateView *)gp_card)->slots[slot_idx].word04 < 1
        || ((EngineDirectoryStateView *)gp_card)->slots[slot_idx].word04 > 3) {
        ((EngineDirectoryStateView *)gp_card)->slots[slot_idx].word04 = 1;
        invalid = 1;
    }
    if (!invalid) {
        ((EngineDirectoryStateView *)gp_card)->slots[slot_idx].title[0] = 0;
        if ((u8)block.header.title[0] >= 0x80) {
            Card_ShiftJisToAscii((u8 *)&block);
        }
        block.header.title[31] = 0;
        BIOS_StrCpy((char *)((EngineDirectoryStateView *)gp_card)->slots[slot_idx].title,
            block.header.title);
    } else {
        __builtin_memcpy(((EngineDirectoryStateView *)gp_card)->slots[slot_idx].title,
            g_str_card_damaged_slot, 13);
    }
    Card_StrToUpper((u8 *)((EngineDirectoryStateView *)gp_card)->slots[slot_idx].title);
    if (invalid) {
        blocks = 1;
    } else {
        blocks = block.header.blocks;
    }
    ((EngineDirectoryStateView *)gp_card)->slots[slot_idx].record_index = cur_rec;
    if (!invalid) {
        for (i = 0; i < ((EngineDirectoryStateView *)gp_card)->slots[slot_idx].word04; i++) {
            if (ENGINE_CARD_SCAN_RETRY(BIOS_Read(descriptor, images[i], 128) == 128, &request))
                return;
        }
    }
    for (i = 0; i < blocks; i++) {
        char part_str[80];
        sprintf(part_str, g_str_card_part_d_of_d, i + 1, blocks);
        Gfx_FormatCardString(((WordView *)&g_card_icon)->v, (slot_idx + i) << 8,
            i == 0 ? (char *)((EngineDirectoryStateView *)gp_card)->slots[slot_idx].title : part_str,
            0x28, 1);
    }
    if (!invalid) {
        for (i = 0; i < blocks; i++) {
            for (j = 0; j < 3; j++) {
                s32 img_idx = j < ((EngineDirectoryStateView *)gp_card)->slots[slot_idx].word04 ? j : 0;
                Gfx_BlitCardIcon(1, ((UWordView *)&g_card_icon_vram_xy)->v, ((slot_idx + i) * 3 + j) << 8,
                    j == 0 ? block.header.palette : NULL,
                    images[img_idx]);
            }
        }
    }
    g_card_slot_valid_flags[slot_idx] = 0;
    g_card_slot_valid_flags[slot_idx] = ((EngineDirectoryStateView *)gp_card)->slots[slot_idx].word04 << 1;
    if (((EngineDirectoryStateView *)gp_card)->slots[slot_idx].word00 != 0) {
        if (!invalid) {
            j = 0;
            do {
                if (BIOS_Read(descriptor, &block, 128) == 128)
                    break;
                j++;
            } while (j < 30);
            if (j == 30) {
                ((EngineDirectoryStateView *)gp_card)->slots[slot_idx].word00 = 0;
                g_card_slot_valid_flags[slot_idx] |= 0x10;
            } else {
                g_card_slot_valid_flags[slot_idx] |= 8;
                g_card_slot_valid_flags[slot_idx] |= *(s32 *)&block << 5;
            }
        }
    }
    if (invalid) {
        g_card_slot_valid_flags[slot_idx] |= 0x20000;
    }
    g_card_slot_valid_flags[slot_idx] |= 1;
    Card_Close(&descriptor);
    if (!invalid) {
        for (j = 1; j < blocks; j++) {
            ((EngineDirectoryStateView *)gp_card)->slots[slot_idx + j] =
                ((EngineDirectoryStateView *)gp_card)->slots[slot_idx];
            g_card_slot_valid_flags[slot_idx + j] = g_card_slot_valid_flags[slot_idx];
        }
    }
    ((EngineDirectoryStateView *)gp_card)->record_cursor++;
    gp_card->word08 += blocks;
}

/* Convert a Shift-JIS memory-card block title in place to printable ASCII. */
void Card_ShiftJisToAscii(u8 *header)
{
    u8 *dst;
    s32 i, j;
    u8 c1, c2, out;
    s32 len = 64;

    header += 4;
    dst = header;
    for (i = 0; i < len; i += 2, header += 2) {
        c1 = header[0];
        c2 = header[1];
        if (c1 < 0x81) {
            out = c1;
            header--;
        } else if (c1 == 0xA0) {
            out = ' ';
            header--;
        } else if (c1 == 0x81) {
            out = '.';
            for (j = 0; g_sjis_ascii_table[j][0] != 0xFF; j++) {
                if (g_sjis_ascii_table[j][1] == c2) {
                    out = g_sjis_ascii_table[j][0];
                    break;
                }
            }
        } else if (c1 == 0x82) {
            out = '?';
            if ((c2 >= 0x60 && c2 <= 0x79) || (c2 >= 0x4F && c2 <= 0x58)) {
                out = c2 - 0x1F;
            }
            if (c2 >= 0x81 && c2 <= 0x9A) {
                out = c2 - 0x20;
            }
        } else if (c1 >= 0xA1 && c1 <= 0xDF) {
            out = '?';
            header--;
        } else {
            out = '?';
        }
        if (out != 0 && (out < 0x20 || out > 0x7E)) {
            out = ' ';
        }
        *dst++ = out;
        if (out == 0) {
            break;
        }
    }
}

/* Convert an ASCII title string in place to big-endian Shift-JIS characters. */
void Card_AsciiToShiftJis(char *title, s32 len)
{
    char buf[64];
    s32 i, j;
    s32 ended = 0;
    u16 *dst;
    char *p;
    u8 c;

    BIOS_StrCpy(buf, title);
    dst = (u16 *)title;
    p = buf;
    for (i = 0; i < len; p++, i++, dst++) {
        c = *p;
        if (ended) {
            *dst = 0x8140;
        } else if (c >= 'a' && c <= 'z') {
            *dst = c + 0x8220;
        } else if ((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')) {
            *dst = c + 0x821F;
        } else {
            *dst = 0x8140;
            for (j = 0; g_sjis_ascii_table[j][0] != 0xFF; j++) {
                if (g_sjis_ascii_table[j][0] == c) {
                    *dst = g_sjis_ascii_table[j][1] | 0x8100;
                    break;
                }
            }
            if (c == 0) {
                ended = 1;
                *dst = 0;
            }
        }
    }
    p = title;
    for (i = 0; i < len; i++, p += 2) {
        c = p[0];
        p[0] = p[1];
        p[1] = c;
    }
}

#define CRASH_EXT_ENGINE_CARD_0
#include "engine_externs.h"
#undef CRASH_EXT_ENGINE_CARD_0

static inline EngineDirectorySlot *engine_selected_file(void)
{
    s32 count = gp_card->word08;
    s32 index = gp_card->word0C;

    if (count > 0 && index < count)
        return &((EngineDirectoryState *)gp_card)->slots[index];
    return NULL;
}

/* This writer reuses its operation counter across filename selection,
 * buffer clearing and I/O attempts; each failed operation runs full cleanup. */
#define ENGINE_CARD_WRITE_RETRY(io, request, attempts) ({ \
    s32 failed; \
    attempts = 0; \
    do { \
        if (io) break; \
        attempts++; \
    } while (attempts < 3); \
    if (attempts >= 3) { \
        Card_OnWriteError(request); \
        failed = 1; \
    } else { \
        failed = 0; \
    } \
    failed; \
})

void Card_PartWrite(s32 mode)
{
    char path[64];
    u8 images[3][0x80];
    EngineCardBlock block;
    EngineFileRequest request;
    s32 descriptor = -1;
    EngineDirectorySlot *slot;
    s32 index, counter;

    slot = engine_selected_file();
    if (mode == 6) {
        gp_save_id = Math_Randb(999999);
        gp_file_is_new = 0;
    }
    request.resource = &descriptor;
    request.pending = (s32 *)&gp_file_is_new;
    request.path = path;
    if (slot != NULL && slot->word00 == 0) {
        sprintf(path, gp_str_card_concat_fmt, gp_str_card_dev_bu00,
            ((EngineDirectoryState *)gp_card)->records[slot->record_index].bytes);
        if (mode == 6) {
            VSync(20);
            if (!ENGINE_CARD_WRITE_RETRY(({
                    s32 status = erase(path);
                    VSync(20);
                    index = status;
                    index;
                }) == 1, &request, counter)) Card_SetState(7);
            return;
        }
    } else if (mode == 6) {
        Card_SetState(7);
        mode = 7;
    }
    if (slot != NULL && slot->word00 != 0) {
        sprintf(path, gp_str_card_concat_fmt, gp_str_card_dev_bu00,
            ((EngineDirectoryState *)gp_card)->records[slot->record_index].bytes);
    } else {
        counter = ((EngineDirectoryState *)gp_card)->record_count - 1;
        index = 0;
        if (counter >= 0) {
            if (Card_EntryNameIs949((char *)((EngineDirectoryState *)gp_card)->records[counter].bytes)) {
                EngineDirectoryRecord *record =
                    &((EngineDirectoryState *)gp_card)->records[counter];
                index = (record->bytes[12] - '0') * 10 + record->bytes[13] - '/';
            }
        }
        if (index >= 100) index = 99;
        sprintf(path, g_str_card_date_dash_fmt, gp_str_card_dev_bu00, g_str_card_bascus_94900, index, gp_save_id);
    }
    if (slot == NULL || slot->word00 == 0) {
        if (mode == 7) {
            VSync(20);
            if (!ENGINE_CARD_WRITE_RETRY(
                    (descriptor = open(path, 0x10200)) >= 0,
                    &request, counter)) {
                BIOS_Printf(g_str_card_file_descriptor_is_d, descriptor);
                Card_Close(&descriptor);
                gp_file_is_new = 1;
                Card_SetState(8);
            }
            return;
        }
    } else if (mode == 7) {
        Card_SetState(8);
    }
    if (ENGINE_CARD_WRITE_RETRY(
            (descriptor = open(path, 2)) >= 0, &request, counter)) return;
    for (counter = 127; counter >= 0; counter--) block.bytes[counter] = 0;
    block.header.magic[0] = 'S';
    block.header.magic[1] = 'C';
    block.header.icons = 0x13;
    block.header.blocks = 1;
    sprintf(block.header.title, g_str_card_title_fmt, g_card_save_pct);
    Card_AsciiToShiftJis(block.header.title, 32);
    Gfx_BlitCardIcon(2, g_card_icon_vram_xy, 0x2D00, block.header.palette, images[0]);
    Gfx_BlitCardIcon(2, g_card_icon_vram_xy, 0x2D00, NULL, images[1]);
    Gfx_BlitCardIcon(2, g_card_icon_vram_xy, 0x2E00, NULL, images[2]);
    if (ENGINE_CARD_WRITE_RETRY(
            BIOS_Write(descriptor, &block, 128) == 128, &request, counter)) return;
    for (index = 0; index < 3; index++) {
        if (ENGINE_CARD_WRITE_RETRY(
                BIOS_Write(descriptor, images[index], 128) == 128,
                &request, counter)) return;
    }
    Card_SaveGlobals(&block.record);
    for (index = 0; index < 5; index++) {
        if (ENGINE_CARD_WRITE_RETRY(
                BIOS_Write(descriptor, &block, 128) == 128,
                &request, counter)) return;
    }
    Card_Close(&descriptor);
    Card_ClearFlags(2);
    Card_SetState(0);
    Card_BeginLoadIfIdle();
    BIOS_StrCpy((char *)&g_card_filename, &path[5]);
    Card_ClearFlags(16);
}

#undef ENGINE_CARD_WRITE_RETRY

/* Word views of the runtime values serialised in the save record. */

void Card_SaveGlobals(EngineSaveRecord *record)
{
    s32 index;
    u32 settings;

    for (index = sizeof(*record) - 1; index >= 0; index--)
        ((u8 *)record)[index] = 0;
    record->word00 = g_item_pool2[0] | (g_key_count_2[0] << 5)
        | (g_tawna_round_done[0] << 10);
    record->word04 = g_item_pool2[0];
    record->word08 = g_new_gem_flag[0];
    record->word0C = g_saved_item_pool2_hi[0];
    record->word10 = g_stereo_disabled[0];
    record->word14 = g_cur_map_level[0];
    record->word18 = g_levels_unlocked[0];
    record->word1C = g_saved_gem_pool_lo[0];
    settings = g_audio_mode_flags[0];
    record->word24 = 0;
    record->word20 = settings;
    record->checksum = Card_Checksum((u8 *)record, sizeof(*record));
}

inline s32 Card_LoadGlobals(EngineSaveRecord *record)
{
    u32 level, settings;
    u32 checksum = record->checksum;

    record->checksum = 0;
    if (checksum != Card_Checksum((u8 *)record, sizeof(*record))) {
        return 0;
    } else {
        Level_ResetGlobals(1);
        level = record->word04;
        g_item_pool2[0] = level;
        g_new_gem_flag[0] = record->word08;
        g_saved_item_pool2_hi[0] = record->word0C;
        g_stereo_disabled[0] = record->word10;
        g_cur_map_level[0] = record->word14;
        g_levels_unlocked[0] = record->word18;
        g_saved_gem_pool_lo[0] = record->word1C;
        settings = record->word20;
        g_audio_mode_flags[0] = settings;
        g_next_level_id[0] = level;
        g_saved_level_id[0] = level;
        return 1;
    }
}

#include "engine/checksum.c"

/* Retry the I/O expression up to three times and release the event request
 * on failure. The expression is evaluated anew for each attempt. */
#define ENGINE_CARD_RETRY(io, request) ({ \
    s32 retries = 0, failed; \
    do { \
        if (io) break; \
        retries++; \
    } while (retries < 3); \
    if (retries >= 3) { \
        Card_OnReadGlobalsError(request); \
        failed = 1; \
    } else { \
        failed = 0; \
    } \
    failed; \
})

s32 Card_PartReadGlobals(void)
{
    char path[64];
    u8 blocks[3][0x80];
    EngineSaveRecord record;
    EngineFileRequest request;
    s32 descriptor = -1;
    EngineDirectorySlot *slot;
    s32 index;

    slot = engine_selected_file();
    request.resource = &descriptor;
    if (slot == NULL || slot->word00 == 0) {
        Card_SetFlags(2);
        Card_SetState(0);
        return 0;
    }
    sprintf(path, gp_str_card_concat_fmt, gp_str_card_dev_bu00,
        ((EngineDirectoryState *)gp_card)->records[slot->record_index].bytes);
    if (ENGINE_CARD_RETRY(
            (descriptor = open(path, 1)) >= 0, &request)) return 0;
    if (ENGINE_CARD_RETRY(
            BIOS_Read(descriptor, &record, sizeof(record)) == sizeof(record),
            &request)) return 0;
    for (index = 0; index < 3; index++) {
        if (ENGINE_CARD_RETRY(
                BIOS_Read(descriptor, blocks[index], sizeof(blocks[index]))
                    == sizeof(blocks[index]), &request)) return 0;
    }
    index = 0;
    do {
        if (ENGINE_CARD_RETRY(
                BIOS_Read(descriptor, &record, sizeof(record)) == sizeof(record),
                &request)) return 0;
        if (Card_LoadGlobals(&record)) break;
        index++;
    } while (index < 5);
    if (index == 5) {
        if (Card_IncRetryCount()) {
            Card_SetState(0);
            Card_SetFlags(2);
        }
    } else {
        Card_SetState(0);
    }
    Card_Close(&descriptor);
    return 1;
}

#undef ENGINE_CARD_RETRY

void Card_Format(void) {
    s32 ready;
    s32 state;
    /* Preserve the retail counter samples around the BIOS call. */
    (void)g_rcnt_ticks;
    ready = BIOS_Format(gp_str_card_dev_bu00);
    (void)g_rcnt_ticks;
    if (ready == 1) {
        Card_ClearFlags(2);
        state = 4;
    } else {
        if (Card_IncRetryCount() == 0) return;
        state = 2;
    }
    Card_SetFlags(state);
    Card_SetState(0);
}

void Card_Noop1(void)
{
}

void Card_Noop2(void)
{
}

void Card_Noop3(void)
{
}

s32 Card_ResetRetryCount(s32 arg0)
{
    gp_card->word14 = 0;
    gp_card->word10 = arg0;
    return 0;
}

s32 Card_IncRetryCount(void) {
    s32 temp_v0;

    temp_v0 = gp_card->word14 + 1;
    gp_card->word14 = temp_v0;
    return temp_v0 >= gp_card->word10;
}

s32 Card_PollSoftEvent(void) {
 Math_Randb(999);
 if (TestEvent(gp_edesc_s_end_io)==1) return 0;
 if (TestEvent(gp_edesc_s_error)==1) return 1;
 if (TestEvent(gp_edesc_s_timeout)==1) return 2;
 if (TestEvent(gp_edesc_s_newdev)==1) {Card_SetFlags(16);return 3;}
 return -1;
}

s32 Card_WaitSoftEvent(void) {
 s32 result;
 do {
  Math_Randb(999);
  if (TestEvent(gp_edesc_s_end_io)==1) result=0;
  else if (TestEvent(gp_edesc_s_error)==1) result=1;
  else if (TestEvent(gp_edesc_s_timeout)==1) result=2;
  else if (TestEvent(gp_edesc_s_newdev)==1) {Card_SetFlags(16);result=3;}
  else result=-1;
 } while (result==-1);
 return result;
}

#include "engine/state_event_test.c"

s32 Card_PollHardEvent(void) {
 Math_Randb(999);
 if (TestEvent(gp_edesc_h_end_io)==1) return 0;
 if (TestEvent(gp_edesc_h_error)==1) return 1;
 if (TestEvent(gp_edesc_h_timeout)==1) return 2;
 if (TestEvent(gp_edesc_h_newdev)==1) return 3;
 return -1;
}

s32 Card_WaitHardEvent(void) {
 s32 result;
 do {
  Math_Randb(999);
  if (TestEvent(gp_edesc_h_end_io)==1) result=0;
  else if (TestEvent(gp_edesc_h_error)==1) result=1;
  else if (TestEvent(gp_edesc_h_timeout)==1) result=2;
  else if (TestEvent(gp_edesc_h_newdev)==1) result=3;
  else result=-1;
 } while (result==-1);
 return result;
}

void Card_ResetHardEvent(void) {
    TestEvent(gp_edesc_h_end_io);
    TestEvent(gp_edesc_h_error);
    TestEvent(gp_edesc_h_timeout);
    TestEvent(gp_edesc_h_newdev);
}

                      /* extern */

s32 Card_EntryNameIs949(const char *arg0) {
    return BIOS_StrNCmp(g_str_card_bascus_94900, arg0, 0xC) == 0;
}

/* Append the search suffix, enumerate records, and cap the reported count. */
s32 Card_CountFiles(const char *path, EngineDirectoryRecord *entries)
{
    char pattern[128];
    s32 count;
    EngineDirectoryRecord *found;

    BIOS_StrCpy(pattern, path);
    BIOS_StrCat(pattern, gp_str_card_wildcard);
    count = 0;
    found = firstfile(pattern, entries);
    while (entries == found) {
        count++;
        entries++;
        found = nextfile(entries);
    }
    if (count >= 16) count = 15;
    return count;
}

/* Sort Crash directory entries ahead of other saves and alphabetically. */
void Card_SortEntries(EngineDirectoryRecord *entries, s32 count)
{
    EngineDirectoryRecord temp;
    s32 i, j, is_crash, match;
    char *entry_name;

    for (i = 0; i < count; i++) {
        entry_name = (char *)&entries[i];
        is_crash = BIOS_StrNCmp(g_str_card_bascus_94900, entry_name, 12) == 0;
        for (j = i + 1; j < count; j++) {
            match = BIOS_StrNCmp(g_str_card_bascus_94900, (char *)&entries[j], 12) == 0;
            if (!is_crash) {
                if (match) {
                    continue;
                }
            } else {
                if (!match) {
                    goto swap;
                }
            }
            match = BIOS_StrCmp(entry_name, (char *)&entries[j]);
            if (match > 0) {
            swap:
                RMemcpy(&entries[i], &temp, 0x14);
                RMemcpy(&entries[j], &entries[i], 0x14);
                RMemcpy(&temp, &entries[j], 0x14);
                i--;
                break;
            }
        }
    }
}

void Card_Close(s32 *arg0) {
    s32 temp_a0;

    temp_a0 = *arg0;
    if (temp_a0 >= 0) {
        close(temp_a0);
    }
    *arg0 = -1;
}

void Card_WaitTicks(u32 ticks) {
    u32 start = g_rcnt_ticks;
    /* Preserve the retail unsigned subtraction order. */
    do {
    } while ((u32)(start - g_rcnt_ticks) < ticks);
}

void Card_StrToUpper(u8 *arg0) {
    u8 *var_a0;
    u8 temp_v1;

    var_a0 = arg0;
    if (*var_a0 != 0) {
        do {
            temp_v1 = *var_a0;
            if ((u32) (temp_v1 - 0x61) < 0x1AU) {
                *var_a0 = temp_v1 + 0xE0;
            }
            var_a0 += 1;
        } while (*var_a0 != 0);
    }
}

s32 Card_BeginLoadIfIdle(void)
{
    if (!(Card_GetFlags() & 13)) {
        EngineDirectoryState *state;
        Card_SetState(1);
        state = (EngineDirectoryState *)gp_card;
        state->prefix.word14 = 0;
        state->prefix.word10 = 64;
        state->empty_scan_count = 0;
        return 0;
    } else {
        return 1;
    }
}

s32 Card_ResetParts(s32 flags)
{
    s32 index;

    gp_card->word08 = 0;
    gp_card->word14 = 0;
    for (index = 14; index >= 0; index--) g_card_slot_valid_flags[index] = 0;
    Card_SetFlags(flags);
    return 0;
}

void Card_InitEmptyStrings(void)
{
    s32 channel;
    for (channel = 0; channel < 15; channel++) {
        Gfx_FormatCardString(g_card_icon, channel << 8, gp_str_card_empty, 0x28, 1);
    }
}

s32 Card_SetFlags(s32 arg0) {
    s32 temp_a0;

    temp_a0 = arg0 | gp_card->word04;
    gp_card->word04 = temp_a0;
    g_card_icon_count = temp_a0;
    return 0;
}

s32 Card_ClearFlags(s32 arg0)
{
    gp_card->word04 = ~arg0 & gp_card->word04;
    return 0;
}

/* The retail reset routine embeds this same mode/flag update. Keep its
 * inline copy and the public setter derived from one implementation. */
static inline s32 engine_set_mode(s32 mode)
{
    s32 flags;

    gp_card->word00 = mode;
    if (mode >= 0) {
        if (mode < 2 || mode == 4) gp_card->word18 = 1;
        if (gp_card->word00 == 0) {
            gp_card->word04 &= ~1;
        } else {
            goto activate;
        }
    } else {
activate:
        flags = gp_card->word04 | 1;
        gp_card->word04 = flags;
        g_card_icon_count = flags;
    }
    return 0;
}

s32 Card_SetState(s32 mode)
{
    return engine_set_mode(mode);
}

s32 Card_GetState(void)
{
    return gp_card->word00;
}

s32 Card_GetFlags(void)
{
    return gp_card->word04;
}

void Card_Init(void) {
EnterCriticalSection();
gp_edesc_s_end_io=OpenEvent(0xF4000001,4,0x2000,NULL);
gp_edesc_s_error=OpenEvent(0xF4000001,32768,0x2000,NULL);
gp_edesc_s_timeout=OpenEvent(0xF4000001,256,0x2000,NULL);
gp_edesc_s_newdev=OpenEvent(0xF4000001,8192,0x2000,NULL);
gp_edesc_h_end_io=OpenEvent(0xF0000011,4,0x2000,NULL);
gp_edesc_h_error=OpenEvent(0xF0000011,32768,0x2000,NULL);
gp_edesc_h_timeout=OpenEvent(0xF0000011,256,0x2000,NULL);
gp_edesc_h_newdev=OpenEvent(0xF0000011,8192,0x2000,NULL);
ExitCriticalSection();
VSync(5);
_card_auto(1);
VSync(5);
InitCARD();
VSync(5);
_bu_init();
VSync(5);
EnableEvent(gp_edesc_s_end_io);
EnableEvent(gp_edesc_s_error);
EnableEvent(gp_edesc_s_timeout);
EnableEvent(gp_edesc_s_newdev);
EnableEvent(gp_edesc_h_end_io);
EnableEvent(gp_edesc_h_error);
EnableEvent(gp_edesc_h_timeout);
EnableEvent(gp_edesc_h_newdev);
}

s32 Card_Kill(void) {
    StartCARD();
    return 0;
}

/* Fifteen consecutive words ending at 0x80061A0C. */

void Card_ResetStateAndFlags(s32 flags)
{
    s32 index, value;

    gp_card->word08 = 0;
    gp_card->word14 = 0;
    for (index = 14; index >= 0; index--) g_card_slot_valid_flags[index] = 0;
    Card_SetFlags(0);
    engine_set_mode(0);
    value = flags | (gp_card->word04 & ~63);
    gp_card->word04 = value;
    g_card_icon_count = value;
}

/* Six-byte state copied as an unaligned record in retail. */
typedef EngineByteRecord ByteRecord;
#define CRASH_EXT_ENGINE_CARD_1
#include "engine_externs.h"
#undef CRASH_EXT_ENGINE_CARD_1

s32 Card_Alloc(void)
{
    EngineState *state = LIBC_Calloc(1, 0x6F4);
    gp_card = state;
    if (gp_card_new_device != 0) {
        state->word04 |= 0x10;
        g_card_icon_count = state->word04;
        sprintf((char *)&g_card_filename, (char *)&gp_str_card_invalid_tag);
    } else {
        state->word04 &= ~0x10;
    }
    Card_ResetStateAndFlags(4);
    return 0;
}

s32 Card_Free(void)
{
    gp_card_new_device = ((u32)gp_card->word04 >> 4) & 1;
    if (gp_card != NULL) {
        LIBC_Free(gp_card);
        gp_card = NULL;
    }
    return 0;
}

/* Each event corresponds to one of the four native completion statuses. */
static inline s32 engine_card_status(void)
{
    if (TestEvent(gp_edesc_h_end_io) == 1) return 0;
    if (TestEvent(gp_edesc_h_error) == 1) return 1;
    if (TestEvent(gp_edesc_h_timeout) == 1) return 2;
    if (TestEvent(gp_edesc_h_newdev) == 1) return 3;
    return -1;
}

s32 Card_TestRead(s32 port)
{
    u8 sector[0x80];
    s32 status;

    BIOS_BZero(sector, sizeof(sector));
    TestEvent(gp_edesc_h_end_io);
    TestEvent(gp_edesc_h_error);
    TestEvent(gp_edesc_h_timeout);
    TestEvent(gp_edesc_h_newdev);
    _card_read();
    StopCARD(port, 0, sector);
    do {
        Math_Randb(999);
        status = engine_card_status();
    } while (status == -1);
    if (status == 0) {
        if (sector[0] == 'M' && sector[1] == 'C') return 1;
        else return 0;
    }
    return -1;
}

s32 Card_ClearFlag20(void)
{
    gp_card->word04 &= ~0x20;
    return 0;
}

/* Alias view of the shared flag word; retain the native ordering of state
 * writes and their externally visible shadow store. */

s32 Card_StartWritePart(s32 value)
{
    EngineState *state = gp_card;

    if (!(state->word04 & 13)) {
        state->word00 = 6;
        engine_flag_shadow[0] = state->word04 |= 1;
        state->word14 = 0;
        state->word10 = 32;
        state->word0C = value;
        return 0;
    } else {
        engine_flag_shadow[0] = state->word04 |= 2;
        return 1;
    }
}
/* Select the directory slot matching the last active save filename. */
s32 Card_StartWriteNamedPart(void)
{
    s32 i;

    if (gp_card->word04 & 13) {
        engine_flag_shadow[0] = gp_card->word04 |= 2;
        return 1;
    }
    for (i = 0; i < gp_card->word08; i++) {
        if (BIOS_StrCmp((char *)&g_card_filename,
                (char *)((EngineDirectoryState *)gp_card)
                    ->records[((EngineDirectoryState *)gp_card)->slots[i].record_index]
                    .bytes) == 0) {
            break;
        }
    }
    if (i != gp_card->word08) {
        gp_card->word0C = i;
        gp_card->word00 = 6;
        engine_flag_shadow[0] = gp_card->word04 |= 1;
        gp_card->word14 = 0;
        gp_card->word10 = 32;
        return 0;
    } else {
        engine_flag_shadow[0] = gp_card->word04 |= 2;
        return 1;
    }
}

s32 Card_InvalidateFilename(void) {
    g_card_filename = gp_str_card_invalid_tag;
    return 0;
}

s32 Card_CheckNewDevice(void)
{
    s32 event;

    if (gp_card ? (gp_card->word04 & 16) : gp_card_new_device) {
        return 1;
    }
    if (BIOS_StrCmp((char *)&g_card_filename, (char *)&gp_str_card_invalid_tag) == 0) {
        return 1;
    }

    TestEvent(gp_edesc_s_end_io);
    TestEvent(gp_edesc_s_error);
    TestEvent(gp_edesc_s_timeout);
    TestEvent(gp_edesc_s_newdev);
    _card_info(0);

again:
    {
        Math_Randb(999);
        if (TestEvent(gp_edesc_s_end_io) == 1) {
            event = 0;
        } else if (TestEvent(gp_edesc_s_error) == 1) {
            event = 1;
        } else if (TestEvent(gp_edesc_s_timeout) == 1) {
            event = 2;
        } else if (TestEvent(gp_edesc_s_newdev) == 1) {
            Card_SetFlags(16);
            event = 3;
        } else {
            event = -1;
        }
    }
    if (event == -1) {
        goto again;
    }
    switch (event) {
    case 0:
        return 0;
    case 1:
    case 2:
    case 3:
        return 1;
    default:
        return 1;
    }
}

s32 Card_ProbeTimeout(void)
{
    s32 event;

    TestEvent(gp_edesc_s_end_io);
    TestEvent(gp_edesc_s_error);
    TestEvent(gp_edesc_s_timeout);
    TestEvent(gp_edesc_s_newdev);
    _card_info(0);

again:
    {
        Math_Randb(999);
        if (TestEvent(gp_edesc_s_end_io) == 1) {
            event = 0;
        } else if (TestEvent(gp_edesc_s_error) == 1) {
            event = 1;
        } else if (TestEvent(gp_edesc_s_timeout) == 1) {
            event = 2;
        } else if (TestEvent(gp_edesc_s_newdev) == 1) {
            Card_SetFlags(16);
            event = 3;
        } else {
            event = -1;
        }
    }
    if (event == -1) {
        goto again;
    }
    {
        s32 result = event;
        if (result != 2) {
            result = 0;
        } else {
            result = 1;
        }
        return result;
    }
}
s32 Card_StartReadPart(s32 value)
{
    EngineState *state = gp_card;

    if (!(state->word04 & 13)) {
        state->word00 = 2;
        engine_flag_shadow[0] = state->word04 |= 1;
        state->word14 = 0;
        state->word10 = 32;
        state->word0C = value;
        return 0;
    } else {
        engine_flag_shadow[0] = state->word04 |= 2;
        return 1;
    }
}
s32 Card_StartFormat(void)
{
    EngineState *state = gp_card;

    if (state->word00 == 0) {
        state->word00 = 3;
        engine_flag_shadow[0] = state->word04 |= 1;
        state->word14 = 0;
        state->word10 = 3;
        return 0;
    } else {
        engine_flag_shadow[0] = state->word04 |= 2;
        return 1;
    }
}

s32 Card_RequestCheck(void) {
    s32 result;
    if (gp_card) {
        Card_ResetStateAndFlags((gp_card->word04 & 0x10) | 4);
        result = 0;
    } else {
        result = 1;
    }
    return result;
}
