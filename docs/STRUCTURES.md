# Recovered structures

These notes were recovered in the matching decompilation. Offsets describe the
PS1 memory image and on-disc records; native host structures may differ where
they hold pointers. Instruction-matching remarks are historical.

The shared headers describe observed layouts on the PS1 32-bit ABI. Padding
marks unknown bytes. A partial view's `sizeof` is not the size of the complete
original object unless an allocation or copy establishes it.

| Header | Evidence and useful offsets |
|---|---|
| `gool.h` | Runtime object: entry `0x28`, flags `0xCC`/`0x120`, stack/PC/frame `0xDC`/`0xE0`/`0xE4`, value `0x138`; frame restore and release routines |
| `gool.h` | Separate defaults block (`0x30` bytes) and collision/message actor view: bounds `0x08`, resource `0x20`, position `0x80`, radius `0xB0` |
| `engine_state.h` | Lifecycle prefix; separate mode-25 allocation ending at `0x35D8`; file cleanup request's descriptor/flag pointers and pathname |
| `engine_state.h` | `0x80` save record with checksum `0x7C`; directory slots `0x28`/stride `0x4C` and records `0x49C`/stride `0x28`, ending at `0x6F4` |
| `resource.h` | Resource entries, handles, `0x2C` slots, eight-byte hash buckets, pool counter at `0x68C`, zone references at `0x23C` |
| `gpu.h`, `render.h` | `0x5C` drawing snapshot; buffer display `0x04`, drawing `0x18`, cursors `0x74`/`0x78`, ordering table `0x88` |

Do not merge views just because old local typedefs were called `Object` or
`Entry`. The message resource's entry type lives at `+4`, whereas a
`ResourceEntry` kind lives at `+8`. Hash buckets contain a tagged integer word
and an ID; they are not twenty-byte resource entries. The `D_800580A4` alias
starts four bytes into a slot but retains the `0x2C` array stride. Its signed
state view remains separate from the full slot's unsigned field.

The runtime object includes the `+4`, `+0x68` and `+0x6C` link fields from
the traversal view: both traversal cleanup and object release pass their
object to `func_8001CDD0`, supporting that shared prefix. `Node` remains a
small traversal view; callback signatures are not changed to assume that every
context argument is an object pointer.

The declarations are included in `m2c_context.h`, so first-pass decompilation
can use known prototypes and layouts. Public prototypes cover the confirmed
resource lookup/release, GPU drawing, traversal, and typed GOOL operations.
Ambiguous callback ABIs and formats retain local declarations pending evidence.

The engine-state storage owner suppresses the header's forward declarations
of its own globals. With GCC 2.6.3, introducing those declarations ahead of the
retail-ordered definitions changes tentative storage order and linked `gp`
offsets. Consumers and m2c still receive the shared declarations.

The setup routine `func_80031BF8` also establishes `word3598` and the paired
words at `0x35A0`/`0x35A4`. The latter receive the value of `D_8006194C` and
remain opaque integer words; copying a register does not establish a pointer
type. The shared layout retains the allocation's verified `0x35D8` size.

The routine now matches all 86 instructions with the stock GCC 2.6.3 profile.
Its scalar operand first translates the requested mode to 5/12 and then writes
the activation flag. It calls `func_80031EB4` without invented arguments: that
callee reads neither incoming argument register, and another caller invokes it
after a call that may clobber those registers, without preparing arguments.

`engine/state.c` owns the retail storage at `0x80056500..0x8005656C` and
`0x80056678..0x80056700`. The first range contains the request's initial value
one, twelve eight-byte name/format records, the lifecycle pointer and flag.
The second contains the cached request, opaque words, and the state globals.
Opaque words are storage views, not claims about the original semantic fields
or translation-unit boundaries. `object_defaults` and `update_request` use the
shared owners with sized-extern GNU `-G8` addressing; the resource-selection
alias retains the native absolute load. Fresh splitting, instruction scoring
of all 46 C functions across these three units, and the complete retail SHA-1
verify the ownership move. The strings are code-level resource identifiers and
format records, not extracted resource payloads.

`func_80017790` now matches 118 instructions as `render/mode_initialize.c`.
It preserves the current distance for unrecognised context modes, maps the
five recognised modes to their native distances, sets the nine matrix
halfwords, and resets the camera words. `RenderProjectionState` describes the
distance and matrix; `RenderCameraPrefix` derives the transform address from
the verified `0x6C` displacement. The matrix pointer is used for both its
first store and the GTE call, reproducing the native shared base register.

Further work on `func_80031EB4` established the map metadata count at `+0xC`
and a signed halfword at map-item offset `+0x18`. The unsigned filter must
compare the sign-extended halfword, not a word or zero-extended value. Native
iteration reads item index two before advancing the cursor; the first m2c
sketch misplaced that advancement. The setup still has a nonzero distance
and is not promoted; its larger stack frame needs evidence rather than
artificial padding.

Three more routines use these recovered views with the stock GCC 2.6.3
profile. `func_8003263C` matches 39 instructions while updating the mode-state
words at `0x3598` and `0x35D4`. `func_8003C714` matches 37 instructions with a
local opaque directory-record type: the native iterator establishes a `0x28`
stride, while the individual fields remain unknown. Its `while` loop counts
successful directory results and caps the count at fifteen.

`func_8003CD60` matches 48 instructions. It clears fifteen consecutive words
at `0x800619D4..0x80061A0C` and embeds the mode/flag update also implemented by
`func_8003CAF8`. A shared `static inline` helper reproduces both that embedded
copy and the existing public setter without changing compiler options. The
linked executable's retail SHA-1 verifies all three new routines and the
refactored setter together.

`EngineSaveRecord` is used by both the 61-instruction writer
`func_8003BE40` and the 50-instruction validator/restorer `func_8003BF34`.
Both process `0x80` bytes with the checksum word at `0x7C` cleared. The writer
also establishes the word at `0x24`; the remaining payload bytes stay opaque.
The same public restorer is inline in the 200-instruction memory-card reader
`func_8003C038`, matching both its separate function and its embedded copy.

The reader establishes selected-file slots at `0x28`, with a `0x4C` stride
and a directory-record index at slot `+8`. Fifteen slots end at `0x49C`, where
fifteen `0x28`-byte directory records fill the lifecycle allocation through
`0x6F4`. The header's directory view preserves the unknown slot fields and
keeps the mode-25 allocation separate. The reader uses the file cleanup
request's resource pointer for its file descriptor and retries each I/O
operation up to three times. A retry expression retains the operation at its
call site; passing a precomputed buffer pointer to an inline helper changes
the native register lifetimes. One counter serves the three-block and
five-record loops. All 200 instructions and the complete retail SHA-1 match.

The three requests `func_8003D028`, `func_8003D3D8`, and `func_8003D444` match
27, 27 and 26 instructions respectively. They use the lifecycle prefix and
a word alias for the shared flag shadow at `0x80061978`. The alias keeps the
state updates and shadow write in their native order. The new save and
directory sizes and offsets are checked by the target ABI assertions.

The 354-instruction writer `func_8003B8B8` uses `EngineCardHeader`: magic
bytes, icon flags and block count precede a 64-byte title at `+4`; sixteen
palette halfwords start at `+0x60`. `EngineCardBlock` reuses the same `0x80`
bytes for this header and an `EngineSaveRecord`. Three separate `0x80`-byte
icon images follow the header. The writer's filename selection, buffer
clearing and retry operations share a working counter, while the I/O result
is captured across the synchronization call. Its normal retry loop can attempt
each operation three times; it writes five copies of the final save record.
All 354 instructions match with the unchanged stock GCC 2.6.3 profile.

This writer also resolves the earlier request-type ambiguity. Its third word
stores the address of the pathname, which `func_8003AE08` forwards to the BIOS
cleanup wrapper. `EngineFileRequest` now types that word as `char *path`;
`EngineEventRequest` remains a compatibility typedef, not a separate layout.
The directory scan identifies `empty_scan_count` at `0x1C`, `record_count`
at `0x20` and `record_cursor` at `0x24`. The writer also uses the last
directory record to select a new filename.

`func_8003C9B8` and `func_8003CA04` each match 19 instructions for directory
refresh and list reset. The 70-instruction sector check `func_8003CEF0`
clears a `0x80`-byte sector buffer, initiates sector-zero I/O and polls the four
completion events. It returns one for an `MC` sector signature, zero for a
bad signature and minus one for an error status. The explicit return paths
retain the native result without relying on a missing C return. This sector
signature is distinct from the writer's `SC` file-header magic. Target ABI
assertions cover the header, working union and corrected pathname pointer.

`func_80031D50` is the first new match using these shared layouts. The native
function uses `EngineMode25State.word359C`, scans texture slot states 8..14
with the recovered `0x2C` stride, and selects a resource item through the first
item's word at `+0x204`. The separate `ResourceItemListView` describes that
item-pointer array at `+0x10`; it must not be mistaken for a page header.
An indexed loop over `ResourceSlotStateView` matches all 89 native instructions
under the unit's stock GCC 2.6.3 profile. The initial manual cursor loop had the
same instruction count but Levenshtein distance four from initialization order.
The linked executable also passes the retail checksum.

A direct field expression can change register allocation or alias analysis.
For the resource flag routines, the matching byte-pointer access is retained,
with `CRASH_OFFSETOF(GoolResourceState, flags2DC)` deriving its offset from the
shared structure instead of a duplicated constant. Do not add packing,
`volatile`, or dummy fields merely to influence compilation.

`func_80034504` increments the word at `0x80034520` with seven matching
instructions. The retained label `func_80034520` contains that writable
counter word; it is accessed through a word-array alias rather than called
as a C function. This does not change the report denominator.

The 337-instruction CD interrupt dispatcher `func_80043BA8` captures an
eight-byte response packet. It waits for a stable three-bit interrupt code,
drains up to eight FIFO bytes and zeroes the unread remainder. Response byte
zero supplies the cached 32-bit status at `D_800555B0`; byte one supplies the
word at `D_800555B4`. The error test uses status bits `0x1D`. A local status
mask is initialized before the padding loop and used for the word-cache
normalization, preserving the native byte-mask instruction with stock GCC
2.6.3 and ASPSX 2.21. The response publisher copies exactly eight bytes.
Scalar and array aliases preserve the SDK's distinct accesses to the volatile
event bytes at `0x8005587C` and `0x8005587D`. The five-entry switch table is
owned by this C unit at `0x8001156C`; the following retained assembly begins
at `0x800440EC`. Both linked instructions and the whole executable match.

Three more card routines match 236 instructions in total. The 76-instruction
dispatcher `func_8003A94C` compares cached flags and mode before choosing
one of nine mode entries, then publishes the flags and slot count. Its
cached words at `0x8005656C` and `0x80056570` both start at `0xFFFFFFFF` so
the first poll registers a change. The 74-instruction `func_8003AA7C` handles
card operation events and resets the list after completion or failure. The
86-instruction `func_8003AC2C` enumerates and sorts directory records,
accepting an empty directory after eleven empty scans. It resets the record
cursor when the list is ready. Distinct switch entries for event codes one
and two share the timeout check without becoming a range comparison. The
existing word alias for the flag shadow preserves the dispatcher's native
store/load order. Target assertions check all three named directory fields;
linked instructions and the complete executable match.

`func_8002E98C` now matches all 76 instructions using `ResourceEntry`,
`GoolActorView`, and a partial `LevelResourceView`. It resolves the level
resource, creates an object with a two-word argument array, copies six bounds
words from resource `+0x510` to actor `+8`, and updates the level globals.
The cache word is copied from resource `+0x50C` to cache `+0x7C`; keeping that
copy and the captured actor pointer reproduces the native load order. The
resource data beginning at `+0x52C` has an unknown length. Target ABI assertions
cover the recovered offsets without claiming the complete resource size.
Both object and linked instruction Levenshtein distances are zero, and the
whole rebuilt executable retains the retail SHA-1.

The adjacent resource chooser `func_8002EABC` matches all 62 instructions.
Its eight-byte name buffer copies a six-byte template and clears the last
two bytes through the BIOS. It selects the third character at random, derives
the fourth from the resource alphabet, and encodes the first five characters.
On the empty-list path, the mode is read before writing the sentinel ID, as
in retail. The mode uses sized-extern gp addressing; unsized views of the
other storage owners retain their absolute loads and stores. Object and
linked instruction distances are zero, the full retail SHA-1 is unchanged,
and all 23 tooling and layout tests pass.
