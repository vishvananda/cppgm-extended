# Source Coverage Analysis Strategy

## Goal

Use compiler source coverage from the full assignment test report to classify
which implementation lines are exercised by tests, which are untested but
reachable, which are configuration-gated, and which are likely dead or
obsolete.

This is a test quality and code health tool, not a pass/fail gate at first.
`main` may have unrelated test failures while another worker is repairing it;
coverage data from completed test processes is still useful.

## Coverage Mechanism

Use Clang source-based coverage:

- compile with `-fprofile-instr-generate -fcoverage-mapping`
- run the assignment tests with `LLVM_PROFILE_FILE` pointing at a profile
  directory
- merge `*.profraw` with `llvm-profdata`
- inspect with `llvm-cov report`, `llvm-cov show`, and `llvm-cov export`

This gives line, region, branch, and function coverage. It also gives template
and inline function coverage through Clang's source mapping, which is important
for this codebase.

Homebrew LLVM is already available on this host:

- `/usr/local/opt/llvm/bin/clang++`
- `/usr/local/opt/llvm/bin/llvm-profdata`
- `/usr/local/opt/llvm/bin/llvm-cov`

## Validated Probe

The mechanics were validated on `pa1`/`pptoken`.

```sh
rm -rf /tmp/cppgm-coverage-probe-obj /tmp/cppgm-coverage-probe-profraw
mkdir -p /tmp/cppgm-coverage-probe-profraw

MAKEFLAGS=-j4 make -C dev pptoken \
  OBJ=/tmp/cppgm-coverage-probe-obj \
  CXX=/usr/local/opt/llvm/bin/clang++ \
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++ \
  CC_FLAGS='-std=gnu++11 -Wall -O0 -g -fprofile-instr-generate -fcoverage-mapping $(CPPGM_STDLIB_FLAGS) $(HOST_CXX_DEFAULT_DEF) $(OBJECT_ROOT_DEFAULT_DEF)'

LLVM_PROFILE_FILE='/tmp/cppgm-coverage-probe-profraw/%m-%p.profraw' \
MAKEFLAGS=-j4 make -C pa1 test \
  CPPGM_SKIP_DEV_REBUILD=1 \
  OBJ=/tmp/cppgm-coverage-probe-obj \
  CXX=/usr/local/opt/llvm/bin/clang++ \
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++ \
  CC_FLAGS='-std=gnu++11 -Wall -O0 -g -fprofile-instr-generate -fcoverage-mapping $(CPPGM_STDLIB_FLAGS)'

/usr/local/opt/llvm/bin/llvm-profdata merge -sparse \
  /tmp/cppgm-coverage-probe-profraw/*.profraw \
  -o /tmp/cppgm-coverage-probe.profdata

/usr/local/opt/llvm/bin/llvm-cov report ./pa1/pptoken \
  -instr-profile=/tmp/cppgm-coverage-probe.profdata \
  dev/src/pptokenizer.cpp dev/src/encoding.cpp
```

Important detail: early PA directories run local wrapper binaries. The coverage
flags must be visible to those submakes too, otherwise the local wrapper link
uses non-coverage flags and fails against coverage-instrumented objects.

## Full Report Workflow

The implemented entry point is:

```sh
make source-coverage-report
```

That calls `scripts/run_source_coverage_report.sh`, which builds coverage
instrumented dev binaries into an isolated object root, runs
`test-report-nobuild`, runs the strict semantic/witness supplemental suite, runs
the standalone `template-kernel` suite, runs a `CPPGM_MEMORY_CENSUS` smoke test,
runs Linux-target executable and object-file smoke tests, runs a host builtin
runtime smoke test, runs a host exception-handling object/roundtrip smoke test,
runs a `CPPGM_SEMANTIC_HOTSPOT` smoke test, runs diagnostic instrumentation
smoke tests for trace/timing/semantic stats/output requirement trace/template
audit knobs, runs a
non-test-runner CLI batch frontend smoke test, runs a tool help/query smoke
test, merges `*.profraw`, and writes
`report.txt`, `coverage.json`, optional HTML, and `summary.txt` under the
coverage root. The supplemental runs keep strict-only witness paths and
template-kernel paths from being mistaken for dead code, keep memory
instrumentation classified as an environment-gated configuration path, keep
Mac-host coverage from treating Linux/ELF target emission and parsing as dead,
keep host Mach-O exception sections from looking like ordinary untested object
paths, keep runtime support functions from being mistaken for compiler
control-flow, keep semantic hotspot profiling classified as an environment-gated
instrumentation path, keep diagnostic trace/timing/metrics/output requirement
trace code classified as environment-gated instrumentation, keep template
upgrade audit logging classified as diagnostic-only, keep non-test-runner batch
frontend handling visible despite the normal test-runner wrapper intercepting
batch mode, and keep user-facing tool query paths from being mistaken for
assignment semantic behavior.

Useful scoped run:

```sh
scripts/run_source_coverage_report.sh \
  --root /tmp/cppgm-source-coverage-pa1 \
  --pas 'pa1' \
  --jobs 4 \
  --subtest-jobs 2 \
  --assignment-jobs 1 \
  --no-html
```

Use `--skip-strict`, `--skip-template-kernel`, `--skip-memory-census`,
`--skip-linux-target`, `--skip-host-runtime`, `--skip-host-eh-object`,
`--skip-semantic-hotspot`, `--skip-diagnostic-instrumentation`,
`--skip-cli-batch`, or `--skip-tool-help`
when the goal is to reproduce assignment-only `test-report` coverage exactly.

Per-assignment attribution is implemented as an optional mode:

```sh
make source-coverage-report-by-pa

scripts/run_source_coverage_report.sh \
  --root /tmp/cppgm-source-coverage-pa1-by-pa \
  --pas 'pa1' \
  --by-pa \
  --jobs 4 \
  --subtest-jobs 2 \
  --assignment-jobs 1 \
  --no-html
```

`--by-pa` writes `by-pa/<pa>/coverage.json`, `by-pa/<pa>/line-counts.txt`,
and a root-level `line-attribution.csv`, `line-attribution.json`,
`line-attribution-summary.txt`, `unhit-lines.txt`, and `review-queue.md`. The
attribution matrix uses `llvm-cov show` line-count text when available because
it matches the reviewable source display. The JSON segment data remains
available as a fallback and for deeper region/branch analysis.

Use an isolated object root so coverage objects do not mix with release,
debug, profile, or self-host objects.

```sh
COV_ROOT=/tmp/cppgm-source-coverage-$(date +%Y%m%d-%H%M%S)
mkdir -p "$COV_ROOT/profraw"

COV_CC_FLAGS='-std=gnu++11 -Wall -O0 -g -fprofile-instr-generate -fcoverage-mapping $(CPPGM_STDLIB_FLAGS) $(HOST_CXX_DEFAULT_DEF) $(OBJECT_ROOT_DEFAULT_DEF)'
COV_PA_CC_FLAGS='-std=gnu++11 -Wall -O0 -g -fprofile-instr-generate -fcoverage-mapping $(CPPGM_STDLIB_FLAGS)'

MAKEFLAGS=-j4 make -C dev all \
  OBJ="$COV_ROOT/obj" \
  GENERATED="$COV_ROOT/obj/generated" \
  CXX=/usr/local/opt/llvm/bin/clang++ \
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++ \
  CC_FLAGS="$COV_CC_FLAGS"

LLVM_PROFILE_FILE="$COV_ROOT/profraw/%m-%p.profraw" \
MAKEFLAGS=-j4 make test-report-nobuild \
  OBJ="$COV_ROOT/obj" \
  GENERATED="$COV_ROOT/obj/generated" \
  CXX=/usr/local/opt/llvm/bin/clang++ \
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++ \
  CC_FLAGS="$COV_PA_CC_FLAGS"

/usr/local/opt/llvm/bin/llvm-profdata merge -sparse \
  "$COV_ROOT"/profraw/*.profraw \
  -o "$COV_ROOT/all.profdata"
```

Then report against the coverage-instrumented executables. The helper script
builds this object list automatically; the manual form is:

```sh
TOOLS='pptoken posttoken ctrlexpr macro preproc recog lowir2cy86 lowiropt lowir2native cppeh cpplink tmplsolve cppgm++ mobjroundtrip nsdecl nsinit cy86'

first=
objects=
for dir in dev pa*; do
  for tool in $TOOLS; do
    path="$dir/$tool"
    if [ -x "$path" ]; then
      if [ -z "$first" ]; then
        first="$path"
      else
        objects="$objects -object=$path"
      fi
    fi
  done
done

/usr/local/opt/llvm/bin/llvm-cov report "$first" $objects \
  -instr-profile="$COV_ROOT/all.profdata" \
  dev

/usr/local/opt/llvm/bin/llvm-cov show "$first" $objects \
  -instr-profile="$COV_ROOT/all.profdata" \
  -format=html \
  -output-dir="$COV_ROOT/html" \
  dev

/usr/local/opt/llvm/bin/llvm-cov export "$first" $objects \
  -instr-profile="$COV_ROOT/all.profdata" \
  dev > "$COV_ROOT/coverage.json"
```

The first version should focus on hit/no-hit line classification. Execution
counts are useful for hotspots, but duplicate linked frontends can make counts
less meaningful than boolean coverage.

## Per-Assignment Attribution

The merged full report answers "is this line exercised at all?" To answer
"which assignment exercises it?", repeat the run with one profile directory per
PA:

```sh
for pa in pa1 pa2 pa3; do
  mkdir -p "$COV_ROOT/by-pa/$pa/profraw"
  LLVM_PROFILE_FILE="$COV_ROOT/by-pa/$pa/profraw/%m-%p.profraw" \
    MAKEFLAGS=-j4 make -C "$pa" test \
      CPPGM_SKIP_DEV_REBUILD=1 \
      OBJ="$COV_ROOT/obj" \
      GENERATED="$COV_ROOT/obj/generated" \
      CXX=/usr/local/opt/llvm/bin/clang++ \
      CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++ \
      CC_FLAGS="$COV_PA_CC_FLAGS" || true
  /usr/local/opt/llvm/bin/llvm-profdata merge -sparse \
    "$COV_ROOT/by-pa/$pa"/profraw/*.profraw \
    -o "$COV_ROOT/by-pa/$pa.profdata" || true
done
```

The implemented script:

- runs the selected PA list into separate profile directories
- exports JSON and line-count text per PA
- builds a source-line to PA-hit matrix
- marks lines as `hit_any`, `hit_pas`, or unhit
- groups unhit lines into review ranges in `review-queue.md`

Exact per-test attribution will need harness support or an app wrapper that
sets `LLVM_PROFILE_FILE` using the test path. Per-PA attribution is cheaper and
should be enough for the first triage pass.

## Filtering

Initial analysis should include:

- `dev/*.cpp`
- `dev/src/**/*.cpp`
- `dev/src/**/*.h` only when the coverage report shows executable inline code

Initial analysis should exclude:

- `obj/`, generated headers, `.my`/`.ref` outputs, and `.dSYM`
- test harness code unless the question is about harness quality; the provided
  script excludes `dev/src/test_runner.cpp` by default
- third-party or course fixture directories

Keep platform and configuration guarded code visible in the report, but tag it
separately rather than treating it as a normal test gap.

## Triage Rubric

Classify each uncovered region into one of these buckets before changing code.

### 1. Test Gap

Use this when the code is reachable, belongs to current behavior, and there is
no existing test exercising it.

Action:

- add a focused test in the earliest PA whose README/spec requires the behavior
- if the behavior is not observable until a later backend/linkage stage, put the
  test in the first PA where it is observable
- add later PA coverage only when the later stage has distinct semantics, such
  as object emission, native linking, EH lowering, RTTI/vtables, or optimization
- prefer tests that assert positive required output; avoid absent-symbol checks
  unless the assignment explicitly owns non-emission behavior

### 2. Dead Or Obsolete Code

Use this when the code has no callers, is shadowed by a newer canonical path, or
only supports a removed mode.

Action:

- confirm with `rg`, call sites, and surrounding control flow
- remove in a small commit when no assignment contract still needs it
- run the nearest PA tests plus any strict or report target that owns the area
- if removal is risky, first add logging/assertion or a temporary diagnostic
  probe to prove it is unreachable under full `test-report`

### 3. Platform Or Configuration Gap

Use this when the code is intentionally gated by host OS, target OS,
optimization level, sanitizer/profile mode, compiler brand, or an environment
flag.

Action:

- do not add a Mac-host test for Linux-only code just to raise the percentage
- record the missing configuration and the command needed to cover it
- add a specific platform/config test only if the project supports running that
  configuration in CI or a repeatable local harness
- examples: Linux object format paths on macOS, Mach-O exception section paths
  on non-Darwin hosts, host ABI variants, ASAN-only paths, debug-info-only
  paths, O1/O2-only optimization paths

### 4. Behavior Bug

Use this when a current test should exercise the code but coverage shows a
different path is taken.

Action:

- inspect the semantic/output trace for the test that should hit the line
- decide whether the wrong path is harmless, a missed optimization, or a
  semantic bug
- fix the implementation first, then add or tighten a regression test in the PA
  that owns the behavior

### 5. Defensive Or Error Path

Use this when the code handles malformed input, impossible internal states, or
diagnostic-only paths.

Action:

- add a negative test only if the assignment specifies the diagnostic or
  rejection behavior
- keep the path if it protects against malformed student input or external
  toolchain behavior
- remove only if the invariant is now enforced earlier and the later check is
  demonstrably redundant

### 6. Instrumentation Artifact

Use this when the uncovered line is an artifact of source mapping, macro layout,
template instantiation, or an inline function compiled into a binary that was
not part of the report object list.

Action:

- verify with `llvm-cov show -show-line-counts-or-regions`
- adjust object-list collection or filters before treating it as a code issue

## Review Loop

For each coverage batch:

1. Generate full merged report.
2. Generate per-PA JSON for attribution.
3. Sort uncovered regions by file and owning subsystem.
4. Classify each region with the rubric.
5. For test gaps, add the smallest PA test that demonstrates the required
   behavior.
6. For dead code, remove only after confirming no assignment contract or
   platform/config mode owns it.
7. For behavior bugs, fix implementation and add a regression.
8. Re-run the relevant PA target and update the coverage notes.

The useful output is not a single percentage. The useful output is a reviewed
list of uncovered regions with one of: `test-needed`, `remove`, `config-gap`,
`bug`, `defensive`, or `artifact`.
