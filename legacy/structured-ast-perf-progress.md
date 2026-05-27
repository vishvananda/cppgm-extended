# Structured-AST Performance Progress

## Active Ownership

This worktree owns performance slices that avoid the witness-emission collapse
surface:

- `dev/src/cppast_parser.*`
- `dev/src/cppast_ast.h`
- `dev/src/template_scope.cpp`
- `dev/src/semantic_output.cpp`
- benchmark/progress tooling under `scripts/` and `docs/`

The post-`89976e2f` performance WIP also has small, measured touches in:

- `dev/src/template_resolution.cpp`
- `dev/src/callsemantic.cpp`

Avoid these files while the witness-collapse worker is active unless there is
an explicit handoff:

- `dev/src/witness_api.*`
- `dev/src/callsemantic.cpp`
- `dev/src/template_argument_semantics.*`
- `dev/src/template_specialization.cpp`
- `dev/src/template_instantiation.cpp`
- `dev/src/semantic_overload.cpp`
- `dev/src/semantic_conversion.cpp`
- `dev/src/template_witness_renderer.cpp`

## Status Legend

- `Not started`: no local implementation work
- `Scoped`: file ownership and benchmark target identified
- `In progress`: implementation underway
- `Builds`: `make -C dev cppgm++` passes
- `Correctness gated`: targeted correctness gates pass
- `Measured`: benchmark report recorded
- `Ready to merge`: correctness and performance results are acceptable
- `Landed`: merged back to integration branch

## Correctness Gates

Use targeted gates for fast iteration:

```sh
make -C dev cppgm++
make test-report ACTIVE_TEST_REPORT_PAS='pa10 pa11 pa12'
make test-report ACTIVE_TEST_REPORT_PAS='pa14 pa15 pa16'
make test-report ACTIVE_TEST_REPORT_PAS='pa18 pa19 pa21 pa22'
```

Use broader gates before merging larger slices:

```sh
make test-report
```

## Performance Benchmarks

Use the structured-AST benchmark runner for fixed single-process timings:

```sh
python3 scripts/run_structured_ast_perf_benchmarks.py \
  --repeat 3 \
  --output-prefix /tmp/cppgm-structured-ast-perf-baseline
```

Compare an after run to a baseline:

```sh
python3 scripts/run_structured_ast_perf_benchmarks.py \
  --repeat 3 \
  --baseline /tmp/cppgm-structured-ast-perf-baseline.json \
  --output-prefix /tmp/cppgm-structured-ast-perf-after
```

Run a focused subset while iterating:

```sh
python3 scripts/run_structured_ast_perf_benchmarks.py \
  --repeat 3 \
  --benchmark pa34-long-unordered-map-find \
  --benchmark pa34-long-recursive-std-function-string \
  --output-prefix /tmp/cppgm-structured-ast-perf-focused
```

Add the focused long-running PA34 compile benchmarks for perf evaluation, not
as a whole-assignment correctness gate:

```sh
python3 scripts/run_structured_ast_perf_benchmarks.py \
  --repeat 3 \
  --include-pa34-perf \
  --output-prefix /tmp/cppgm-structured-ast-perf-pa34
```

Frozen self-compile benchmarks are optional until the current branch is green
on that surface:

```sh
python3 scripts/run_structured_ast_perf_benchmarks.py \
  --repeat 3 \
  --include-self-compile \
  --output-prefix /tmp/cppgm-structured-ast-perf-self
```

Run counters separately from timing runs:

```sh
python3 scripts/run_structured_ast_perf_benchmarks.py \
  --repeat 1 \
  --counters \
  --output-prefix /tmp/cppgm-structured-ast-perf-counters
```

Current benchmark manifest:

Default current-pass benchmarks:

- `pa10-template-function-body-ast`:
  `cppgm++ --emit-ast` on `pa10/tests/spec/205-template-function-body.t`
- `pa10-template-class-body-ast`:
  `cppgm++ --emit-ast` on `pa10/tests/spec/206-template-class-body.t`
- `pa10-forward-unknown-nested-template-ast`:
  `cppgm++ --emit-ast` on
  `pa10/tests/spec/243-forward-unknown-nested-template-in-ctor-body.t`
- `pa10-member-template-less-greater-ast`:
  `cppgm++ --emit-ast` on
  `pa10/tests/spec/244-member-template-if-less-template-call.t`
- `pa10-template-function-pointer-ast`:
  `cppgm++ --emit-ast` on
  `pa10/tests/spec/246-template-id-function-pointer-argument.t`
- `pa10-dependent-template-keyword-ast`:
  `cppgm++ --emit-ast` on
  `pa10/tests/spec/270-dependent-template-keyword-nested-angle.t`
- `pa10-nested-qualified-template-id-ast`:
  `cppgm++ --emit-ast` on
  `pa10/tests/spec/271-nested-qualified-template-id-template-args.t`
- `pa18-template-shift-stress-lowir`:
  `cppgm++ --emit-lowir` on
  `pa18/tests/general/192-template-operator-shift-stress-chain.t`
- `pa18-pack-forward-lowir`:
  `cppgm++ --emit-lowir` on
  `pa18/tests/general/203-function-template-pack-forward-call.t`
- `pa18-repeated-template-call-lowir`:
  `cppgm++ --emit-lowir` on
  `pa18/tests/general/222-repeated-implicit-function-template-call.t`
- `pa21-inline-class-template-member-lowir`:
  `cppgm++ --emit-lowir` on
  `pa21/tests/general/420-inline-class-template-member-required-output.t`
- `pa21-local-variable-template-lowir`:
  `cppgm++ --emit-lowir` on
  `pa21/tests/general/438-local-variable-template-keeps-concrete-class-instantiation.t`
- `pa21-partial-specialization-alias-lowir`:
  `cppgm++ --emit-lowir` on
  `pa21/tests/general/437-partial-specialization-alias-pattern.t`

Focused PA34 perf benchmarks selected with `--include-pa34-perf`:

- `pa34-long-unordered-map-find`:
  `cppgm++ -c` on `pa34/tests/compile/655-const-unordered-map-find.t`
- `pa34-long-istream-static-member-mask`:
  `cppgm++ -c` on `pa34/tests/compile/658-istream-static-member-mask-access.t`
- `pa34-long-ostringstream-unsigned-int`:
  `cppgm++ -c` on `pa34/tests/compile/662-hosted-ostringstream-unsigned-int.t`
- `pa34-long-vector-bool-storage`:
  `cppgm++ -c` on
  `pa34/tests/compile/679-hosted-vector-bool-storage-allocator-static-cast.t`
- `pa34-long-recursive-std-function-string`:
  `cppgm++ -c` on
  `pa34/tests/compile/680-hosted-recursive-std-function-string-substr.t`

Optional self-compile benchmarks selected with `--include-self-compile`:

- frozen self-compile cases under `benchmarks/self_compile/stable/`

Do not run the whole PA34 harness as the routine perf-slice correctness gate;
use the focused PA34 compile cases for longer timing signal instead. These
benchmarks are marked as known-error/measurable until the hosted-header
semantic surface is green; they should be used for relative wall/RSS movement,
not as correctness evidence.

## Decision Rule

Call a slice a performance win only when:

- the target benchmark median improves outside run-to-run noise
- the control set does not materially regress
- correctness gates pass
- any counter changes line up with the intended mechanism

Treat median wall-clock changes under roughly `3-5%` as inconclusive unless
semantic counters clearly prove that the targeted work disappeared.

## Patch Tracker

### Current Slice - Parser/AST Locality And Resolve Cache Probes

Status: Correctness gated and measured

Base integration commit: `89976e2f0eee6d15360a4d8a2129f0fb045fb085`

Touched files:

- `dev/src/cppast_ast.h`
- `dev/src/cppast_parser.cpp`
- `dev/src/cppast_parser.h`
- `dev/src/template_resolution.cpp`
- `dev/src/callsemantic.cpp`

Implemented:

- `CppAstLazyVector<T>` for rare always-present `CppAstNode` vectors:
  qualifier template-id syntax, qualifier type syntax, exception type-id
  syntax, ABI tags, alignment text, and alignment syntax nodes
- parser simple-declaration helper ownership cleanup so the remaining direct
  `children.push_back(local)` parser sites are moved on successful paths
- conservative deduction-guide start prefilter before attempting the full
  deduction-guide parser
- parser-level declaration-start probe cache for non-type template default
  argument boundary checks
- lazy template-argument dependency scans and a no-ellipsis fast path around
  bound pack expansion
- resolve-template-arguments cache enabled while template argument source
  locations are active
- memory-census support for `CppAstLazyVector<T>`

Correctness:

- [x] `make -C dev cppgm++`
- [x] `make test-report ACTIVE_TEST_REPORT_PAS='pa10 pa11 pa12'`
  (passed; `269 / 269`)
- [x] `make test-report ACTIVE_TEST_REPORT_PAS='pa14 pa15 pa16'`
  (passed; `266 / 266`)
- [x] `make test-report ACTIVE_TEST_REPORT_PAS='pa18 pa19 pa21 pa22'`
  (passed; `521 / 521`)

Performance:

- Clean baseline worktree:
  `/tmp/cppgm-baseline-89976e2f-20260428`
- Default baseline JSON:
  `/tmp/cppgm-structured-ast-perf-baseline-89976e2f-r3.json`
- Default current JSON:
  `/tmp/cppgm-structured-ast-perf-current-89976e2f-wip-r3b.json`
- Selected PA34 baseline JSON:
  `/tmp/cppgm-structured-ast-perf-pa34-selected-baseline-89976e2f-r3.json`
- Selected PA34 current JSON:
  `/tmp/cppgm-structured-ast-perf-pa34-selected-current-89976e2f-wip-r3.json`

Result:

- Default small controls are wall-clock neutral, with medians between roughly
  `-1.13%` and `+1.76%`.
- Selected PA34 known-error timing probes show a consistent RSS reduction:
  `pa34-long-unordered-map-find` `-21.23%`,
  `pa34-long-ostringstream-unsigned-int` `-17.66%`, and
  `pa34-long-recursive-std-function-string` `-17.80%`.
- Selected PA34 wall time is still neutral/inconclusive:
  `+0.10%`, `-2.11%`, and `+2.16%` respectively.

Decision:

- This is a real memory/locality improvement, not a demonstrated wall-clock
  win. Keep the measurement separate from future work that aims to reduce
  elapsed time.

### Patch 4 - Parser Speculation Churn

Status: In progress

Branch: `codex/structured-ast-perf-20260428`

Touched files:

- `dev/src/cppast_parser.cpp`
- `dev/src/cppast_parser.h`
- `scripts/run_structured_ast_perf_benchmarks.py`
- `legacy/structured-ast-perf-progress.md`
- `legacy/structured-ast-perf-worktree-split.md`

Imported WIP:

- `RecogTokenRangeSequence` for bounded nested-parser ranges in
  non-type template defaults and fold operands
- `suppress_template_argument_fragment_syntax` inheritance and the
  discarded declaration probe guard
- cheap declaration-start prefilter plus per-call probe cache in
  `parse_non_type_template_default_argument`, so obvious punctuation after
  a top-level `>` no longer constructs a nested parser just to reject a
  declaration boundary
- template-argument fragment type-parser guard for obvious expression-only
  fragments such as literals and unary expressions, plus move construction
  for captured fragment syntax nodes

Additional current-branch phase:

- `parse_template_argument_fragment_syntax` now uses `RecogTokenRangeSequence`
  instead of copying a `vector<RecogToken>` and appending an EOF token for
  every fragment candidate
- `parse_template_argument_fragment_node` moves the selected parsed
  type/expression node out of the local syntax wrapper instead of copying it

Deferred from the broader WIP because it changed current PA10 refs:

- wider range-view replacement outside the fragment parser

Correctness:

- [x] `make -C dev cppgm++`
- [ ] `make test-report ACTIVE_TEST_REPORT_PAS='pa10 pa11 pa12'`
  (latest run: `265 / 269`; failures are the same four known baseline PA10
  ref mismatches)
- [x] `make test-report ACTIVE_TEST_REPORT_PAS='pa18 pa19 pa21 pa22'`
  (passed; harness summary reported `0 / 0` aggregated counts)
- [x] after declaration-probe prefilter, forced parser object rebuild plus
  `make -C dev cppgm++`
- [ ] after declaration-probe prefilter,
  `make test-report ACTIVE_TEST_REPORT_PAS='pa10 pa11 pa12'`
  (latest run: `265 / 269`; same four known baseline PA10 ref mismatches)
- [x] after declaration-probe prefilter,
  `make test-report ACTIVE_TEST_REPORT_PAS='pa18 pa19 pa21 pa22'`
  (passed; harness summary reported `0 / 0` aggregated counts)
- [x] after declaration-probe prefilter,
  `make test-report ACTIVE_TEST_REPORT_PAS='pa14 pa15 pa16'`
  (passed; `266 / 266`)
- [x] after fragment type-parser guard, forced parser object rebuild plus
  `make -C dev cppgm++`
- [ ] after fragment type-parser guard,
  `make test-report ACTIVE_TEST_REPORT_PAS='pa10 pa11 pa12'`
  (latest run: `265 / 269`; same four known baseline PA10 ref mismatches)
- [x] after fragment type-parser guard,
  `make test-report ACTIVE_TEST_REPORT_PAS='pa18 pa19 pa21 pa22'`
  (passed; harness summary reported `0 / 0` aggregated counts)
- [x] after fragment type-parser guard,
  `make test-report ACTIVE_TEST_REPORT_PAS='pa14 pa15 pa16'`
  (passed; `266 / 266`)
- [x] after fragment range-view phase, `make -C dev cppgm++`
- [x] after fragment range-view phase,
  `make test-report ACTIVE_TEST_REPORT_PAS='pa10 pa11 pa12'`
  (passed; `269 / 269`)
- [x] after fragment range-view phase,
  `make test-report ACTIVE_TEST_REPORT_PAS='pa14 pa15 pa16'`
  (passed; `266 / 266`)
- [x] after fragment range-view phase,
  `make test-report ACTIVE_TEST_REPORT_PAS='pa18 pa19 pa21 pa22'`
  (passed; `521 / 521`)

Performance:

- Baseline JSON:
  `/tmp/cppgm-structured-ast-perf-integration-compiler-baseline.json`
- After JSON:
  `/tmp/cppgm-structured-ast-perf-current-after-move-scope2.json`
- Result: default benchmark set is too small/noisy for a defensible win claim
  so far; the clean rerun is flat within roughly +/-1% wall time.
- PA34 timing probe:
  `/tmp/cppgm-structured-ast-perf-pa34-current-r3.json` compared to
  `/tmp/cppgm-structured-ast-perf-pa34-integration-r3.json` shows
  `pa34-long-unordered-map-find` known-error median wall time at `13.934s`
  vs `25.588s` baseline. Treat this as an early directional signal only
  because the known-error path and host load are noisy.
- Tooling smoke: `/tmp/cppgm-structured-ast-perf-default-smoke2.json`
  completed `13/13` default benchmarks with `--repeat 1`.
- Declaration-probe prefilter smoke:
  `/tmp/cppgm-structured-ast-perf-after-decl-probe-prefilter-smoke.json`
  completed `13/13` default benchmarks with `--repeat 1`.
- Fragment type-parser guard:
  `/tmp/cppgm-structured-ast-perf-after-fragment-type-guard-r3.json`
  completed `13/13` default benchmarks with `--repeat 3`. PA10 AST cases
  remain roughly flat versus `/tmp/cppgm-structured-ast-perf-integration-compiler-baseline.json`;
  PA18 `template-operator-shift-stress-chain` and `function-template-pack-forward-call`
  show `-25.35%` and `-24.88%` median wall-clock respectively on this small
  run. Treat that as a promising targeted signal, not a broad performance
  conclusion.
- Fragment range-view phase:
  `/tmp/cppgm-structured-ast-perf-fragment-range-view-r3.json` completed
  `13/13` default benchmarks with `--repeat 3`, compared against
  `/tmp/cppgm-structured-ast-perf-current-89976e2f-wip-r3b.json`.
  Default medians are still neutral/noisy. The focused PA34 `655` probe
  `/tmp/cppgm-structured-ast-perf-pa34-655-fragment-range-view-r3.json`
  measured `-1.52%` wall and `+0.53%` RSS versus the previous slice, which is
  not enough to claim a material win but is not a regression.

Notes:

- The initial seed intentionally avoids `callsemantic.cpp` and template
  argument semantic plumbing to keep clear of witness-collapse work.

### Patch 5 - Declaration Parser Dispatch Guards

Status: Correctness gated and measured

Touched files:

- `dev/src/cppast_parser.cpp`

Implemented:

- cheap first-token guards before attempting namespace declarations, explicit
  instantiations, and linkage specifications from the general declaration
  parser
- explicit-instantiation guard avoids speculating on ordinary
  `template <...>` declarations

Correctness:

- [x] `make -C dev cppgm++`
- [x] `make test-report ACTIVE_TEST_REPORT_PAS='pa10 pa11 pa12'`
  (passed; `269 / 269`)
- [x] `make test-report ACTIVE_TEST_REPORT_PAS='pa14 pa15 pa16'`
  (passed; `266 / 266`)
- [x] `make test-report ACTIVE_TEST_REPORT_PAS='pa18 pa19 pa21 pa22'`
  (passed; `521 / 521`)

Performance:

- Default benchmark JSON:
  `/tmp/cppgm-structured-ast-perf-decl-dispatch-guards-r3.json`
- Focused PA34 JSON:
  `/tmp/cppgm-structured-ast-perf-pa34-655-decl-dispatch-guards-r3.json`
- Result: default benchmarks are mixed/noisy with small wall-clock movement;
  the focused long PA34 `655` probe measured `-2.85%` median wall time and
  `+2.25%` RSS versus the previous fragment range-view slice. Keep this as a
  long-path parser speculation win, not a broad benchmark-set win.

### Patch 4 Follow-up - Template Special-Member Prefilter

Status: Correctness gated and measured

Touched files:

- `dev/src/cppast_parser.cpp`
- `dev/src/cppast_parser.h`

Implemented:

- `parse_class_member` now cheaply scans past a leading `template <...>` clause
  before attempting `parse_template_special_member_declaration`
- ordinary member templates whose post-clause token cannot start a
  constructor/destructor/conversion candidate skip the expensive failed
  special-member parse
- if the cheap scanner cannot confidently find the template-parameter-clause
  end, the parser falls back to the old full parse path

Correctness:

- [x] `make -C dev cppgm++`
- [x] `make test-report ACTIVE_TEST_REPORT_PAS='pa10 pa11 pa12 pa14 pa15 pa16 pa18 pa21 pa22'`
  (passed; `973 / 973`)

Performance:

- Fresh clean-head PA34 `662` baseline:
  `/tmp/cppgm-structured-ast-perf-clean-head-44565939-pa34-662-r3b.json`
- Focused PA34 `662` current JSON:
  `/tmp/cppgm-structured-ast-perf-template-special-member-prefilter-safe-pa34-662-r3.json`
- Default controls:
  `/tmp/cppgm-structured-ast-perf-template-special-member-prefilter-default-r3.json`
- Result: focused PA34 `662` known-error timing probe improved `-5.41%`
  median wall time (`14.118s` vs `14.925s`) with flat RSS (`+0.02%`).
  Default controls completed `13 / 13` with no failures.

### Patch 8 Follow-up - Borrow Template-Id Source Arguments

Status: Correctness gated and measured

Touched files:

- `dev/src/template_api.h`
- `dev/src/template_api.cpp`
- `dev/src/template_argument_semantics.cpp`
- `dev/src/template_resolution.cpp`
- `dev/src/callsemantic.cpp`
- `dev/src/semantic_consteval.cpp`

Implemented:

- `ScopedTemplateIdSourceArguments` now accepts a movable argument-text vector
  and moves it into the active frame
- hot readers can borrow the current source-argument vector via
  `current_template_id_source_arguments_ptr` instead of copying it out of the
  stack/cache when they only need to inspect it
- source-argument guard call sites move local vectors once ownership has
  transferred into the scoped guard

Correctness:

- [x] `make -C dev cppgm++`
- [x] `make test-report ACTIVE_TEST_REPORT_PAS='pa10 pa11 pa12'`
  (passed; `269 / 269`)
- [x] `make test-report ACTIVE_TEST_REPORT_PAS='pa14 pa15 pa16'`
  (passed; `266 / 266`)
- [x] `make test-report ACTIVE_TEST_REPORT_PAS='pa18 pa19 pa21 pa22'`
  (passed; `521 / 521`)

Performance:

- Default benchmark JSON:
  `/tmp/cppgm-structured-ast-perf-source-arg-borrow-r3.json`
- Focused PA34 JSON:
  `/tmp/cppgm-structured-ast-perf-pa34-655-source-arg-borrow-r3.json`
- Result: default controls remain within noise. The focused long PA34 `655`
  probe measured `-1.16%` median wall time and `+0.36%` RSS versus the
  previous declaration-dispatch slice. Treat this as reduced copy churn, not a
  broad compile-time win.

### Patch 7 Probe - Reuse Class Output Readiness Locally

Status: Correctness gated and measured

Touched files:

- `dev/src/semantic_output.cpp`

Implemented:

- class output computes instantiated-class output readiness lazily once per
  class-output pass and reuses it for vtable, member-function, conversion
  operator, special-member, and defaulted-method emission decisions
- removed the now-unused `should_emit_instantiated_class_method_definition`
  wrapper that recomputed readiness internally

Correctness:

- [x] `make -C dev cppgm++`
- [x] `make test-report ACTIVE_TEST_REPORT_PAS='pa10 pa11 pa12'`
  (passed; `269 / 269`)
- [x] `make test-report ACTIVE_TEST_REPORT_PAS='pa14 pa15 pa16'`
  (passed; `266 / 266`)
- [x] `make test-report ACTIVE_TEST_REPORT_PAS='pa18 pa19 pa21 pa22'`
  (passed; `521 / 521`)

Performance:

- Default benchmark JSON:
  `/tmp/cppgm-structured-ast-perf-class-readiness-reuse-r3.json`
- Focused PA34 JSON:
  `/tmp/cppgm-structured-ast-perf-pa34-655-class-readiness-reuse-r3.json`
- PA21 hotspot JSON:
  `/tmp/cppgm-structured-ast-perf-pa21-inline-hotspot-class-readiness-reuse.json`
- Result: default controls moved `-0.92%` to `-3.27%` wall time versus the
  previous source-argument slice. The PA34 `655` known-error probe measured
  `+2.38%` wall and `-5.78%` RSS, which is inconclusive on that early-error
  path. The PA21 hotspot count for `instantiated_class_output_readiness` did
  not drop, so this should be treated as a small local reuse cleanup rather
  than the broader Patch 7 worklist/readiness-cache win.

### Patch 1 Probe - Normalize Template Argument Texts Once

Status: Correctness gated and measured

Touched files:

- `dev/src/template_resolution.cpp`

Implemented:

- `resolve_template_arguments` now trims each explicit/expanded argument text
  once while building `expanded_texts`
- the resolve-template-arguments cache key reuses that normalized text vector
  instead of trimming and copying each text a second time
- expanded pack arguments are normalized before both cache-key construction
  and per-argument resolution

Correctness:

- [x] `make -C dev cppgm++`
- [x] `make test-report ACTIVE_TEST_REPORT_PAS='pa10 pa11 pa12'`
  (passed; `269 / 269`)
- [x] `make test-report ACTIVE_TEST_REPORT_PAS='pa14 pa15 pa16'`
  (passed; `266 / 266`)
- [x] `make test-report ACTIVE_TEST_REPORT_PAS='pa18 pa19 pa21 pa22'`
  (passed; `521 / 521`)

Performance:

- Exact previous-commit baseline worktree:
  `/tmp/cppgm-baseline-2b6102d6-20260428`
- Focused PA34 `662` baseline JSON:
  `/tmp/cppgm-structured-ast-perf-pa34-662-baseline-2b6102d6-r3.json`
- Focused PA34 `662` current JSON:
  `/tmp/cppgm-structured-ast-perf-pa34-662-template-arg-normalize-r3.json`
- Default current JSON:
  `/tmp/cppgm-structured-ast-perf-template-arg-normalize-r3.json`
- Result: `pa34-long-ostringstream-unsigned-int` measured `-1.31%` median
  wall time and `+0.23%` RSS versus the exact previous commit. Default
  controls are flat/noisy within about `+/-1%` wall time. Keep this as a
  focused string-normalization cleanup on the hot resolve-template-arguments
  path, not a broad Patch 1 win.

### Patch 1 Probe - Trim Resolver Bookkeeping

Status: Correctness gated and measured

Touched files:

- `dev/src/template_resolution.cpp`

Implemented:

- `resolve_template_arguments` no longer maintains the expanded argument syntax
  side vector when the caller did not provide argument syntaxes
- per-argument source-location lookup is skipped when the template-argument
  source-location stack is inactive
- nested template-id source argument texts are only materialized when the
  syntax has a usable witness source location, matching the existing
  `ScopedTemplateIdSourceArguments` activation condition

Rejected during this phase:

- an inline-storage rewrite of `ResolveTemplateArgumentsCacheKey` was tested and
  then removed after the focused PA34 `662` probe regressed by `+22.51%`
  median wall time; do not revive that shape without stronger allocation and
  cache-locality evidence

Correctness:

- [x] `make -C dev cppgm++`
- [x] `make test-report ACTIVE_TEST_REPORT_PAS='pa10 pa11 pa12'`
  (passed; `269 / 269`)
- [x] `make test-report ACTIVE_TEST_REPORT_PAS='pa14 pa15 pa16'`
  (passed; `266 / 266`)
- [x] `make test-report ACTIVE_TEST_REPORT_PAS='pa18 pa19 pa21 pa22'`
  (passed; `521 / 521`)

Performance:

- Previous phase baseline JSON:
  `/tmp/cppgm-structured-ast-perf-template-arg-normalize-r3.json`
- Default current JSON:
  `/tmp/cppgm-structured-ast-perf-resolve-bookkeeping-r3b.json`
- Focused PA34 `662` baseline JSON:
  `/tmp/cppgm-structured-ast-perf-pa34-662-template-arg-normalize-r3.json`
- Focused PA34 `662` current JSON:
  `/tmp/cppgm-structured-ast-perf-pa34-662-resolve-bookkeeping-r3.json`
- Rejected inline-key experiment JSON:
  `/tmp/cppgm-structured-ast-perf-pa34-662-resolve-setup-r3.json`
- Result: `pa34-long-ostringstream-unsigned-int` measured `-3.87%` median
  wall time and `-0.26%` RSS versus the previous template-argument
  normalization phase. The default control rerun completed `13 / 13` but was
  uniformly about `+1%` to `+2%` wall time against the prior JSON; treat that as
  small/noisy control movement rather than a broad win. This slice is kept for
  the focused PA34 resolver-path signal and because the changed guards should
  only remove unused bookkeeping.

### Patch 1 Probe - Hash Resolver Cache Keys While Building

Status: Correctness gated and measured

Touched files:

- `dev/src/template_resolution.cpp`

Implemented:

- `make_resolve_template_arguments_cache_key` now computes the hash while it
  copies parameter and argument text components into the cache key
- the key keeps the existing vector-backed representation; this intentionally
  avoids the rejected inline-storage cache-key shape from the previous phase
- expanded argument texts are copied into the key with `reserve`/`push_back`
  while hashing, avoiding a separate vector assignment plus hash pass

Correctness:

- [x] `make -C dev cppgm++`
- [x] `make test-report ACTIVE_TEST_REPORT_PAS='pa10 pa11 pa12'`
  (passed; `269 / 269`)
- [x] `make test-report ACTIVE_TEST_REPORT_PAS='pa14 pa15 pa16'`
  (passed; `266 / 266`)
- [x] `make test-report ACTIVE_TEST_REPORT_PAS='pa18 pa19 pa21 pa22'`
  (passed; `521 / 521`)

Performance:

- Previous phase baseline JSON:
  `/tmp/cppgm-structured-ast-perf-resolve-bookkeeping-r3b.json`
- Default current JSON:
  `/tmp/cppgm-structured-ast-perf-resolve-key-build-r3.json`
- Focused PA34 `662` baseline JSON:
  `/tmp/cppgm-structured-ast-perf-pa34-662-resolve-bookkeeping-r3.json`
- Focused PA34 `662` current JSON:
  `/tmp/cppgm-structured-ast-perf-pa34-662-resolve-key-build-r3.json`
- Result: default controls are stable versus the previous phase, with most
  median wall deltas between about `-1%` and `+0.5%`. The focused PA34 `662`
  probe measured another `-1.72%` median wall time and `+0.08%` RSS versus the
  bookkeeping phase; one run was a `30.818s` outlier, but the other two runs
  were `15.996s` and `16.103s`.

### Patch 1 Probe - Reuse Class Instantiation Keys

Status: Correctness gated and measured

Touched files:

- `dev/src/callsemantic.cpp`

Implemented:

- `reference_class_template_instantiation_with_syntax` now passes the
  canonical argument key it already computed into the selected-instantiation
  reference path
- the public/virtual `reference_selected_class_template_instantiation`
  signature stays unchanged; the key reuse is an internal helper so other
  callers keep the existing contract
- `reference_selected_class_template_instantiation_with_key` falls back to
  computing the key itself when no precomputed key is available

Rejected during this phase:

- a one-entry "last resolve-template-arguments result" cache was tested and
  removed after the focused PA34 `662` probe regressed by `+12.00%` median wall
  time; direct input comparison before cache-key construction was more
  expensive than the locality it captured

Correctness:

- [x] `make -C dev cppgm++`
- [x] `make test-report ACTIVE_TEST_REPORT_PAS='pa10 pa11 pa12'`
  (passed; `269 / 269`)
- [x] `make test-report ACTIVE_TEST_REPORT_PAS='pa14 pa15 pa16'`
  (passed; `266 / 266`)
- [x] `make test-report ACTIVE_TEST_REPORT_PAS='pa18 pa19 pa21 pa22'`
  (passed; `521 / 521`)

Performance:

- Previous phase baseline JSON:
  `/tmp/cppgm-structured-ast-perf-resolve-key-build-r3.json`
- Default current JSON:
  `/tmp/cppgm-structured-ast-perf-reuse-class-inst-key-r3b.json`
- Focused PA34 `662` baseline JSON:
  `/tmp/cppgm-structured-ast-perf-pa34-662-resolve-key-build-r3.json`
- Focused PA34 `662` current JSON:
  `/tmp/cppgm-structured-ast-perf-pa34-662-reuse-class-inst-key-r3.json`
- Rejected last-cache experiment JSON:
  `/tmp/cppgm-structured-ast-perf-pa34-662-resolve-last-cache-r3.json`
- Result: `pa34-long-ostringstream-unsigned-int` measured `-13.88%` median
  wall time and `-0.06%` RSS versus the previous resolver-key phase. The
  default control repeat completed `13 / 13`; PA10 AST controls stayed roughly
  within `+/-2%`, while several subsecond PA18/PA21 lowir controls reported
  about `-24%` to `-26%` wall time. Treat those lowir control deltas as
  supportive but noisy because the absolute medians are below a second and
  similar scheduler effects have appeared in this worktree.

### Patch 3 - Shrink `CppAstNode` And Stop Copying It

Status: Measured

Touched files:

- `dev/src/cppast_ast.h`
- `dev/src/cppast_parser.cpp`

Implemented:

- targeted `std::move` push/assignment in expression, postfix, template,
  enum, and small declaration builders where the local node is dead after
  transfer
- a second targeted move pass across static assertions, special member
  parsing, qualified special members, bit-fields, function definitions,
  simple-declaration declarator lists, structured bindings, and initializer
  nodes
- a wider parser-only move pass across type-id/specifier/declarator
  construction, parameter/default-argument parsing, statement builders,
  lambda parsing, `new`/cast/type-trait expressions, fold expressions, and
  braced/designated initializer nodes
- rvalue AST syntax setter overloads in `cppast_ast.h`, plus parser call
  sites that move qualified-name, template-id, conversion type-id, exception
  type-id, and qualifier syntax payloads into nodes when the local syntax
  object is dead after transfer
- parser loop ownership cleanups that update name-lookup state before moving
  declarations, class members, template parameters, block items, asm clauses,
  and throw/call subnodes into their parent nodes
- inherited-name-lookup empty-local short-circuit in nested parser setup

Deferred:

- boxing rare `CppAstNode` aux fields behind an aux object
- remaining parser push sites that still need the local node for
  name-scope updates or other follow-up analysis

Correctness:

- [x] `make -C dev cppgm++`
- [ ] `make test-report ACTIVE_TEST_REPORT_PAS='pa10 pa11 pa12'`
  (latest run: `265 / 269`; failures are the same four known baseline PA10
  ref mismatches)
- [x] `make test-report ACTIVE_TEST_REPORT_PAS='pa18 pa19 pa21 pa22'`
  (passed; harness summary reported `0 / 0` aggregated counts)
- [x] after second move pass, `make -C dev cppgm++`
- [ ] after second move pass,
  `make test-report ACTIVE_TEST_REPORT_PAS='pa10 pa11 pa12'`
  (latest run: `265 / 269`; same four known baseline PA10 ref mismatches)
- [x] after second move pass,
  `make test-report ACTIVE_TEST_REPORT_PAS='pa18 pa19 pa21 pa22'`
  (passed; harness summary reported `0 / 0` aggregated counts)
- [x] after wider move pass, forced parser object rebuild plus
  `make -C dev cppgm++`
- [ ] after wider move pass,
  `make test-report ACTIVE_TEST_REPORT_PAS='pa10 pa11 pa12'`
  (latest run: `265 / 269`; same four known baseline PA10 ref mismatches)
- [x] after wider move pass,
  `make test-report ACTIVE_TEST_REPORT_PAS='pa18 pa19 pa21 pa22'`
  (passed; harness summary reported `0 / 0` aggregated counts)
- [x] after wider move pass,
  `make test-report ACTIVE_TEST_REPORT_PAS='pa14 pa15 pa16'`
  (passed; `266 / 266`)
- [x] after AST setter move overloads, forced shared AST-dependent rebuild plus
  `make -C dev cppgm++`
- [ ] after AST setter move overloads,
  `make test-report ACTIVE_TEST_REPORT_PAS='pa10 pa11 pa12'`
  (latest run: `265 / 269`; same four known baseline PA10 ref mismatches)
- [x] after AST setter move overloads,
  `make test-report ACTIVE_TEST_REPORT_PAS='pa18 pa19 pa21 pa22'`
  (passed; harness summary reported `0 / 0` aggregated counts)
- [x] after AST setter move overloads,
  `make test-report ACTIVE_TEST_REPORT_PAS='pa14 pa15 pa16'`
  (passed; `266 / 266`)
- [x] after parser loop ownership cleanup, forced parser object rebuild plus
  `make -C dev cppgm++`
- [ ] after parser loop ownership cleanup,
  `make test-report ACTIVE_TEST_REPORT_PAS='pa10 pa11 pa12'`
  (latest run: `265 / 269`; same four known baseline PA10 ref mismatches)
- [x] after parser loop ownership cleanup,
  `make test-report ACTIVE_TEST_REPORT_PAS='pa18 pa19 pa21 pa22'`
  (passed; harness summary reported `0 / 0` aggregated counts)
- [x] after parser loop ownership cleanup,
  `make test-report ACTIVE_TEST_REPORT_PAS='pa14 pa15 pa16'`
  (passed; `266 / 266`)

Performance:

- Baseline JSON:
  `/tmp/cppgm-structured-ast-perf-integration-compiler-baseline.json`
- After JSON:
  `/tmp/cppgm-structured-ast-perf-current-after-move-scope2.json`
- After second move pass:
  `/tmp/cppgm-structured-ast-perf-after-parser-move-r3.json`
- After wider move pass smoke:
  `/tmp/cppgm-structured-ast-perf-after-wide-parser-move-smoke.json`
- Focused PA34 known-error probe after wider move pass:
  `/tmp/cppgm-structured-ast-perf-pa34-after-wide-parser-move-smoke2.json`
- After AST setter move smoke:
  `/tmp/cppgm-structured-ast-perf-after-ast-setter-move-smoke.json`
- After AST setter move repeat-3:
  `/tmp/cppgm-structured-ast-perf-after-ast-setter-move-r3.json`
- After parser loop ownership cleanup smoke:
  `/tmp/cppgm-structured-ast-perf-after-parser-lookup-move-smoke.json`
- Result: no measurable movement on the default small benchmark set.
  The post-second-move and wider-move timing runs completed all default
  benchmarks, but concurrent work in the integration tree made the subsecond
  medians noisy; do not claim a win from those runs. The AST setter repeat-3
  run completed `13/13`; PA10 AST medians are within noise versus the
  integration baseline, while the larger PA18/PA21 lowir cases remain about
  `25%` faster than that baseline. Treat the lowir result as cumulative branch
  signal, not proof that the setter overload slice alone produced the gain.
  The parser loop cleanup smoke completed `13/13`, but repeat-1 timings are
  noise-only and should not be used for a win/regression claim. The single
  PA34 probe measured `pa34-long-unordered-map-find` at `27.067s`
  as a known-error timing probe, `+5.78%` versus the older baseline JSON and
  `-2.90%` RSS; this is repeat-1 and not enough to call a regression or win.

### Patch 6 - Scope Fingerprint Fast Path

Status: Builds

Touched files:

- `dev/src/template_scope.cpp`

Correctness:

- [x] `make -C dev cppgm++`
- [x] `make test-report ACTIVE_TEST_REPORT_PAS='pa18 pa19 pa21 pa22'`
  (passed; harness summary reported `0 / 0` aggregated counts)

Performance:

- Baseline JSON:
  `/tmp/cppgm-structured-ast-perf-integration-compiler-baseline.json`
- After JSON:
  `/tmp/cppgm-structured-ast-perf-current-after-move-scope2.json`
- Result: timing deltas are inconclusive on the default small benchmark set.

### Patch 7 - Class-Method Emit Worklist

Status: Deferred after failed ordering check

Touched files:

- none remaining; the attempted `dev/src/semantic_output.cpp` /
  `dev/src/semantic_output.h` change was reverted

Correctness:

- [x] `make -C dev cppgm++`
- [x] direct PA15 repro:
  `../dev/cppgm++ --emit-lowir -O0 -o tests/spec/273-global-class-array-init.my --witness tests/spec/273-global-class-array-init.my.witness tests/spec/273-global-class-array-init.t`
  followed by `diff -u ...ref ...my`
- [x] `make test-report ACTIVE_TEST_REPORT_PAS='pa14 pa15 pa16 pa18 pa21 pa22'`
  (passed; `266 / 266`)

Performance:

- Baseline JSON:
- After JSON:
- Result: no Patch 7 perf data. The index-based worklist changed LowIR
  function ordering for `pa15/tests/spec/273-global-class-array-init.t`
  by emitting `Entry::~Entry` before `Entry::Entry`, so this slice needs a
  more conservative design that preserves the existing synthesized-method
  fixpoint order.
