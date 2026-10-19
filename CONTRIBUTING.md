# Contributing

This is a native C11 project. Keep the intended game behavior and data formats,
and improve the code for modern hosts. There is no byte-match requirement, no
stock PS1 compiler requirement, no PS1 checksum gate and no decompilation
progress ratchet.

## Development workflow

1. Put shared declarations in subsystem headers, using ordinary C types and
   functions.
2. Remove or simplify PS1 APIs the game does not need. Use native C or our
   PsyZ fork where needed, and keep gameplay behavior unchanged.
3. Add a ported module to the native CMake target once its dependencies work.
4. Build with `cmake -S . -B build` and `cmake --build build --parallel`.
5. Validate changed behavior with focused native tests and, as modules become
   runnable, with reproducible gameplay scenarios.

Port the existing modules in place. Do not write a parallel engine, and do not
extract functions into new files only to make them compile. Repair the original
module for host pointers, memory, PsyZ and disc access.

Do not preserve register assignments, instruction scheduling, padding, linker
order or code addresses that existed only to make the compiler match. Assembly
that does real work, including the 41 hand-written scratchpad routines in
`src/main/engine/gte_render.c`, must be translated to C before its module is
ported. Do not replace missing functionality with no-op stubs that report
success.

Serialized assets and save data keep their defined widths, offsets and
endianness. Host pointers must use pointer types. Host structures can be
redesigned independently of the original RAM layout. Check layout assertions
against serialized formats, and never force host pointers into 32 bits.

Use names supported by evidence: c1 function order and semantics, strings or
the call graph. c1 and c1c have no license, so take facts from them, not code.

See [architecture](docs/ARCHITECTURE.md), [conventions](docs/CONVENTIONS.md)
and [the port plan](docs/PC_PORT_PLAN.md).

PS1 hardware fidelity is not an acceptance requirement. Ordinary host buffers
can replace scratchpad memory and fixed RAM regions. The asset layer reads the
user-supplied CUE/BIN image and needs no physical CD drive. Keep logical asset
identity, state and reset semantics, and the event ordering visible in
gameplay.

The PsyZ submodule points to
[khasinski/parasite-eve-psyz](https://github.com/khasinski/parasite-eve-psyz),
the fork shared with the Parasite Eve port. Generic runtime, render and audio
features that are missing can be implemented there, with library tests and game
regression tests. Pin an explicitly verified commit in this project, never a
moving branch. Upstream is `Xeeynamo/psyz`; send reusable fixes there.

Never commit retail inputs, extracted assets, SDK or compiler binaries, build
output or local experiments.
