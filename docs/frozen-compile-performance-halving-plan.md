# Frozen Compile Performance Halving Plan

Status: active implementation plan

Baseline compiler checkpoint: `42d55c49c`

Frozen workload:

```sh
./dev/cppgm++ \
  -I benchmarks/self_compile/stable/include \
  -c \
  -o /tmp/cppgm-perf-check.o \
  benchmarks/self_compile/stable/semantic_overload.cpp
```

## Goal

Cut the frozen compile by at least half without RTO, LTO, or a semantics change.
Retired instructions provide the primary proof because host load makes wall time
unstable. A final quiet-machine run must also show a median wall time at or below
half of the starting median.

The committed-content checkpoint records these three-run medians:

| Signal | Starting value | Halving target |
| --- | ---: | ---: |
| retired instructions | `174,157,770,944` | `<= 87,078,885,472` |
| wall time | `40.02 s` | `<= 20.01 s` on the same quiet host |
| maximum RSS | `761,204,736 B` | no confirmed regression above the project gate |
| peak footprint | `568,758,272 B` | no regression above the project gate |

The fresh report lives at `/tmp/cppgm-perf-halving-42d55c49c.json` and names
`42d55c49c` as its head. The preserved checkpoint object lives at
`/tmp/cppgm-perf-halving-42d55c49c.o` and has SHA-256
`4fc1303ac95464ca600a882acc5f7489e021daf265e64c251c5db51b708c55c4`.

The frozen manifest must remain at epoch `9764b3835`. The performance script
must verify the source digest, the 51-header set, each header digest, and the
aggregate closure digest before each run.

## Correctness checkpoint

Commit `42d55c49c` closes the alias-consolidation divergences that preceded this
work. The retained evidence includes:

- all configured direct strict suites: PA19 `279/279`, PA20 `158/158`, PA22
  `293/293`, PA23 `385/385`, and PA24 `415/415`;
- the full direct report: `4859/4863` during a concurrent performance run, with
  the four PA30 failures passing `88/88` in an isolated PA30 report;
- focused PA19, PA23, and PA24 validation: `1117/1117`;
- the three-run frozen compile result above, which improves the pre-fix
  instruction count by `0.22%` and stays within both memory gates.

`docs/implemented/witness-alias-consolidation-correctness-adjudication.md`
records the fixes, reference ownership, and validation details. Performance
work starts from that commit and must preserve its behavior.

## Ralph evidence reviewed

The source study used `ralph:work/fable` at `4e65fdd7b`. The relevant records
are `PLAN-PERF.md`, `PLAN-PERF2.md`, `PLAN-ENDGAME.md`, and their linked commits.
The two compilers use different semantic models, so this plan ports measured
mechanisms instead of copying patches.

Ralph measured these results:

| Change | Ralph evidence | Local conclusion |
| --- | --- | --- |
| budget failed deferred-body rebinding | `semantic_overload` fell from `213.2 s` to `58.8 s`; `compile_unit` fell from `334 s` to `77.9 s` | Search for readiness retries tied to unrelated global progress. The local compiler already uses queue indexes, so it needs reason-specific rearming rather than Ralph's retry budget. |
| repair one changed registry entry | `semantic_model` fell from `33.1 s` to `29.8 s` | Audit invalidation sites for whole-cache rebuilds. Do not add a registry abstraction without a matching local invalidation. |
| armed ready-member set | `semantic_overload` fell from `45.6 s` to `39.7 s`; `compile_unit` fell from `76.9 s` to `53.0 s` | Apply the armed-record rule to local pending output entries that still retry after unrelated progress. Preserve first-queue order. |
| memoized directive closure | removed `27.9M` lookup-path allocations in one callgrind run | Local lookup still builds `std::set<const Scope *>` closures for using-directives. Source-point visibility requires a token-aware design. |
| full type hash-consing | `semantic_overload` fell from `30.7 s` to `29.1 s`; RSS fell from `2.66 GB` to `2.36 GB` | The local compiler already interns fundamentals, wrappers, and function types. Extend interning only after an eligibility and mutability audit, and land the pointer fast paths with it. |
| partial type interning | a `std::map` pointer/reference-only attempt regressed `5-6%` | Do not land isolated factory maps. A retained type slice must reduce downstream equality or conversion work in the same commit. |
| throw-free expected probes | `__cxa_throw` count fell from `277k` to `153k` | Count local failure classes first. Convert only expected control-flow failures, while preserving diagnostic exceptions at hard-error boundaries. |
| base and lookup epoch stamps | removed per-walk visited-container construction | Local semantic lookup contains many pointer `std::set` and `unordered_set` walks. Stable `ClassInfo` and `Scope` objects can carry query epochs after a reentrancy audit. |
| scratch-vector leasing | removed repeated deduction and lookup vector allocation | Add depth-indexed scratch only where allocation counts prove reuse and no returned view escapes the call. |
| representation side records | `ScopeBinding` fell from `960 B` to `400 B`; `SemNode` fell from `592 B` to `288 B` | The local tree already compacted `Type` and `CallSemNode`. Use the retained-memory census to choose the next local record. |
| copy-free LowIR inlining | `post_token` fell from `112.5 s` to `57.1 s` | The local inliner already rebuilds affected blocks without a whole-function rollback copy. Keep Ralph's callee-fact and no-expansion principles for a measured local inliner case. |
| dense LowIR slot IDs | `lower_builtin` fell from `109.9 s` to `42.9 s` | Local value propagation still uses `unordered_map<string, Operand>` and several slot passes use `map<string, ...>`. Profile the frozen LowIR share before changing the IR representation. |
| jemalloc | about `11-12%` on Ralph | Treat an allocator swap as an optional deployment experiment. It does not count toward the structural halving plan and must work on every supported host. |
| LTO or RTO | flat or a few percent, with slower rebuilds | Excluded from this plan. The normal developer build remains the measurement target. |

Ralph also recorded failed approaches that this work must avoid:

- low retry budgets changed output order;
- editing a retry queue in place changed semantics and was reverted;
- deduction memoization stayed flat after paying for key construction;
- arity prefilters saved under one percent;
- allocation tuning without allocation removal had no effect.

Each local change needs a measured avoided operation. A new cache must report
its lookup count, hit count, miss count, entry count, and invalidation count in
stats mode before it can support a performance claim.

## Local profile

A stats-enabled compile at `42d55c49c` produced
`/tmp/cppgm-frozen-phases-42d55c49c.err`. The semantic phases consumed:

| Phase | Time |
| --- | ---: |
| declaration collection | `5.375 s` |
| initial output | `10.221 s` |
| instantiated-template output fixpoint work | `5.376 s` |
| synthetic-function output fixpoint work | `1.199 s` |
| late required-function output | `0.646 s` |
| late synthesized output | `6.233 s` |
| required-definition refresh and callee closure | `0.032 s` |

The fixpoint ran five iterations. Its queues report:

| Queue | Attempts | Emissions |
| --- | ---: | ---: |
| instantiated classes | `4,074` | `1,024` |
| instantiated functions | `1,745` | `1,292` |
| late synthesized methods | `7,782` | `3,450` |
| late synthesized static functions | `871` | `270` |

The existing code already advances indexes through new work. The
`rescanned-emitted-nodes=392820` counter comes from the recursive callee walk
over each newly emitted tree; it does not prove a whole-tree rescan. Any output
optimization must distinguish first processing from a pending record retried
without a relevant readiness change.

The 10.6-second sample at
`/tmp/cppgm-frozen-sample-42d55c49c.txt` covers the early semantic portion. Its
inclusive sample leaders include:

- `lookup_type_from_ast_node`: `1,914` samples;
- `parse_type_id_ast`: `1,143` samples;
- `operator new`: `1,105` samples;
- `lookup_type_node`: `926` samples;
- `resolve_instantiated_dependent_type`: `909` samples;
- alias-template instantiation: `823` samples;
- `resolve_instantiated_dependent_type_uncached`: `736` samples.

The profile points to two large local surfaces: output readiness work and
repeated construction during dependent type and alias resolution. Later phases
must collect a full-run sample before ranking LowIR work.

## Work already present in this compiler

Several Ralph ideas already have local equivalents:

- `semantic_output::OutputState` keeps monotonic indexes for required
  definitions, instantiated classes and functions, synthetic functions,
  deferred constexpr output, class methods, static functions, and emitted
  callees.
- pending output records use retry vectors, and class records carry queued
  flags for late static-member work.
- `cpp_decl_model.cpp` uses canonical fundamental nodes, a wrapper-type hash
  cache, and a function-type hash cache.
- `Type` moved rare named metadata behind a side record and now occupies 280
  bytes. The guarded dependency walk cut instructions by `4.45%`, RSS by
  `8.93%`, and footprint by `9.56%` when it landed.
- the compiler interns text and parsed name atoms, caches ADL scope results,
  caches function-template deduction, and caches several dependent-resolution
  results.
- the LowIR inliner constructs replacement blocks. It does not take Ralph's
  old whole-function rollback snapshot.

The plan must improve these designs instead of adding parallel caches or
second ownership paths.

## Commit gate

Every retained code change gets its own commit. Documentation may accompany
the code when it records that commit's evidence. No performance code commit
lands until it passes each gate below.

### 1. Focused development checks

Build the changed frontend and run the earliest owning PA tests. Run any
reducer added for the change under both CPPGM and the relevant host compiler.
Use cache-off or instrumentation-off parity when the change touches a cache.

### 2. Frozen progress screen

Run one candidate measurement against the immutable baseline:

```sh
scripts/validate_perf_regression.py check \
  --baseline /tmp/cppgm-perf-halving-42d55c49c.json \
  --report /tmp/cppgm-<change>-screen.json \
  --runs 1
```

Reject an unchanged candidate after a hard regression. Investigate a result
within `0.5%` of baseline before running broad tests because instruction noise
can hide a flat change. Keep a small change only when it removes a measured
operation, holds instructions within the gate, and enables a named later
slice. Mark such a commit as infrastructure in the tracker.

### 3. Output identity

Compare the frozen object with the checkpoint object byte for byte:

```sh
cmp /tmp/cppgm-perf-halving-42d55c49c.o /tmp/cppgm-perf-check.o
```

For semantic or LowIR changes, also run direct LowIR comparison through the
strict harness. A byte difference blocks the change until the implementation
explains and adjudicates it as a correctness fix. Performance work may not
regenerate references to hide drift.

### 4. Required correctness gates

Run the configured strict suites and the full report:

```sh
CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 make test-strict
CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 make test-report
```

The full report includes PA9; no separate PA9 command belongs in this plan.
Both commands must report zero failures. A concurrency-related harness failure
needs an isolated rerun of the owner plus a clean broad rerun before commit.

### 5. Retained performance measurement

Collect a three-run report:

```sh
scripts/validate_perf_regression.py check \
  --baseline /tmp/cppgm-perf-halving-42d55c49c.json \
  --report /tmp/cppgm-<change>-final.json
```

Record instructions, RSS, footprint, frozen-object identity, strict totals,
full-report totals, and the commit hash in the implementation ledger. Wall
time remains supporting evidence during loaded runs.

### 6. Commit discipline

Commit one measured mechanism at a time. Keep a rejected experiment out of the
next candidate. Add its result to the rejected-work section when the result
changes future design choices.

Run `make inception` after a group of internal representation changes and at
the final halving checkpoint. The per-commit gate uses strict and full reports;
inception supplies the final self-host check.

## Implementation sequence

### Phase 0: freeze the starting artifacts

Record a baseline at `42d55c49c` with three sequential runs. Preserve its
object before any code edit.

```sh
scripts/validate_perf_regression.py record \
  --baseline /tmp/cppgm-perf-halving-42d55c49c.json \
  --runs 3

cp /tmp/cppgm-perf-check.o /tmp/cppgm-perf-halving-42d55c49c.o
```

Record a full semantic phase report and a full-run sample. The existing sample
ends during the early semantic work, so it cannot rank LowIR functions.

Exit evidence:

- baseline JSON names `42d55c49c`;
- the script verifies epoch `9764b3835`;
- the preserved object hash appears in this document;
- no compiler processes remain after the sequential batch.

### Phase 1: arm pending output by readiness reason

`semantic_output.cpp` retries pending instantiated classes, pending
instantiated functions, late required functions, class methods, and static
functions whenever any fixpoint state changes. Several retry reasons depend on
specific events: class completion, definition acquisition, template argument
completion, declaration-scope availability, or an output requirement upgrade.

First add stats-only counters for each pending reason:

- attempts and successful emissions;
- retries with an unchanged reason generation;
- arms caused by each readiness event;
- maximum pending age in fixpoint rounds.

Then give each pending record a reason mask and the relevant generation. The
producer that changes the required state rearms the record. Preserve stable
first-insertion order with a queue sequence, and let a final diagnostic sweep
check records that remain pending after semantic progress stops.

Touch points:

- `dev/src/semantic_output.{h,cpp}`;
- `dev/src/callsemantic.cpp`;
- `dev/src/semantic_template_output_policy.*`;
- output requirement producers in `dev/src/callsemantic/` and template
  instantiation modules.

Safety rules:

- no numeric retry budget;
- no in-place mutation while traversing a queue;
- no ordering by pointer or hash table iteration;
- a readiness transition must rearm every record that observes it;
- the final sweep must diagnose a missed producer during development.

Target: remove at least half of pending attempts that see no relevant readiness
change. Seek a `10-15%` instruction reduction from the phase before advancing.
Retain a smaller result only when the counters prove the remaining work emits
new output.

### Phase 2: remove repeated dependent type and alias construction

The early sample spends most of its time parsing type-id ASTs, resolving
dependent types, and instantiating aliases. Existing caches still miss often:
`dependent_type_resolution` reported `42,859` misses against `6,674` hits in
the stats run, while alias paths reconstruct lookup text, argument vectors, and
AST fragments.

Add attribution before changing behavior:

- count `parse_type_id_ast` calls by AST node, scope identity, and mode;
- count dependent-resolution misses by type kind and terminal status;
- count alias substitution attempts by declaration and canonical argument
  identity;
- count repeated failures before any scope or healing generation changes;
- count AST clones and total cloned nodes by substitution caller.

Implement the largest proven repetition in this order:

1. Cache immutable type-id parse results by stable AST identity, semantic scope,
   lookup generation, and parse mode. Do not cache nodes that depend on a local
   template environment unless the key includes its canonical binding identity.
2. Cache concrete alias-substitution failures by alias declaration, canonical
   arguments, owner identity, and the generation that can heal the failure.
   Rethrow the original diagnostic class at hard-error consumers.
3. Replace recursion `std::set` objects with inline depth scratch or query
   epochs where the stack profile proves meaningful allocation volume.
4. Reuse substitution scratch vectors by recursion depth after an escape audit.

The cache must reject source-location, witness-capture, or diagnostic modes
whose observable output depends on the current occurrence. The normal compile
must not run witness-only source reconstruction.

Target: another `10-15%` instruction reduction. Require cache hit rates and
avoided clone counts in the commit evidence.

### Phase 3: complete eligible type interning as one program

The current compiler canonicalizes fundamentals, wrappers, and function types.
It still allocates member pointers and arrays on each factory call, and named
types can carry mutable semantic metadata. Ralph's failed partial attempt shows
that another isolated factory cache can cost more than it saves.

Start with an immutability audit:

- list every `Type` field written after construction;
- identify copy-then-modify helpers;
- classify named, dependent, placeholder, pattern, and source-bearing types as
  ineligible until proven immutable;
- verify the lifetime of every `ClassInfo`, template declaration, and scope
  pointer reachable from an eligible node.

Land one cohesive implementation:

- compute a shallow structural hash whose field set matches `type_equals`;
- require interned children before interning a parent;
- use an allocation-free lookup probe in an open-addressed or reserved hash
  table;
- mark interned nodes and clear that mark on mutable copies;
- route all eligible factories through the table;
- make `type_equals` return pointer equality when both operands carry the
  interned mark;
- key conversion and base-path memos by interned identity in the same slice
  when the profile shows repeated pairs.

Keep dependent and mutable named nodes out of the first eligible set. Extend
eligibility only after byte identity and the full gates pass.

Target: `5-10%` instructions and a measurable allocation or memory reduction.
Reject the implementation if table work exceeds avoided construction and
downstream comparisons.

### Phase 4: remove lookup walk allocation

`semantic_lookup.cpp` and the template modules create pointer sets for
using-directive traversal, base traversal, associated-class collection, and
cycle guards. The frozen metrics record `158,941` scope cache key calls and
more than `1.8M` `class_info_for_type` calls, so small per-query allocations can
spread across the compile.

Implement measured families separately:

#### Using-directive closure

Cache the closure on directive-bearing namespace scopes with a directive
generation. Directiveless lexical scopes may share the nearest parent's
closure. Include source-token visibility in the cache contract: use a fast
complete-closure entry only after the last relevant directive, and use a
token-keyed entry or the current traversal before that point.

Preserve namespace injection scope, ambiguity, hiding, inline namespace rules,
and declaration-point visibility. Keep output order tied to source order.

#### Base and lookup query epochs

Add per-query epochs to stable `ClassInfo` and `Scope` records for reentrant
walks. Use a query object that restores or advances state across nested calls.
Keep inline storage plus overflow as the fallback for types or transient
records that cannot carry a stamp.

#### Scratch collections

Lease result vectors and visited overflow storage by recursion depth. Clear
elements before reuse and prove no pointer, iterator, or span escapes the call.

Target: `5-10%` instructions with fewer allocation calls. Land each family in
its own commit.

### Phase 5: replace expected exception control flow

The local semantic and template sources contain 211 matching substitution
catch, rethrow, and throw sites. The short early sample shows little unwind
time, so site count alone cannot justify a rewrite.

Add a stats-only census that records:

- throws by diagnostic kind and producer;
- catches by consumer and disposition;
- rethrow depth;
- failures served by an existing failure cache.

Convert the highest-volume expected probe to a typed status result. Keep
exceptions for hard semantic diagnostics and unexpected invariant failures.
Use scope guards for teardown so the status path and exception path restore the
same semantic state.

Target: retain only a change that removes at least one high-volume throw class
and improves instructions. Do not convert broad APIs from a static site count.

### Phase 6: shrink the next high-volume semantic records

Run `CPPGM_MEMORY_CENSUS=1` on the frozen compile after phases 1 through 5. Use
retained bytes and allocation count together to select a record.

Candidate transformations:

- move rare `FunctionBinding`, `ClassInfo`, or `Scope` fields behind a side
  record;
- allocate lazy containers only after first insertion;
- keep function-only binding payload outside records that represent types,
  values, or namespaces;
- replace repeated owned name strings with intern-pool handles where consumers
  do not depend on lexical iteration order;
- use an arena for translation-unit-lifetime records after an ownership audit.

Add `static_assert(sizeof(...))` or a size census for the selected record. Keep
the common fields together and measure lookup cache behavior, since a smaller
record can still regress if it adds an extra pointer chase on the hot path.

Target: `5-10%` instructions or enough memory reduction to enable the next
algorithmic slice. The final memory signals must remain within the project
gates.

### Phase 7: optimize the measured LowIR long pole

Collect a full-run sample and phase timers after semantic work drops. Rank the
LowIR pipeline from that evidence.

Likely candidates:

1. Assign dense numeric IDs to temps and slots at function entry. Use vectors
   for value propagation, debug locations, use counts, and slot summaries.
   Preserve the original strings for output and diagnostics.
2. Cache callee-static inliner facts for one program pass: eligibility,
   instruction count, EH shape, return shape, and definition discipline.
3. Keep inliner bookkeeping off callers with no inlineable call. Update temp
   use counts during splice or recompute only after a batch.
4. Replace whole `ValueEnvironment` copies at CFG joins with generation-tagged
   dense state or copy-on-write storage.

Do not assume Ralph's `post_token` pathology exists here. A candidate needs a
local phase timer or sample count. Keep LowIR text and the frozen object byte
identical.

Target: close the remaining distance to `87,078,885,472` instructions. Split
dense IDs, inliner facts, and CFG state into separate commits unless one change
needs the representation from another.

### Phase 8: final halving proof

Run the following from a clean tracked tree:

```sh
CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 make test-strict
CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 make test-report
make inception

scripts/validate_perf_regression.py check \
  --baseline /tmp/cppgm-perf-halving-42d55c49c.json \
  --report /tmp/cppgm-perf-halving-final.json
```

Then run a quiet three-run wall-time confirmation. The final report must show:

- retired instructions at or below `87,078,885,472`;
- quiet median wall time at or below `20.01 s`;
- zero strict failures and zero full-report failures;
- byte-identical frozen object output;
- a passing inception comparison;
- no confirmed RSS regression and no footprint regression beyond the project
  tolerances.

## Decision rules

Use these rules throughout the work:

- Preserve the baseline. Never rebaseline to make a candidate look faster.
- Compare the same compiler build, standard library, frozen epoch, and command.
- Run performance batches sequentially. Check for leftover compiler processes
  after each batch.
- Keep correctness fixes separate from performance commits. Add a reducer and
  adjudication when profiling exposes a latent bug.
- Reject a cache whose key construction consumes its saved work.
- Reject iteration-order changes unless the language or output contract calls
  for them and the change receives separate correctness adjudication.
- Stop expanding one technique after two measured flat or regressing
  implementations. Return to the profile.
- Exclude RTO, LTO, and PGO from progress accounting. They slow the edit-build
  cycle and cannot supply the requested halving.

## Implementation ledger

Fill one row after each retained commit.

| Commit | Mechanism | Instructions | Change from start | Max RSS | Footprint | Frozen bytes | Strict | Full report | Evidence |
| --- | --- | ---: | ---: | ---: | ---: | --- | --- | --- | --- |
| `42d55c49c` | correctness checkpoint | `174,157,770,944` | `0.00%` | `761,204,736` | `568,758,272` | SHA-256 `4fc1303a...5c4` | pass | pass with isolated PA30 confirmation | `/tmp/cppgm-perf-halving-42d55c49c.json` |

## Rejected work ledger

Record the code shape, measured result, and reason for rejection. Remove the
experiment before starting the next candidate.

| Experiment | Result | Decision | Evidence |
| --- | --- | --- | --- |
| none in this performance program | | | |
