# Repository guide

This is the complete cppgm-extended compiler solution and the source of its
student export. The course has 35 cumulative assignments; see [ROADMAP.md](ROADMAP.md).
Follow [spec.md](spec.md) for the compiler architecture, including parsing
source once, consuming typed facts and avoiding speculative work on hot paths.
The original CPPGM material has been substantially adapted; preserve inherited
attribution and consult [NOTICE](NOTICE), not historical PA numbers, for origins.

## Ownership

- Put compiler changes in `dev/` and `dev/src/`. Keep tool source lists in
  `dev/frontend_source_sets.mk` current.
- Treat `pa1/` through `pa35/` as handouts, tests, references and wrappers.
  Read the owning handout before changing a milestone's behavior.
- Required fixtures live in the earliest owning `paN/tests/`, in the cluster
  of the latest feature they exercise, inside a lane that assignment's
  Makefile runs (`make test-harness` checks). `student.tests/` is for
  personal tests run explicitly; default suites do not discover it.
- Keep reusable logic in the existing compiler modules. Prefer typed parser
  and semantic data over reparsing source or rendered output.
- Never hand-edit generated references, `.my*` outputs, logs or `obj/`.
  Do not commit build artifacts or unrelated reference churn.
- Inspect Git state from the repository root; other worktrees may be active.

## Build and test

Linux defaults to `g++`; macOS prefers installed Homebrew LLVM. Pass both
compiler variables when selecting a host toolchain:

```sh
make CXX=clang++ CPPGM_HOST_CXX=clang++
make test-pa8
CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 make test-report
make test-debuginfo
make -C pa34 test-through-pa5 CXX=../dev/cppgm++ CPPGM_HOST_CXX=g++
make inception
```

Use the same host compiler and standard-library flags across a build. During
self-hosting, keep a real host compiler in `CPPGM_HOST_CXX`. Isolate self-host
or mixed-toolchain objects with `INCEPTION_OBJ_ROOT_BASE` or `OBJ`.

After compiler changes, run the affected suite, full strict report, debug-info
checks and self-host through PA5. Run inception when changing self-host
behavior, and `make -C pa35 test-smoke` when changing the optimizer, the
backend or the lowering; `make -C pa35 ref-test` is PA35's release check. PA34 owns the `test-through-*` ladder; root `make inception`
compares the rebuilt compiler.

See [TESTING_AND_REFERENCES.md](TESTING_AND_REFERENCES.md) for test selection,
reference regeneration and the distinction between this checkout and the
student export. Reference wrappers here invoke tools built from `dev/`.

## Audits

After compiler changes, run all architecture and placement checks; `make test`
does not run them:

```sh
make audit-lowir-contract audit-compiler-layout audit-compiler-exceptions \
  audit-frontend-source-sets audit-builtin-registry-tables \
  audit-scaffold-usage audit-scaffold-spellings audit-clang-format
perl scripts/cppgm_file_audit.pl --paths dev
python3 scripts/audit_pa_feature_placement.py --fail-on-early
```

`audit-clang-format` checks every C++ source under `dev/` (the scaffolds
included) and `scripts/` against the root `.clang-format`; format with
`clang-format -i`. It needs clang-format 19 or newer, since 18 formats this
tree differently; CI pins 21.1.8 (`pip install clang-format==21.1.8`), and
`CLANG_FORMAT` selects the binary. Test fixtures and `benchmarks/` are never
reformatted: their text and line numbers are inputs.

The file audit enforces file/function limits, counting lines of code (blank
and comment-only lines are not counted; a dispatch switch may run longer), and
forbids environment reads in `dev/src/`; it skips the shipped scaffold files.
The student export ships the audit, where those files are the student's own
code and are audited (they must pass; only the batch test runner is skipped).
Run `make test-harness` after changing harnesses or export logic. Validate a
student export after changing shipped files or their discovery.
Student documents are listed in `scripts/student_export_documents.txt`;
keep run plans, audits and implementation journals out of the export.

## Performance

Measure changes that can affect compile time, allocation, memory or self-host
throughput. `scripts/validate_perf_regression.py` provides the macOS
hardware-counter gate. For wall-time ABBA, use immutable A/B binaries, identical
frozen inputs, A/A calibration and paired blocks; retain every observation and
verify output equality.

## Layout

- `dev/src/lex`, `preprocess`, `syntax`, `sema`, `types`: tokens and
  literals, the preprocessor, the parser and syntax tree, semantic analysis,
  and the fundamental types.
- `abi`, `lower`, `lowir`, `opt`: Itanium names (with PA9's fact reader),
  lowering of the analyzed program to LowIR, the LowIR program with its text
  reader, writer and validator, and the LowIR optimizer. PA8's construction
  exercises are in `lowir/exercises.cpp`.
- `native`: instruction selection to machine IR, x86-64 encoding, objects,
  the course runtime and the linker; `driver`: `cppgm++`'s modes and host
  toolchain; `support`: shared utilities.
- The shipped scaffold files (`dev_support_files` in
  `scripts/export_student_repo.sh`) are course files and the vocabulary the
  implementation builds on as shipped. `audit-scaffold-usage` checks, from the
  build's dependency files, that each is compiled in and that the scaffold-only
  `support/not_implemented.h` is not; `audit-scaffold-spellings` checks that the
  literal words of the LowIR and MIR writers are vocabulary spellings.
- `scripts/`: runners, comparison, audits and export; `shared/`: source grammar.
- `doc/`: the C++ draft and Itanium ABI reference.
- `scripts/architecture/`: live audit tables.
- `docs/student-export-root/`: documents installed at the student repo root.
- `docs/v4/`: historical migration records; their original
  paths and numbers are historical evidence, not current workflow instructions.
- `benchmarks/`: fixed performance inputs; `obj/`: generated artifacts.

The nine tools are `pptoken`, `posttoken`, `ppexpr`, `preproc`, `cppgm++`,
`abimangle`, `lowir`, `lowiropt` and `lowir2native`. They all build from `dev/`.
