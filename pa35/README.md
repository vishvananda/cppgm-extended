## CPPGM Programming Assignment 35 (`cppgm++ -c`)

### Overview

PA35 is the hosted header-compile assignment. It is split out of PA34: where
PA34 establishes that hosted source/header inputs preprocess and compile at all
(intrinsics, parser concessions, builtin traits, header-free conformance
anchors, and lightweight C-wrapper header smokes). PA35 raises the bar to
**compiling the heaviest real C++ standard-library headers** end to end with
`cppgm++ -c`, within a workable time and memory budget.

By PA34, hosted headers and source should preprocess and compile for the
lighter hosted workload. PA35 raises the bar to large, template- and trait-heavy
STL headers (`<vector>`, `<unordered_map>`, `<tuple>`, `<random>`,
`<functional>`, and the iostream/string/exception machinery).

This is the perf-gated tier of hosted compatibility. It is not a link or runtime
test. The PA35 tests require clean object emission from heavy hosted headers;
later hosted tests exercise whether emitted header code also links and runs.

To complete PA35, implement these goals:

- compile the heaviest hosted standard-library headers cleanly with `cppgm++ -c`
- carry the template instantiation, trait evaluation, and overload-resolution
  depth those headers exercise without exponential blow-up
- stay within a workable compile time and memory budget on the heavy-header
  workload (the perf gate that distinguishes PA35 from the lighter PA34 surface)

### Prerequisites

Complete PA34 before starting this assignment.

You will want to reuse:

- the full earlier language, template, semantic, and lowering stack
- the PA34 hosted preprocess/compile compatibility surface (intrinsics, parser
  concessions, builtin traits/types, header-free conformance anchors, and
  lightweight C-wrapper header smokes)
- the PA32/PA33 `cppgm++ -c` host-object path

The tests assume a Linux shell environment with `make`, `bash`, `perl`, and a
working host C++ compiler with hosted C++ headers installed. You may override the
compiler with `CXX=...`. `CPPGM_HOST_CXX` selects the host compiler used for
builtin macro/include probing; if unset it defaults to `CXX`. When you use a
non-default standard library, pass the same choice through `CPPGM_STDLIB_FLAGS`
so the course compiler and host compiler agree.

### Starter Kit

The starter kit provides:

- `dev/cppgm++.cpp`, populated from the `cppgm++` scaffold for the cumulative
  PA10+ compiler driver
- the shared `dev/` sources needed by the scaffold
- `pa35/cppgm++.cpp`, a link to `../dev/cppgm++.cpp`
- `pa35/Makefile`
- `pa35/scripts/`, the hosted `-c` compile test harness (shared with PA34)
- `pa35/tests/compile/`, the heavy-STL header-compile tests and checked-in
  reference files

Put your code changes in `dev/`, especially `dev/cppgm++.cpp` and the
shared implementation files it calls. Do not edit generated `.my` files. Test
inputs and references are part of the handout unless your instructor asks you to
add or update tests.

There is no separate PA35 reference binary in the starter kit. The checked-in
`.ref.*` files are the oracle.

### Command-Line Contract

PA35 introduces no new `cppgm++` flags. It reuses the compile-mode surface
already required by PA34:

```sh
cppgm++ -c -o <objfile> <srcfile>
cppgm++ -c -isystem <dir> -o <objfile> <srcfile>
cppgm++ -c -D <macro> -U <macro> -include <file> -o <objfile> <srcfile>
```

### Output Format

`cppgm++ -c` shall write a host-linker-compatible relocatable object as in
PA32/PA33/PA34. For PA35 the emitted object is discarded — the requirement is
simply that the heavy hosted header compiles cleanly to an object without error.

The hosted header path should still lower through the same LowIR representation
used by `cppgm++ --emit-lowir`. Performance work may avoid unnecessary file
I/O, but it must not create a separate hosted-only backend route that depends
on facts unavailable in serialized LowIR.

### Error Handling

If preprocessing, parsing, semantic analysis, lowering, object emission, or
output writing fails, `cppgm++` shall exit with failure. Exact diagnostics are
not the grading contract; the harness compares exit status and checked output
sidecars.

### Testing

Run the PA35 suite with:

```sh
make test                       # non-batch
make test CPPGM_BATCH_TESTS=1   # batch worker
```

To run one test through the shared check target:

```sh
make check TEST=tests/compile/600-const-unordered-map-find.t
```

The local tests live in `tests/compile/`. Each test is `.t` (source) plus an
empty `.ref` base, a `.ref.exit_status` of `EXIT_SUCCESS`, and a `.ref.stdout`;
it passes iff `cppgm++ -c` compiles it cleanly. Each test includes a real heavy
header together with a cheat-proof anchor — a trait, `decltype`, `sizeof`, or
`static_assert` that cannot be satisfied without genuinely compiling the header,
so a test cannot pass by skipping or stubbing the include.

Optional `x.no-exceptions` and `x.cxx-standard` sidecars disable exceptions or
select the language mode for a compile test. The standard sidecar currently
accepts `c++11` and `c++14`.

### Required Implementation Surface

To complete PA35, support:

- compiling the heaviest hosted standard-library headers within the perf budget
- the template, trait, and overload-resolution depth those headers exercise
  during `-c` compilation

The PA35 tests do not require:

- new intrinsics, parser concessions, header-free conformance anchors, or
  lightweight hosted C-wrapper behavior beyond the hosted compatibility already
  needed before PA35
- hosted header-emitted link/runtime behavior
- bootstrap or self-host builds

If a failing PA35 test exposes a bug in syntax, semantic analysis, template
handling, lowering, or object emission that is shared with earlier language
features, fix the shared compiler behavior rather than adding a PA35-only path.

### Design Notes (Non-Normative)

The heavy headers stress the same machinery PA34 enables, just far harder:
deeply nested template instantiation, trait and `decltype` evaluation, partial
specialization selection, and large overload sets. The productive failures here
are usually performance cliffs (re-resolving the same bound template pack,
re-instantiating the same specialization) rather than missing features. Prefer
memoizing repeated resolution over deepening recursion guards.

### After PA35

Later hosted tests keep the same hosted header environment and then check that
the emitted code from those headers also links and runs.
