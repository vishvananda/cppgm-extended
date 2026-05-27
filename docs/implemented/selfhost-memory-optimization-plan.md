# Self-Host Memory Optimization Plan

## Goal

Reduce compiler memory usage in a disciplined way without:

- breaking correctness
- hiding real semantic work behind special cases
- regressing compile time badly
- destabilizing the active `pa38` ladder

This plan is specifically for the current compiler-memory problem discovered
while compiling large semantic translation units with a host-built `cppgm++`.

## Why This Needs A Separate Process

We currently have two overlapping problems:

1. a baseline "always expensive" memory profile on heavy semantic files
2. a newer pathological blowup where certain local-type/template rewrite paths
   allocate unboundedly and push the machine into swap

Those two problems should not be mixed together.

If we only look at full ladder runs, the numbers are too noisy:

- multiple self-host compiles run in parallel
- stale processes can contaminate memory readings
- swap pressure from one TU can distort another
- it becomes hard to tell whether a fix improved the compiler or just changed
  the scheduling

So this plan uses isolated one-file compiles first, then hands validated fixes
back to the self-host lane as a separate follow-up decision rather than making
the ladder itself part of this plan's completion gate.

## Current Status

The first known catastrophic path has already been fixed:

- bound-type identifier rewriting was expanding
  `allocator<T>::void_pointer` into `allocator<T><T>::void_pointer`
- that manufactured ever-larger template-id text
- the rewrite recursion guard never saw the same text twice
- `semantic_expression.cpp` could then run into multi-GB allocator growth and
  swap storms

That specific growth loop is now considered closed.

So the active work under this plan is:

- keep the pathological benchmark in the loop as a regression guard
- reduce the baseline heavy memory cost on files like `semantic_lookup.cpp`
- watch for any new catastrophic growth variant while doing that work

The first implementation batch under this tracked plan does two things:

- adds a repeatable benchmark helper in
  [scripts/selfhost_memory_bench.py](/private/tmp/cppgm-cleanup-passb-20260409/scripts/selfhost_memory_bench.py)
- lands a first narrow baseline-memory slice by caching repeated
  `rewrite_bound_type_names_in_text(...)` results per semantic scope

The current instrumentation batch adds:

- `CPPGM_MEMORY_CENSUS=1`, which dumps a late semantic retained-memory census
- opt-in cache toggles for the heaviest by-value semantic caches:
  - `CPPGM_DISABLE_TEMPLATE_ID_CACHE`
  - `CPPGM_DISABLE_BOUND_TYPE_REWRITE_CACHE`
  - `CPPGM_DISABLE_EXPRESSION_FRAGMENT_CACHE`
  - `CPPGM_DISABLE_TYPE_FRAGMENT_CACHE`

Current concrete finding on the frozen `semantic_lookup.cpp` control benchmark:

- baseline host-built `cppgm++`: `94.25s`, max RSS `1794952 KB`
- dominant retained bucket: cached type-fragment AST payload
  - `cache.type_fragment.node`: about `326 MB`
  - `cache.type_fragment`: about `86 MB`
- disabling only the type-fragment cache drops max RSS to `1565704 KB`
  (about `-12.8%`) but raises wall time to `101.96s` (about `+8.2%`)
- the type-fragment cache is still materially reused
  - hits `591081`
  - misses `333316`
  - hit rate about `63.9%`

So the next accepted optimization target is not "turn the cache off". It is:

- keep the reuse
- reduce the retained payload of cached type fragments
- likely by replacing stored full `CppAstNode` trees on the common
  `parse_type_fragment(...)` then `parse_type_id(...)` path with a smaller
  semantic result or another cheaper intermediate representation

That slice is now implemented:

- the common semantic `text -> TypePtr` path now uses a dedicated
  `parsed_type_text` cache instead of retaining full fragment ASTs
- the semantic callers that were immediately doing
  `parse_type_fragment(...)` then `parse_type_id(...)` were rerouted onto that
  lighter cache

Current post-slice result on the same frozen `semantic_lookup.cpp` benchmark:

- updated host-built `cppgm++`: `92.82s`, max RSS `1646544 KB`
- change relative to the earlier baseline:
  - wall time: about `-1.5%`
  - max RSS: about `-8.3%`
- retained `cache.type_fragment` payload dropped to `0`
- retained `cache.parsed_type_text`: about `50.8 MB`
- `parsed_type_text` cache hit rate: about `62.5%`

So the accepted direction was correct:

- keep the reuse
- stop retaining syntax trees on the common semantic path
- move the retained payload into cheaper semantic results

The next baseline-memory targets after this slice are now the remaining large
text/map buckets, headed by:

- `cache.template_placeholder_mentions`: about `103 MB`
- `cache.bound_type_name_rewrite`: about `82.5 MB`
- `cache.tokenized_text`: about `79 MB`
- `scope`: about `77.7 MB`

That next slice is also now complete.

The accepted follow-on batch adds:

- a shared interned semantic text pool for repeated query strings
- structured cache keys over interned text for the hot text-heavy semantic
  caches, instead of repeated synthesized key strings and nested
  `unordered_map<size_t, unordered_map<string, ...>>` shapes

Current result on the same frozen `semantic_lookup.cpp` benchmark:

- updated host-built `cppgm++`: `93.07s`, max RSS `1343800 KB`
- change relative to the original earlier baseline:
  - wall time: about `-1.3%`
  - max RSS: about `-25.1%`

The retained-memory census after this accepted slice shows the string-heavy
caches materially reduced rather than merely moved:

- `cache.template_placeholder_mentions`: about `25.9 MB`
- `cache.bound_type_name_rewrite`: about `11.2 MB`
- `cache.non_namespace_binding_mentions`: about `20.3 MB`
- `cache.dependent_non_namespace_binding_mentions`: about `20.3 MB`
- `cache.interned_text_pool`: about `11.9 MB`

The pathological guard benchmark also remains healthy:

- `semantic_expression.cpp`: `108.87s`, max RSS `1520444 KB`
- no swap-storm or runaway allocator shape returned

Two additional candidate slices against `cache.identifier_tokens` were tried
and intentionally rejected:

- interned-pointer identifier sets
- sorted `vector<string>` identifier sets

Both reduced retained memory only modestly and drove the `semantic_lookup.cpp`
control benchmark up to about `148s` to `150s`, so they failed the Phase 3
time-preservation rule and were not kept.

Validation on the accepted state is clean enough to close this plan:

- `pa18`: `47 / 47`
- `pa21`: `76 / 76`
- targeted `pa35` hosted checks: pass
- broad `make test-report ORDERED=false`: `1838 / 1839`
- the lone timeout in that historical run was
  `pa18/tests/spec/219-function-template-default-allocator-local-lambda.t`
  which was later identified as a misplaced hosted compile test and moved to
  [720-hosted-function-template-default-allocator-local-lambda-compile.t](/Users/vishvananda/cppgm/pa34/tests/compile/720-hosted-function-template-default-allocator-local-lambda-compile.t)

## Working Rule

Do not use a parallel self-host ladder run as the primary memory benchmark.

For memory work:

1. build `./dev/cppgm++` with the host compiler
2. run one benchmark translation unit at a time
3. measure memory on that single process
4. validate correctness and performance before treating the slice as ready for
   wider self-host use

The ladder is a separate integration lane, not the measurement harness for this
plan.

## Benchmark Set

Use two benchmark classes throughout this plan.

### 1. Stable Heavy Control

Use:

- `dev/src/semantic_lookup.cpp`

Purpose:

- track the baseline heavy memory cost that already exists even when the
  pathological recursion is absent
- prove that a fix for the explosion does not accidentally make ordinary heavy
  semantic compiles slower or larger

### 2. Pathological Growth Target

Use:

- `dev/src/semantic_expression.cpp`

Purpose:

- retain coverage for the local-type/template rewrite path that recently blew
  up into multi-GB allocator growth and swap storms
- ensure the known catastrophic rewrite loop stays dead while baseline memory
  work continues

## Freeze The Benchmark Input

Before each optimization round, freeze the benchmark source files into a
scratch directory keyed by commit:

```sh
python3 scripts/selfhost_memory_bench.py freeze
```

This prevents implementation edits from silently changing the benchmark input
mid-investigation.

It is acceptable that the include graph still comes from the live tree during
the first phase. If include churn becomes a problem later, add a second phase
that snapshots the relevant headers too.

## Baselines To Record

For each frozen benchmark file, record two baselines.

### A. Clang Baseline

Compile the frozen file with host Clang:

```sh
python3 scripts/selfhost_memory_bench.py run --tool clang --bench semantic_expression
python3 scripts/selfhost_memory_bench.py run --tool clang --bench semantic_lookup
```

This is not expected to match `cppgm++` semantically feature-for-feature, but
it is still the right baseline for:

- ordinary C++ compiler memory usage
- rough wall-time expectations
- whether we are doing dramatically more allocation than a mature compiler on
  the same TU

### B. Current `cppgm++` Baseline

Build `cppgm++` with the host compiler, then compile the same frozen file:

```sh
make -j1 -C dev cppgm++ CPPGM_TEST_RUNNER=1
python3 scripts/selfhost_memory_bench.py run --tool cppgm --bench semantic_expression
python3 scripts/selfhost_memory_bench.py run --tool cppgm --bench semantic_lookup
```

Use the minimal portable `cppgm++` command line for these probes:

- `-I./dev/src`
- `-c`
- `-o`

Do not add extra driver flags unless they are needed for the benchmark,
because older or alternate compiler-driver surfaces may parse them
differently.

## Measurement Protocol

For every benchmark run, collect:

- wall time
- max RSS
- one `sample` where available
- one `vmmap` where available

Suggested pattern:

```sh
python3 scripts/selfhost_memory_bench.py run \
  --tool cppgm \
  --bench semantic_expression \
  --collect-diagnostics \
  --ps-snapshots 6
```

The RSS timeline is important because it distinguishes:

- a large but bounded compile
- a monotonic runaway allocator pattern

## Optimization Phases

### Phase 1. Kill Catastrophic Growth

Current state:

- completed for the first known allocator-rewrite catastrophe
- remains in the plan as the response pattern if a new swap-storm variant is
  discovered

Success bar:

- `semantic_expression.cpp` completes without swap-storm behavior
- RSS no longer grows without bound
- `sample` no longer collapses into obvious self-recursive rewrite expansion

Method:

1. identify the precise mutation path from the `sample` and targeted logging
2. fix that specific growth mechanism
3. rerun the exact one-file benchmark
4. confirm the pathological shape is gone before moving on

This phase is about removing "forever" behavior first, not polishing the
baseline.

### Phase 2. Reduce Baseline Memory

Current state:

- completed for the current tracked tranche:
  - late semantic memory census is available
  - the initial dominant retained baseline bucket was the cached type-fragment AST payload
  - that payload was replaced on the common semantic path by the lighter
    `parsed_type_text` cache
  - the next accepted slice reduced the remaining large text/map buckets with
    the shared interned-text and structured-key cleanup
  - subsequent identifier-token experiments were rejected because they traded
    too much wall time for too little additional RSS improvement

Once the catastrophic growth is gone, use `semantic_lookup.cpp` as the control
benchmark to reduce ordinary memory cost.

Look for:

- repeated string rewriting
- repeated parse/lookup of equivalent type text
- allocator churn from transient rewritten strings
- caches that miss because text is rewritten into many semantically equivalent
  forms

The baseline-control file should remain the main driver here so we do not
optimize only one pathological surface.

### Phase 3. Preserve Or Improve Time

Any memory reduction that significantly regresses compile time should be
treated as suspect.

For each accepted memory fix, compare:

- wall time against the pre-fix `cppgm++` baseline
- wall time against the Clang baseline

The goal is not "beat Clang". The goal is:

- no catastrophic memory growth
- better or comparable `cppgm++` wall time on the benchmark TUs

## Validation Loop For Each Candidate Fix

Every memory fix candidate should follow this loop:

1. rerun the isolated pathological benchmark
2. rerun the isolated heavy control benchmark
3. compare wall time and RSS to the previous local baseline
4. rerun the owner tests for the touched feature
5. rerun the newly added dirty-tree regressions
6. rerun the relevant assignment suite
7. only then decide whether the change is ready for a separate `pa38`
   integration handoff

Minimum validation set for the current issue family:

- full `pa18`
- new local-template regressions in `pa18`
- new concrete-instantiation regression in `pa21`
- new hosted runtime smoke in `pa35`

If a memory fix touches broader template, lookup, or semantic rewriting code,
prefer also rerunning:

- `make test-report`

before committing a coherent batch.

## Guardrails

- Do not paper over the issue by special-casing one benchmark file name.
- Do not disable required semantic rewriting globally just to make memory look
  better.
- Do not accept a fix that only turns an infinite recursion into an extremely
  slow linear blowup.
- Do not use ladder throughput alone as proof that memory is now healthy.
- Do not keep multiple benchmark compiles alive at once.

## Deliverables

This plan is complete when we have all of the following:

1. a repeatable frozen benchmark workflow
2. recorded Clang and `cppgm++` baselines for the benchmark TUs
3. the catastrophic `semantic_expression.cpp` growth removed
4. documented memory/time deltas on `semantic_lookup.cpp`
5. no regression in the targeted owner suites
6. the validated fix is ready for later self-host integration without requiring
   the ladder to run as part of this plan

Current status:

- completed

## After This Plan

Once the catastrophic path is gone and the baseline has improved materially,
move the remaining work into a smaller follow-up tracker focused on:

- cache hit-rate improvements
- string canonicalization reduction
- benchmark automation
- broader self-host memory profiling during later `pa38` and `pa36` phases
