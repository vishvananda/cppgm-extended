# Repository Guide

## Scope

This repository contains the active implementation of the CPPGM self-hosting
C++ compiler, plus the assignment wrappers, reference fixtures, and validation
harnesses used to keep each milestone stable.

The main rule for code changes is:

- Put production compiler changes in `dev/` and especially `dev/src/`.
- Treat `pa1/` through `pa39/` primarily as assignment specifications, tests,
  wrappers, and reference fixtures unless a task explicitly asks to update a
  test, harness, or reference.

## Current State

All assignment buildouts through PA39 exist in this checkout. The compiler is no
longer an early PA1-PA9 starter implementation; it has the full frontend,
semantic analysis, LowIR generation, native object/link surface, optimization
passes, hosted compatibility work, and the PA39 self-host ladder.

Important user-facing binaries built from `dev/`:

- `pptoken`, `posttoken`, `ctrlexpr`, `macro`, `preproc`, `recog`
- `nsdecl`, `nsinit`, `cy86`
- `cppgm++`
- `lowir2cy86`, `lowiropt`, `lowir2native`
- `cpplink`, `cppeh`

The assignment arc is roughly:

- PA1-PA9: preprocessing, recognition, namespace semantics, CY86 scaffold
- PA10-PA12: AST, types/lookup, calls/conversions/overload resolution
- PA13-PA14: LowIR contract and source-to-LowIR lowering
- PA15-PA22: classes, value semantics, virtuals, templates, witness output
- PA23-PA25: native execution, linking, exception/runtime surfaces
- PA26-PA30: language completion and compile/link driver integration
- PA31-PA36: ABI naming, object generation, host ABI, hosted headers
- PA37-PA38: LowIR and machine-backend optimization
- PA39: staged self-host ladder

## Top-Level Layout

- `Makefile`: root orchestration for builds, global reports, strict tests,
  reference regeneration, and the PA39 inception wrapper.
- `dev/`: active compiler implementation and build rules.
- `dev/src/`: shared compiler libraries used by all frontend tools.
- `dev/frontend_source_sets.mk`: checked-in reduced-link source-set manifest
  for each frontend binary.
- `pa1/` through `pa39/`: milestone specs, wrappers, tests, scripts, and refs.
- `cppgm.tests/`: shared external course tests used by assignment harnesses.
- `scripts/`: shared test runners, comparison tools, PA39/self-host helpers,
  and validation scripts.
- `docs/`: active plans, current trackers, operational references, and the
  `docs/implemented/` archive for completed plan records.
- `legacy/`: obsolete process docs, historical frontier trackers, old
  investigations, and completed holding-area notes.
- `benchmarks/`: stable compile/performance inputs.
- `validation/`: auxiliary validation fixtures.
- `obj/`: shared generated files and build artifacts. Do not hand-edit or
  commit generated contents from here.

## Build Defaults

On Linux, the root, `dev/`, and `pa39` Makefiles default to `g++`, which uses
libstdc++ by default. On macOS, they prefer Homebrew LLVM when available:

- `/usr/local/opt/llvm/bin/clang++`
- `/opt/homebrew/opt/llvm/bin/clang++`

If you pass a compiler explicitly, also pass the host compiler when needed:

```sh
make CXX=/usr/local/opt/llvm/bin/clang++ \
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
```

`CPPGM_HOST_CXX` controls the host compiler path embedded in compiler defaults
and generated host configuration. If building with `CXX=../dev/cppgm++`, use a
real host compiler for `CPPGM_HOST_CXX`.

The shared `obj/` root assumes `CXX` and `CPPGM_HOST_CXX` are the same host
compiler. For self-host or mixed-compiler experiments, use an isolated object
root such as `OBJ=../obj/pa39/...` or another task-specific directory.

## Main Build And Test Commands

From the repository root:

```sh
make
make test
make test-pa22
make test-report
make test-report ACTIVE_TEST_REPORT_PAS='pa15'
make test-report-through-pa22
make ref-test
make ref-test-pa22
make test-strict
CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 make test-strict
make test-strict-pa22
make inception
```

Current behavior:

- `make` builds all `dev/` frontends.
- `make test` builds once and runs the non-experimental assignment tests
  (`pa1` through `pa38` by default).
- `make test-paN` builds once and runs one assignment.
- `make ref-test` regenerates assignment `.ref` outputs. Later PA Makefiles
  invoke provided `*-ref` binaries directly and fail if the ref binary is
  missing.
- `make ref-test-paN` regenerates refs for one assignment.
- `make test-report` is the preferred broad regression surface. It runs
  assignment tests with keep-going behavior, per-PA summaries, parallelism, and
  stall reporting.
- `ACTIVE_TEST_REPORT_PAS='pa12 pa15'` narrows `test-report` to selected PAs.
  Passing a single PA here is the usual way to get all failures for that PA at
  once.
- `make test-report-through-paN` runs the report suite through a milestone.
- `make test-strict` runs the stricter witness/reference suites configured by
  `STRICT_PAS` (`pa18 pa19 pa21 pa22` by default).
- `CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1` enables direct LowIR text comparison in
  the shared comparison harness.
- `make inception` builds the final PA39 self-host target and compares it with
  the host-built `cppgm++`.

Useful knobs:

- `ORDERED=false` streams completed `test-report` output as jobs finish.
- `TEST_REPORT_SUBTEST_JOBS`, `TEST_REPORT_ASSIGNMENT_JOBS`, and
  `STRICT_SUBTEST_JOBS` tune parallelism.
- `TEST_REPORT_STALL_SEC` controls stalled-test progress messages.
- `CPPGM_TEST_RUNNER=0` disables the compiled-in batch test runner path.
- `CPPGM_STDLIB_FLAGS=...` passes additional standard-library flags through
  compiler and test builds.

## PA39 Self-Host Surfaces

PA39 builds staged checkpoint binaries with `cppgm++`. Its main goal is
inception: rebuilding `cppgm++` with `cppgm++` and matching the host build. The
`pa39` wrapper owns the intermediate `test-through-*` ladder; the root Makefile
keeps only the final `inception` shortcut.

Useful direct PA39 commands:

```sh
make -C pa39 cppgm++-self CXX=../dev/cppgm++ \
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
make -C pa39 test-through-pa10 CXX=../dev/cppgm++ \
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
make -C pa39 test-through-pa38 CXX=../dev/cppgm++ \
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
make inception CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
```

Use `INCEPTION_OBJ_ROOT_BASE=...` when comparing host-compiler lanes or preserving
multiple self-host object trees.

## Performance Checks

Use the documented performance gate when a change could affect compile time,
allocation behavior, memory use, or self-host throughput:

```sh
scripts/validate_perf_regression.py record \
  --baseline /tmp/cppgm-semantic-overload-baseline.json \
  --runs 3

scripts/validate_perf_regression.py check \
  --baseline /tmp/cppgm-semantic-overload-baseline.json
```

The primary signal is hardware instruction count plus resident/footprint memory,
not wall time. Wall time is recorded but is too noisy on a loaded host to be the
main gate.

## `dev/src/` Orientation

The current implementation is split across these broad areas:

- Tokenization/preprocessing: `pptokenizer.*`, `posttokenizer.*`,
  `calculator.*`, `macroizer.*`, `preprocessor.*`, `preproc_output.*`
- Parsing and AST: `recog_parser.*`, `recog_token_*`, `cppast_*`,
  `cpp_decl_ast.*`, `template_angle_parser.*`
- Type and semantic model: `cpp_decl_model.*`, `semantic_model.*`,
  `semantic_*`, `callsemantic.*`, `callsemantic/`, `typesemantic.*`
- Templates and witness output: `template_*`, `witness_api.*`,
  `witness_text.*`, `template_witness_renderer.*`
- LowIR: `lowir_internal.*`, `lowirgensemantic.*`, `lowir_backend.*`,
  `lowir_cy86_backend.*`, `lowir_optimizer.*`, `lowir_driver_frontend.*`,
  `lowir_tool_cli.*`
- Native/object/link/runtime: `lowir_machine_ir.*`, `lowir_object_backend.*`,
  `machine_*`, `native_format.*`, `elf_writer.*`, `macho_writer.*`,
  `runtime_symbol_policy.*`, `eh_runtime.*`, `host_*`
- Tool and batch frontends: `cpp_tool_cli.*`, `cpp_driver_frontend.*`,
  `cpp_batch_frontend.*`, `cli_batch_frontend.*`
- Metrics/debug/audit helpers: `semantic_metrics.*`, `semantic_hotspot.*`,
  `semantic_trace.*`, `semantic_fallback_audit.*`, `parser_trace.*`

Prefer extending these shared modules over duplicating logic in assignment
directories. For new frontend object ownership, keep
`dev/frontend_source_sets.mk` in sync.

## Assignment Directory Guidance

The `pa*` directories are thin wrappers around the compiler implementation.
They contain README/spec text, local tests, grammar notes, scripts, Makefiles,
and reference outputs.

When changing tests or refs:

- Keep tests in the earliest PA that owns the behavior.
- Prefer small, reduced tests that isolate the semantic issue.
- Use `make ref-test-paN` or `make -C paN ref-test` to regenerate reference
  outputs. The export workflow is expected to provide `*-ref` binaries; there
  is no fallback to the student/current compiler.
- Do not update witness refs from `cppgm++`; witness refs should come from the
  clang witness-generation path.
- For LowIR drift, decide whether it is a real behavior change or a reference
  update before changing refs.
- Avoid editing `.my`, `testout`, generated logs, or local binaries.

## Editing Guidance

- Keep production code in `dev/` or `dev/src/`.
- Keep changes scoped to the behavior under investigation.
- Preserve existing local patterns and helper APIs before introducing new
  abstractions.
- Prefer structured parsers and semantic data over source-text reparsing or
  ad hoc string matching.
- Treat witness-output differences as possible semantic bugs first. Avoid
  witness-renderer recovery hacks unless the surrounding design explicitly
  calls for formatting-only logic.
- Avoid committing generated artifacts from `obj/`, profiler output, `.my`
  files, `.ref` churn not required by the task, or temporary reports.
- Inspect git state from the repository root. Multiple worktrees are often in
  use, so confirm the active checkout before editing.

## Useful Starting Points

For broad changes, start with:

- `Makefile`
- `dev/Makefile`
- `dev/frontend_source_sets.mk`
- `dev/cppgm++.cpp`
- `dev/src/cpp_tool_cli.*`
- `dev/src/cpp_driver_frontend.*`
- `dev/src/cpp_batch_frontend.*`
- `dev/src/callsemantic.cpp`
- `dev/src/callsemantic/`
- `dev/src/semantic_*.{h,cpp}`
- `dev/src/template_*.{h,cpp}`
- `dev/src/witness_api.*`
- `dev/src/lowirgensemantic.*`
- `dev/src/lowir_internal.*`
- `dev/src/lowir_optimizer.*`
- `dev/src/lowir_machine_ir.*`
- `dev/src/lowir_object_backend.*`
- `pa39/README.md`
- `pa39/Makefile`
- `docs/performance-regression-validation.md`

For milestone-specific behavior, read the corresponding `paN/README.md` and
then follow the wrapper to the real implementation in `dev/`.
