# PA10-31 Binary Consolidation Plan

## Goal

Reduce the number of binaries students build and reason about for `pa10` through
`pa31`, while keeping the assignment story easy to follow.

The current layout follows the `pa1`-`pa9` pattern too literally:

- each assignment introduces a new binary name
- many of those binaries are now thin wrappers over the same implementation
- students are asked to think in terms of tool renaming, when the real work is
  feature growth inside one compiler pipeline

The proposed direction is:

- one primary C++-source compiler binary: `cppgm++`
- a small number of explicit output/driver flags
- later assignments extend existing behavior instead of renaming the tool
- separate utilities only when the input format changes away from C++ source

## Recommended Binary Model

### Primary student-facing binary

Use one main binary from `pa10` onward:

- `cppgm++`

This binary should support explicit long-form emit flags:

- `--emit-ast`
- `--emit-types`
- `--emit-semantics`
- `--emit-lowir`

Example student invocations:

```bash
./cppgm++ --emit-ast -o out file.cpp
./cppgm++ --emit-types -o out file.cpp
./cppgm++ --emit-semantics -o out file.cpp
./cppgm++ --emit-lowir -o out file.cpp
```

The `lowir` mode is the natural continuation of the earlier text emit modes.

### When to keep separate binaries

Keep separate binaries only when the input domain is different enough that the
tool is no longer conceptually "the same compiler with more output options."

That suggests:

- `cppgm++`
  - input: C++ source
  - modes: `--emit-ast`, `--emit-types`, `--emit-semantics`, `--emit-lowir`
  - later also supports `-E`, `-c`, and full compile/link behavior

- `lowir2cy86`
  - input: LowIR text
  - output: CY86 text/program image

Everything else should be considered for consolidation into `cppgm++` unless we
have a strong pedagogical reason to preserve a separate tool name.

## Family Mapping

### Text-output family

Today:

- `cppast`
- `cpptypes`
- `cppcalls`

Proposed:

- `cppgm++ --emit-ast`
- `cppgm++ --emit-types`
- `cppgm++ --emit-semantics`

### LowIR-output family

Today:

- `cpplowir`
- `cppclasses`
- `cppvalue`
- `cppvirt`
- `cpptemplates`
- `cppmeta`
- `cppconstexpr`
- `cpptemplatecomplete`
- `cpplangcore`
- `cpplangcomplete`
- `cppobjectcomplete`
- `cppmultivirt`

These are already the same frontend in practice.

Proposed:

- `cppgm++ --emit-lowir`

Each later assignment extends the semantics supported by `--emit-lowir`, but
the binary name does not change.

### Toolchain/hosted family

Today:

- `cpptoolchain`
- `cpphostinterop`
- `cpphostcompat`

Proposed:

- all folded into `cppgm++`

Student-facing behavior:

- preprocessing only: `-E`
- compile to object: `-c`
- full compile/link: default

Pedagogically, this matches how real compilers behave and is easier to explain
than introducing three separate names.

### LowIR backend family

Today:

- `lowir2native`
- `cpplink`
- `cppeh`

Proposed options:

1. conservative:
   - keep one backend utility name, for example `lowir2native`
   - add flags for link mode
2. aggressive:
   - fold this into `cppgm++` too, but only if we explicitly want the compiler
     to accept LowIR inputs as well as C++ inputs

Recommendation:

- keep this as a separate backend-stage utility
- do not make students learn it until the course reaches the backend/link stage

This keeps the main student story simple:

- `cppgm++` handles C++ source
- `lowir2cy86` and any LowIR-native tool handle backend-stage inputs

## README Story

The key improvement is to shift from "new binary each assignment" to
"same compiler, new capability each assignment."

### PA10 README shape

Suggested language:

> Beginning with PA10, the course reuses the same compiler binary, `cppgm++`.
> This assignment introduces AST output. The compiler should accept:
>
> ```bash
> ./cppgm++ --emit-ast -o <outfile> <srcfile>...
> ```
>
> For this milestone, only `--emit-ast` is required. Later assignments extend
> the same binary with additional output modes instead of renaming the tool.

### PA11 README shape

> Continue extending `cppgm++`.
> All behavior from PA10 remains required.
> Add support for:
>
> ```bash
> ./cppgm++ --emit-types -o <outfile> <srcfile>...
> ```
>
> The `--emit-ast` mode must continue to work unchanged.

### PA12 README shape

> Continue extending `cppgm++`.
> All behavior from PA10 and PA11 remains required.
> Add support for:
>
> ```bash
> ./cppgm++ --emit-semantics -o <outfile> <srcfile>...
> ```

### PA14 README shape

> Continue extending `cppgm++`.
> Add support for LowIR output:
>
> ```bash
> ./cppgm++ --emit-lowir -o <outfile> <srcfile>...
> ```
>
> Later assignments extend the language features supported by this LowIR output.
> They do not introduce a new compiler binary.

### PA15-PA28 README pattern

Each of these should say some version of:

> Continue extending `cppgm++ --emit-lowir`.
> All prior supported LowIR behavior remains required.
> This assignment adds support for the following language features:

This is much clearer than renaming the compiler every time.

### PA29 README shape

> `cppgm++` now begins to act like a real toolchain driver.
> In addition to prior emit modes, it must support:
>
> ```bash
> ./cppgm++ -c -o <outfile> <srcfile>
> ```
>
> For this milestone, compile-only object emission is required.

### PA30 README shape

> Extend `cppgm++` with hosted object-generation compatibility.
> The compile-only interface remains:
>
> ```bash
> ./cppgm++ -c -o <outfile> <srcfile>
> ```
>
> This assignment expands the set of hosted C++ source inputs that must compile
> correctly.

### PA31 README shape

> Extend `cppgm++` with preprocessing-only support:
>
> ```bash
> ./cppgm++ -E -o <outfile> <srcfile>
> ```
>
> and with stronger hosted compile compatibility.

## Why This Is Better For Students

Students learn:

- one compiler name
- one consistent CLI shape
- assignments add capabilities instead of renaming tools

This is closer to real compilers and reduces accidental complexity.

The current many-binary model hides the fact that the assignments are really
building one evolving compiler.

## Implementation Strategy

### Phase 1: Interface consolidation

Introduce `cppgm++` support for:

- `--emit-ast`
- `--emit-types`
- `--emit-semantics`
- `--emit-lowir`

Replace the old assignment wrapper names in `pa10`-`pa31` rather than keeping
aliases. The student-facing story should be one compiler binary from `pa10`
forward.

### Phase 2: Remove redundant dev entrypoints

After tests are switched:

- remove `cppast`, `cpptypes`, `cppcalls`
- remove the many lowir-family entrypoints that just call
  `run_cpp_to_lowir_frontend(argc, argv)`

### Phase 3: Fold driver-family binaries into `cppgm++`

Unify:

- `cpptoolchain`
- `cpphostinterop`
- `cpphostcompat`

around the standard compiler-style interface:

- `-E`
- `-c`
- full compile/link

### Phase 4: Simplify PA Makefiles

After interface consolidation:

- `pa10`-`pa31` Makefiles should stop naming assignment-specific binaries
- each should invoke `../dev/cppgm++` with the right flags
- shared boilerplate should move into one included make fragment

## Recommendation

Use the following student-facing policy:

- from `pa10` onward, teach only `cppgm++`
- use `--emit-ast`, `--emit-types`, `--emit-semantics`, and `--emit-lowir`
  for text and LowIR output
- use standard compiler flags `-E` and `-c` for later stages
- keep separate tool names only when the input is not C++ source anymore

This is the clearest long-term model and gives the biggest binary/build
consolidation win.
