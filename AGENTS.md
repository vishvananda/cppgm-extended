# Repository guide

This is the complete cppgm-extended compiler solution and the source of its
student export. The course has 34 cumulative assignments; see [ROADMAP.md](ROADMAP.md).
Follow [spec.md](spec.md) for the compiler architecture, including parsing
source once, consuming typed facts and avoiding speculative work on hot paths.
The original CPPGM material has been substantially adapted; preserve inherited
attribution and consult [NOTICE](NOTICE), not historical PA numbers, for origins.

## Ownership

- Put compiler changes in `dev/` and `dev/src/`. Keep tool source lists in
  `dev/frontend_source_sets.mk` and the owner ledgers in `doc/` current.
- Treat `pa1/` through `pa34/` as handouts, tests, references and wrappers.
  Read the owning handout before changing a milestone's behavior.
- Required fixtures live in the earliest owning `paN/tests/`, in the cluster
  of the latest feature they exercise. `student.tests/` is for personal tests
  run explicitly; default suites do not discover it.
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
make test-variants
make -C pa34 test-through-pa5 CXX=../dev/cppgm++ CPPGM_HOST_CXX=g++
make inception
```

Use the same host compiler and standard-library flags across a build. During
self-hosting, keep a real host compiler in `CPPGM_HOST_CXX`. Isolate self-host
or mixed-toolchain objects with `INCEPTION_OBJ_ROOT_BASE` or `OBJ`.

After compiler changes, run the affected suite, full strict report, debug-info
checks and self-host through PA5. Run backend variants when changing lowering
or optimization, and inception when changing self-host behavior. PA34 owns the
`test-through-*` ladder; root `make inception` compares the rebuilt compiler.

See [TESTING_AND_REFERENCES.md](TESTING_AND_REFERENCES.md) for test selection,
reference regeneration and the distinction between this checkout and the
student export. Reference wrappers here invoke tools built from `dev/`.

## Audits

After compiler changes, run all architecture and placement checks; `make test`
does not run them:

```sh
make audit-lowir-contract audit-compiler-layout audit-compiler-rename-manifest \
  audit-compiler-exceptions audit-frontend-source-sets audit-semantic-owners \
  audit-builtin-registry-tables audit-lowering-owners audit-native-owners
perl scripts/cppgm_file_audit.pl --paths dev
python3 scripts/audit_pa_feature_placement.py --fail-on-early
```

The file audit enforces file/function limits and forbids environment reads in
`dev/src/`. Run `make test-harness` after changing harnesses or export logic.
Validate a student export after changing shipped files or their discovery.

## Performance

Measure changes that can affect compile time, allocation, memory or self-host
throughput. [The performance guide](docs/performance-regression-validation.md)
documents the macOS hardware-counter gate. When wall-time ABBA is requested,
use immutable A/B binaries, identical frozen inputs, A/A calibration and paired
blocks; retain every observation and verify output equality. The
[consolidation measurement record](docs/early-assignment-consolidation-abba-validation.md)
shows the native Linux protocol and its accepted evidence.

## Layout

- `dev/src/preprocess`, `syntax`, `semantic`: source-language processing.
- `abi`, `lowering`, `lowir`, `native`: ABI names, typed IR, optimization and
  native code. PA8's LowIR construction work lives in `lowir/intro/`.
- `compiler_object`, `support`: object/link support and shared utilities.
- `scripts/`: runners, comparison, audits and export; `shared/`: source grammar.
- `doc/`: live architecture ledgers and reference material.
- `docs/student-export-root/`: documents installed at the student repo root.
- `docs/implemented/`, `docs/v4/`: historical plans and records; their original
  paths and numbers are historical evidence, not current workflow instructions.
- `benchmarks/`: fixed performance inputs; `obj/`: generated artifacts.

The nine tools are `pptoken`, `posttoken`, `ppexpr`, `preproc`, `cppgm++`,
`abimangle`, `lowir`, `lowiropt` and `lowir2native`. They all build from `dev/`.
