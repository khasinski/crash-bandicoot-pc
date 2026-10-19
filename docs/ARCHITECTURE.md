# Architecture

The native target runs the original C modules, repaired in place, with CMake
and PsyZ. While the original game is being restored, no parallel engine, new
gameplay files or extracted functions are permitted.

The memory map and addresses below describe the original game. They help with
asset decoding and porting research. They are not host addresses, build inputs
or layout constraints, and this project does not rebuild or byte-match PS1
binaries.

## The original program

Crash Bandicoot (USA) boots `SCUS_949.00`, a PS-X EXE loaded at 0x80011000.
crt0 (0x8003E018) clears BSS, initializes the heap at the end of BSS and calls
`CORE_Main` (`src/main/core/main.c`). `CORE_Main` initializes the engine,
enters `CORE_Loop` with the map level (0x19) and shuts down when the loop
exits.

Almost all gameplay logic is GOOL bytecode, not native code. The executable is
the engine:

- `ns/`: the NS paging system. Each level is an NSD index plus an NSF file
  of 64 KiB pages on the disc (for example `S1/S0000019.NSD` and `.NSF`, in
  the `S0` to `S3` directories). Entries are
  addressed by 32-bit entry IDs (EIDs) and are paged in on demand.
- `gool/`: the GOOL interpreter and object system. Objects are allocated from a
  fixed pool, linked in parent/child/sibling trees, and run per-frame code,
  event handlers and state transitions from GOOL executables in the NSF.
- `level/`: zones, paths, collision octrees, checkpoints and spawning.
- `cam/`: camera follow along zone paths.
- `render/` and `engine/gte_render.c`: transformation, projection and
  primitive submission through the GTE and GPU ordering tables. The 41
  hand-written routines in `gte_render.c` ran from the scratchpad. They
  decode world vertex streams (SLST), transform models and sprites, and query
  collision.
- `engine/`: audio (SPU voices, MIDI sequences, CD audio), memory card,
  CD reading, the frame state and assorted helpers.
- `sdk/`: the reconstructed PsyQ libraries (libapi, libcd, libgpu, libgte,
  libspu, libsnd, BIOS and kernel stubs).

## Native design

`main()` parses the command line, opens the user's CUE/BIN image and starts
`CORE_Main`. PsyZ provides the PsyQ API surface (GPU, GTE, SPU, pads, memory
card, CD) on top of SDL, so the reconstructed `sdk/` sources are not compiled
into the native executable. They stay in the tree as a behavioral reference.

The CD layer reads ISO 9660 files from the image. The NS system loads NSD and
NSF pages from those files directly; no CD drive timing is emulated. CD audio
and XA streams are played through PsyZ.

Scratchpad buffers and the fixed RAM regions the engine assumes (the page
pool, object pool and ordering tables) become ordinary host allocations or
statics. The hand-written scratchpad routines are translated to C that uses
the PsyZ GTE API.

### Host pointers

The PS1 build stores pointers in 32-bit words throughout: NS entry pointers in
page tables, GOOL object links in the object field array, and code pointers in
dispatch tables. On a 64-bit host these words cannot hold a pointer. Each
module joining the build must resolve this explicitly by one of these means:

- Typed pointer fields in host-only structures, when the value never reaches
  serialized data or GOOL.
- Handles (pool indices or arena offsets) when GOOL bytecode reads or writes
  the word as a 32-bit value. GOOL object references are the main case.
- Explicit decoding of on-disc records into host structures at load time,
  converting offsets into pointers.

Never truncate a pointer to 32 bits, and never assume `sizeof(void *) == 4`.

## Data formats

NSD, NSF, entry and GOOL executable formats are described in
[STRUCTURES.md](STRUCTURES.md) and [GOOL_LISP.md](GOOL_LISP.md).
`tools/scripts/gool_lisp.py` decompiles and reassembles GOOL bytecode and
round-trips every executable on the USA disc.
