# Game memory model

The engine's C code was written for a 32-bit machine. It stores pointers in
32-bit words, overlays views of different types on the same memory, resolves
disc references in place and compares pointers against negative error codes.
The native port keeps that memory model rather than rewriting every data
structure.

## The RAM image

`g_ps1_ram` is an 8 MiB host block that mirrors PS1 RAM (2 MiB, followed by
room the native build may use). `src/main/ram_layout.S` is generated from the
decompilation's symbol table by `tools/scripts/gen_ram_layout.py`. It places
every data symbol of the original executable at its original address, so:

- C code declares globals with ordinary `extern` declarations, of any type.
- Views that span several globals, or alias part of one, keep working.
- At startup the platform layer copies `SCUS_949.00` from the user's disc into
  the block. Initial values never enter the repository.
- The game's own heap (`InitHeap` at the end of BSS) allocates from the block,
  so NS pages, objects and buffers also live in game memory.

`g_ps1_scratch` mirrors the 1 KiB scratchpad at 0x1F800000.

Do not define a global that is in `ram_layout.S` in C; declare it `extern`.
File-local `static` data and constants may stay in host memory unless their
address is stored in game memory.

## Pointers in game memory

A word in game memory that holds a pointer holds a **PS1 address**
(0x80xxxxxx, or 0x1F800xxx for the scratchpad). Struct fields and globals of
that kind are declared `RAM_REF(type)`, which is a `uint32_t` that names the
pointee for the reader. Convert explicitly:

| Need | Write |
|---|---|
| dereference a stored address | `RAM_AT(type, word)` (e.g. `RAM_AT(GoolObj, obj->parent)->x`) |
| store a host pointer | `word = Ram_Addr(ptr)` |
| raw host pointer | `Ram_Ptr(addr)` |
| call a stored code address | `RAM_FN(type, word)(args)` |
| store a function | `word = Ram_FnAddr(fn)` |

`Ram_Ptr` and `Ram_Addr` abort on an address or pointer outside game memory,
including an error code. Test error codes before converting.

Host pointers (locals, parameters declared as pointers) are ordinary C
pointers into the block. Pointer arithmetic on them works as on the PS1.
Never cast a pointer to `u32`/`s32` or back; use the conversions.

## Addresses and error codes

The engine returns a pointer or a small negative error code from the same
function, and tests `(s32)p < -0xFE` (a valid KSEG0 address is negative as
`s32`). Where the decompiled code treats a value this way, keep it a PS1
address in an `s32`/`u32`:

- Functions that may return an error code return `s32` (the address or the
  code). Examples: `Ns_Open`, `Ns_Lookup`, `Ns_PageStruct`, `Ns_PagePhysical`.
  Callers test the code first and then use `RAM_AT`.
- Functions that always return a valid pointer return a host pointer.
- Tagged words (low bits used as flags, EIDs with bit 0 set, texture or SPU
  addresses with bit 1 set) stay integers; convert only the untagged case.

Code that compares addresses as integers (`(s32)a < (s32)b`) compares host
pointers instead when both point into game memory.

## Code addresses

Code pointers in game memory (callbacks in data tables, function fields of
objects) hold the PS1 address of the function. `src/main/ram_code.inc`,
generated with the RAM layout, lists the PS1 address of every game function
defined in C. `RAM_FN` finds the native function; a missing entry aborts with
the address. Regenerate the list after adding or renaming a game function
whose address is stored.

## The SDK boundary

Calls into PsyZ take host pointers. PS1 structures in game memory are passed
directly when PsyZ shares their layout (RECT, DISPENV, MATRIX, VECTOR,
SVECTOR, CVECTOR, CdlLOC) and converted when it does not (`SpuVoiceAttr`,
`SpuReverbAttr`, `SpuCommonAttr` and other types with `u_long` fields).

The game's GPU packets, ordering tables and drawing environments stay in PS1
format in game memory. `include/gpu.h` redirects the game's libgpu calls to
`src/platform/gpu.c`, which walks the PS1 packet chain and writes GP0 words to
PsyZ.

## Function contracts

Fix the semantics, not only the types. Where the original fell off the end
of a non-void function or passed an ignored argument, check the original
instructions (the decompilation's objects can be disassembled) and write the
value the original produced. Do not leave `return` paths undefined.
