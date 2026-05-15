## CPPGM Programming Assignment 25 (`cppeh`)

### Overview

Write one C++ application called `cppeh`.

`cppeh` has two modes:

- compile mode (`-c`), which takes as input one or more LowIR source files and writes one
  exception-capable machine-object file
- link mode (the default), which takes as input one or more such machine-object files and
  writes a linked native executable program

PA25 extends the PA24 separate-compilation pipeline with the first compiler-private
exception/runtime ABI layer. The assignment boundary is still source-driven: tests
begin from LowIR source files, compile those sources to objects with `cppeh -c`, then link
them with `cppeh`.

Like PA24, the object format used between those two tools is part of your implementation.
It must be stable enough for your own `cppeh` link mode to read, but it is not compared directly by
the PA25 test suite.

That means you may choose any object-file encoding you want, as long as it preserves the
information your linker/runtime glue needs. In practice, your object format should at least
carry:

- target identity
- code bytes and data bytes
- global and local symbol definitions
- relocations for cross-object code/data references
- enough section information to distinguish code from data
- any extra support data your PA25 exception/runtime ABI needs at link time

The PA25 suite validates the behaviour of the whole `cppeh -c -> cppeh` pipeline, not one
specific serialized object syntax.

This is intentionally a private runtime boundary, not a host-toolchain interoperability
assignment. Ordinary host ABI/runtime behaviour for `cppgm++` objects is owned later by
PA31. PA25 owns the internal `cppeh` path and the reserved internal exception-role family
used by that path, such as `@__cppgm_eh_top`, `@__cppgm_eh_value`,
`@__cppgm_eh_type`, and `@__cppgm_eh_unhandled`.

Those reserved names describe the logical PA25 runtime roles. The backing object-file
symbol names and internal object encoding that implement those roles may remain private
and implementation-defined. The PA25 contract is the deterministic behavior of the
pipeline and the reserved runtime roles visible through the documented interface, not one
mandatory private backing-name scheme.

### Prerequisites

You should complete Programming Assignment 24 before starting this assignment.

You will want to reuse:

- the PA13 LowIR parser and LowIR specification
- the PA23 machine-IR/native-backend lowering path
- the PA24 object emission and linking path
- any reusable runtime/startup glue from PA23 and PA24

PA25 tests compile, link, and execute generated native programs. Your
development host therefore needs an x86-64 Linux execution environment. In
compile mode, `--target linux` selects the object target explicitly; with no
`--target`, the object should use the Linux target.

### Starter Kit

The starter kit contains:

- `pa25/README.md`, `pa25/Makefile`, and the test scripts in `pa25/scripts/`
- a student-editable `dev/cppeh.cpp` starter scaffold
- the `pa25/cppeh.cpp` symlink back to `../dev/cppeh.cpp`
- shared support sources and headers under `dev/src/`
- a local test suite under `pa25/tests/`
- the grammar for this assignment called `pa25.gram`
- the authoritative LowIR specification in `../pa13/lowir.md`
- an HTML grammar explorer of `pa25.gram` in the sub-directory `grammar/`
- checked-in golden `.ref` result files under `tests/`

Students should implement the assignment in `dev/cppeh.cpp` and any reusable
student-owned helpers they add under `dev/src/`. The assignment directory, grammar files,
test fixtures, comparison scripts, and checked-in reference outputs are support
files, not implementation files to edit for normal solutions. The shared support files
provide reusable infrastructure and earlier assignment machinery; they do not implement the
new PA25 private EH/runtime contract for you.

Unlike PA1-PA9, there is no external reference binary for PA25. The checked-in `.ref`
result files are the default oracle.

### Driver Surface For This Assignment

Required in PA25:

- `--help` / `-h`
- compile mode: `-c`
- default link mode
- `-o <outfile>`
- `--dump-link-map <mapfile>`
- `--target <target>` in compile mode

PA25 does not introduce a later follow-on binary of its own. What remains out of
scope here is different work:

- ordinary host-toolchain and host-ABI interoperability for `cppgm++`, which belongs to
  PA31
- hosted source/header compatibility for `cppgm++`, which belongs to PA33

### Input / Command-Line Arguments

Behaviour is undefined unless the command-line arguments match one of:

    $ cppeh -c -o <objfile> <srcfile1> <srcfile2> ... <srcfileN>
    $ cppeh -c --target <target> -o <objfile> <srcfile1> <srcfile2> ... <srcfileN>

    $ cppeh -o <outfile> <objfile1> <objfile2> ... <objfileN>
    $ cppeh --dump-link-map <mapfile> -o <outfile> <objfile1> <objfile2> ... <objfileN>

In compile mode (`-c`), each `<srcfileK>` is a LowIR source file matching `pa25.gram`.

In link mode, each `<objfileK>` is a machine-object file produced by `cppeh -c` (or by a
compatible implementation-defined writer from the same compiler).

All object files given to one `cppeh` link invocation must target the same native backend
target.

### Output Format

In compile mode, `cppeh` shall write one machine-object file to `<objfile>`.

In link mode, `cppeh` shall write a native executable program to `<outfile>`.

If `--dump-link-map <mapfile>` is provided in link mode, `cppeh` shall also write a deterministic
link-map dump to `<mapfile>`.

The `--dump-link-map` output format is part of the PA25 contract. For a successful link it
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

PA25 may inject internal runtime support objects or symbols while linking. Those internal
implementation details are not part of the link-map contract. The `object` lines and
the visible `symbol` lines must still correspond to the user-supplied input objects and the
linked symbol set, not internal helper objects.

The PA25 tests compare link-map files structurally. The symbolic content and the ordering
of the lines are part of the contract, while the numeric size and offset fields are checked
for internal consistency rather than literal equality.

In particular:

- the header lines must appear in the order shown above
- the `object` lines must appear in the same order as the input object files on the
  `cppeh` command line
- the `symbol` lines must appear in deterministic sorted symbol-name order
- the numeric size and offset fields must be valid integers with sensible section-relative
  ranges, but they do not need to match the checked-in `.ref.map` numbers exactly

The exact object-file encoding is not directly compared by the PA25 tests. The exact final
binary encoding is also not directly compared. Instead, the tests compare:

- the overall pipeline exit status
- the structural shape and numeric sanity of the link-map dump for successful links
- the generated program exit status
- the generated program standard output

### Error Handling

If an error occurs while parsing LowIR or writing a machine-object file in compile mode,
`cppeh` shall `EXIT_FAILURE`.

If an error occurs while parsing object files, resolving symbols, applying relocations,
constructing the runtime ABI support, or writing the native output in link mode, `cppeh` shall
`EXIT_FAILURE`.

Important PA25 error cases include:

- duplicate global symbol definitions across separately compiled objects
- unresolved external symbols
- missing `@main`
- mixed-target object files
- user code attempting to define reserved runtime-support names needed by the PA25 ABI

### Standard Output / Error

Standard output and standard error are ignored for automated testing of `cppeh`.

You are free to use them for debugging, tracing, or diagnostics.

### Testing

PA25 tests are source-driven.

The local checked-in tests live in `tests/general/`. That directory contains
LowIR companion source files, checked-in link-map references, expected program
behavior, and deterministic private-EH runtime outcomes for the PA25 private
exception/runtime ABI surface. PA25 has no `tests/spec/` directory because the
tested contract is the compiler-private `cppeh` runtime path rather than a
directly cited N3485 C++ source-language clause.

For each test anchor `x.t`, the real LowIR inputs are the companion files:

    x.t.1
    x.t.2
    ...

For each test case `x`:

- `cppeh -c` is executed once for each companion source file to produce intermediate
  object files
- `cppeh` is executed on those generated object files to produce `x.my.program`
- `cppeh` is also executed with `--dump-link-map` to produce `x.my.map`
- the overall pipeline exit status is recorded in `x.my.impl.exit_status`
- if linking succeeded, `x.my.program` is executed
- its standard output is recorded in `x.my.program.stdout`
- its numeric exit status is recorded in `x.my.program.exit_status`

The checked-in `.ref` files are compared the same way:

- `x.ref.impl.exit_status`
- `x.ref.map` (compared structurally, not by exact numeric offsets)
- `x.ref.program.stdout`
- `x.ref.program.exit_status`

This means the PA25 suite validates:

- that `cppeh -c` emits usable objects
- that `cppeh` link mode can link those objects correctly
- that the linked program behaves as expected under the PA25 exception/runtime ABI

without freezing one exact object-file text format.

The shipped PA25 tests are the contract for this milestone. They intentionally
check runtime behavior, deterministic link-map shape, and private-EH outcomes
without requiring students to match the system C++ exception metadata.

### PA25 Syntax Spec

The authoritative LowIR input syntax for PA25 is `pa25.gram`.

As in PA24, that grammar defines the accepted LowIR input syntax only. The machine-object
format used between `cppeh -c` and `cppeh` link mode is an implementation detail of the PA25
toolchain and is not itself the syntax contract for the assignment.

A checked-in HTML grammar explorer for `pa25.gram` lives in `grammar/`. Treat
`pa25.gram` as the source of truth.

### Assignment Boundary

PA25 must support the PA24 LowIR family plus the explicit exception/runtime instructions
documented in `../pa13/lowir.md`:

- `eh_try`
- `eh_cleanup`
- `eh_end`
- `throw`
- `exception`
- `resume`

Within the supported subset, PA25 should:

- compile exception-capable LowIR to deterministic machine-object files
- link those objects into native executables
- preserve dynamic handler-stack behaviour across separately compiled units
- preserve cleanup-and-resume behaviour during exceptional control flow
- reject symbol conflicts with the reserved PA25 runtime names
- provide the private runtime support needed for those behaviors to execute after link

### Student Requirements

For grading purposes, PA25 owns the compiler-private EH/runtime ABI surface.

That means a PA25 implementation must provide, in some implementation-defined form:

- the reserved internal PA25 EH role family used by generated code, such as
  `@__cppgm_eh_top`, `@__cppgm_eh_value`, `@__cppgm_eh_type`, and
  `@__cppgm_eh_unhandled`
- the runtime behavior for handler transfer, cleanup execution, `exception`, and `resume`
- any helper runtime object, archive, or linker-injected support needed so PA25-linked
  programs actually run

PA25 does not require lowering onto the host platform's native C++ EH ABI.
It is acceptable to keep a private EH/runtime layer here as long as:

- the contract is deterministic
- user code cannot redefine the reserved runtime names
- separately compiled PA25 programs still link and execute correctly

In other words, PA25's job is:

- make the `cppeh -c -> cppeh` path work
- preserve the reserved PA25 EH runtime-role family
- keep the private runtime/link-map contract deterministic

PA25's job is not:

- ordinary host linker interoperability for `cppgm++ -c` objects
- host C++ ABI compatibility for RTTI, vtables, or foreign-library interop
- hosted-header or vendor-extension compatibility

To complete PA25, implement these goals:

1. Catch-style handler transfer.
   A thrown value must transfer control to the nearest active `eh_try` target and make the
   current exception value visible through `exception`.

2. Cross-object exception transfer.
   An exception thrown in one separately compiled object must be catchable in another.

3. Cleanup-and-resume behaviour.
   Cleanup handlers must run during unwinding and `resume` must continue the in-flight
   exception rather than creating a new one.

4. Deterministic unhandled-exception behaviour.
   Unhandled exceptions must follow one documented runtime path with deterministic program
   behaviour.

5. Link-map stability.
   Even if the implementation injects internal runtime support while linking, the visible
   link map must stay deterministic and source-oriented. The entire PA25 suite checks this.

### Out Of Scope

The following are explicitly out of scope for PA25:

- zero-cost unwinding metadata
- host-platform exception interoperation
- typed catch matching beyond the lowered scalar/pointer payload subset
- destructor synthesis from source-language `try`/`catch` semantics
- exceptions thrown across foreign object formats or libraries

Inputs that rely on those features have undefined behaviour for this milestone.

### Stage Handoff

The intended next stage is PA26, which closes the first tier of remaining language features
needed before a broad self-hosting compiler is realistic.

So PA25 should leave behind:

- a stable private LowIR exception/runtime ABI
- an exception-capable separate-compilation and linking pipeline for `cppeh`
- no requirement that later milestones invent a second private exception backend boundary
- a runtime/lowering foundation that PA26 can reuse while adding:
  - `auto` variable deduction
  - direct braced initialization
  - captureless lambdas
  - range-for over arrays and braced-init lists

PA27 then owns the remaining advanced language-closure work over the current
single-inheritance model, including capturing lambdas, real `std::initializer_list`
interoperation, RTTI, `typeid`, and pointer-form `dynamic_cast`. PA28 then picks up the
remaining multiple-inheritance and ABI-heavy object-model work. Ordinary host-toolchain and
host-ABI ownership for `cppgm++` objects remains a later PA31 concern, separate from the
private `cppeh` runtime boundary here.

### Design Notes (Non-Normative)

The cleanest PA25 structure is:

- reuse the PA24 object format or a compatible variant
- lower the PA25 exception instructions into the same machine/object pipeline
- inject any internal runtime support behind the object/link-map contract
- keep the link map and observable runtime behaviour deterministic

PA25 does not require the system C++ exception ABI. The assignment is about a
deterministic private compiler/runtime boundary over the Linux object and
executable path.

The same monotonic-extension rule applies here:

- adding PA25 support should not perturb PA24 behaviour for programs that remain entirely
  within the PA24 subset
- exception/runtime machinery should only become visible when the input actually uses the
  PA25 exception forms
