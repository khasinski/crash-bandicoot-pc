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
- `render/`: transformation, projection and primitive submission through
  the GTE and GPU ordering tables, including the former hand-written
  scratchpad routines: SLST decoding (`slst_decode.c`), model and sprite
  transforms (`model_transform.c`), world transforms (`world_transform.c`,
  `world_transform_fx.c`) and object matrices (`object_matrix.c`).
  `level/solid_query.c` holds their collision queries.
- `engine/`: audio (SPU voices, MIDI sequences, CD audio), memory card,
  CD reading, the frame state and assorted helpers.

## Native design

`src/platform/main.c` takes the role of the BIOS loader and crt0. It mounts the
user's CUE/BIN image in PsyZ, loads `SCUS_949.00` into the game's memory,
sets up the heap as crt0 does and calls the original `CORE_Main` on a stack
inside game memory. PsyZ provides the PsyQ API surface (GPU, GTE, SPU, pads,
memory card, CD) on top of SDL. The reconstructed PsyQ libraries, BIOS veneers
and crt0 of the matching decompilation are not part of this tree; they were
written as MIPS inline assembly for byte-exact matching and have no native
meaning. The decompilation remains their behavioral reference.

The game reads the disc itself: `Cd_ReadFileSys` walks the ISO 9660
directories and the NS system reads NSD and NSF pages with `CdRead`, which
PsyZ serves from the image. No CD drive timing is emulated.

### Game memory

Game data lives in a host block that mirrors PS1 RAM, with every global at
its original address and words that hold pointers keeping PS1 addresses.
This preserves the engine's in-place resolution of disc references, the
GOOL object fields that bytecode reads as words, and the views that overlay
several globals. [MEMORY_MODEL.md](MEMORY_MODEL.md) describes the model and
the conversions host code uses.

### Hand-written routines

The engine's hand-written R3000 routines (0x800334A0..0x8003A94C: SLST
decoding, transformation, projection, collision queries, object matrices) are
written in C with the GTE macros of `include/gte.h`, which drive PsyZ's
register-level GTE. They keep the original's observable behaviour exactly:
the same game memory and scratchpad writes, GTE register state and results,
including the original's quirks (each is commented where it is kept).

`-DCRASH_GTE_ORACLE=ON` builds a check of that. It translates the original
instructions to C from the matching decompilation's build
(`tools/scripts/gen_gte_render.py`, `CRASH_DECOMP_DIR`, needs pyelftools)
and runs every call of a ported routine twice, translation first, from the
same state, aborting on any difference (`src/main/engine/gte_oracle.c`).
`tools/scripts/gte_oracle_check.sh` runs it over attract mode and every
level. Register save areas in the scratchpad that nothing reads are the only
exemption. The normal build contains no translation.

### Graphics

The game builds GPU packets, ordering tables and drawing environments in PS1
format in game memory. `include/gpu.h` redirects its libgpu calls to
`src/platform/gpu.c`, which walks the PS1 packet chain and writes the GP0
words to PsyZ's GPU.

## Data formats

NSD, NSF, entry and GOOL executable formats are described in
[STRUCTURES.md](STRUCTURES.md) and [GOOL_LISP.md](GOOL_LISP.md).
`tools/scripts/gool_lisp.py` decompiles and reassembles GOOL bytecode and
round-trips every executable on the USA disc.
