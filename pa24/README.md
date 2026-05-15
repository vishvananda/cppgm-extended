## CPPGM Programming Assignment 24 (`cpplink`)

### Overview

Write one C++ application called `cpplink`.

`cpplink` has two modes:

- compile mode (`-c`), which takes as input one or more LowIR source files and writes one
  machine-object file
- link mode (the default), which takes as input one or more machine-object files and writes
  a linked native executable program

PA24 separates compilation from linking. The assignment boundary is source-driven:
tests begin from LowIR source files, compile those sources to objects with `cpplink -c`,
then link them with `cpplink`.

The object format used between those two tools is part of your implementation. It must be
stable enough for your own `cpplink` link mode to read, but it is not compared directly by the PA24
test suite.

That means you may choose any object-file encoding you want, as long as it preserves the
information your linker needs. In practice, your object format should at least carry:

- target identity
- code bytes and data bytes
- global and local symbol definitions
- relocations for cross-object code/data references
- enough section information to distinguish code from data

The PA24 suite validates the behaviour of the whole `cpplink -c -> cpplink` pipeline, not
one specific serialized object syntax.

### Prerequisites

You should complete Programming Assignment 23 before starting this assignment.

You will want to reuse:

- the PA13 LowIR parser and LowIR specification
- the PA23 machine-IR/native-backend knowledge
- the existing ELF writer
- any reusable layout helpers for code/data placement and relocation patching

PA24 tests compile, link, and execute generated native programs. Your
development host therefore needs an x86-64 Linux execution environment. In
compile mode, `--target linux` selects the object target explicitly; with no
`--target`, the object should use the Linux target.

### Starter Kit

The starter kit contains:

- `pa24/README.md`, `pa24/Makefile`, and the test scripts in `pa24/scripts/`
- a student-editable `dev/cpplink.cpp` starter scaffold
- the `pa24/cpplink.cpp` symlink back to `../dev/cpplink.cpp`
- shared support sources and headers under `dev/src/`
- a local test suite under `pa24/tests/`
- the grammar for this assignment called `pa24.gram`
- an HTML grammar explorer of `pa24.gram` in the sub-directory `grammar/`
- checked-in golden `.ref` result files under `tests/`

Students should implement the assignment in `dev/cpplink.cpp` and any reusable
student-owned helpers they add under `dev/src/`. The assignment directory, grammar files,
test fixtures, comparison scripts, and checked-in reference outputs are support
files, not implementation files to edit for normal solutions. The shared support files
provide reusable infrastructure and earlier assignment machinery; they do not implement the
new PA24 object/link contract for you.

Unlike PA1-PA9, there is no external reference binary for PA24. The checked-in `.ref`
result files are the default oracle.

### Driver Surface For This Assignment

Required in PA24:

- `--help` / `-h`
- compile mode: `-c`
- default link mode
- `-o <outfile>`
- `--dump-link-map <mapfile>`
- `--target <target>` in compile mode

Not yet required here:

- the private exception/runtime ABI path
- ordinary host-toolchain or host-ABI interoperability for `cppgm++`

The follow-on private EH/runtime pipeline is owned by PA25 `cppeh`.

### Input / Command-Line Arguments

Behaviour is undefined unless the command-line arguments match one of:

    $ cpplink -c -o <objfile> <srcfile1> <srcfile2> ... <srcfileN>
    $ cpplink -c --target <target> -o <objfile> <srcfile1> <srcfile2> ... <srcfileN>

    $ cpplink -o <outfile> <objfile1> <objfile2> ... <objfileN>
    $ cpplink --dump-link-map <mapfile> -o <outfile> <objfile1> <objfile2> ... <objfileN>

In compile mode (`-c`), each `<srcfileK>` is a LowIR source file matching `pa24.gram`.

In link mode, each `<objfileK>` is a machine-object file produced by `cpplink -c` (or by a
compatible implementation-defined object writer from the same compiler).

All object files given to one `cpplink` link invocation must target the same native backend
target.

### Output Format

In compile mode, `cpplink` shall write one machine-object file to `<objfile>`.

In link mode, `cpplink` shall write a native executable program to `<outfile>`.

If `--dump-link-map <mapfile>` is provided in link mode, `cpplink` shall also write a deterministic
link-map dump to `<mapfile>`.

The `--dump-link-map` output format is part of the PA24 contract. For a successful link it
shall have this shape:

    link_map x86_64 <target>
    startup_size <n>
    code_size <n>
    data_size <n>
    object 0 code_offset <n> data_offset <n>
    object 1 code_offset <n> data_offset <n>
    ...
    symbol <name> code <n>
    symbol <name> data <n>
    ...

The lines have the following meaning:

- `link_map x86_64 <target>`
  identifies the machine family and final native target string
- `startup_size <n>`
  is the size in bytes of the startup stub placed before linked object code
- `code_size <n>`
  is the total linked code region size in bytes, including the startup stub
- `data_size <n>`
  is the total linked payload size in bytes after code and data placement
- each `object <i> ...` line
  gives the final linked code and data offsets for the `i`th input object, in the same
  order the object files were passed on the command line
- each `symbol <name> ...` line
  gives the final linked offset of one global symbol, with `code` or `data` identifying
  the section it belongs to

The PA24 tests compare link-map files structurally. The symbolic content and the ordering
of the lines are part of the contract, while the numeric size and offset fields are checked
for internal consistency rather than literal equality.

In particular:

- the header lines must appear in the order shown above
- the `object` lines must appear in the same order as the input object files on the
  `cpplink` command line
- the `symbol` lines must appear in deterministic sorted symbol-name order
- the numeric size and offset fields must be valid integers with sensible section-relative
  ranges, but they do not need to match the checked-in `.ref.map` numbers exactly

The exact object-file encoding is not directly compared by the PA24 tests. The exact final
binary encoding is also not directly compared. Instead, the tests compare:

- the overall pipeline exit status
- the structural shape and numeric sanity of the link-map dump for successful links
- the generated program exit status
- the generated program standard output

### Error Handling

If an error occurs while parsing LowIR or writing a machine-object file in compile mode,
`cpplink` shall `EXIT_FAILURE`.

If an error occurs while parsing object files, resolving symbols, applying relocations, or
writing the native output in link mode, `cpplink` shall `EXIT_FAILURE`.

Important PA24 error cases include:

- duplicate global symbol definitions across separately compiled objects
- unresolved external symbols
- missing `@main`
- mixed-target object files

### Standard Output / Error

Standard output and standard error are ignored for automated testing of `cpplink`.

You are free to use them for debugging, tracing, or diagnostics.

### Testing

PA24 tests are source-driven.

The local checked-in tests live in `tests/general/`. That directory contains
LowIR companion source files, checked-in link-map references, expected program
behavior, and deterministic link-error cases for the PA24 separate-compilation
and link/runtime surface. PA24 has no `tests/spec/` directory because the
contract here is the compiler-owned object/link pipeline, not a directly cited
N3485 C++ source-language clause.

For each test anchor `x.t`, the real LowIR inputs are the companion files:

    x.t.1
    x.t.2
    ...

For each test case `x`:

- `cpplink -c` is executed once for each companion source file to produce intermediate object
  files
- `cpplink` is executed on those generated object files to produce `x.my.program`
- `cpplink` is also executed with `--dump-link-map` to produce `x.my.map`
- the overall pipeline exit status is recorded in `x.my.impl.exit_status`
- if linking succeeded, `x.my.program` is executed
- its standard output is recorded in `x.my.program.stdout`
- its numeric exit status is recorded in `x.my.program.exit_status`

The checked-in `.ref` files are compared the same way:

- `x.ref.impl.exit_status`
- `x.ref.map` (compared structurally, not by exact numeric offsets)
- `x.ref.program.stdout`
- `x.ref.program.exit_status`

This means the PA24 suite validates:

- that `cpplink -c` emits usable objects
- that `cpplink` link mode can link those objects correctly
- that the linked program behaves as expected

without freezing one exact object-file text format.

For implementers, this means the stable externally visible contracts are:

- `cpplink -c` accepts LowIR source files
- `cpplink` link mode accepts the matching implementation-defined object files
- `cpplink --dump-link-map` emits the required textual link-map format
- the final linked executable behaves correctly

The shipped PA24 tests are the contract for this milestone.

### PA24 Syntax Spec

The authoritative source-language syntax for PA24 is `pa24.gram`.

As in PA23, that grammar defines the accepted LowIR input syntax only. The machine-object
format used between `cpplink -c` and `cpplink` link mode is an implementation detail of the PA24
toolchain and is not itself the syntax contract for the assignment.

A checked-in HTML grammar explorer for `pa24.gram` lives in `grammar/`. Treat
`pa24.gram` as the source of truth.

### Assignment Boundary

PA24 must support separate compilation and linking for the LowIR family already defined for
PA13 and used by PA14-PA23, including:

- globals and structured global data
- floating scalar code/data, including `f32`, `f64`, and `f80`
- atomic scalar code/data and fence operations already defined in PA13 / PA23
- direct and indirect calls
- startup/shutdown hooks
- direct cross-object data references
- bulk memory operations defined in LowIR

Within the supported subset, PA24 should:

- emit deterministic machine-object files from LowIR through `cpplink -c`
- resolve direct cross-object calls
- resolve cross-object global/data references
- preserve startup/shutdown hook aggregation across separately compiled units
- reject duplicate global definitions
- reject unresolved symbols
- link the resulting objects into a native executable

When choosing an internal object format, keep in mind that PA24 is testing separate
compilation rather than one exact file syntax. The important thing is that independently
compiled objects still preserve enough symbol and relocation information for the linker to:

- resolve direct calls
- resolve address-bearing data references
- preserve the atomic machine/code generation path across separately compiled objects
- preserve startup/shutdown hooks across object boundaries
- produce a deterministic link map

To complete PA24, implement these goals:

1. Separate compilation from source.
   Source files are compiled independently to objects before linking.

2. Cross-object direct call resolution.
   The linker must resolve direct calls between separately compiled units.

3. Cross-object data resolution.
   The linker must resolve globals and address-bearing data across separately compiled
   units.

4. Startup and shutdown aggregation.
   The linker must discover and wire `@__cppgm_init`, `@main`, and `@__cppgm_fini` across
   multiple objects.

5. Link-time error handling.
   Duplicate global definitions, unresolved externals, and missing `@main` must be
   rejected at the correct stage.

### Out Of Scope

The following are explicitly out of scope for PA24:

- archives and archive search rules
- dynamic linking
- exception runtime metadata
- dead-code elimination or link-time optimization

### Stage Handoff

The intended next stage is PA25, which adds exception/runtime ABI machinery on top of the
PA23 native backend path and the PA24 separate-compilation toolchain.

So PA24 should leave behind:

- a stable `LowIR -> object` boundary
- a stable linker/link-map boundary
- reusable relocation and symbol-resolution infrastructure

### Design Notes (Non-Normative)

The cleanest PA24 structure is:

- parse LowIR into a structured internal representation
- lower that representation into an implementation-defined machine-object file
- parse those machine-object files into a structured object representation for the linker
- build a global and per-object symbol view
- lay out code and data for the linked image
- apply relocations into that linked image
- emit the final executable image
- dump a deterministic link map for testing

PA24 is the point where the backend becomes a real multi-file toolchain rather than only a
single-invocation compiler.
