# C conventions

Use C11 and the native compiler. Prefer clear functions, typed pointers,
subsystem headers and names that explain the role of a value. Existing
address-based names (`func_8002A82C`, `D_80060DF0`) can be renamed as their
meaning becomes clear, using the evidence rules in
[CONTRIBUTING.md](../CONTRIBUTING.md).

Use fixed-width integer types for serialized data. Separate disc records from
host runtime structures, and explicitly decode byte order, offsets and indices
before turning them into pointers. Do not assume `long` or a pointer is 32
bits. Use `size_t` for sizes, `ptrdiff_t` for pointer differences, and
`uintptr_t` only when an integer representation of a pointer is necessary.

Keep the packing and layout assertions that file formats need. Original code
addresses, register assignments, compiler scheduling barriers
(`KEEP_REGISTER_VALUE`, `__asm__` aliases), per-file historical compiler
flags and padding added to match instructions are not part of the native
design. Remove them from a module when it joins the native build.

Hardware access belongs in PsyZ integration or a small platform adapter.
MIPS and GTE routines still do real work and must be implemented in C, or
replaced by an equivalent API, before integration. A no-op substitute is
appropriate only when the operation has no native meaning.

Validate behavior with native tests and gameplay scenarios. Compiler warnings,
sanitizers and tests should cover each module as it joins the host build.
