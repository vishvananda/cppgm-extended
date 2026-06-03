## CPPGM Programming Assignment 35 (`cppgm++ -c`)

### Overview

PA35 is the hosted header-compile assignment. It is split out of PA34: where
PA34 establishes that hosted source and headers preprocess and compile at all
(intrinsics, parser concessions, builtin traits, header-free conformance
anchors), PA35 raises the bar to **compiling the heaviest real standard-library
headers** end to end with `cppgm++ -c`.

By PA34, hosted headers and source should preprocess and compile. PA35 owns the
next question: can `cppgm++` compile the large, template- and trait-heavy STL
headers (`<vector>`, `<unordered_map>`, `<tuple>`, `<random>`, `<functional>`,
the iostream/string/exception machinery, and friends) that later bootstrap-style
builds depend on, within a reasonable time and memory budget?

This is the perf-gated tier of hosted compatibility: each test includes a real
heavy header and a cheat-proof anchor (a trait/`decltype`/`sizeof`/
`static_assert` that cannot be satisfied without genuinely compiling the header).
A passing run means the object emits cleanly; it is not a link or runtime test
(that contract belongs to PA36).

### What's here

- `pa35/cppgm++.cpp`, a link to `../dev/cppgm++.cpp`
- `pa35/Makefile`
- `pa35/scripts/`, the hosted `-c` compile test harness (shared with PA34)
- `pa35/tests/compile/`, the heavy-STL header-compile tests and checked-in
  reference files

### Running the tests

```sh
make -C pa35 test                 # non-batch
make -C pa35 test CPPGM_BATCH_TESTS=1   # batch worker
make -C pa35 check TEST=tests/compile/<name>.t
```

Each test is `.t` (source) + an empty `.ref` base + `.ref.exit_status`
(`EXIT_SUCCESS`) + `.ref.stdout`; it passes iff `cppgm++ -c` compiles it
cleanly. The emitted object itself is discarded.

### Assignment Boundary

PA35 owns:

- compiling the heaviest hosted standard-library headers within the perf budget
- the template, trait, and overload-resolution depth those headers exercise
  during `-c` compilation

PA35 does not own:

- intrinsics, parser concessions, and header-free conformance anchors (PA34)
- hosted header-emitted link/runtime behavior (PA36)
- bootstrap or self-host builds

### Stage Handoff

The previous stage is PA34 (hosted intrinsics and header-free compile
conformance). The next stage is PA36, which keeps the same hosted header
environment but raises the contract from "it compiles" to "its emitted code also
links and runs."
