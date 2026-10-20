#include "common.h"

/*
 * SLST delta decoding (0x800334A0..0x80033FBC). A zone path keeps one sorted
 * polygon list per step; the SLST entry stores the first and last lists in
 * full and deltas in between. Slst_Update applies a delta to the current list
 * (the reference) and writes the next one (the destination), going forward or
 * backward along the path.
 *
 * The original was hand-written assembly: Slst_RDecodeForward branches into
 * Slst_RDecodeBackward's bulk copy, which ends in the tails the symbol table
 * calls RMemcpy_0..RMemcpy_4. Here they are one merge with a plain halfword
 * copy.
 */

/* A polygon list: a count, then the polygon ids in draw order. */
typedef struct SlstList {
    u16 count;
    u16 flags;
    u16 items[];
} SlstList;

/*
 * A delta between two lists. The positions below are in halfwords from
 * `removed_end`. Forward, the runs from `runs` to `removed_end` remove items
 * of the reference, the runs up to `swaps_start` add items, and the entries up
 * to `swaps_end` then swap pairs of items of the result. Backward reverses
 * every step.
 */
typedef struct SlstDelta {
    u16 swaps_end;
    u16 flags; /* bit 0: a delta rather than a full list (Slst_Update) */
    u16 removed_end;
    u16 swaps_start;
    u16 runs[];
} SlstDelta;

/*
 * Runs of a delta. A header halfword holds the list position of the first item
 * (bits 0..11) and the item count minus one (bits 12..15); the item words
 * follow. Bit 15 of each later item word advances the position by one, and
 * the low 15 bits are the polygon id. 0xFFFF, or a header in the last
 * halfword of the section, ends the section.
 */
typedef struct SlstRun {
    const u16 *next, *end;
    s32 pos;  /* position of the current item */
    s32 left; /* items left in the current run; 0 when the section is done */
    u16 word; /* current item word */
} SlstRun;

#define SLST_END 0xFFFF
/* Position of an ended section: past any list. */
#define SLST_NO_POS (SLST_END << 8)

/* Reads the next run header and its first item. */
static void Slst_RunBegin(SlstRun *run)
{
    u16 header = *run->next++;

    if (run->next < run->end && header != SLST_END) {
        run->pos = header & 0xFFF;
        run->left = (header >> 12) + 1;
        run->word = *run->next++;
    } else {
        run->pos = SLST_NO_POS;
        run->left = 0;
    }
}

/* Moves to the next item of the run, or to the next run. */
static void Slst_RunStep(SlstRun *run)
{
    if (--run->left > 0) {
        run->word = *run->next++;
        run->pos += run->word >> 15;
    } else {
        Slst_RunBegin(run);
    }
}

static void Slst_RunInit(SlstRun *run, const u16 *start, const u16 *end)
{
    run->next = start;
    run->end = end;
    run->word = 0;
    Slst_RunBegin(run);
}

static s32 Slst_Min(s32 a, s32 b)
{
    return a < b ? a : b;
}

/*
 * Builds `dst` from `ref`: drops the items of the `removed` runs, inserts
 * those of the `added` runs, and copies the rest. Returns the item count.
 * The two directions turn run positions into reference indices differently
 * (see remove_at and add_at).
 */
static s32 Slst_Merge(SlstRun *removed, SlstRun *added, const SlstList *ref, SlstList *dst,
                      int backward)
{
    const u16 *src = ref->items;
    u16 *out = dst->items;
    s32 ref_count = ref->count;
    s32 taken = 0;   /* reference items consumed (copied or removed) */
    s32 n_removed = 0;
    s32 n_added = 0;
    s32 count = 0;   /* items written */

    while (taken < ref_count || added->left > 0 || removed->left > 0) {
        s32 remove_at, add_at;

        /* Reference index of the next removed item and of the item the next
         * added one goes before. */
        if (backward) {
            remove_at = removed->pos + n_removed;
            add_at = added->pos + n_removed - n_added - 1;
        } else {
            remove_at = removed->pos - 1;
            add_at = added->pos + n_removed;
        }

        if (taken < ref_count && removed->left > 0 && remove_at == taken) {
            src++;
            taken++;
            n_removed++;
            Slst_RunStep(removed);
        } else if (added->left > 0 && add_at == taken) {
            *out++ = added->word & 0x7FFF;
            count++;
            n_added++;
            Slst_RunStep(added);
        } else if (taken < ref_count) {
            /* Copy up to the next removal or insertion, at least one item. */
            s32 n = Slst_Min(Slst_Min(remove_at - taken, add_at - taken), ref_count - taken - 1);
            s32 i;

            if (n < 2)
                n = 1;
            for (i = 0; i < n; i++)
                *out++ = *src++;
            count += n;
            taken += n;
        }
    }
    return count;
}

/*
 * Decodes one swap entry at `entries[*at]` and advances `*at` past it,
 * reading forward. Entries are one of:
 *   1sss siii iiii iiii  index i (11 bits), span s (4 bits)
 *   01dd dddd ddds ssss  index = previous index + d, span s + 16
 *   00.. iiii iiii iiii  index i (12 bits), then a halfword with the span
 * The pair swapped is (index, index + span + 1).
 */
static void Slst_SwapForward(const u16 *entries, s32 *at, s32 *index, s32 *span)
{
    u16 e = entries[(*at)++];

    if (e & 0x8000) {
        *span = (e >> 11) & 0xF;
        *index = e & 0x7FF;
    } else if ((e & 0xC000) == 0x4000) {
        *span = (e & 0x1F) + 16;
        *index += (e >> 5) & 0x1FF;
    } else {
        *index = e & 0xFFF;
        *span = entries[(*at)++] & 0xFFF;
    }
}

static void Slst_Swap(u16 *items, s32 index, s32 span)
{
    u16 a = items[index];
    u16 b = items[index + span + 1];

    items[index + span + 1] = a;
    items[index] = b;
}

/*
 * The original saves s0..s7, t8, t9, k0, k1, gp, sp, fp and ra in the first
 * 64 bytes of the scratchpad and restores them on return. Nothing reads the
 * copies, but the scratchpad is compared against the translation, so this
 * stores what the translation stores when called from C: zero registers, its
 * stack (0x807FFB00) and its return sentinel.
 */
static void Slst_SaveRegisters(void)
{
    u32 *save = RAM_AT(u32, RAM_SCRATCH_BASE);

    memset(save, 0, 16 * sizeof(u32));
    save[13] = 0x807FFB00u;
    save[15] = 0xFFFFFFF0u;
}

/* Applies `source`, a delta, to `reference` and writes the next list of the
 * path to `destination`. */
void Slst_RDecodeForward(void *source, void *reference, void *destination)
{
    const SlstDelta *delta = source;
    const u16 *base = &delta->removed_end;
    SlstList *dst = destination;
    SlstRun removed, added;
    s32 count, at, index = 0, span;

    Slst_SaveRegisters();
    Slst_RunInit(&removed, delta->runs, base + delta->removed_end);
    Slst_RunInit(&added, base + delta->removed_end, base + delta->swaps_start);
    count = Slst_Merge(&removed, &added, reference, dst, 0);

    /* Swaps of the result, in order, up to the end marker. The index of a
     * relative entry starts from 0. */
    at = delta->swaps_start;
    while (at < delta->swaps_end) {
        if (base[at] == SLST_END)
            break;
        Slst_SwapForward(base, &at, &index, &span);
        Slst_Swap(dst->items, index, span);
    }
    dst->count = count;
}

/* Undoes `source`, the delta that leads to `reference`, and writes the
 * previous list of the path to `destination`. The swaps are undone on
 * `reference` itself, in reverse order. */
void Slst_RDecodeBackward(void *source, void *reference, void *destination)
{
    const SlstDelta *delta = source;
    const u16 *base = &delta->removed_end;
    SlstList *ref = reference;
    SlstRun removed, added;
    s32 at = delta->swaps_end - 1;

    Slst_SaveRegisters();
    while (at >= delta->swaps_start) {
        u16 e = base[at--];
        s32 index, span;

        if (e == SLST_END)
            continue;
        if (e & 0x8000) {
            span = (e >> 11) & 0xF;
            index = e & 0x7FF;
        } else if ((e & 0xC000) == 0x4000) {
            /* Relative to the index of the entry before it: a short entry
             * holds it in its low 11 bits, a long one in its first
             * halfword. */
            u16 prev = base[at];

            index = (prev & 0x8000) ? (prev & 0x7FF) : (base[at - 1] & 0xFFF);
            index += (e >> 5) & 0x1FF;
            span = (e & 0x1F) + 16;
        } else {
            /* Second halfword of a long entry. */
            span = e & 0xFFF;
            index = base[at--] & 0xFFF;
        }
        Slst_Swap(ref->items, index, span);
    }

    /* The items forward added are removed, and those it removed come back. */
    Slst_RunInit(&removed, base + delta->removed_end, base + delta->swaps_start);
    Slst_RunInit(&added, delta->runs, base + delta->removed_end);
    ((SlstList *)destination)->count = Slst_Merge(&removed, &added, ref, destination, 1);
}
