# Boost B2 Frontier V2 Plan

## Purpose

Restart the Boost 1.91.0 compatibility frontier from the first tracked suite
after the semantic text-reparse removal and the subsequent compiler fixes.
Every Boost suite is unverified for V2 until it is rerun from the V2 baseline.
V1 results are useful diagnostic history, but they do not count as V2 passes.

V2 keeps the original sequential frontier strategy:

1. Run one suite at a time in the established inventory order.
2. Stop at the first real compiler failure.
3. Reduce the failure and place the regression in the earliest owning PA.
4. Fix the typed compiler path, validate it, and commit it before continuing.
5. Rerun the focused Boost target and the entire current suite.

V2 adds a fixed cumulative performance budget. A rolling performance baseline
may explain one change, but it must not replace the fixed V2 baseline or hide
aggregate regression.

## Canonical State

- compiler baseline commit: `db9879223b4249a9e5516de0205e76fa158d5549`
- compiler baseline branch: `main`
- active frontier branch: `boost-frontier-v2`
- Boost tree: `/Users/vishvananda/boost_1_91_0`
- ordered suite inventory: `docs/boost-b2-suite-status-20260511.md`
- V2 tracker: `docs/boost-b2-frontier-v2-tracker.md`
- V1 historical tracker: `docs/boost-b2-frontier-tracker-20260519.md`
- first suite: `libs/accumulators/test`
- suite count: 147, with `libs/parameter_python/test` excluded by the existing
  inventory rule

Do not copy V1 suite statuses into the V2 tracker. The old tracker remains a
searchable source for prior reducers, diagnostics, performance investigations,
and platform-specific details.

## Baseline Bootstrap

Complete these gates before crediting suite 1:

1. Confirm the frontier branch starts at the compiler baseline commit and has
   no unrelated tracked changes.
2. Record the host compiler, standard library, OS, CPU, Boost version/tree,
   B2 wrapper path, and job count in the tracker.
3. Build `dev/cppgm++` without warnings.
4. Confirm the full direct-LowIR report and strict report pass.
5. Confirm the strict text-reparse audit is zero.
6. Record a three-run performance baseline at the exact V2 start commit. This
   was completed for the historical live-header epoch; do not rerun it.
7. Confirm the PR-triggered inception comparison for the baseline commit
   succeeds.

```sh
make -C dev -j8 cppgm++

CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 ORDERED=false \
  make test-report

CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 \
  make test-strict

python3 scripts/audit_text_reparse.py --strict --list-sites
python3 -m unittest scripts.tests.test_audit_text_reparse

scripts/validate_perf_regression.py check \
  --baseline /tmp/cppgm-boost-frontier-v2-frozen-header-epoch-9764b3835.json \
  --runs 3 \
  --report /tmp/cppgm-boost-frontier-v2-candidate.json
```

The original `db9879223` measurement is historical evidence for the former
live-project-header epoch. The current immutable baseline was recorded once
from exact compiler commit `9764b3835` against the checked-in 51-header closure
under `benchmarks/self_compile/stable/include`. If either baseline file is lost,
do not remeasure a parent or bless the current frontier head; restore the
recorded baseline or treat the performance gate as blocked.

## Suite Intake

Run suites in the numeric order already parsed by
`scripts/run_boost_b2_suite_survey.py`. Use a forced `-a` rebuild so a V1 B2
cache cannot turn a stale target into a V2 pass.

Every survey or direct B2 command must set `CPPGM_B2_CXX`,
`CPPGM_B2_HOST_CC`, and `CPPGM_B2_HOST_CXX` in that command's environment.
Do not rely on the wrapper's default compiler path: it points at the canonical
frontier worktree and that binary may be rebuilt by another active run. For an
independent worktree, use its absolute `dev/cppgm++` path and record the pinned
compiler commit in the tracker.

Start V2 with:

```sh
env CPPGM_BOOST_B2_FRONTIER=1 \
  CPPGM_B2_CXX=/Users/vishvananda/cppgm-extended/dev/cppgm++ \
  CPPGM_B2_HOST_CC=/usr/local/opt/llvm/bin/clang \
  CPPGM_B2_HOST_CXX=/usr/local/opt/llvm/bin/clang++ \
  python3 scripts/run_boost_b2_suite_survey.py \
  --suite 1 \
  --jobs 8 \
  --timeout 1800 \
  --output-dir /tmp/boost-frontier-v2-suite-001-db9879223
```

For later suites, replace `--suite 1` with the inventory number. The survey
runner owns timeout cleanup and kills the B2/compiler process group when a
suite times out.

A suite is a V2 pass only when:

- a forced full run completes;
- positive compile, link, and run targets succeed;
- expected-fail targets fail for the expected reason;
- no compiler crash, hang, timeout, or silent target skip remains; and
- the tracker records the command, compiler commit, duration, result summary,
  and log directory.

A timeout without a failure marker is not a pass. Increase the bounded timeout
or document a deliberate split that proves the full suite. Configuration,
missing dependency, platform policy, and B2 alternative-selection failures
must be classified separately from compiler failures.

## Failure Frontier

When a suite does not pass:

1. Read the full B2 summary and identify the first causal compiler failure;
   parallel log order does not establish causality.
2. Rerun the smallest failing Boost target, normally with `JOBS=1` or `JOBS=4`,
   and retain its exact log.
3. Reduce the failure outside the assignment tree first.
4. Prefer a non-STL reducer whenever the language or ABI rule can be expressed
   without hosted headers. This places the regression earlier and avoids
   mistaking library size for compiler stress.
5. Verify the reducer fails on the current frontier head before editing the
   compiler. Record the pre-fix diagnostic and command.
6. Determine whether the failure is parse, semantic, LowIR, object, link, or
   runtime behavior before choosing an owner PA.

If a small source has disproportionate compile cost, treat it as a possible
algorithm defect. Look for repeated normal-path witness work, quadratic walks,
unbounded retries, and completion cycles before adding a cache or increasing a
timeout.

## Implementation Rules

- Production fixes belong in `dev/`, normally `dev/src/`.
- Use structured AST, type, template argument, declaration, scope, LowIR, and
  ABI metadata. Do not tokenize, split, parse, or pattern-match saved semantic
  source text.
- Parsing belongs to the initial parse. A semantic failure is not permission to
  reparse a type, expression, template-id, owner/member name, or symbol spelling.
- Keep `scripts/audit_text_reparse.py --strict --list-sites` at zero.
- Symbol construction and symbol tracing must use the typed `abimangle` path.
  If typed ABI data is missing, update the owning assignment and scaffold
  instead of deriving it from rendered symbol text.
- Do not add Boost-, libc++-, or libstdc++-spelling special cases when the real
  rule is a language, ABI, or hosted-compatibility rule.
- A cache is an optimization, not a correctness repair. Prove the uncached
  algorithm is correct, terminating, and not pathologically repetitive first.
  Add cache-disabled or state-transition coverage when a cache could otherwise
  mask an invalid algorithm.
- Preserve existing cache invalidation and class/template completion rules.
  Do not widen a cache key to rendered template or type text.
- Keep edits scoped. Separate local gate repairs from the active Boost frontier
  when they are independently reviewable.

## Regression Placement

Place each reducer in the earliest PA and first correct cluster that owns its
essential behavior. Later hosted PAs are appropriate only when the test
essentially depends on hosted headers, host ABI, or host linking.

For every touched PA:

```sh
python3 scripts/audit_pa_feature_placement.py \
  --pa paNN \
  --no-course \
  --fail-on-early
```

Use the current sidecar conventions. Do not generate witness references from
`cppgm++`; use the established reference/witness path. Decide whether LowIR
drift is a compiler bug or a justified reference change before editing refs.

## Validation Ladder

For each compiler fix, run these gates in order:

1. Rebuild the affected frontend and `dev/cppgm++` with warnings visible.
2. Run the reduced repro and the checked-in regression.
3. Run the owner PA with direct LowIR comparison where applicable.
4. Run the strict direct-LowIR suites when templates, witness output, overload
   resolution, type semantics, or shared output behavior changed.
5. Run the placement audit for every touched PA.
6. Run the strict text-reparse audit; run its unit tests when the audit changes.
7. Run the performance gate for every production compiler change.
8. Rerun the focused Boost target with `-a`.
9. Rerun the full current Boost suite with `-a`.
10. Run `git diff --check` and inspect the final diff.

Before marking a suite closed after one or more compiler fixes, run the full
direct-LowIR report. Test-only tracker updates do not require a performance
run, but they still require placement and focused test validation.

## Performance Policy

### Fixed cumulative gate

The immutable baseline is:

```text
/tmp/cppgm-boost-frontier-v2-frozen-header-epoch-9764b3835.json
```

Every production compiler commit is checked against that file:

```sh
scripts/validate_perf_regression.py check \
  --baseline /tmp/cppgm-boost-frontier-v2-frozen-header-epoch-9764b3835.json \
  --runs 3 \
  --report /tmp/cppgm-boost-frontier-v2-candidate.json
```

The workload is the frozen `semantic_overload.cpp` plus the exact 51-header
closure under `benchmarks/self_compile/stable/include`. The checked-in epoch
manifest pins the source digest, header membership, every header digest, and
closure hash
`7c8a5445f33f04b314de98e6a099de4d75124b4bb032fc97ee5055e56d4827c8`.
The gate verifies that manifest before invoking the compiler. Never substitute
live `dev/src` headers, and never rerun the parent compiler for a candidate
comparison.

Hardware instruction count is the primary gate. Maximum RSS and peak footprint
are co-primary memory guards. Wall time and elapsed cycles are diagnostic only.
Use the script defaults unless the user explicitly approves another budget:

- instructions: at most `+1.00%` versus the fixed baseline;
- maximum RSS: at most `+3.00%`;
- peak footprint: at most `+3.00%`.

Do not refresh the fixed baseline after an accepted fix. A rolling delta may be
calculated only from already recorded immutable medians; do not remeasure its
parent. Environment or workload changes require an explicitly approved new
epoch, not blessing the current head as a new baseline.

### Early warning and optimization

- Any instruction increase above `+0.25%` gets a tracker note explaining the
  affected path.
- Above `+0.50%`, collect semantic hotspot counters and an OS sample before
  accepting the fix, even if the hard gate still passes.
- A hard-gate failure blocks the frontier. Tighten the algorithm, remove normal
  path work, split unrelated changes, or revert before running the next suite.
- Near-threshold results should be repeated without code changes. Use medians;
  do not select the best run.
- Do not run hotspot instrumentation during the hardware-counter gate.

Useful hotspot instrumentation:

```sh
CPPGM_SEMANTIC_STATS=1 \
CPPGM_SEMANTIC_PHASE_STATS=1 \
CPPGM_SEMANTIC_HOTSPOT=1 \
CPPGM_SEMANTIC_HOTSPOT_DUMP_QUERY='*' \
CPPGM_SEMANTIC_HOTSPOT_DUMP_FRAGMENT='*' \
CPPGM_SEMANTIC_HOTSPOT_DUMP_LIMIT=32 \
  ./dev/cppgm++ <normal compile arguments> 2>/tmp/cppgm-v2-hotspot.log
```

On macOS, sample a long-running compiler with:

```sh
sample <cppgm-pid> 10 1 -file /tmp/cppgm-v2.sample.txt
```

On Linux, use `perf record -g -- <compile command>` and inspect `perf report`.
Correlate samples with hotspot counts before choosing an optimization. A high
cache hit rate does not prove the underlying algorithm is valid.

## Tracker And Commit Discipline

- `docs/boost-b2-frontier-v2-tracker.md` is the only live V2 cursor.
- Record the suite result and active frontier immediately after each forced run.
- Record pre-fix failure evidence before implementation.
- Record fixed-baseline performance numbers before accepting a compiler fix.
- Commit each coherent compiler fix with its reducer and tracker row.
- Do not combine unrelated Boost failures just to reduce commit count.
- Do not edit the V1 tracker except to correct a historical factual error.
- Never commit B2 build output, `.my` files, profiler data, `/tmp` reports, or
  generated `obj/` contents.

## Completion Criteria

V2 is complete when all 147 inventory suites have fresh V2 evidence, every
real compiler failure has an owning regression, the direct and strict reports
pass, the text-reparse audit remains zero, inception passes at the final head,
and the final compiler stays within the fixed V2 instruction and memory budget.
