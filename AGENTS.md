# Repository Guide

## Scope

This repository holds the CPPGM self-hosting C++ compiler, the thirty-nine
assignments that build it up, and the harnesses that keep every milestone
stable.  It is also the source of the student-facing assignment export.

The main rule for code changes is:

- Put production compiler changes in `dev/` and especially `dev/src/`.
- Treat `pa1/` through `pa39/` as assignment specifications, tests, wrappers
  and reference outputs unless a task explicitly asks to change a test, a
  harness or a reference.

## Current State

All assignment buildouts through PA39 exist in this checkout.  The compiler
has the full frontend, semantic analysis, LowIR generation, the native object
and link surface, the LowIR and machine optimizers, hosted compatibility and
the PA39 self-host ladder.  The reference implementation and the course
solution are the same tree: every reference output is regenerated from
`dev/`, and the assignment `*-ref` wrappers point at the tools built there.

Binaries built from `dev/`:

- `pptoken`, `posttoken`, `ctrlexpr`, `macro`, `preproc`, `recog`
- `nsdecl`, `nsinit`, `cy86`
- `cppgm++`
- `abimangle`, `lowir2cy86`, `lowiropt`, `lowir2native`

The assignment arc is in `ROADMAP.md`.

## Top-Level Layout

- `Makefile`: root orchestration for builds, the global test report, the
  debug-info and design-variant suites, reference regeneration, the audits
  and the PA39 inception wrapper.
- `dev/`: the compiler implementation, its build rules and the tool mains.
  `dev/*-scaffold.cpp` are the starting points the student export installs.
- `dev/src/`: the compiler libraries, by stage: `preprocess`, `recognition`,
  `syntax`, `semantic`, `namespace_semantics`, `namespace_initialization`,
  `abi`, `lowering`, `lowir`, `native`, `cy86`, `compiler_object`, `support`.
- `dev/frontend_source_sets.mk`: the checked-in source set of each binary.
- `pa1/` through `pa39/`: handouts, wrappers, tests, scripts and references.
  Every test an assignment runs lives under its own `paN/tests/`; see
  `docs/student-export-root/TESTING_AND_REFERENCES.md` for the buckets, the
  regression lane (`paN/tests/regression/`), the controls
  (`paN/tests/controls/`) and the numbering rule.
- `cppgm.tests/`: only `undefined/`, inputs whose outcome the course leaves
  unspecified; no lane runs them.
- `scripts/`: the shared test runners and comparison, the checker scripts
  the controls use, the audits, the placement auditor and the export.
- `doc/`: the ledgers the audits read (symbol owners, the LowIR contract, the
  PA contract allowlist) and the backend review records.
- `docs/`: active plans and trackers, `docs/student-export-root/` (the root
  documents of the export), `docs/v4/` (the trackers of the move that made
  this tree the reference), and `docs/implemented/` for finished plans.
- `shared/`, `docker/`, `benchmarks/`: the assignments' shared grammar and
  runtime material, the PA9 container, and stable performance inputs.
- `obj/`: generated files and build artifacts.  Never hand-edit or commit.

## Build Defaults

On Linux the root, `dev/` and `pa39` Makefiles default to `g++`.  On macOS
they prefer Homebrew LLVM when it is installed
(`/usr/local/opt/llvm/bin/clang++`, `/opt/homebrew/opt/llvm/bin/clang++`).
When you pass a compiler explicitly, pass the host compiler too:

```sh
make CXX=clang++ CPPGM_HOST_CXX=clang++
```

`CPPGM_HOST_CXX` is the host compiler the built compiler embeds in its
defaults and generated host configuration.  When building with
`CXX=../dev/cppgm++`, keep a real host compiler in `CPPGM_HOST_CXX`.

The shared `obj/` root assumes `CXX` and `CPPGM_HOST_CXX` are the same host
compiler; self-host and mixed-compiler work uses its own object root
(`OBJ=../obj/pa39/...`).

## Main Build And Test Commands

From the repository root:

```sh
make
make test
make test-pa22
make test-report
make test-report ACTIVE_TEST_REPORT_PAS='pa15'
make test-report-through-pa22
CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 make test-report
make test-debuginfo
make test-variants
make ref-test
make ref-test-pa22
make ref-test-debuginfo
make inception
```

- `make` builds every `dev/` tool.
- `make test` builds once and runs the assignment suites (`pa1` through
  `pa38`); `make test-paN` runs one.
- `make test-report` is the broad regression surface: keep-going, per-PA
  summaries, parallelism and stall reporting.  `ACTIVE_TEST_REPORT_PAS`
  narrows it; `ORDERED=false` streams output as jobs finish.  CI runs it
  with `CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1`, the byte-exact LowIR comparison,
  so references must match the current build exactly.
- `make test-debuginfo` runs the debug-info preservation suites of PA37 and
  PA38; `make test-variants` runs the PA38 suite under the other backend
  designs (`dev/src/backend_variant.h`).
- `make ref-test` regenerates every reference from the tools in `dev/`;
  `make ref-test-paN` one assignment; `make ref-test-debuginfo` the
  debug-info references.  A regeneration on an unchanged compiler leaves a
  clean tree; the export verifies that.
- `make inception` builds the PA39 self-host target and compares it with the
  host build.  The CI gates that `test-report` does not cover are
  `make test-debuginfo` and `make -C pa39 test-through-pa10 CXX=../dev/cppgm++`;
  run both before calling a compiler change done.

Knobs: `TEST_REPORT_SUBTEST_JOBS`, `TEST_REPORT_ASSIGNMENT_JOBS`,
`TEST_REPORT_STALL_SEC`, `CPPGM_TEST_RUNNER=0` (disables the compiled-in
batch runner), `CPPGM_STDLIB_FLAGS=...`.

## Audits

The audits are the repository's architecture policy and nothing in
`make test` runs them, so run them all after a round of compiler changes:

```sh
make audit-lowir-contract audit-compiler-layout audit-compiler-rename-manifest \
  audit-compiler-exceptions audit-frontend-source-sets audit-semantic-owners \
  audit-builtin-registry-tables audit-lowering-owners audit-native-owners
perl scripts/cppgm_file_audit.pl --paths dev
python3 scripts/audit_pa_feature_placement.py --fail-on-early
```

The file audit enforces file and function size limits and forbids
environment reads in `dev/src`.  The placement auditor is the judge of where
a fixture sits: the assignment that owns the latest feature it uses, in that
feature's cluster (`paN/tests/general/300-x.t`).  It is a CI job.

## PA39 Self-Host Surfaces

PA39 builds staged checkpoint binaries with `cppgm++`.  Its goal is
inception: rebuilding `cppgm++` with `cppgm++` and matching the host build.
The `pa39` wrapper owns the `test-through-*` ladder; the root Makefile keeps
only `inception`.

```sh
make -C pa39 cppgm++-self CXX=../dev/cppgm++
make -C pa39 test-through-pa10 CXX=../dev/cppgm++
make -C pa39 test-through-pa38 CXX=../dev/cppgm++
make inception
```

Use `INCEPTION_OBJ_ROOT_BASE=...` to keep several self-host object trees.

## Performance Checks

Use the performance gate when a change could affect compile time,
allocation, memory or self-host throughput:

```sh
scripts/validate_perf_regression.py record \
  --baseline /tmp/cppgm-semantic-overload-baseline.json --runs 3
scripts/validate_perf_regression.py check \
  --baseline /tmp/cppgm-semantic-overload-baseline.json
```

The signal is the hardware instruction count plus resident memory; wall
time is recorded but too noisy to gate on.

## `dev/src/` Orientation

- `preprocess/`: phases 1 to 4, tokens, macros, the control expression and
  the hosted builtin registry.
- `recognition/`, `syntax/`: the recognizer and the parser.
- `semantic/`: the semantic model, analysis, constants, lifetime and
  templates; `namespace_semantics/` and `namespace_initialization/` are the
  PA7 and PA8 surfaces.
- `abi/`: the typed Itanium ABI model and encoder (PA14).
- `lowering/`: source to LowIR, by concern (`calls`, `control`, `objects`,
  `extensions`, `presentation`, `core`).
- `lowir/`: the LowIR model, reader and writer, the optimizer (`optimize/`)
  and the CY86 backend.
- `native/`: LowIR to machine code: analysis, lowering, allocation, frame,
  encoding, exception tables and the object writers.
- `cy86/`, `compiler_object/`, `support/`: the CY86 assembler, the object
  format shared with the driver, and testing and numeric support.

Prefer extending these modules over duplicating logic in an assignment
directory.  Keep `dev/frontend_source_sets.mk` and the owner ledgers in
`doc/` in step with new files.

## Assignment Directory Guidance

The `pa*` directories are thin wrappers around the compiler: handout, local
tests, grammar notes, scripts, Makefile and references.

When changing tests or references:

- Put a test in the earliest assignment that owns the behaviour, in the
  cluster of the latest feature it exercises; the placement auditor tells
  you when it disagrees.
- Prefer small, reduced tests that isolate one issue.
- A fixture whose only justification is the shape the course solution
  produces belongs in the regression lane, `paN/tests/regression/`.
- Regenerate references with `make ref-test-paN`; decide first whether a
  LowIR difference is a behaviour change or a presentation change
  (`pa13/lowir.md` lists what the comparison absorbs and what it enforces).
- Never edit `.my*` outputs, `testout`, logs or built binaries.

## Editing Guidance

- Keep production code in `dev/` and `dev/src/`.
- Keep changes scoped to the behaviour under investigation.
- Preserve the existing local patterns and helper APIs before introducing
  new abstractions.
- Prefer structured parsers and semantic data over reparsing source text.
- Do not commit generated artifacts from `obj/`, profiler output, `.my`
  files, reference churn the task does not need, or temporary reports.
- Inspect git state from the repository root; several worktrees are often
  in use.

## Useful Starting Points

- `Makefile`, `dev/Makefile`, `dev/frontend_source_sets.mk`
- `dev/cppgm++.cpp` (the driver) and `dev/src/support/testing/`
- `dev/src/semantic/analysis/analyzer.h`
- `dev/src/lowering/core/program_lowerer.cpp` and `lowering/core/driver.cpp`
- `dev/src/lowir/optimize/pipeline.cpp`
- `dev/src/native/lowering/function.cpp` and `native/allocation/`
- `pa13/lowir.md`, `pa39/README.md`, `pa39/Makefile`
- `docs/performance-regression-validation.md`
- `docs/PLAN-CPPGM-EXTENDED-V4.md` and `docs/v4/`

For milestone-specific behaviour, read the assignment's `README.md` and
follow the wrapper to the implementation in `dev/`.
