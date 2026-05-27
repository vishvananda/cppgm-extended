# CPPGM Performance Analysis and Implementation Plan

This document reconciles the earlier code-reading analysis with direct timing
and hotspot measurements from the current `dev/` compiler.

The short version:

- The compile-time gap against `clang++` is real.
- Preprocessing is slower than host compilers, but semantic analysis is the
  dominant multiplier.
- The largest structural problems are repeated semantic fixpoint rescans,
  repeated fragment reparsing, parser speculation/backtracking, overload
  candidate deduplication, and scope/cache key churn.
- Some plausible bottlenecks exist but are lower priority because current
  measurements do not show them dominating wall clock.

## Execution Rules

Treat the numbers in this document as seed evidence, not permanent truth.
Before starting Patch 0, rerun the baseline commands on current `HEAD` and
record fresh artifacts.

Use these rules for the implementation pass:

- use build-first root entry points by default:
  - `make build`
  - `make run-cppgm CPPGM_ARGS='...'`
  - `make bootstrap-self-sweep-nobuild ...`
- only use `./dev/cppgm++` directly after an explicit fresh `make build` when
  the goal is to measure compiler runtime without make's no-op overhead
- do not compare parallel sweep wall times across runs; use `jobs=1` for
  stable before/after performance comparisons
- keep one active performance patch at a time
- for each patch:
  1. capture baseline artifacts
  2. implement the code change
  3. run the targeted regression subset for that patch
  4. rerun the stable performance benchmarks
  5. run full `make test-report`
  6. commit before moving to the next patch
- do not batch multiple performance patches before an all-green checkpoint

Recommended artifact layout per patch:

- `/tmp/cppgm-perf/<patch-name>/baseline-*`
- `/tmp/cppgm-perf/<patch-name>/after-*`
- include `git rev-parse --short HEAD` in the notes for each capture

## Current Measured Evidence

## Representative Translation Unit Timing

Measured on `dev/src/constant_value.cpp`:

| Command | Time |
| --- | ---: |
| `clang++ -E` | `0.281s` |
| `cppgm++ -E` | `2.573s` |
| `clang++ -c` | `1.269s` |
| `cppgm++ -c` | `13.669s` |

This shows:

- preprocessing alone is already about `9x` slower than `clang++`
- full compilation is about `10.8x` slower
- the semantic path adds much more cost than preprocessing alone

## Internal Phase Split

Measured with `cppgm++` on the same file:

| Phase | Command | Time |
| --- | --- | ---: |
| preprocess only | `cppgm++ -E` | `3.15s` |
| preprocess + parse AST | `cppgm++ --emit-ast` | `5.35s` |
| preprocess + parse + semantic output | `cppgm++ --emit-semantics` | `15.33s` |
| preprocess + parse + semantics + LowIR text | `cppgm++ --emit-lowir` | `15.41s` |
| full compile | `cppgm++ -c` | `16.38s` |

Interpretation:

- preprocessing is expensive
- parsing adds noticeable but smaller cost
- semantic analysis is the dominant added phase
- LowIR generation and object emission are comparatively small on this file

## Self-Compile Sweep Snapshot

Using:

```bash
python3 scripts/report_self_compile_sweep.py \
  --compiler ./dev/cppgm++ \
  --host-cxx clang++ \
  --jobs 6 \
  --timeout-sec 20 \
  --output-prefix /tmp/cppgm-self-sweep
```

Many successful files already take `10s` to `18s` to compile. Representative
slow files from the sweep were:

- `dev/src/recog_token_buffer.cpp` at `18.5s`
- `dev/src/constant_value.cpp` at `18.1s`
- `dev/src/parser_trace.cpp` at `17.9s`
- `dev/src/rtti_names.cpp` at `17.2s`
- `dev/src/cppast_dump.cpp` at `16.7s`

That is important because several of these are not huge source files. It points
to repeated work in included headers, templates, and semantic rescans rather
than only raw source size.

## Instrumented Semantic Run

On an instrumented compile of `dev/src/constant_value.cpp` with:

```bash
env \
  CPPGM_FILE_TIMING=1 \
  CPPGM_FILE_TIMING_LIMIT=40 \
  CPPGM_SEMANTIC_HOTSPOT=1 \
  CPPGM_SEMANTIC_STATS=1 \
  CPPGM_SEMANTIC_CACHE_STATS=1 \
  CPPGM_HOST_CXX=clang++ \
  ./dev/cppgm++ -c -I dev/src -o /tmp/constant_value.o dev/src/constant_value.cpp
```

the semantic layer reported:

- `template-instantiation-requests=6440`
- `template-definition-upgrades=5180`
- `required-definition-requests=29672`
- `query_requests=334264`
- `fragment_requests=88842`
- `fragment_parses=47494`

The top timed locations were mostly libc++ headers:

- `semantic.class-reference` in `allocator_traits.h`
- `semantic.class-reference` in `iterator_traits.h`
- `parser.declaration` in `string`
- `parser.declaration` in `locale`
- `parser.declaration` in `deque`
- `parser.declaration` in `__tree`
- `parser.declaration` in `__hash_table`

This is the main signal: the compiler is repeatedly re-analyzing template-heavy
header constructs.

## Reconciliation with the Earlier Analysis

## Confirmed High-Value Hypotheses

The earlier analysis was correct about these:

1. `instantiated_functions` uses linear deduplication in
   `dev/src/output_requirement_engine.cpp`.
   This is a clear O(n^2) path.

2. Overload candidate deduplication in `dev/src/semantic_overload.cpp`
   uses nested loops.
   This is on a hot path and worth fixing early.

3. `scope_cache_key()` is recomputed by walking ancestor scopes on many cache
   lookups in `dev/src/callsemantic.cpp`.

4. The semantic fixpoint loop repeatedly revisits large previously-processed
   state in `dev/src/callsemantic.cpp` and `dev/src/semantic_output.cpp`.

5. The semantic subsystem is the dominant phase, not LowIR or object emission.

## Additional High-Impact Findings Not Called Out Strongly Enough Earlier

1. Fragment reparsing is a first-wave problem.

   The semantic layer repeatedly parses synthetic snippets such as:

   - `using __cppgm_fragment = ...;`
   - `int __cppgm_fragment = ...;`

   implemented in `dev/src/semantic_fragment_parser.cpp`.

   This is not just a constant-factor issue. The instrumented run showed
   `88,842` fragment requests and `47,494` fragment parses on a single TU.

2. Parser speculation and nested reparsing are structural costs.

   `CppAstParser::parse_declaration()` tries many alternate parses from the
   same token position.

   `parse_parenthesized_type_id_or_expression()` also instantiates nested
   parsers over token suffixes and slices to resolve ambiguities.

   This is a likely source of superlinear work in template-heavy headers.

3. Preprocessing is slow enough to deserve its own workstream.

   The semantic layer is still the main bottleneck, but `cppgm++ -E` being far
   slower than host preprocessors means preprocessing should not be ignored.

## Hypotheses That Are Valid but Lower Priority Right Now

1. LowIR symbol lookup fallback in `dev/src/lowirgensemantic.cpp`

   This is worth cleaning up, but current phase timing shows LowIR is not the
   dominant incremental phase on the measured TU.

2. Flattening all nested cache maps into one giant flat hash table

   This may help constants, but current measurements suggest the bigger problem
   is cache miss frequency and repeated key construction, not only nested map
   indirection.

3. Replacing all scope maps with a single unified binding map

   This is architecturally plausible, but it is a larger semantic refactor than
   the first-wave optimizations and should come later if the earlier wins are
   not enough.

## Corrections to the Earlier Analysis

1. The semantic metrics environment variable is:

   - `CPPGM_SEMANTIC_STATS=1`

   not `CPPGM_SEMANTIC_METRICS=1`.

2. The `--emit-semantics` frontend requires:

   ```bash
   ./dev/cppgm++ --emit-semantics -o <outfile> <src...>
   ```

3. LowIR/object generation should not be treated as the first optimization
   target until semantic costs are materially reduced.

## Primary Bottlenecks

## 1. Semantic Fixpoint Rescans

Files:

- `dev/src/callsemantic.cpp`
- `dev/src/semantic_output.cpp`

Problem:

- the analyzer runs a do-while loop until output state stops changing
- each iteration can reprocess large vectors and can recursively walk already
  emitted output trees
- this is worklist-shaped logic implemented as repeated whole-structure scans

Expected fix:

- move to explicit incremental work queues
- process only newly added required definitions, newly instantiated
  classes/functions, and newly emitted bodies

## 2. Fragment Reparsing of Text into Synthetic Translation Units

Files:

- `dev/src/callsemantic.cpp`
- `dev/src/semantic_fragment_parser.cpp`

Problem:

- many semantic queries reconstruct type or expression text
- misses in the fragment caches trigger full parse work through synthetic
  wrapper translation units
- this is heavily exercised by templates and traits code in libc++

Expected fix:

- add direct fragment parser entry points
- reduce dependence on synthetic TU wrappers
- improve cache keys and cache hit rates

## 3. Parser Ambiguity Speculation

File:

- `dev/src/cppast_parser.cpp`

Problem:

- declaration parsing backtracks through many alternatives
- some ambiguity resolution constructs nested parsers over slices/suffixes
- repeated speculative parsing in headers can cascade badly

Expected fix:

- memoize ambiguity probes
- strengthen lookahead classification
- avoid creating nested parser instances in hot ambiguity paths when a cheaper
  syntactic predicate would do

## 4. Overload Candidate and Conversion Repetition

File:

- `dev/src/semantic_overload.cpp`

Problem:

- overload candidate dedupe is O(n^2)
- conversions are repeatedly recomputed for many candidates
- this sits on the critical path for normal expression analysis

Expected fix:

- hash-based candidate dedupe
- conversion result memoization within a semantic analysis pass or within a
  top-level call-resolution context

## 5. Scope/Cache Key Churn

Files:

- `dev/src/callsemantic.cpp`
- `dev/src/template_resolution.cpp`
- `dev/src/semantic_model.h`

Problem:

- scope keys are rebuilt repeatedly by walking parent scopes
- template resolution cache keys serialize large strings
- fragment keys are string-heavy

Expected fix:

- cache stable scope fingerprints on `Scope`
- invalidate only on binding-fingerprint changes
- reduce string concatenation in hot cache lookups

## 6. Preprocessing Cost

Files:

- `dev/src/pptokenizer.cpp`
- `dev/src/preprocessor.cpp`
- `dev/src/posttokenizer.cpp`

Problem:

- preprocessing alone is already much slower than host compilers
- some of the slow successful self-sweep files are near the front of the
  pipeline and still expensive

Expected fix:

- measure first
- then optimize only after the semantic first-wave fixes are in place, unless
  phase timing still shows preprocessing dominating some workloads

## Performance Engineering Workflow

## Baseline Commands

### Correctness Baseline

Use the root regression suite:

```bash
make test-report
```

Relevant Makefile entry points:

- `make test-report`
- `make test-report-nobuild`

### Self-Compile Baseline

```bash
make build
make bootstrap-self-sweep-nobuild \
  BOOTSTRAP_SELF_SWEEP_JOBS=$(getconf _NPROCESSORS_ONLN 2>/dev/null || sysctl -n hw.ncpu) \
  BOOTSTRAP_SELF_SWEEP_TIMEOUT_SEC=20 \
  BOOTSTRAP_SELF_SWEEP_OUTPUT_PREFIX=/tmp/cppgm-self-sweep
```

Also keep a stable frozen subset benchmark for repeatable before/after
comparisons. This must be single-threaded.

```bash
OUTPUT_PREFIX=/tmp/cppgm-self-sweep-stable \
TIMEOUT_SEC=60 \
scripts/run_frozen_self_compile_benchmarks.sh
```

For historical compiler comparisons, point `COMPILER` at the other worktree's
binary while keeping the frozen corpus in the current repo:

```bash
COMPILER=/tmp/cppgm-patch0/dev/cppgm++ \
OUTPUT_PREFIX=/tmp/cppgm-self-sweep-stable-patch0 \
TIMEOUT_SEC=60 \
scripts/run_frozen_self_compile_benchmarks.sh
```

And keep a second slow-frontier subset for the current bootstrap timeout
family:

```bash
python3 scripts/report_self_compile_sweep.py \
  --compiler ./dev/cppgm++ \
  --host-cxx clang++ \
  --jobs 1 \
  --timeout-sec 120 \
  --output-prefix /tmp/cppgm-self-sweep-slow \
  --source dev/src/callsemantic.cpp \
  --source dev/src/semantic_output.cpp \
  --source dev/src/template_instantiation.cpp \
  --source dev/src/template_resolution.cpp
```

### Semantic Hotspot Baseline

```bash
env \
  CPPGM_FILE_TIMING=1 \
  CPPGM_FILE_TIMING_LIMIT=40 \
  CPPGM_SEMANTIC_HOTSPOT=1 \
  CPPGM_SEMANTIC_STATS=1 \
  CPPGM_SEMANTIC_CACHE_STATS=1 \
  CPPGM_HOST_CXX=clang++ \
  ./dev/cppgm++ -c -I dev/src -o /tmp/out.o dev/src/constant_value.cpp \
  2>/tmp/cppgm-hotspot.log
```

### Phase Timing Baseline

```bash
env CPPGM_HOST_CXX=clang++ /usr/bin/time -lp \
  ./dev/cppgm++ --emit-semantics -o /tmp/out.sem dev/src/constant_value.cpp
```

Repeat for:

- `-E`
- `--emit-ast`
- `--emit-semantics`
- `--emit-lowir`
- `-c`

### Sampling Profilers

macOS:

```bash
xcrun xctrace record --template 'Time Profiler' --launch -- \
  ./dev/cppgm++ -c -I dev/src -o /tmp/out.o dev/src/constant_value.cpp
```

or:

```bash
sample <pid> 5 1 -mayDie -file /tmp/sample.txt
```

Linux:

```bash
perf record -g ./dev/cppgm++ -c -I dev/src -o /tmp/out.o dev/src/constant_value.cpp
perf report
```

## Regression Discipline

Every performance patch should have:

1. a correctness gate
2. a stable performance benchmark
3. a before/after measurement artifact

### Regression Gates by Change Area

For preprocessing and parser changes:

```bash
make test-report ACTIVE_TEST_REPORT_PAS='pa1 pa2 pa3 pa4 pa5 pa6 pa34'
```

For semantic, template, output, and codegen changes:

```bash
make test-report ACTIVE_TEST_REPORT_PAS='pa35 pa34 pa27 pa26 pa22 pa21 pa18 pa16 pa15 pa14 pa12'
```

Before merge or batching multiple performance patches:

```bash
make test-report
```

### Performance Gates

For every patch, rerun at least:

```bash
OUTPUT_PREFIX=/tmp/cppgm-self-sweep-stable \
TIMEOUT_SEC=60 \
scripts/run_frozen_self_compile_benchmarks.sh
```

For semantic and template hotspot work, also rerun the slow-frontier subset
with `--jobs 1 --timeout-sec 120`.

Acceptance rule:

- no `make test-report` regression
- measurable improvement on at least one stable benchmark file
- no significant slowdown on the rest of the stable benchmark set
- if the change targets the current timeout frontier, at least one
  slow-frontier benchmark must either get faster or move out of timeout
  classification

Preferred measurement rule:

- run each stable benchmark command at least 3 times
- compare medians, not the best single run
- only claim a win from noisy measurements when the improvement is clearly
  larger than run-to-run variance

## Ordered Implementation Sequence

This is the sequence to implement, from highest expected payoff per unit risk
to larger structural changes.

## Patch 0: Add Permanent Phase and Hotspot Instrumentation

Goal:

- make future optimization work measurable without ad hoc edits

Changes:

- add per-phase timers in `dev/src/cpp_toolchain.cpp`
- add counters for:
  - fixpoint iterations
  - total rescanned emitted nodes
  - overload candidate counts
  - conversion attempt counts
  - `scope_cache_key()` call counts
  - fragment parse miss counts by kind
- keep everything behind env vars or a `--time-report` style flag

Why first:

- this makes every later patch easier to validate
- it reduces guesswork and prevents local wins that simply move cost elsewhere

Regression:

- `make test-report ACTIVE_TEST_REPORT_PAS='pa1 pa2 pa3 pa4 pa5 pa6 pa34'`
- `make test-report ACTIVE_TEST_REPORT_PAS='pa35 pa34 pa27 pa26 pa22 pa21 pa18 pa16 pa15 pa14 pa12'`

Performance check:

- rerun the stable self-compile subset
- rerun the semantic hotspot baseline

## Patch 1: Remove Obvious O(n^2) Deduplication

Goal:

- eliminate avoidable quadratic behavior in hot containers

Changes:

1. add `instantiated_function_set` beside `instantiated_functions`
   in output requirement tracking
2. replace the linear `std::find()` in
   `dev/src/output_requirement_engine.cpp`
3. replace overload dedupe nested loops in
   `dev/src/semantic_overload.cpp`
   with a hashed identity key

Why second:

- low risk
- easy to reason about
- likely immediate payoff in template-heavy code

Regression:

- `make test-report ACTIVE_TEST_REPORT_PAS='pa35 pa34 pa27 pa26 pa22 pa21 pa18 pa16 pa15 pa14 pa12'`

Performance check:

- compare overload counters and sweep timings before/after

## Patch 2: Cache Scope Fingerprints and Reduce Hot Cache-Key Rebuilds

Goal:

- cut repeated ancestor walks and repeated string key construction

Changes:

- add cached fingerprint fields to `semantic_model::Scope`
- invalidate cached fingerprints when `binding_fingerprint_epoch` changes
- switch hot caches in `callsemantic.cpp` and `template_resolution.cpp`
  to use memoized scope fingerprints
- avoid repeated recomputation of fragment/scope keys inside a single request

Why third:

- mechanical enough to be safe
- touches many hot caches
- likely helps both semantic and parser-adjacent code

Regression:

- `make test-report ACTIVE_TEST_REPORT_PAS='pa35 pa34 pa27 pa26 pa22 pa21 pa18 pa16 pa15 pa14 pa12'`

Performance check:

- `scope_cache_key()` call count and cache-hit timings must improve

## Patch 3: Convert Semantic Fixpoint Rescans into Incremental Worklists

Goal:

- stop rescanning the entire emitted output and all tracked state on every
  fixpoint iteration

Changes:

- introduce explicit work queues for:
  - newly required function definitions
  - newly instantiated functions
  - newly instantiated classes
  - newly emitted bodies requiring callee-closure expansion
- replace whole-tree rescans in `expand_emitted_output_callee_closure()`
  with incremental processing of newly emitted subtrees
- replace repeated full passes over state vectors where possible

Why fourth:

- this is likely the biggest structural semantic win
- higher risk than the earlier patches, so it should be done with better
  instrumentation already in place

Regression:

- `make test-report ACTIVE_TEST_REPORT_PAS='pa35 pa34 pa27 pa26 pa22 pa21 pa18 pa16 pa15 pa14 pa12'`
- `make test-report`

Performance check:

- fixpoint iteration count may stay similar, but rescanned-node count should
  collapse
- semantic phase time on `constant_value.cpp` should materially drop

## Patch 4: Memoize Conversion Results in Overload Resolution

Goal:

- stop recomputing the same conversions across many overload candidates

Changes:

- add a conversion memoization cache keyed by:
  - source type identity
  - target type identity
  - relevant conversion options
- scope the cache to a top-level declaration analysis or call-resolution
  context to keep invalidation simple

Why fifth:

- high leverage in normal expression-heavy and STL-heavy code
- easier to validate after Patch 1 has already cleaned up candidate dedupe

Regression:

- `make test-report ACTIVE_TEST_REPORT_PAS='pa35 pa34 pa27 pa26 pa22 pa21 pa18 pa16 pa15 pa14 pa12'`

Performance check:

- conversion-attempt counters should fall sharply

## Architectural Direction After Patch 4

The measured gap versus `clang++`/`g++` no longer looks like a missing 5%
micro-optimization. It looks like an architectural gap:

- hot semantic work still crosses a text boundary too often
- many cache keys are serialized strings rather than canonical semantic
  identities
- dependent lookup frequently rewrites text and reparses fragments instead of
  querying structured semantic objects

That means the remaining plan should optimize for this transition:

- keep source text for diagnostics, repros, and stable test output
- stop using text as the authoritative representation in hot semantic paths
- move hot queries toward canonical names, template-argument identities, and
  explicit dependency edges

This does **not** require an immediate rewrite of the entire semantic node
model. `CallSemNode`, `ClassInfo`, `FunctionBinding`, and existing AST nodes can
stay in place while a new canonical key/query layer is introduced beside them.
The migration plan should be:

1. add canonical identities and structured query objects
2. route the hot caches and dependent lookup paths through them
3. keep text-based fallback paths temporarily, with counters
4. only then consider shrinking or removing text-authoritative fields

## Patch 5: Canonicalize Hot Semantic Keys

Goal:

- move the hottest semantic cache and lookup boundaries away from raw text and
  toward canonical, reusable identities

Changes:

- audit the current text-authoritative hot paths and choose the first target
  from the measured hotspot set:
  - `qualified_name_cache`
  - `unscoped_template_id_cache`
  - `template_placeholder_mentions_cache`
  - `dependent_non_namespace_binding_mentions_cache`
  - `dependent_type_resolution_cache`
  - `resolve_template_arguments_cache`
- introduce canonical key helpers for:
  - interned identifiers or identifier-like handles
  - qualified-name segment lists
  - template-argument list identity
  - scope plus lookup-environment identity
- keep the original strings for diagnostics and source fidelity, but stop using
  them as the primary authority for the selected hot cache path

Why fifth:

- the earlier wins came from reducing repeated work, not from changing
  containers
- the next order-of-magnitude gains require reducing string-mediated semantic
  queries

Regression:

- `make test-report ACTIVE_TEST_REPORT_PAS='pa35 pa34 pa27 pa26 pa22 pa21 pa18 pa16 pa15 pa14 pa12'`
- `make test-report`

Performance check:

- semantic work counters may stay flat
- wall time on the frozen corpus must still improve
- if cache counters stay flat and wall time regresses, revert and choose a
  different text-authoritative boundary

## Patch 6: Replace Text Rewriting in Dependent Type and Name Resolution

Goal:

- stop answering dependent semantic questions by rewriting text and reparsing it

Changes:

- introduce structured dependent lookup request types for the hottest paths,
  starting with named type and template-id resolution
- replace flows like:
  - `rewrite_bound_*_in_text(...)`
  - `rewrite_visible_named_type_aliases_*`
  - `parse_type_fragment(...)`
  with structural substitution and lookup on canonicalized components
- keep the current text-based path as a measured fallback while coverage grows

Why sixth:

- this is the clearest current architectural difference from mainstream
  compilers
- it directly attacks the highest-cost semantic text boundary

Regression:

- `make test-report ACTIVE_TEST_REPORT_PAS='pa35 pa34 pa27 pa26 pa22 pa21 pa18 pa16 pa15 pa14 pa12'`
- `make test-report`

Performance check:

- dependent-type fallback count should fall
- fragment parse counts should fall on the same workloads
- semantic time should drop materially on STL-heavy benchmark files

## Patch 7: Demote Fragment Parsing to Rare Fallback

Goal:

- make fragment parsing an exception path rather than a normal semantic service

Changes:

- treat `parse_type_fragment()` and `parse_expression_fragment()` as fallback
  paths only
- add direct structured entry points for the common fragment callers that
  already have enough parsed context
- record explicit fallback reasons so remaining fragment use can be ranked and
  removed
- delete wrapper-TU round trips where the information already exists in parsed
  or canonicalized form

Why seventh:

- current measurements show fragment caching helps, but the real problem is that
  fragment parsing is still normal
- once Patch 6 exists, this becomes a controlled cleanup instead of a blind
  parser rewrite

Regression:

- `make test-report ACTIVE_TEST_REPORT_PAS='pa35 pa34 pa27 pa26 pa22 pa21 pa18 pa16 pa15 pa14 pa12'`
- `make test-report`

Performance check:

- fragment requests may remain high for a while
- fragment parses and fallback count should collapse
- semantic phase time should improve much more than container-only patches

## Patch 8: Canonicalize Template and Scope Query State

Goal:

- stop serializing template and scope state into large ad hoc strings on hot
  paths

Changes:

- replace string-based template resolution keys with structured key objects
  built from canonical template argument identities and scope identities
- reduce dependence on `arg_texts`, `return_arg_texts`, and similar serialized
  forms in hot query code
- centralize semantic key construction so caches do not each invent their own
  string protocol
- migrate the next tier of high-traffic caches after Patch 5 proves the model

Why eighth:

- once the first canonical key path works, the template/scope caches are the
  next largest text churn source

Regression:

- `make test-report ACTIVE_TEST_REPORT_PAS='pa35 pa34 pa27 pa26 pa22 pa21 pa18 pa16 pa15 pa14 pa12'`
- `make test-report`

Performance check:

- wall time should improve even if semantic event counts remain similar
- string-heavy cache-key construction should stop dominating hot samples

## Patch 9: Build an Incremental Semantic Dependency Engine

Goal:

- ask expensive semantic questions once and wake only the dependents that
  actually need reprocessing

Changes:

- record dependency edges between:
  - function bindings
  - class completions
  - template instantiations
  - required-definition consumers
- extend the existing incremental output work into a more explicit dependency
  graph instead of repeated broad semantic queries
- make remaining completion and required-definition propagation dependency-driven

Why ninth:

- this is the structural answer to the remaining fixpoint-style churn after the
  text boundaries have been reduced
- it is high risk and should come only after canonical identities exist

Regression:

- `make test-report ACTIVE_TEST_REPORT_PAS='pa35 pa34 pa27 pa26 pa22 pa21 pa18 pa16 pa15 pa14 pa12'`
- `make test-report`

Performance check:

- repeated required-definition requests should collapse
- remaining fixpoint churn should fall further
- broad semantic passes should become closer to true worklists than rescans

## Deferred Secondary Passes

These are still worth doing, but only after the semantic architecture above is
measurably better:

- parser speculation and nested reparsing work in `CppAstParser`
- preprocessor/tokenizer tuning
- container swaps and other micro-optimizations
- backend-only cleanup if backend phases become visible in samples

## What Success Looks Like

Short-term:

- stable benchmark files materially faster
- semantic hotspot counts reduced, especially:
  - fragment parses
  - repeated required-definition requests
  - overload dedupe work
  - scope-key recomputation

Medium-term:

- self-compile sweep timings improve across the stable benchmark subset
- the hottest semantic files stop scaling like repeated text rewriting and
  reparsing loops
- slower successful files move materially down without relying on trace or
  metrics compile-out tricks

Long-term:

- the compiler still will not match `clang++` immediately, but it should stop
  exhibiting obviously superlinear behavior on template-heavy headers

## Immediate Next Three Patches

If only the first three implementation tasks are queued now, they should be:

1. Patch 5: canonicalize the first hot semantic text boundary
2. Patch 6: replace text rewriting in dependent type and name resolution
3. Patch 7: demote fragment parsing to fallback-only service

Those are now the best payoff-to-risk next steps because they attack the main
architectural difference the measurements exposed: too much sema still flows
through text rather than canonical semantic identities.
