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
must collect a full-run sample before ranking LowIR work. That full-run sample
was collected after the first retained cache change and is recorded under
Phase 7.

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

Phase 1 audit result at `27d6bc9d0`: a temporary reason census found only 352
pending outcomes in the frozen compile. Instantiated classes accounted for 308
missing-node outcomes and 10 placeholder-scope outcomes. Instantiated functions
accounted for 34 missing-demand outcomes. Late required functions, methods, and
static functions produced no pending outcomes. The remaining attempts either
emitted new output or dismissed a newly observed record. The implementation
already contains the useful worklists, so the census code was removed and no
queue behavior changed.

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

First retained Phase 2 slice: the analyzer-wide dependent-type resolution
cache recorded 6,674 hits and 42,859 misses. Each probe built a recursive
structural string and interned it before map lookup. A controlled cache-off
compile improved instructions and cut retained memory, while the inner
per-resolution recursion cache still handled cycles and duplicate child work.
The retained change removes the analyzer-wide cache and replaces the inner
active `std::set<const Type *>` with a thread-local LIFO vector. The frozen
object remains byte-identical. Configured direct strict passes `1530/1530`, and
the full direct report passes `4863/4863`.

Three-run medians against `42d55c49c`:

| Signal | Candidate | Change |
| --- | ---: | ---: |
| retired instructions | `173,508,264,894` | `-649,506,050` (`-0.37%`) |
| maximum RSS | `743,952,384 B` | `-17,252,352 B` (`-2.27%`) |
| peak footprint | `557,821,952 B` | `-10,936,320 B` (`-1.92%`) |
| wall time | `41.84 s` | `+4.55%`, informational under host load |

Evidence: `/tmp/cppgm-dependent-resolution-negative-cache-removal-final.json`.

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

Phase 3 census at `787af744a` closed the proposed remaining-factory expansion.
The frozen compile constructed 3,277 arrays and 37 member pointers, too few to
repay a new interning table. The same census measured 2,550,753 `type_equals`
calls and 1,403,227 structural comparisons. Named types accounted for 911,499
of those structural comparisons. After 39,849 exact-key matches, 871,650 calls
constructed two stripped-key strings to support elaborated type prefixes; only
six calls matched through that compatibility rule.

Commit `05b1eb38d` keeps the rule but represents each stripped key as an offset
into its source string. Unequal named keys now compare their remaining lengths
and character ranges without constructing temporary strings. The code accepts
`class` and `struct` compatibility, rejects incompatible `union` and
`enum` prefixes, and preserves the exact-key fast path.

The frozen object remains byte-identical with SHA-256
`4fc1303ac95464ca600a882acc5f7489e021daf265e64c251c5db51b708c55c4`.
Configured direct strict passes `1530/1530`; the full direct report, including
PA9 through its normal lane, passes `4863/4863`.

Three-run medians against `42d55c49c`:

| Signal | Candidate | Change |
| --- | ---: | ---: |
| retired instructions | `137,038,283,448` | `-37,119,487,496` (`-21.31%`); `-1.11%` from `4ed53c57e` |
| maximum RSS | `745,762,816 B` | `-15,441,920 B` (`-2.03%`) |
| peak footprint | `556,085,248 B` | `-12,673,024 B` (`-2.23%`) |
| elapsed cycles | `106,466,772,358` | `-17.11%` |
| wall time | `35.54 s` | `-11.19%`, informational under host load |

Evidence: `/tmp/cppgm-type-factory-census-2.stderr`,
`/tmp/cppgm-named-key-view-screen.json`,
`/tmp/cppgm-named-key-view-test-strict.log`,
`/tmp/cppgm-named-key-view-test-report.log`, and
`/tmp/cppgm-named-key-view-final.json`.

The post-`05b1eb38d` sample attributed 100 immediate `memcmp` samples to
`semantic_utils::strip_elaborated_type_prefix`. A frozen census measured
3,881,111 calls: 3,344,409 had no prefix, while 536,702 matched one of the six
accepted spellings. The helper received 780,075 temporary strings.

Commit `1060e05df` classifies the prefix family from the first byte. Most
misses now skip textual comparison; `class`, `struct`, and `union` need one
comparison, while the `enum` family preserves the required longest-prefix
order. An rvalue overload uses the same classifier and erases a matched prefix
from a temporary buffer. Lvalue callers keep the existing owned-result API.

The frozen object remains byte-identical with SHA-256
`4fc1303ac95464ca600a882acc5f7489e021daf265e64c251c5db51b708c55c4`.
Configured direct strict passes `1530/1530`; the full direct report, including
PA9 through its normal lane, passes `4863/4863`.

Three-run medians against `42d55c49c`:

| Signal | Candidate | Change |
| --- | ---: | ---: |
| retired instructions | `136,011,111,282` | `-38,146,659,662` (`-21.90%`); `-0.75%` from `05b1eb38d` |
| maximum RSS | `735,920,128 B` | `-25,284,608 B` (`-3.32%`) |
| peak footprint | `556,093,440 B` | `-12,664,832 B` (`-2.23%`) |
| elapsed cycles | `98,365,785,242` | `-23.42%` |
| wall time | `24.96 s` | `-37.63%`, informational under host load |

Evidence: `/tmp/cppgm-elaborated-prefix-census.stderr`,
`/tmp/cppgm-elaborated-prefix-rvalue-census.stderr`,
`/tmp/cppgm-elaborated-prefix-dispatch-screen.json`,
`/tmp/cppgm-elaborated-prefix-dispatch-rvalue-test-strict.log`,
`/tmp/cppgm-elaborated-prefix-dispatch-rvalue-test-report.log`, and
`/tmp/cppgm-elaborated-prefix-dispatch-rvalue-final.json`.

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

First retained Phase 4 slice: visible-scope collection for template body
checking used the same spelling for three separate operations. It interned the
name for the visible-name set, looked it up in the atom-keyed value-type map,
and interned it again when recording a missing type. The loop now interns the
spelling once and uses that atom for both containers. It preserves the original
shadowing rule, including replacement of a null inner entry by a non-null outer
type. No new cache or lifetime rule is involved.

The frozen object remains byte-identical with SHA-256
`4fc1303ac95464ca600a882acc5f7489e021daf265e64c251c5db51b708c55c4`.
Configured direct strict passes `1530/1530`; the full direct report passes
`4863/4863`.

Three-run medians against `42d55c49c`:

| Signal | Candidate | Change |
| --- | ---: | ---: |
| retired instructions | `165,685,333,837` | `-8,472,437,107` (`-4.86%`) |
| maximum RSS | `749,821,952 B` | `-11,382,784 B` (`-1.50%`) |
| peak footprint | `557,666,304 B` | `-11,091,968 B` (`-1.95%`) |
| elapsed cycles | `123,872,411,586` | `-3.56%` |
| wall time | `38.64 s` | `-3.45%`, informational under host load |

Evidence: `/tmp/cppgm-template-body-single-atom-final.json`.

Second retained Phase 4 slice: the service-layer function-local type overlay
walked every named type in every intervening non-namespace scope, although it
only consumes bindings whose semantic key contains the function-local marker.
A frozen census measured 24,616 overlay calls across 123,204 scopes. They
scanned 5,097,223 named-type entries to find 1,911 local candidates and bind
591 names.

Each scope now derives a lazy index containing only its function-local type
names. Empty results remain allocation-free, and a changed named-type count
invalidates the index; lexical function-local bindings are monotonic. Cached
names are looked up again before use, so unordered-map rehashing cannot leave
dangling entries. The instrumented candidate recorded 103,812 cache hits,
19,392 misses, 82 invalidations, and only 37,549 named-type inspections, a
99.3% reduction. It found the same 1,911 candidates and bound the same 591
names. The temporary census was removed before commit.

The frozen object remains byte-identical with SHA-256
`4fc1303ac95464ca600a882acc5f7489e021daf265e64c251c5db51b708c55c4`.
Configured direct strict passes `1530/1530`; the full direct report, including
PA9 through its normal lane, passes `4863/4863`.

Three-run medians against `42d55c49c`:

| Signal | Candidate | Change |
| --- | ---: | ---: |
| retired instructions | `145,188,653,153` | `-28,969,117,791` (`-16.63%`); `-0.48%` from `87f17bd97` |
| maximum RSS | `740,917,248 B` | `-20,287,488 B` (`-2.67%`) |
| peak footprint | `556,396,544 B` | `-12,361,728 B` (`-2.17%`) |
| elapsed cycles | `113,811,972,760` | `-11.39%` |
| wall time | `42.49 s` | `+6.17%`, informational under host load |

Evidence: `/tmp/cppgm-local-type-overlay-census.stderr`,
`/tmp/cppgm-local-type-overlay-index-census.stderr`, and
`/tmp/cppgm-local-type-overlay-index-final.json`.

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

First retained Phase 6 slice: a fresh full-run profile at `773cadc65`, stored
at `/tmp/cppgm-frozen-full-sample-773cadc65.txt`, attributed 36.2 seconds to
semantic analysis and 3.3 seconds to LowIR and object output. Allocation and
free routines were the two largest leaf groups. The template-substitution and
mangling AST clone helpers both called mutable sparse-field accessors even
when the corresponding source fields were empty. Each call materialized a
`CppAstSparseData` record that the clone did not need. The helpers now test the
source fields first, while preserving the exact set of sparse fields they copy
when those fields are present. Pack-substitution clones already share the
sparse record through its copy-on-write pointer and needed no change.

The frozen object remains byte-identical with SHA-256
`4fc1303ac95464ca600a882acc5f7489e021daf265e64c251c5db51b708c55c4`.
Configured direct strict passes `1530/1530`; the full direct report passes
`4863/4863`. Three-run medians against `42d55c49c` are
`163,459,605,743` instructions (`-6.14%`), `738,054,144 B` maximum RSS,
`553,398,272 B` peak footprint, `122,453,802,165` cycles, and `38.21 s` wall
time. The instruction result is `-0.48%` from `773cadc65`; the retained change
also removes the profiled per-clone allocation and reduces both memory signals.
Evidence: `/tmp/cppgm-sparse-clone-materialization-final.json`.

Second retained Phase 6 slice: alias-template instantiation looked up its
argument-identity cache before constructing an instantiation scope, but did
not consume an ordinary hit until after that scope was allocated, populated,
and the full substitution machinery had been prepared. The cache's store
policy already puts a result under the ordinary argument key only when the
result is independent of the use scope. The implementation now returns an
ordinary cached result immediately when it is null or no longer depends on a
template parameter. Dependent results, including the scope-sensitive form,
continue through the original scope construction, redirect, repair, and
substitution path. Source-occurrence completion remains in the caller and is
therefore unchanged on the fast path.

The frozen object remains byte-identical with SHA-256
`4fc1303ac95464ca600a882acc5f7489e021daf265e64c251c5db51b708c55c4`.
Configured direct strict passes `1530/1530`; the full direct report passes
`4863/4863`. Three-run medians against `42d55c49c` are
`160,251,233,762` instructions (`-7.99%`), `740,306,944 B` maximum RSS,
`552,783,872 B` peak footprint, `121,147,496,331` cycles, and `38.38 s` wall
time. The instruction result is `-1.96%` from the sparse-clone checkpoint.
Evidence: `/tmp/cppgm-alias-concrete-hit-final.json`.

Third retained Phase 6 slice: `CppAstLazyVector` used unique ownership, so each
copy of a populated AST side vector copied its elements and their nested
syntax. Template substitution and mangling make many such copies before
changing only a subset. The container now uses an intrusive, non-atomic shared
holder. Const access shares the holder, and every mutable entry point detaches
the vector first. Empty vectors remain allocation-free, the container remains
one pointer wide, and callers keep value semantics.

The frozen object remains byte-identical with SHA-256
`4fc1303ac95464ca600a882acc5f7489e021daf265e64c251c5db51b708c55c4`.
Configured direct strict passes `1530/1530`; the full direct report passes
`4863/4863`.

Three-run medians against `42d55c49c`:

| Signal | Candidate | Change |
| --- | ---: | ---: |
| retired instructions | `159,287,615,401` | `-14,870,155,543` (`-8.54%`); `-0.60%` from `dc49aaa16` |
| maximum RSS | `741,462,016 B` | `-19,742,720 B` (`-2.59%`) |
| peak footprint | `552,189,952 B` | `-16,568,320 B` (`-2.91%`) |
| elapsed cycles | `121,861,759,732` | `-5.13%` |
| wall time | `40.83 s` | `+2.02%`, informational under host load |

Evidence: `/tmp/cppgm-ast-lazy-vector-cow-final.json`.

Fourth retained Phase 6 slice: the profile at
`/tmp/cppgm-frozen-full-sample-a1f5d1db1.txt` attributed 426 sampled AST copy
constructor calls to child-vector copies. Four hot declarator filters copied a
complete subtree, discarded the new root's children, and then copied or
recursively filtered the retained children again. A shared helper now copies
all root metadata while leaving `children` empty. The filters build the final
child vector once. This keeps the ordinary parser representation unchanged and
does not add a holder allocation to every non-leaf node.

The frozen object remains byte-identical with SHA-256
`4fc1303ac95464ca600a882acc5f7489e021daf265e64c251c5db51b708c55c4`.
Configured direct strict passes `1530/1530`; the full direct report, including
PA9 through its normal lane, passes `4863/4863`.

Three-run medians against `42d55c49c`:

| Signal | Candidate | Change |
| --- | ---: | ---: |
| retired instructions | `158,427,135,579` | `-15,730,635,365` (`-9.03%`); `-0.54%` from `a1f5d1db1` |
| maximum RSS | `734,437,376 B` | `-26,767,360 B` (`-3.52%`) |
| peak footprint | `552,079,360 B` | `-16,678,912 B` (`-2.93%`) |
| elapsed cycles | `123,265,820,186` | `-4.03%` |
| wall time | `39.97 s` | `-0.12%`, informational under host load |

Evidence: `/tmp/cppgm-ast-filter-shallow-copy-final.json`.

Fifth retained Phase 6 slice: the post-filter profile at
`/tmp/cppgm-frozen-full-sample-c0155750b.txt` attributed 5,579 inclusive
samples to lazy named-member collection and 2,624 to its per-name class scan.
A temporary frozen-workload census measured 14,526 scans for 13,655 unique
class/name queries. Those scans made 4.19 million declaration probes, 13.84
million recursive declarator visits, and 3.99 million identifier comparisons.
Of the 4,253 queried classes, 1,927 were queried for more than one name, with a
maximum of 38.

`ClassInfo` now builds a name-to-declaration-position index on first use. Each
entry retains the access at that source position. Consumers use the index to
select candidates, then run the existing exact declaration predicate before
materializing a member. The cache records its source node and rebuilds if
template selection changes that node. The optional memory census includes the
index's table, strings, and candidate vectors; the frozen compile reports
`2,760,312 B` for that bucket.

The frozen object remains byte-identical with SHA-256
`4fc1303ac95464ca600a882acc5f7489e021daf265e64c251c5db51b708c55c4`.
Configured direct strict passes `1530/1530`; the full direct report, including
PA9 through its normal lane, passes `4863/4863`.

Three-run medians against `42d55c49c`:

| Signal | Candidate | Change |
| --- | ---: | ---: |
| retired instructions | `157,041,590,005` | `-17,116,180,939` (`-9.83%`); `-0.87%` from `c0155750b` |
| maximum RSS | `750,534,656 B` | `-10,670,080 B` (`-1.40%`) |
| peak footprint | `555,393,024 B` | `-13,365,248 B` (`-2.35%`) |
| elapsed cycles | `120,005,029,370` | `-6.57%` |
| wall time | `39.07 s` | `-2.37%`, informational under host load |

Evidence: `/tmp/cppgm-reference-named-member-census.stderr`,
`/tmp/cppgm-reference-index-memory-census.stderr`, and
`/tmp/cppgm-reference-member-name-index-final.json`.

### Phase 7: optimize the measured LowIR long pole

Collect a full-run sample and phase timers after semantic work drops. Rank the
LowIR pipeline from that evidence.

A 60-second full-run sample at `01f875b8f`, stored at
`/tmp/cppgm-frozen-full-sample-01f875b8f.txt`, reached the native backend. The
largest compiler-owned leaf was `collect_temp_intervals` with 316 samples.
String hashing had 267 leaf samples and string-set insertion had 246, while
malloc and free routines occupied the first two leaf positions. The call graph
also placed `collect_temp_intervals` on a broad native-backend path. This was
enough local evidence to begin with interval analysis instead of assuming the
LowIR inliner or value environment was the first long pole.

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

First retained Phase 7 slice: `build_layout` used to compute ordinary temp
intervals six times per function and forwarded-parameter intervals twice.
These calls repeated definition indexing, block liveness, tree lookup, string
comparison, and interval sorting. The backend now computes one base interval
set, one allocation-specific set after direct-call index discovery, and one
forwarded-parameter set. Consumers reuse the matching immutable result. The
change leaves temp names, interval ordering, register allocation decisions,
debug ranges, and emitted bytes unchanged.

The frozen object remains byte-identical with SHA-256
`4fc1303ac95464ca600a882acc5f7489e021daf265e64c251c5db51b708c55c4`.
Configured direct strict passes `1530/1530`; the full direct report, including
PA9 through its normal lane, passes `4863/4863`.

Three-run medians against `42d55c49c`:

| Signal | Candidate | Change |
| --- | ---: | ---: |
| retired instructions | `166,898,155,701` | `-7,259,615,243` (`-4.17%`) |
| maximum RSS | `745,816,064 B` | `-15,388,672 B` (`-2.02%`) |
| peak footprint | `557,416,448 B` | `-11,341,824 B` (`-1.99%`) |
| elapsed cycles | `126,815,470,440` | `-1.27%` |
| wall time | `41.18 s` | `+2.90%`, informational under host load |

Evidence: `/tmp/cppgm-interval-reuse-final.json`.

Second retained Phase 7 slice: runtime function-reference collection receives
typed `CallSemNode` symbols from semantic analysis. When that identity already
has an exported object symbol, it is sufficient to retain the reference, but
the collector previously queried the generated-function set, semantic symbol
map, and function lookup index first. The test order now accepts the existing
exported identity before those fallback ownership probes. Reserved runtime,
backend passthrough, and registry-only symbols retain their original checks.

The frozen object remains byte-identical with SHA-256
`4fc1303ac95464ca600a882acc5f7489e021daf265e64c251c5db51b708c55c4`.
Configured direct strict passes `1530/1530`; the full direct report passes
`4863/4863`.

Three-run medians against `42d55c49c`:

| Signal | Candidate | Change |
| --- | ---: | ---: |
| retired instructions | `164,248,241,098` | `-9,909,529,846` (`-5.69%`) |
| maximum RSS | `748,720,128 B` | `-12,484,608 B` (`-1.64%`) |
| peak footprint | `557,678,592 B` | `-11,079,680 B` (`-1.95%`) |
| elapsed cycles | `123,432,987,278` | `-3.90%` |
| wall time | `38.82 s` | `-3.00%`, informational under host load |

Evidence: `/tmp/cppgm-runtime-symbol-identity-fast-path-final.json`.

Third retained Phase 7 slice: function-symbol collection and lookup repeatedly
serialized semantic function types into stable structural strings. A frozen
census measured 305,404 key requests for 7,876 distinct `Type` objects:
297,528 requests, or 97.4%, repeated a key already constructed during the same
LowIR generation call. Each miss paid for recursive type traversal and a fresh
`ostringstream`; the function-symbol lookup index cached keys for its entries
but did not cover the other producers and probes.

The generator now caches stable keys by `Type` address for the lifetime of one
`build_lowir_program` call. An outer frame clears and reserves the table before
generation and clears it again afterward, so raw addresses cannot survive into
the next batch compilation. Nested frames share the live table. The semantic
type graph remains owned and read-only for that scope. Setting
`CPPGM_DIAGNOSTIC_STABLE_FUNCTION_TYPE_KEY_CACHE=1` reports lookups, hits,
misses, entries, and scope invalidations; the frozen compile reports 305,404,
297,528, 7,876, 7,876, and 1 respectively.

The frozen object remains byte-identical with SHA-256
`4fc1303ac95464ca600a882acc5f7489e021daf265e64c251c5db51b708c55c4`.
Configured direct strict passes `1530/1530`; the full direct report, including
PA9 through its normal lane, passes `4863/4863`.

Three-run medians against `42d55c49c`:

| Signal | Candidate | Change |
| --- | ---: | ---: |
| retired instructions | `152,441,499,735` | `-21,716,271,209` (`-12.47%`); `-2.93%` from `a28f2dda1` |
| maximum RSS | `747,036,672 B` | `-14,168,064 B` (`-1.86%`) |
| peak footprint | `555,773,952 B` | `-12,984,320 B` (`-2.28%`) |
| elapsed cycles | `116,548,128,285` | `-9.26%` |
| wall time | `35.73 s` | `-10.72%`, informational under host load |

Evidence: `/tmp/cppgm-stable-key-cache-diagnostic.stderr` and
`/tmp/cppgm-stable-function-type-key-cache-final.json`.

Fourth retained Phase 7 slice: runtime-symbol classification normalized the
query and then linearly tested every runtime spelling, explicit object alias,
and generated operator alias. It also normalized each table spelling again on
every probe. A frozen census measured 120,058 classification calls,
10,395,300 table-entry probes, and only 816 matches. Most ordinary function
symbols therefore paid for a complete scan of the 87-entry runtime table.

The classifier now builds one immutable hash index from normalized runtime
names and aliases. An input spelling is normalized only when its prefix can be
affected by the normalization rules. The index retains the lowest table
position for a collision, preserving the original first-entry precedence.

The frozen object remains byte-identical with SHA-256
`4fc1303ac95464ca600a882acc5f7489e021daf265e64c251c5db51b708c55c4`.
Configured direct strict passes `1530/1530`; the full direct report, including
PA9 through its normal lane, passes `4863/4863`.

Three-run medians against `42d55c49c`:

| Signal | Candidate | Change |
| --- | ---: | ---: |
| retired instructions | `145,884,874,306` | `-28,272,896,638` (`-16.23%`); `-4.30%` from `80c3e4d47` |
| maximum RSS | `744,771,584 B` | `-16,433,152 B` (`-2.16%`) |
| peak footprint | `556,154,880 B` | `-12,603,392 B` (`-2.22%`) |

The host was under unrelated load during this batch, so its wall time and
cycle count are not progress evidence. Evidence:
`/tmp/cppgm-runtime-symbol-policy-census.stderr` and
`/tmp/cppgm-runtime-symbol-index-final.json`.

Fifth retained Phase 7 slice: the post-overlay full-run profile at
`/tmp/cppgm-frozen-full-sample-91c21326ad.txt` showed the function-symbol
lookup index rebuilding inside runtime-reference collection. A temporary
frozen census measured 22 full rebuilds of an index that finished with 9,956
entries. Runtime discovery added 717 constructor/destructor symbol mappings
and 717 matching entries during that walk; an unresolved reference after a
group of additions rebuilt every name table, compact name, simple name,
mapped symbol, and stable type key.

`FunctionSymbolLookupIndex` now supports appending one entry. When special
member discovery changes an already-current index, it inserts the mapped
symbol and appends the entry to the same four lookup structures. If the index
is already dirty, the original lazy full rebuild remains authoritative. This
keeps entry order, name compaction, type keys, and all pre-runtime symbol
collection behavior unchanged while preventing phase-local additions from
invalidating completed work.

The frozen object remains byte-identical with SHA-256
`4fc1303ac95464ca600a882acc5f7489e021daf265e64c251c5db51b708c55c4`.
Configured direct strict passes `1530/1530`; the full direct report, including
PA9 through its normal lane, passes `4863/4863`.

Three-run medians against `42d55c49c`:

| Signal | Candidate | Change |
| --- | ---: | ---: |
| retired instructions | `143,082,634,066` | `-31,075,136,878` (`-17.84%`); `-1.45%` from `faf3a29a6` |
| maximum RSS | `754,479,104 B` | `-6,725,632 B` (`-0.88%`) |
| peak footprint | `556,146,688 B` | `-12,611,584 B` (`-2.22%`) |
| elapsed cycles | `115,719,518,200` | `-9.91%` |
| wall time | `43.27 s` | `+8.12%`, informational under host load |

Evidence: `/tmp/cppgm-function-symbol-index-census.stderr` and
`/tmp/cppgm-function-symbol-index-incremental-final.json`.

Sixth retained Phase 7 slice: the post-index full-run profile at
`/tmp/cppgm-frozen-full-sample-8a727d91c.txt` attributed sampled allocation
work to recursive `CallSemNode` copies made through `ExprInfo`. The accompanying
memory census at `/tmp/cppgm-memory-census-8a727d91c.stderr` measured 201,463
retained CallSem output nodes and about 59 MiB of retained CallSem storage, so
copying even transient semantic subtrees was worth screening.

`ExprInfo`'s existing move operations are now declared `noexcept`, allowing
`vector<ExprInfo>` growth to relocate values instead of falling back to deep
copies. Overload analysis also transfers terminal local values into candidate
records and argument vectors after making only the independent copies that
those records retain. Function-template argument-option enumeration moves the
selected option into the deduction vector and restores it after the recursive
combination, preserving every option without cloning its CallSem subtree.
Implicit-object ranking uses a reference to either the converted or original
value when neither needs ownership.

The one-run ownership screen used `142,054,830,302` instructions and produced
the exact frozen object. The committed three-run confirmation names
`39d241018` as its head. The frozen object remains byte-identical with SHA-256
`4fc1303ac95464ca600a882acc5f7489e021daf265e64c251c5db51b708c55c4`.
Configured direct strict passes `1530/1530`; the full direct report, including
PA9 through its normal lane, passes `4863/4863`.

Three-run medians against `42d55c49c`:

| Signal | Candidate | Change |
| --- | ---: | ---: |
| retired instructions | `142,363,891,265` | `-31,793,879,679` (`-18.26%`); `-0.50%` from `70607afcd` |
| maximum RSS | `747,724,800 B` | `-13,479,936 B` (`-1.77%`) |
| peak footprint | `556,298,240 B` | `-12,460,032 B` (`-2.19%`) |
| elapsed cycles | `109,027,398,176` | `-15.12%`; `-5.78%` from `70607afcd` |
| wall time | `34.09 s` | `-14.82%`, informational under host load |

Evidence: `/tmp/cppgm-callsem-option-moves-screen.json`,
`/tmp/cppgm-callsem-noexcept-moves-screen.json`,
`/tmp/cppgm-callsem-ownership-moves-screen.json`, and
`/tmp/cppgm-callsem-ownership-moves-final.json`.

Seventh retained Phase 7 slice: a post-ownership clone census measured
1,202,262 AST nodes cloned for template substitution. The function-type pack
probe in `expand_template_argument_inputs` accounted for 402,159 of those
nodes. It cloned every source syntax whose text matched the current argument,
then immediately rejected the clone unless its type-id described a direct
function type with a non-varargs pack parameter.

The caller now applies that same read-only structural predicate to the source
syntax before cloning. Qualifying arguments still enter the existing cloning
and expansion helper unchanged; nonqualifying arguments skip construction of
a temporary that the helper could not mutate successfully. The temporary
census was removed before commit.

The frozen object remains byte-identical with SHA-256
`4fc1303ac95464ca600a882acc5f7489e021daf265e64c251c5db51b708c55c4`.
Configured direct strict passes `1530/1530`; the full direct report, including
PA9 through its normal lane, passes `4863/4863`.

Three-run medians against `42d55c49c`:

| Signal | Candidate | Change |
| --- | ---: | ---: |
| retired instructions | `140,749,490,447` | `-33,408,280,497` (`-19.18%`); `-1.13%` from `39d241018` |
| maximum RSS | `744,112,128 B` | `-17,092,608 B` (`-2.25%`) |
| peak footprint | `556,294,144 B` | `-12,464,128 B` (`-2.19%`) |
| elapsed cycles | `109,060,116,025` | `-15.09%` |
| wall time | `35.37 s` | `-11.62%`, informational under host load |

Evidence: `/tmp/cppgm-template-clone-origin-census.stderr`,
`/tmp/cppgm-function-pack-clone-guard-screen.json`, and
`/tmp/cppgm-function-pack-clone-guard-final.json`.

Eighth retained Phase 7 slice: initial LowIR function-symbol collection
performed 12,060 entry lookups. The old lookup scanned the growing entry
vector and made 55,477,400 probes to find 44,811 same-name candidates and
2,821 matching entries. The frozen program finished with 9,239 entries.

`ProgramGenerator` now records each entry position under its exact function
name. A lookup visits those positions in insertion order and applies the same
`type_equals` and stable-type-key tests. Initial collection and runtime special
member discovery both update the index when they append an entry.

The frozen object remains byte-identical with SHA-256
`4fc1303ac95464ca600a882acc5f7489e021daf265e64c251c5db51b708c55c4`.
Configured direct strict passes `1530/1530`; the full direct report, including
PA9 through its normal lane, passes `4863/4863`.

Three-run medians against `42d55c49c`:

| Signal | Candidate | Change |
| --- | ---: | ---: |
| retired instructions | `138,575,302,610` | `-35,582,468,334` (`-20.43%`); `-1.54%` from `a268247dc` |
| maximum RSS | `742,748,160 B` | `-18,456,576 B` (`-2.42%`) |
| peak footprint | `556,113,920 B` | `-12,644,352 B` (`-2.22%`) |
| elapsed cycles | `107,346,055,232` | `-16.43%` |
| wall time | `35.11 s` | `-12.27%`, informational under host load |

Evidence: `/tmp/cppgm-function-symbol-entry-scan.stderr`,
`/tmp/cppgm-function-symbol-entry-name-index-screen.json`, and
`/tmp/cppgm-function-symbol-entry-name-index-final.json`.

Ninth retained Phase 7 slice: `FunctionLayout` kept four high-volume machine
backend indexes in ordered maps even though their consumers only probe by
name. The storage-offset, storage-type, floating-register, and temporary-
definition indexes now use reserved hash maps. The integer register map stays
ordered because register-allocation and frame-layout code traverse it. The
smaller forwarded-parameter, promoted-slot, alias, and branch-load maps also
stay ordered after a broader conversion screened worse.

Two independent three-run batches initially disagreed by `0.42%` for the same
candidate binary. A contemporaneous alternating A/B run therefore compared
the exact prior-commit compiler with the candidate on the same frozen input.
The prior compiler used a median `136,352,479,271` instructions and the
candidate used `135,565,226,107`, a `0.577%` reduction. Median RSS changed from
`735,928,320 B` to `735,244,288 B`; median footprint changed from
`555,683,840 B` to `555,634,688 B`. All six objects had the frozen SHA-256.

The post-commit absolute three-run result names `1a96ce861` as its head. The
frozen object remains byte-identical with SHA-256
`4fc1303ac95464ca600a882acc5f7489e021daf265e64c251c5db51b708c55c4`.
Configured direct strict passes `1530/1530`; the full direct report, including
PA9 through its normal lane, passes `4863/4863`.

Absolute three-run medians against `42d55c49c`:

| Signal | Candidate | Change |
| --- | ---: | ---: |
| retired instructions | `135,861,805,665` | `-38,295,965,279` (`-21.99%`) |
| maximum RSS | `736,321,536 B` | `-24,883,200 B` (`-3.27%`) |
| peak footprint | `555,786,240 B` | `-12,972,032 B` (`-2.28%`) |
| elapsed cycles | `107,109,005,429` | `-16.61%` |
| wall time | `28.11 s` | `-29.76%`, informational under host load |

Evidence: `/tmp/cppgm-layout-lookup-hashes-screen.json`,
`/tmp/cppgm-layout-lookup-hashes-decision.json`,
`/tmp/cppgm-layout-lookup-hashes-test-strict.log`,
`/tmp/cppgm-layout-lookup-hashes-test-report.log`,
`/tmp/cppgm-layout-lookup-hashes-final.json`, and the alternating A/B records
`/tmp/cppgm-layout-ab-{base,candidate}-{1,2,3}.time`.

Tenth retained Phase 7 slice: using-directive lookup created a tree-backed
visited set for every namespace-graph traversal. A temporary frozen census
recorded `1,658,095` traversal sets and `3,908,076` total visited scopes. No
set visited more than six scopes: `824,396` visited one, `44,720` visited two,
`360,364` visited three, `252,767` visited four, `153,008` visited five, and
`22,840` visited six. The replacement keeps eight pointers inline, compares
them linearly, and preserves an overflow vector for larger source programs.

The clean decision batch used a median `133,940,628,877` instructions versus
`135,434,571,754` for the contemporaneous retained compiler, a `1.103%`
reduction. Decision-batch median RSS was `731,324,416 B` and footprint was
`555,806,720 B`. The post-commit absolute batch ran under heavier host load,
so its cycles, wall time, and RSS moved independently of the stable retired-
instruction count; those secondary signals remain recorded below without
using them to inflate the result.

The post-commit three-run result names `70fa6d879` as its head. The frozen
object remains byte-identical with SHA-256
`4fc1303ac95464ca600a882acc5f7489e021daf265e64c251c5db51b708c55c4`.
Configured direct strict passes `1530/1530`; the full direct report, including
PA9 through its normal lane, passes `4863/4863`.

Absolute three-run medians against `42d55c49c`:

| Signal | Candidate | Change |
| --- | ---: | ---: |
| retired instructions | `133,973,655,610` | `-40,184,115,334` (`-23.07%`) |
| maximum RSS | `747,077,632 B` | `-14,127,104 B` (`-1.86%`) |
| peak footprint | `555,909,120 B` | `-12,849,152 B` (`-2.26%`) |
| elapsed cycles | `107,714,930,773` | `-16.14%` |
| wall time | `29.05 s` | `-27.41%`, informational under host load |

Evidence: `/tmp/cppgm-using-directive-inline-visited-screen.json`,
`/tmp/cppgm-using-directive-inline-visited-decision.json`, and
`/tmp/cppgm-using-directive-inline-visited-final.json`.

Eleventh retained Phase 7 slice: template-body validation represented every
set of interned names as a node-based hash set. These sets are copied while
the recursive validator enters declarations and control-flow scopes. The
earlier ownership census had measured `16,469` name-set copies containing
`582,942` entries, while the fresh post-lookup profile still attributed 42
direct atom-set insertion allocations to visible-value collection. `AtomNameSet`
now stores atoms in sorted contiguous order, uses lower-bound insertion, and
uses binary search for membership. The representation remains private to the
existing set interface, and atom identity preserves the prior equality rule.

The initial three-run candidate median was `132,862,900,577` instructions.
Because that sat on the retention boundary, an alternating binary A/B made the
decision: the parent used median `134,023,112,895` and the candidate used
`132,805,080,448`, a `0.909%` reduction. Candidate and parent median RSS were
`747,556,864 B` and `743,571,456 B`; footprints were `556,322,816 B` and
`556,011,520 B`. The small RSS increase stayed within tolerance, while
footprint was effectively flat. All six paired outputs had the frozen hash.

The post-commit three-run result names `af290029f` as its head. The frozen
object remains byte-identical with SHA-256
`4fc1303ac95464ca600a882acc5f7489e021daf265e64c251c5db51b708c55c4`.
Configured direct strict passes `1530/1530`; the full direct report, including
PA9 through its normal lane, passes `4863/4863`.

Absolute three-run medians against `42d55c49c`:

| Signal | Candidate | Change |
| --- | ---: | ---: |
| retired instructions | `132,942,123,779` | `-41,215,647,165` (`-23.67%`) |
| maximum RSS | `749,297,664 B` | `-11,907,072 B` (`-1.56%`) |
| peak footprint | `556,949,504 B` | `-11,808,768 B` (`-2.08%`) |
| elapsed cycles | `106,905,875,700` | `-16.77%` |
| wall time | `31.00 s` | `-22.54%`, informational under host load |

Evidence: `/tmp/cppgm-template-body-sorted-atoms-screen.json`,
`/tmp/cppgm-template-body-sorted-atoms-decision.json`,
`/tmp/cppgm-template-body-sorted-atoms-final.json`, the alternating records
`/tmp/cppgm-sorted-atoms-ab-{parent,candidate}-{1,2,3}.json`, and
`/tmp/cppgm-using-directive-profile.sample.txt`.

Twelfth retained Phase 7 slice: `build_layout` ran
`collect_temp_intervals` twice for every LowIR function. The first result
supported dead-result, direct-branch, and call-index decisions as well as
debug ranges. The second rebuilt the interval map, rescanned every
instruction, and recomputed block liveness only to extend source-pointer
lifetimes for the selected direct call-index temporaries. The fresh profile
attributed 70 leaf samples to interval collection and 91 to `build_layout`.

Register allocation now copies the first interval vector, indexes its entries
by name, scans call sites for the selected index temporaries, and extends only
their source intervals. The scan preserves the original call-like positions
for ordinary calls, TLS address materialization, and i128 helpers, then marks
newly crossing intervals live across calls. Debug intervals keep the original
first-pass result.

The one-run screen used `131,969,763,580` instructions. An alternating binary
A/B made the retention decision: the parent used median `132,950,915,256`
and the candidate used `131,968,317,406`, a `0.739%` reduction. Every paired
run emitted the frozen object.

The post-commit three-run result names `650dc50cd` as its head. The frozen
object remains byte-identical with SHA-256
`4fc1303ac95464ca600a882acc5f7489e021daf265e64c251c5db51b708c55c4`.
Configured direct strict passes `1530/1530`; the full direct report, including
PA9 through its normal lane, passes `4863/4863`.

Absolute three-run medians against `42d55c49c`:

| Signal | Candidate | Change |
| --- | ---: | ---: |
| retired instructions | `132,299,680,071` | `-41,858,090,873` (`-24.03%`) |
| maximum RSS | `695,853,056 B` | `-65,351,680 B` (`-8.59%`) |
| peak footprint | `556,654,592 B` | `-12,103,680 B` (`-2.13%`) |
| elapsed cycles | `97,889,941,035` | `-23.79%` |
| wall time | `27.47 s` | `-31.36%`, informational under host load |

Evidence: `/tmp/cppgm-register-interval-reuse-screen.json`,
`/tmp/cppgm-register-interval-reuse-final.json`, the alternating records
`/tmp/cppgm-register-interval-ab-{parent,candidate}-{1,2,3}.json`, and
`/tmp/cppgm-using-directive-profile.sample.txt`.

Thirteenth retained Phase 7 slice: backend layout construction copied every
LowIR temp definition into two lookup indexes, copied function parameter
vectors into a program-wide signature index, and copied the thread-local
global set into every `FunctionLayout`. Direct-call index classification also
rescanned the whole function once for each candidate temporary.

The indexes now borrow immutable instruction and parameter records for the
lifetime of layout construction and MIR emission. Register assignment receives
the builder's thread-local set directly. Lookup-only function-name and
parameter indexes use reserved hash tables, and direct-call classification
collects candidates before making one call-site pass. No output-producing
traversal changed order.

The one-run screen used `131,165,238,771` instructions. An alternating binary
A/B made the retention decision: the parent used median `132,035,283,775`
and the candidate used `130,913,920,619`, a `0.849%` reduction. Every paired
run emitted the frozen object.

The post-commit three-run result names `59505722b` as its head. The frozen
object remains byte-identical with SHA-256
`4fc1303ac95464ca600a882acc5f7489e021daf265e64c251c5db51b708c55c4`.
Configured direct strict passes `1530/1530`; the full direct report, including
PA9 through its normal lane, passes `4863/4863`.

Absolute three-run medians against `42d55c49c`:

| Signal | Candidate | Change |
| --- | ---: | ---: |
| retired instructions | `130,886,247,307` | `-43,271,523,637` (`-24.85%`) |
| maximum RSS | `735,780,864 B` | `-25,423,872 B` (`-3.34%`) |
| peak footprint | `556,519,424 B` | `-12,238,848 B` (`-2.15%`) |
| elapsed cycles | `95,061,322,786` | `-25.99%` |
| wall time | `24.44 s` | `-38.93%`, informational under host load |

Evidence: `/tmp/cppgm-backend-borrowed-metadata-screen.json`,
`/tmp/cppgm-backend-borrowed-metadata-final.json`, and the alternating records
`/tmp/cppgm-backend-metadata-ab-{parent,candidate}-{1,2,3}.json`.

Fourteenth retained Phase 7 slice: the fresh profile attributed 192 allocation
samples to `CppAstLazyVector<CppAstNode>::mutable_vector()` and another 74 to
the template-ID specialization. Template-substitution and mangling clone paths
call `reserve(source.size())` for each lazy AST side vector. When the source
was empty, `reserve(0)` still called `mutable_vector()` and allocated an empty
holder, contradicting the wrapper's allocation-free-empty invariant.

`CppAstLazyVector::reserve(0)` is now a no-op, matching `std::vector` behavior.
Positive reservations still allocate or detach through the existing mutable
path, so populated vectors keep the same copy-on-write semantics.

The one-run screen used `128,775,420,933` instructions. An alternating binary
A/B made the retention decision: the parent used median `131,291,483,775`
and the candidate used `128,612,847,943`, a `2.040%` reduction. Candidate and
parent median RSS were `724,021,248 B` and `737,230,848 B`; footprints were
`554,491,904 B` and `556,228,608 B`. Every paired run emitted the frozen
object.

The post-commit three-run result names `8e46f9fc2` as its head. The frozen
object remains byte-identical with SHA-256
`4fc1303ac95464ca600a882acc5f7489e021daf265e64c251c5db51b708c55c4`.
PA10 passes `157/157`, configured direct strict passes `1530/1530`, and the
full direct report, including PA9 through its normal lane, passes `4863/4863`.

Absolute three-run medians against `42d55c49c`:

| Signal | Candidate | Change |
| --- | ---: | ---: |
| retired instructions | `128,444,585,958` | `-45,713,184,986` (`-26.25%`) |
| maximum RSS | `734,158,848 B` | `-27,045,888 B` (`-3.55%`) |
| peak footprint | `554,516,480 B` | `-14,241,792 B` (`-2.50%`) |
| elapsed cycles | `94,878,363,108` | `-26.13%` |
| wall time | `24.88 s` | `-37.83%`, informational under host load |

Evidence: `/tmp/cppgm-backend-metadata-profile.sample.txt`,
`/tmp/cppgm-lazy-reserve-zero-screen.json`,
`/tmp/cppgm-lazy-reserve-zero-final.json`, and the alternating records
`/tmp/cppgm-lazy-reserve-ab-{parent,candidate}-{1,2,3}.json`.

Fifteenth retained Phase 7 slice: constructor and destructor output analyzed
the same function body independently for the Itanium base and complete object
entry points. The compiler already emits an object-symbol alias between those
entry points when the owner has no virtual bases. In that case their typed
children are identical; only root identity metadata differs. The complete
entry now copies the previously emitted base entry, then replaces its symbol,
entry-point tag, and object-alias metadata. Classes with virtual bases and
deleting destructor entries still take the independent semantic path.

The semantic census confirms that the change removes 2,267 body analyses.
Instantiated-template output fell from 3,144 body emissions to 2,439, while
late synthesized output fell from 5,216 to 3,654. The candidate emits the
byte-identical frozen object. PA17 passes `228/228`, configured direct strict
passes `1530/1530`, and the full direct report passes `4863/4863`.

Absolute three-run medians against `42d55c49c`:

| Signal | Candidate | Change |
| --- | ---: | ---: |
| retired instructions | `127,129,199,627` | `-47,028,571,317` (`-27.00%`); `-1.02%` from `8e46f9fc2` |
| maximum RSS | `737,443,840 B` | `-23,760,896 B` (`-3.12%`) |
| peak footprint | `551,108,608 B` | `-17,649,664 B` (`-3.10%`) |
| wall time | `30.75 s` | `-23.16%`, informational under host load |

Evidence: `/tmp/cppgm-special-member-base-body-reuse-screen.json`,
`/tmp/cppgm-special-member-base-body-reuse-stats.stderr`, and
`/tmp/cppgm-special-member-base-body-reuse-final.json`.

The post-slice full-run sample at
`/tmp/cppgm-post-special-reuse-profile.sample.txt` contains 19,671 main-thread
samples and emits the frozen object. Type parsing and dependent resolution
remain the largest compiler-owned stacks: `lookup_type_from_ast_node` has
2,499 inclusive samples, `parse_type_id_ast` has 1,529,
`resolve_instantiated_dependent_type` has 1,508, and
`resolve_instantiated_dependent_type_uncached` has 1,276. The allocation and
string-comparison leaves remain broad costs rather than one untested owner.

Sixteenth retained Phase 7 slice: dependent named-type resolution copied or
formatted a recursion key and inserted it into a tree on every active call.
The replacement guard keeps the active path in a thread-local vector. Nonempty
names retain the old scope-and-string equality rule; unnamed types retain the
old scope-and-type-identity rule. Nested resolution unwinds in stack order, so
an active frame removes the final entry without a lookup.

The same path scanned structured type spelling for local dependent
placeholders before checking whether a template parameter had an exact bound.
An exact concrete result or exact dependent result is decisive, so those cases
now return before the scan. Unresolved names still run the original scan and
all following resolution paths.

A temporary frozen census measured 221,628 guard calls, 40,689 inline probes,
782 recursion rejections, and a maximum depth of four. Exact binding avoided
95,972 placeholder scans: 16,440 resolved exact bindings and 79,532 exact
dependent bindings. The remaining 124,874 paths still performed the scan. The
instrumentation was removed before commit.

The two parts screened below the retention floor in isolation. Their combined
three-pair binary comparison measured parent median `126,974,551,093` and
candidate median `126,253,188,892`, a `0.568%` reduction. Median RSS changed
from `738,619,392 B` to `735,580,160 B`; footprint changed from
`550,838,272 B` to `550,793,216 B`. All six outputs had the frozen SHA-256.
Focused PA22 and PA24 validation passes `730/730`, configured direct strict
passes `1530/1530`, and the full direct report, including PA9 through its
normal lane, passes `4863/4863`.

Absolute three-run medians against `42d55c49c`:

| Signal | Candidate | Change |
| --- | ---: | ---: |
| retired instructions | `126,447,955,718` | `-47,709,815,226` (`-27.39%`); `-0.536%` from `a74749dcd` |
| maximum RSS | `734,490,624 B` | `-26,714,112 B` (`-3.51%`) |
| peak footprint | `550,830,080 B` | `-17,928,192 B` (`-3.15%`) |
| elapsed cycles | `92,892,551,820` | `-27.68%` |
| wall time | `24.31 s` | `-39.26%`, informational under host load |

Evidence: `/tmp/cppgm-dependent-name-fast-path-combined-screen.time`,
`/tmp/cppgm-dependent-name-fast-path-census.stderr`,
`/tmp/cppgm-dependent-name-combined-ab-{parent,candidate}-{1,2,3}.time`,
`/tmp/cppgm-dependent-name-fast-path-focused.log`,
`/tmp/cppgm-dependent-name-fast-path-test-strict.log`,
`/tmp/cppgm-dependent-name-fast-path-test-report.log`, and
`/tmp/cppgm-dependent-name-fast-path-final.json`.

Seventeenth retained Phase 7 slice: the post-dependent-name profile still
placed `memcmp` first among non-allocation leaves with 917 samples. The earlier
type-equality census measured 911,499 named-type structural comparisons. Only
six unequal spellings matched through elaborated-prefix compatibility, but
every unequal spelling still had to classify prefixes and compare character
ranges.

Each named type now retains an interned identity for its prefix-stripped key
and the corresponding elaborated-prefix category. Named equality compares the
identity pointers, then preserves the existing rule that an unprefixed name
matches a class, struct, union, or enum spelling while incompatible prefixed
categories remain unequal. The original key string remains authoritative for
lookup, diagnostics, mangling, and output. A single setter refreshes the
identity at construction and at the four post-construction key rewrite sites.

Two local-named-type collectors also used tree sets of copied key strings and
class pointers while walking the same identities. They now keep their small
per-operation visit populations in contiguous vectors, comparing the interned
key pointer or `ClassInfo` pointer. The fresh profile attributed 46 immediate
allocation samples to the template-instantiation collector before this
change.

The identity-only screen used `126,014,200,185` instructions. Adding the two
collectors improved the combined screen to `125,384,775,343`. A three-pair
binary comparison measured parent median `126,417,396,602` and candidate
median `125,607,532,538`, a `0.641%` reduction. Paired median RSS increased
from `731,701,248 B` to `743,436,288 B`, and footprint increased from
`550,633,472 B` to `554,647,552 B`; both remain below the original frozen
baseline. All six paired objects had the frozen SHA-256. Configured direct
strict passes `1530/1530`, and the full direct report, including PA9 through
its normal lane, passes `4863/4863`.

Absolute three-run medians against `42d55c49c`:

| Signal | Candidate | Change |
| --- | ---: | ---: |
| retired instructions | `125,553,969,817` | `-48,603,801,127` (`-27.91%`); `-0.707%` from `d5900e76d` |
| maximum RSS | `730,492,928 B` | `-30,711,808 B` (`-4.03%`) |
| peak footprint | `554,708,992 B` | `-14,049,280 B` (`-2.47%`) |
| elapsed cycles | `92,352,778,977` | `-28.10%` |
| wall time | `24.19 s` | `-39.56%`, informational under host load |

The halving target still requires `38,475,084,345` fewer instructions, or a
`30.64%` reduction from this checkpoint.

Evidence: `/tmp/cppgm-post-dependent-name-profile.sample.txt`,
`/tmp/cppgm-named-key-identity-screen.json`,
`/tmp/cppgm-named-key-identity-collectors-screen.json`,
`/tmp/cppgm-named-identity-ab-{parent,candidate}-{1,2,3}.time`,
`/tmp/cppgm-named-key-identity-test-strict.log`,
`/tmp/cppgm-named-key-identity-test-report.log`, and
`/tmp/cppgm-named-key-identity-final.json`.

Eighteenth retained Phase 7 slice: a focused alias census separated the common
libc++ forwarding aliases by dependency state. It found 3,052 concrete and
4,167 dependent `__enable_if_t` uses, plus 925 concrete and 852 dependent
`__void_t` uses. A fixed-void shortcut was exact but flat, so the retained work
targets the larger `enable_if` population.

After normal template-argument resolution has proved both arguments concrete,
the compiler now recognizes only the canonical standard-library
`enable_if<condition, type>::type` forwarding pattern. A true condition returns
the already resolved type argument without building an alias instantiation
scope, generating a string cache key, or parsing the forwarding type-id. A
false condition raises the expected substitution failure. Dependent arguments
remain on the general alias path, and witness capture disables the shortcut so
source-event construction stays unchanged.

Each immutable alias declaration memoizes its structural classification in one
byte. A temporary stats build measured 11,769 classification lookups, 11,328
hits, 441 misses and entries, and zero invalidations. The shortcut returned
1,779 types and raised 1,273 expected SFINAE failures. The stats build emitted
the frozen object exactly; its counters were removed from the normal release
layout after the census.

The frozen object remains byte-identical with SHA-256
`4fc1303ac95464ca600a882acc5f7489e021daf265e64c251c5db51b708c55c4`.
Focused PA23 and PA24 direct validation passes `822/822`. Configured direct
strict passes `1530/1530`, and the full direct report, including PA9 through
its normal lane, passes `4863/4863`.

Absolute three-run medians against `42d55c49c`:

| Signal | Candidate | Change |
| --- | ---: | ---: |
| retired instructions | `124,285,854,757` | `-49,871,916,187` (`-28.64%`); `-1.010%` from `2b72a6b7b` |
| maximum RSS | `740,732,928 B` | `-20,471,808 B` (`-2.69%`) |
| peak footprint | `554,344,448 B` | `-14,413,824 B` (`-2.53%`) |
| elapsed cycles | `92,157,627,550` | `-28.25%` |
| wall time | `23.90 s` | `-40.28%`, informational under host load |

The halving target still requires `37,206,969,285` fewer instructions, or a
`29.94%` reduction from this checkpoint.

Evidence: `/tmp/cppgm-alias-fast-path-census.stderr`,
`/tmp/cppgm-void-alias-fast-path-screen.json`,
`/tmp/cppgm-enable-if-alias-stats.stderr`,
`/tmp/cppgm-enable-if-focused-report.log`,
`/tmp/cppgm-enable-if-alias-test-strict.log`,
`/tmp/cppgm-enable-if-alias-test-report.log`, and
`/tmp/cppgm-enable-if-alias-final.json`.

#### Post-enable-if profile refresh

A 30-second sample at `414b12972` captured 19,927 main-thread samples in
`/tmp/cppgm-current-profile.sample.txt`. Allocation routines remain the
largest leaf family. Semantic output dominates the owned stacks: function
output contributed 10,116 samples through statement analysis, while expression
analysis split its largest child stacks between binary expressions at 8,177
and call expressions at 6,763. The sample attributes about 15% of the run to
LowIR and native output, so the next large reduction must remove semantic work.

The matching phase report at `/tmp/cppgm-current-phases.stderr` records 6,522
function-body emissions and 87,018 expression analyses across output phases.
The compiler completed 3,146 classes; 62,051 of its 70,079 completion calls
returned an existing complete class. Its output queues ran five rounds,
but each round emitted new work, and the earlier pending-reason census found no
large retry population.

The memory census at `/tmp/cppgm-current-memory-census.stderr` found 358,362
source AST nodes, 101,042 types, 201,463 retained CallSem nodes, 36,327 scopes,
33,713 function bindings, and 7,583 classes. Subsequent layout and lazy-storage
screens reduced memory but missed the instruction floor. After the retained
registration-time synthesized-linkage change, `3686d87b0` is the current
decision baseline at `118,245,739,070` instructions. Reaching the halving
target requires another `31,166,853,598` instructions, or `26.36%` of the
current total.

#### Release frame-pointer checkpoint

Homebrew clang kept frame pointers in the optimized macOS build even at
`-O3`. An isolated `-fomit-frame-pointer` build reduced a one-run screen to
`120,116,240,309` instructions and preserved the frozen object. The retained
release-only flag then confirmed at a three-run median of `120,162,630,879`,
`3.318%` below `cfc773417` and `31.00%` below the original baseline. Debug
builds retain `-fno-omit-frame-pointer` for stack inspection.

The direct strict gate passed `1530/1530`, and the direct full report passed
`4863/4863`. Evidence is in
`/tmp/cppgm-omit-frame-pointer-screen.json`,
`/tmp/cppgm-omit-frame-pointer-final.json`,
`/tmp/cppgm-omit-frame-pointer-test-strict.log`, and
`/tmp/cppgm-omit-frame-pointer-test-report.log`.

#### Template-parameter type canonicalization checkpoint

The exact-string census showed thousands of repeated template-parameter type
keys. These types already compare by semantic payload and are fully initialized
by `make_template_parameter_type`. The retained cache shares a type only when
its normalized display, semantic payload, and source name all match, preserving
diagnostic and witness spelling.

A stats-only census recorded 62,264 probes, 61,675 hits, 589 misses and
entries, and no stale evictions. The clean three-run median is
`119,571,899,320` instructions, `0.492%` below `32f889b69` and `31.34%` below
the original baseline. Maximum RSS fell to `711,860,224 B`, and footprint fell
to `524,099,584 B`, about 30 MiB below the prior checkpoint. The result is
retained as an infrastructure exception to the `0.5%` floor because it removes
61,675 duplicate type constructions, establishes canonical pointer identity,
and materially reduces both memory signals.

The frozen object remained exact. The direct strict gate passed `1530/1530`,
and the direct full report passed `4863/4863`. Evidence is in
`/tmp/cppgm-template-parameter-type-intern-census.stderr`,
`/tmp/cppgm-template-parameter-type-intern-final.json`,
`/tmp/cppgm-template-parameter-type-intern-test-strict.log`, and
`/tmp/cppgm-template-parameter-type-intern-test-report.log`.

#### Registration-time synthesized-linkage checkpoint

The current profile attributed about 1,697 inclusive samples to function-symbol
identity construction, including 595 beneath linkage upgrades. Eight implicit
or inherited special-member paths registered a function with provisional
linkage and then immediately regenerated its symbol with the already-known
final synthesized linkage. The retained change supplies that linkage in the
registration request and removes the redundant upgrade. It adds no cache or
invalidation state.

The clean three-run median is `118,245,739,070` instructions, `1.109%` below
`56128c65d` and `32.10%` below the original baseline. Maximum RSS is
`700,035,072 B`, and footprint is `524,410,880 B`. The frozen object remained
exact with SHA-256 `4fc1303ac95464ca600a882acc5f7489e021daf265e64c251c5db51b708c55c4`.
The direct strict gate passed `1530/1530`, and the direct full report passed
`4863/4863`.

Evidence is in `/tmp/cppgm-post-template-type-frame-profile-2.sample.txt`,
`/tmp/cppgm-synthesized-linkage-registration-screen.json`,
`/tmp/cppgm-synthesized-linkage-registration-final.json`,
`/tmp/cppgm-synthesized-linkage-registration-strict.log`, and
`/tmp/cppgm-synthesized-linkage-registration-test-report.log`.

#### Expected semantic-probe status checkpoint

The exception census at the retained parent counted 6,138 throws. Expected
constructor probes accounted for 3,320 `NoViableConstructorError` throws:
3,309 user-defined conversion constructor probes and 11 implicit default-
constructor viability checks. The constructor service now marks those two
selection profiles as expected failures, allowing the overload selector to
return its existing null result before formatting and throwing the diagnostic.
The one functional-cast failure remains an exception.

False direct standard `enable_if` conditions accounted for another 1,273
substitution failures. Earlier broad status experiments changed overload
selection by suppressing nested recoverable SFINAE. The retained receiver is
owned by the non-type-parameter resolver that previously caught the exception,
and an alias-instantiation depth guard permits only the receiver's outermost
alias call to report status. It removes 1,145 throws while leaving 128 nested
false-`enable_if` exceptions on their original recovery path. The combined
census reports 1,673 total throws, including one constructor failure.

A one-run composite screen used `117,580,638,540` instructions. The decisive
three-pair comparison measured parent median `118,649,290,551` and candidate
median `117,493,606,873`, a `0.974%` reduction, with all three pairs agreeing.
Median RSS improved from `706,756,608 B` to `704,536,576 B`; footprint changed
from `524,627,968 B` to `524,775,424 B`, a `0.028%` increase inside the hard
gate. All six paired objects had the frozen SHA-256.

The clean post-commit three-run median is `117,674,519,926` instructions,
`32.43%` below the original baseline. Maximum RSS is `712,704,000 B`, and
footprint is `524,689,408 B`. The contemporaneous paired result, rather than
the non-contemporaneous absolute checkpoint difference, supplies the CPU-lane
retention decision. Direct strict passes `1530/1530`, and the full direct
report passes `4863/4863`.

Evidence is in `/tmp/cppgm-exception-census-messages.stderr`,
`/tmp/cppgm-exception-status-composite-census.stderr`,
`/tmp/cppgm-exception-status-composite-screen.json`,
`/tmp/cppgm-exception-{parent,candidate}-{1,2,3}.time`,
`/tmp/cppgm-exception-status-composite-final.json`,
`/tmp/cppgm-exception-status-composite-test-strict.log`, and
`/tmp/cppgm-exception-status-composite-test-report.log`.

#### Template-body scope-view and bulk-sort checkpoint

The earlier source-view retry cached `(Atom, ValueBinding *)` entries but fed
all 1,825,503 replayed entries through ordered vector insertion. The retained
composite reuses that live mapped-value view, appends each scope chain as one
batch, and sorts and deduplicates the destination once. Map insertions change
the cached count. Scope copy, move, erase, clear, and swap paths discard the
view when they can invalidate its node pointers.

The first three-pair batch measured parent and candidate medians of
`117,562,208,035` and `116,991,657,009` instructions, a `0.485%` reduction.
Median RSS improved by `0.394%`; footprint increased by `0.215%`; the balance
score was `0.682`. All three instruction and RSS pairs agreed. The independent
confirmation measured a `0.474%` instruction reduction, a `0.598%` RSS
improvement, and a `0.190%` footprint increase. Two of three RSS pairs agreed,
and the balance score was `0.773`. Both batches meet the balanced lane, and all
twelve objects have the frozen SHA-256.

The clean three-run median at `2ba26c3f4` is `116,933,622,740` instructions,
`32.86%` below the original baseline. Maximum RSS is `718,651,392 B`, and
footprint is `525,627,392 B`. The focused PA19 report passes `295/295`, direct
strict passes `1530/1530`, and the full direct report passes `4863/4863`.

Evidence is in `/tmp/cppgm-template-visible-composite-once-screen.json`,
`/tmp/cppgm-template-visible-composite-{parent,candidate}-{1,2,3}.time`,
`/tmp/cppgm-template-visible-composite-{parent,candidate}-{4,5,6}.time`,
`/tmp/cppgm-template-visible-composite-final.json`,
`/tmp/cppgm-template-visible-composite-test-pa19.log`,
`/tmp/cppgm-template-visible-composite-test-strict.log`, and
`/tmp/cppgm-template-visible-composite-test-report.log`.

#### Template-parameter placeholder-identity checkpoint

The frozen census counted `1,329,940` calls to
`find_template_parameter(TypePtr)`, including `1,197,826` named inputs. Lists
with one or two parameters accounted for `1,053,259` calls. The type overload
found `240,112` parameters: `239,256` from the named key, `844` from the
semantic payload, and `12` from the source spelling. Each named-key hit shared
an interned identity with its parameter placeholder.

The retained path stores that identity on `TemplateParameterInfo`, preserves
it through copy, move, and redeclaration merges, and checks pointer identity
before string lookup. It requires an unprefixed named key so elaborated
class/enum normalization cannot create a false match. The fallback path
borrows the three type strings and handles the remaining `856` hits.

The first guarded three-pair batch improved instructions by `0.197%`, RSS by
`0.814%`, and regressed footprint by `0.120%`, producing a `0.604` balance
score. The independent batch improved instructions by `0.828%`, RSS by
`0.257%`, and regressed footprint by `0.038%`. All pair directions agreed for
instructions, and all twelve objects had the frozen SHA-256.

The clean three-run median at `0e5b1af2d` is `116,375,621,217` instructions,
`33.18%` below the original baseline. Maximum RSS is `708,911,104 B`, and
footprint is `525,758,464 B`. Direct strict passes `1530/1530`; the full direct
report passes `4863/4863`.

Evidence is in `/tmp/cppgm-template-parameter-identity-census.stderr`,
`/tmp/cppgm-template-parameter-identity-current-screen.json`,
`/tmp/cppgm-template-parameter-identity-guarded-{parent,candidate}-{1,2,3}.time`,
`/tmp/cppgm-template-parameter-identity-guarded-{parent,candidate}-{4,5,6}.time`,
`/tmp/cppgm-template-parameter-identity-final.json`,
`/tmp/cppgm-template-parameter-identity-guarded-test-strict.log`, and
`/tmp/cppgm-template-parameter-identity-guarded-test-report.log`.

#### `CppAstNode` layout checkpoint

The declaration-only field reorder reduces `CppAstNode` from 192 to 176 bytes.
It does not change the type or ownership of any field, and the source has no
aggregate `CppAstNode` initializers whose meaning could depend on declaration
order. A host-compiler assertion confirmed the 176-byte layout without making
that libc++-specific size a portable source requirement.

Three current interleaved pairs measured parent and candidate instruction
medians of `116,150,212,823` and `116,057,787,624`, a `0.080%` improvement.
Median RSS fell from `711,909,376 B` to `702,754,816 B`, or `1.286%`.
Footprint fell from `525,848,576 B` to `516,509,696 B`, or `1.776%` and
`9,338,880 B`. The footprint result independently meets the memory-density
lane, so RSS is supporting evidence rather than the deciding signal. All six
objects had the frozen SHA-256.

The clean three-run median at `4d0fddd3e` is `116,167,632,347` instructions,
`33.30%` below the original baseline. Maximum RSS is `701,468,672 B`, and
footprint is `516,448,256 B`. The focused PA10 report passes `157/157`, direct
strict passes `1530/1530`, and the full direct report passes `4863/4863`.

Evidence is in `/tmp/cppgm-ast-layout-current-screen.time`,
`/tmp/cppgm-ast-layout-{parent,candidate}-{1,2,3}.time`,
`/tmp/cppgm-ast-layout-final.json`,
`/tmp/cppgm-ast-layout-test-pa10.log`,
`/tmp/cppgm-ast-layout-test-strict.log`, and
`/tmp/cppgm-ast-layout-test-report.log`.

#### Primary-placeholder shape checkpoint

The revised leaf census found `760,824` calls that checked whether a class
instantiation's arguments were the source template's own placeholders. Those
calls covered only `4,132` candidate classes. The retained three-state byte is
valid only after both private callers establish source-template identity. The
two argument-state writers reset it before changing the argument vector.

An environment-gated cache census measured `754,890` hits and `5,934` misses.
Only `15` first computations were true; `5,919` were false. The writers made
`20,914` invalidation calls, including `1,928` that cleared a live result. The
census path was removed before performance and correctness validation.

Three interleaved pairs measured parent and candidate instruction medians of
`116,217,254,527` and `115,590,286,386`, a `0.539%` improvement. All three
instruction pairs agreed. Median RSS improved by `0.420%`, and footprint
improved by `0.006%`. Every object had the frozen SHA-256, so the candidate
meets the CPU lane without relying on memory noise.

A broader form also retained a positive dependent source-owner pointer. Its
first paired batch had a `0.948` balance score, but the required independent
batch scored only `0.406` and regressed RSS by `0.440%`. That pointer was
removed; null and source-owner transitions still run through the original map
search.

The clean three-run median at `9ad60e6c6` is `115,576,376,842` instructions,
`33.64%` below the original baseline. Maximum RSS is `707,428,352 B`, and
footprint is `516,362,240 B`. The focused PA19 report passes `295/295`, direct
strict passes `1530/1530`, and the full direct report passes `4863/4863`.

Evidence is in `/tmp/cppgm-revised-profile.sample.txt`,
`/tmp/cppgm-revised-leaf-census.stderr`,
`/tmp/cppgm-primary-placeholder-census.stderr`,
`/tmp/cppgm-primary-placeholder-byte-{parent,candidate}-{1,2,3}.time`,
`/tmp/cppgm-primary-placeholder-final.json`,
`/tmp/cppgm-primary-placeholder-test-pa19.log`,
`/tmp/cppgm-primary-placeholder-test-strict.log`, and
`/tmp/cppgm-primary-placeholder-test-report.log`.

#### Named-type display view checkpoint

The revised leaf census also found `927,511` named-type display requests over
only `5,158` distinct types. `922,353` requests repeated a prior result, no
result changed, and the returned text totalled `139,245,070` bytes. The two
older experiments kept every compacted display eagerly but still returned it
by value. They paid for the common result copy and therefore measured flat.

The retained form leaves display compaction in place. On first demand it
reconstructs the text into the existing `Type::named_display` string, then
returns a borrowed reference on that and later requests. The reference is
owned by the stripped named type and is valid while that type graph is alive.
No caller stores the reference beyond the owning expression. This removes
both repeated reconstruction and the by-value copy without adding a table,
key, epoch, or invalidation path.

Three interleaved pairs measured parent and candidate instruction medians of
`115,568,617,192` and `112,226,094,977`, a `2.892%` improvement. All three
instruction pairs agreed. Median RSS regressed by `0.188%`, and footprint
regressed by `0.306%`; both remain inside the CPU lane's hard gates. Every
object had the frozen SHA-256.

The clean three-run median at `8d58a3d6a` is `112,101,667,646` instructions,
`35.63%` below the original baseline. Maximum RSS is `703,864,832 B`, and
footprint is `517,873,664 B`. The focused PA14 report passes `111/111`, direct
strict passes `1530/1530`, and the full direct report passes `4863/4863`.

Evidence is in `/tmp/cppgm-revised-profile.sample.txt`,
`/tmp/cppgm-revised-leaf-census.stderr`,
`/tmp/cppgm-display-view-screen.json`,
`/tmp/cppgm-display-view-{parent,candidate}-{1,2,3}.time`,
`/tmp/cppgm-display-view-final.json`,
`/tmp/cppgm-display-view-test-pa14.log`,
`/tmp/cppgm-display-view-test-strict.log`, and
`/tmp/cppgm-display-view-test-report.log`.

#### Inline-namespace directive checkpoint

The post-display sample exposed `import_inline_namespace_members` as a new
leaf. A frozen census counted only `556` import calls, but each reopen replayed
the inline namespace's complete accumulated contents into its parent. Across
the run that meant scanning `1,855` namespace entries, `61,162` named types,
`7,442` values, `192,368` function bindings, `118,067` class templates,
`214,976` function-template entries, `46,257` alias templates, and `24,313`
variable templates. A smaller change that replaced `count` plus `operator[]`
with one map insertion emitted exact bytes but regressed instructions to
`112,236,996,211`.

Inline-namespace visibility is already represented by the implicit
using-directive. The retained form stops copying bindings into the enclosing
namespace and leaves lookup to the existing inline-child and using-directive
walkers. It still bumps the parent's binding epoch after every completed
inline-namespace block, so parent-scoped template caches observe declarations
added on a reopen. Removing the copies exposed one ownership bug in the first
full report: `int X::deep` created a second `X::deep` when the prior declaration
belonged to `X::Y::Z` through nested inline namespaces. Qualified variable
parse-scope resolution now follows the found binding back to its real
declaration scope. The existing PA7 reducer
`280-inline-namespace-qualified-lookup.t` catches this case.

Three final interleaved pairs measured parent and candidate instruction
medians of `112,347,566,251` and `111,103,959,589`, a `1.107%` improvement.
All three pairs agreed, at `0.920%`, `1.191%`, and `1.122%`. Median cycles
improved by `1.591%`; median RSS fell by `7,409,664 B`, or `1.035%`; and
footprint fell by `204,800 B`, or `0.040%`. Median real and user time improved
by `4.024%` and `3.821%`, although wall time remains advisory. Every paired
object had the frozen SHA-256. This clears the CPU lane and improves both
memory signals.

The clean three-run median at `589b40ac8` is `111,031,651,654` instructions,
`36.25%` below the original baseline. Maximum RSS is `712,437,760 B`, and
footprint is `517,394,432 B`. PA7 passes `41/41`, direct strict passes
`1530/1530`, and the full direct report passes `4863/4863`.

Evidence is in `/tmp/cppgm-post-display-profile.sample.txt`,
`/tmp/cppgm-inline-namespace-import-census.stderr`,
`/tmp/cppgm-inline-namespace-single-probe-screen.json`,
`/tmp/cppgm-inline-namespace-directive-only-screen.json`,
`/tmp/cppgm-inline-namespace-conservative-screen.json`,
`/tmp/cppgm-inline-final-{parent,candidate}-{1,2,3}.time`, and
`/tmp/cppgm-inline-namespace-directive-final.json`.

#### Contiguous parser lookup-snapshot checkpoint

The first post-inline sample exposed
`CppAstParser::snapshot_name_lookup_state` as a new allocation-heavy leaf.
The frozen compile takes `6,746` snapshots. They retain `25,458` stack scopes
with `41,846` name atoms and `10,501` namespace scopes with `16,426` name
atoms. The old snapshot representation placed every retained scope in its own
`unordered_set`, even though a snapshot is immutable and is read only when a
lazy function body restores the parser's live lookup state.

Two intermediate representations were rejected under the multi-signal rubric,
not the former instruction-only rule. A vector of atoms per scope reduced RSS
and footprint but regressed instructions by about `0.14%`; its footprint gain
was only about `0.26%`, so it missed the memory-density lane. Flattening the
atoms while retaining separate boundary vectors improved instructions by
`0.109%` and footprint by `0.424%`, but regressed RSS by `1.31%`; its score and
instruction component both missed the balanced lane.

The retained form stores each stack as one null-delimited atom vector plus a
scope count. Namespace snapshots similarly pair one vector of scope names with
one null-delimited atom vector. A null atom cannot be an interned identifier,
so the delimiter does not widen the name domain. Snapshot creation owns the
vectors, snapshots remain immutable, and restoration rebuilds the preexisting
live `NameSet` hash tables before lookup resumes. There is no cache or
invalidation state.

Three interleaved pairs measured parent and candidate instruction medians of
`111,148,795,852` and `110,922,055,728`, a `0.204%` improvement. All three
pairs agreed. Median footprint fell by `3,117,056 B`, or `0.602%`, and median
RSS fell by `5,939,200 B`, or `0.846%`. The balance score is `0.806`, so the
candidate clears the balanced CPU-and-memory lane without relying on RSS as
the decisive memory signal. Cycles regressed by `0.478%`; that advisory result
does not cancel the exact instruction and footprint gains, but it prevents an
allocation-and-latency claim.

The clean three-run median at `9bcbc6fc6` is `110,890,016,889` instructions,
`36.33%` below the original baseline. Maximum RSS is `696,147,968 B`, and
footprint is `514,641,920 B`. PA19 passes `295/295`, direct strict passes
`1530/1530`, and the full direct report passes `4863/4863`.

Evidence is in `/tmp/cppgm-post-inline-profile.sample.txt`,
`/tmp/cppgm-compact-snapshot-census.stderr`,
`/tmp/cppgm-compact-lookup-snapshot-screen.json`,
`/tmp/cppgm-flat-lookup-snapshot-screen.json`,
`/tmp/cppgm-flat-snapshot-{parent,candidate}-{1,2,3}.time`,
`/tmp/cppgm-delimited-lookup-snapshot-screen.json`,
`/tmp/cppgm-delimited-snapshot-{parent,candidate}-{1,2,3}.time`, and
`/tmp/cppgm-lookup-snapshot-final.json`.

#### Multi-signal retention rubric, adopted at `3686d87b0`

The former rolling `0.5%` instruction floor was useful for rejecting noise, but
it was too blunt. It undervalued changes that reduce CPU work and memory
together, and it could not recognize an allocator change whose benefit appears
in CPU time rather than retired instructions. Use the following rubric for new
work and for retries from the rejected-work ledger.

Every lane has the same hard gates:

- the frozen object has SHA-256 `4fc1303a...5c4`;
- focused tests pass, followed by direct strict `1530/1530` and full report
  `4863/4863` before retention;
- the normal portable release build remains the measured configuration, with
  no reduced hardening or host-specific target requirement;
- a non-CPU lane may regress instructions by at most `0.15%`; every lane caps
  peak-footprint regression at `1%` and confirmed RSS regression at `3%`;
- a cache has a measured key, hit, miss, entry, and invalidation population;
  allocation claims count calls and requested bytes before screening; and
- the implementation has an explicit lifetime and invalidation contract. A
  candidate that crosses a numerical lane can still be rejected for a concrete
  correctness or maintenance cost, but not merely because it misses the old
  instruction-only floor.

A one-run screen can reject an obvious loss, but it cannot retain a change.
Retention uses at least three sequential interleaved parent/candidate pairs.
The median must move in the claimed direction and at least two of the three
pairs must agree. When RSS supplies the decisive benefit, run a second
independent three-pair batch because RSS is the noisiest memory signal.

For the balanced lane, define `I`, `F`, and `R` as the percentage reductions in
retired instructions, peak footprint, and maximum RSS. Positive values are
improvements. The balance score is:

```text
I + max(F, 0.5 * R)
```

Taking the larger memory term avoids counting the same storage twice. RSS has
half weight because it is less stable than the macOS footprint counter. A
candidate qualifies through one of these lanes:

| Lane | Acceptance rule | Required evidence |
| --- | --- | --- |
| CPU throughput | `I >= 0.50%` | Three interleaved pairs; memory stays inside the hard gates. |
| balanced CPU and memory | `I >= 0.15%`, neither memory signal regresses by more than `0.25%`, and the balance score is at least `0.50` | Three interleaved pairs; repeat the batch when RSS, rather than footprint, makes the score cross `0.50`. |
| memory density | instruction regression is no worse than `0.15%`, and either footprint improves by at least `1%` and `4 MiB`, or footprint improves by at least `0.5%` while confirmed RSS improves by at least `1%` | Three interleaved pairs, retained-size census, and an independent RSS confirmation when the second form is used. |
| allocation and latency | at least `100,000` allocation/deallocation calls or `8 MiB` of requested allocation traffic disappear; instruction and memory hard gates hold; quiet median wall time improves by at least `1%`, with user time or cycles agreeing | Five quiet interleaved pairs plus the allocation census. Allocation count by itself is not enough. |

The allocation lane treats removed allocator traffic as a reason to measure,
not as proof of speed. Allocator calls can be cheap, and a pool can trade them
for lookup work or retained pages. This is why a slight instruction regression
is permitted only when quiet end-to-end latency confirms the benefit. The
balanced lane handles the other important case directly: a change can qualify
below the `0.5%` instruction threshold when it improves both CPU work and
memory by a material combined amount.

Two sub-threshold edits may be measured together only when they share a hot
path, state contract, or data representation. The composite must qualify as a
whole. This permits, for example, removing two expected-exception producers
through one status boundary. It does not permit bundling unrelated micro-edits
to manufacture a score.

#### Rejected-work checkpoint review, started at `3686d87b0`

The checkpoint review covered all 138 current rows in the rejected-work
ledger. It used absolute instructions removed, overlap with later retained
work, a fresh release-binary sample, and the multi-signal rubric above. The
denominator change alone does not justify a retry. A rejected form reopens when one of
these conditions holds:

- its prior result reaches a retention lane's decision threshold and needs the
  prescribed paired evidence;
- a retained change supplied an invariant or removed a cost that the rejected
  form needed;
- a fresh census shows a larger population or a changed cost distribution;
- a new form removes setup that made the first form flat.

The former instruction-only floor at this checkpoint was `591,228,695`
instructions. The fresh compile at
`/tmp/cppgm-review-current.o` has the frozen SHA-256. Its phase and sample
records are `/tmp/cppgm-review-current-phases.stderr` and
`/tmp/cppgm-review-current.sample.txt`. The release binary omits frame
pointers, so the fresh sample supplies leaf attribution. The frame-pointer
profile at `/tmp/cppgm-post-template-type-frame-profile-2.sample.txt` supplies
caller attribution for the same source checkpoint before the final linkage
slice.

The closest old results normalize as follows:

| Rejected form | Prior absolute saving | Share of current checkpoint | Review decision |
| --- | ---: | ---: | --- |
| cache an interned contiguous view of each scope's values for template-body validation | `613,276,351` | `0.519%` | Retry with the retained sorted atom destination and the existing scope binding epoch. The old eager form ran before `AtomNameSet` became contiguous. The fresh collector remains a 129-sample leaf. |
| reuse temporary buffers in `trim_space` | `592,264,893` for the best early confirmation | `0.501%` | Do not retry unchanged. A later rvalue-overload form at the 128.445B checkpoint measured flat. Reopen only after a call census identifies a larger temporary-producing family or a form that avoids copying lvalue inputs. |
| consume normalized input with one stream-buffer fetch per byte | `511,468,490` | `0.433%` | Reopen the one-fetch form for a current paired decision. Its historical balance score is `0.564`; the 8 KiB buffered form remains closed. Preserve the trigraph and UTF-8 lookahead protocol. |
| return exact false `enable_if` probes as a status | `495,112,821` for the best byte-exact form | `0.419%` | Redesign the status boundary. The fresh sample has 92 `__cxa_throw` leaves. Count all expected substitution failures, then carry status to the precise catch owner without changing nested SFINAE recovery. |
| move completed LowIR records and instruction strings | `457,438,165` | `0.387%` | Do not retry unchanged. Later backend borrowing overlaps the ownership work, and LowIR plus native output accounts for about 15% of the run. Reopen only with a current copy census. |
| memoize the preceding dependent-resolution root | `425,704,536` | `0.360%` | Reopen after a current repeat-population census. The historical paired balance score is `0.540`, but the retained dependent-name guard and exact-binding reorder overlap this path. |
| pool `CallSemNode` child buffers | `392,794,783` | `0.332%` | Keep the measured slab form rejected: its `1.043%` footprint regression exceeds the hard gate. Reopen only a bounded or promptly released slab form, then require the allocation-and-latency proof. |
| replace the template-angle dense vectors with a sparse cache | `385,352,187` | `0.326%` | Reopen only the 16-entry sparse form for a paired decision; its one-run balance score is `0.537`. The active-key, split, and generation-tagged forms remain closed. |
| borrow strings while finding a template parameter | `383,792,415` | `0.325%` | Reopen the underlying ownership removal through current identities. Its historical paired balance score is `0.793`; census atom-identity matches before adding fields to `TemplateParameterInfo`. |
| skip alias source-occurrence materialization | `379,275,374` | `0.321%` | Keep closed. The later, broader witness-disabled bypass measured flat at the current checkpoint. |

Two earlier below-floor pieces already demonstrate the right combination rule.
The dependent-name LIFO guard saved `0.478%`, and the exact-binding reorder
saved `0.338%`. Their shared resolver slice saved `0.568%` and became
`d5900e76d`. Template-parameter type canonicalization also changed from a flat
early cache to a retained `0.492%` slice after the key and lifetime contract
were narrowed. These cases support cohesive retries based on new invariants.
They do not support combining unrelated micro-edits to cross the floor.

The full ledger re-audit changes these old decisions:

| Prior rejection | Historical `I / F / R` | Rubric result | Current action |
| --- | ---: | --- | --- |
| one-fetch normalized input | `+0.351 / +0.214 / +0.178%` | balance score `0.564`; historical result was only a screen | The current retry regressed instructions by `0.181%` and RSS by `0.686%`. Keep the two-fetch path. |
| one-entry dependent-resolution root memo | `+0.314 / +0.090 / +0.451%` | balance score `0.540`; paired evidence existed | The current census found the same population, but the retry scored only `0.106`. Keep the direct path. |
| borrow template-parameter candidate strings | `+0.283 / -0.041 / +1.019%` | balance score `0.793`; RSS needed independent confirmation | The current identity-based form passed the balanced and CPU lanes and was retained in `0e5b1af2d`. |
| bulk-sort visible template-body names | `+0.236 / +0.066 / +0.637%` | balance score `0.555`; historical result was only a screen | The cohesive source-view and destination-sort form passed two balanced batches and was retained in `2ba26c3f4`. |
| 16-entry sparse template-angle cache | `+0.322 / +0.116 / +0.430%` | balance score `0.537`; historical result was only a screen | The current retry scored `0.442`; the packed refinement scored `0.284`. Keep the dense vectors. |
| reorder `CppAstNode` into 176 bytes | `+0.147 / +1.689 / +0.695%` | memory-density lane; historical result was only a screen | Run a current paired decision and retained-size census. |
| compact `CallSemNode` child storage | `-0.022 / +0.563 / +1.169%` | memory-density lane; historical result was only a screen | The current 72-byte retry still missed, but packing the adjacent kind/category fields made the same child-storage design a cohesive 64-byte record. The composite passed the density lane and was retained in `d4fe39d7c`. |
| allocate `Scope::named_type_access` on first use | `+0.170 / +0.058 / +1.237%` | balance score `0.788`; historical result was only a screen and RSS supplied most of the score | The current retry scored `0.083` and regressed instructions and RSS. Keep the direct map. |
| inline `strip_top_level_cv` and return the selected pointer by reference | `+0.165 / -0.074 / +1.980%` | balance score `1.155`; historical result was only a screen and RSS supplied most of the score | The current retry improved instructions in all three pairs but scored only `0.225` because memory was flat to worse. Keep the value-returning API. |
| retain compacted named-type display strings | `+0.035 / unknown / negative one-run RSS` | misses every lane in its original by-value form | Reopen through a new ownership form after the fresh census found `922,353` stable repeats. The borrowed lazy view removes reconstruction and the return copy, clears the CPU lane at `2.892%`, and is retained in `8d58a3d6a`. |

Three historical results need no retry. The early by-value trim result has a
`0.618` balance score, but the later current-code rvalue screen was flat. The
old `CppAst` bitfield layout meets the density lane but is superseded by the
smaller and faster 176-byte field reorder above. The raw template-parameter
type cache easily meets the density lane, but the narrower retained
canonicalization already supplies that memory saving with an instruction win.

The allocation re-audit does not retroactively accept a custom allocator. The
CallSem child-buffer pool reused `1,570,427` allocation requests and improved
instructions by `0.306%`, but increased footprint by `1.043%` and has no quiet
latency batch. It is a worthwhile design lead for bounded slab retention or
prompt page release, not a passing result. FunctionBinding pooling, alias
source materialization, and zero-allocation lookup walkers lack the required
allocation volume or quiet latency evidence and remain closed.

The latest rows remain closed. ABI type-IR borrowing, substitution child
moves, alias-observation bypass, early negative class admission, CallSem
coallocation, and FunctionBinding compaction all ran against the 118.246B
checkpoint. Each form was flat or regressed instructions. Denominator shrink
cannot change those decisions.

The symbol-registration rejections also lost their original population. The
old rehydration census counted 19,000 linkage-upgrade calls. Current metrics
report 1,102 symbol-upgrade requests after registration-time synthesized
linkage. Do not retry the rehydration guard, deferred provisional symbol
construction, or complete-entry symbol reuse without a new census.

#### Packed CallSem output checkpoint

The refined review reopened the one-pointer `CallSemNode` child container. Its
historical 72-byte record reduced memory but missed the density threshold in a
confirmation batch. A fresh adjacent-field audit found the missing cohesive
piece: `CallSemKind` and `CallValueCategory` occupied two full words beside a
partially used 64-bit flag group. Giving both enums an unsigned 64-bit
underlying type and storing them in guarded 7-bit and 2-bit fields lets the
one-pointer child representation reduce `CallSemNode` from 88 bytes to 64.
The unsigned underlying type is required: a signed 7-bit enum field cannot
represent the final five of the current 69 kinds. A compile-time bound now
prevents future kinds from overflowing the field.

`CallSemChildren` owns one allocation containing an eight-byte size/capacity
header followed by the nodes. Copies are deep, moves transfer the allocation,
and `clear` destroys nodes while retaining capacity. Its iterator and
reference invalidation rules match the vector operations it exposes: reserve,
growth, and insert invalidate them. The container has no cache, epoch, pool,
or cross-owner state. It deliberately provides only the operations used by
CallSem construction, so the lifetime contract remains local despite the
larger implementation than `std::vector`.

The retained census at
`/tmp/cppgm-callsem-packed-memory-census.stderr` counts `201,463` output nodes.
Inline node storage falls from `17,728,744 B` to `12,893,632 B`, a
`4,835,112 B` reduction. Child backing storage falls from `18,620,272 B` to
`14,451,376 B`, another `4,168,896 B`. The census therefore attributes
`9,004,008 B` less retained CallSem storage to the composite representation.
The child container does not claim an allocation-count win: a nonempty child
array still uses one allocation, as the old vector buffer did.

Three sequential interleaved pairs measured parent and candidate instruction
medians of `108,423,136,754` and `108,389,359,491`, a `0.031%` improvement;
two instruction pairs agreed. Median footprint fell from `514,498,560 B` to
`509,136,896 B`, a `5,361,664 B` or `1.042%` improvement, and all three pairs
agreed. Median RSS rose `0.334%`, within the hard gate and not used for the
decision. Cycles improved `0.203%` with all three pairs agreeing. This clears
the first memory-density rule without relying on RSS. The post-commit
three-run record is `108,502,199,146` instructions, `696,795,136 B` maximum
RSS, and `509,206,528 B` footprint; a small rolling instruction increase is
allowed because the paired density decision stays inside the `0.15%` CPU cap.

Every measured object has frozen SHA-256 `4fc1303a...5c4`. Focused PA12, PA15,
PA19, and PA22 pass `878/878`; direct strict passes `1530/1530`; and the full
direct report passes `4863/4863`. The portable C++11 release build uses normal
`operator new` and adds no host-specific flags. Evidence is in
`/tmp/cppgm-callsem-kind-flags-screen.json`,
`/tmp/cppgm-callsem-packed-children-screen.json`,
`/tmp/cppgm-callsem-packed-{parent,candidate}-pair{1,2,3}.json`,
`/tmp/cppgm-callsem-packed-memory-census.stderr`, and
`/tmp/cppgm-callsem-packed-children-final.json`.

#### Revised investigation order

The `36.33%` cumulative reduction leaves `23,811,131,417` instructions. A chain
of boundary-sized representation changes will not close that gap. New work
must start with operation counts and favor semantic work removal. Use this
order:

1. Completed: retry the template-body scope-value snapshot on the current
   representation. A node-pointer cache reduced 1,825,503 map-entry scans to
   3,092 first-build scans across 44,552 scope visits. The lean form stored a
   cache for nonempty scopes and reset it at each erase, clear, or swap.
   Three interleaved binary pairs measured `118,654,162,513` instructions for
   the retained compiler and `118,365,277,484` for the candidate, a `0.243%`
   reduction. Both memory signals regressed slightly, giving a balance score
   of only `0.158`; the candidate also misses the other lanes. We rejected the
   source-map snapshot and restored the direct scan.
2. Completed: retain the cohesive expected-exception status slice. The
   constructor-selection boundary removes 3,320 expected throws. The root-only
   standard `enable_if` receiver removes 1,145 more while preserving 128 nested
   recoverable SFINAE throws. The paired composite improves instructions by
   `0.974%`, emits exact bytes, and passes strict `1530/1530` plus full report
   `4863/4863`.
3. Completed: retain the visible-name bulk sort with the scope-value snapshot.
   The composite avoids repeated source interning and per-name destination
   insertion. Two independent three-pair batches produce balance scores of
   `0.682` and `0.773`; every frozen object is exact, and direct strict plus the
   full report pass.
4. Completed: reject the current one-fetch normalizer and template-angle
   retries. One-fetch regressed instructions by `0.181%` and RSS by `0.686%`
   in the current three-pair batch. The 16-entry sparse angle table improved
   instructions by `0.202%` and footprint by `0.241%`, for a `0.442` balance
   score. Packing each angle-cache state into its index word reduced that to a
   `0.105%` instruction gain and a `0.284` score. Every object remained exact;
   we restored both parent implementations.
5. Completed: retain template-parameter placeholder identity. The frozen
   census found `239,256` direct identity hits among `1,329,940` type-overload
   calls. The guarded path keeps the `856` semantic-payload and source-name
   fallbacks. One paired batch meets the balanced lane at `0.604`; an
   independent batch improves instructions by `0.828%` and meets the CPU lane.
6. Completed: reject the current dependent-resolution last-root memo. The
   census found `45,220` consecutive-key hits among `307,202` roots, close to
   the historical `45,005`. Recomputing every hit produced `45,220` identical
   results and no transitions. The current three-pair retry improved median
   instructions by only `0.093%` and footprint by `0.012%`, while RSS regressed
   by `0.704%`. Its `0.106` balance score misses every retention lane, so the
   direct path remains.
7. Completed: reject both balanced-lane omissions from the first ledger audit.
   Reusing `LazyMap` for `Scope::named_type_access` regressed instructions by
   `0.019%` and RSS by `1.685%`; its `0.102%` footprint improvement produced a
   `0.083` score. Inlining `strip_top_level_cv` and returning its selected
   pointer by reference improved instructions by `0.233%`, with all three
   pairs agreeing, but regressed RSS by `0.393%` and footprint by `0.008%`.
   Its `0.225` score does not justify a compiler-wide ownership API change.
8. Completed: retain the 176-byte `CppAstNode` layout and reject compact
   CallSem child storage. The AST reorder improves footprint by `1.776%` and
   `9,338,880 B`, independently meeting the memory-density lane while leaving
   instructions flat. Compact CallSem storage passed the first batch with a
   `1.622%` RSS improvement, but the required independent batch improved RSS
   by only `0.801%`. Its `0.780%` footprint and `0.077%` instruction gains miss
   the other lanes, so `std::vector` remains.
9. Completed: finish the revised profile leaves. Retain the
   primary-placeholder shape byte: `754,890` cache hits clear the CPU lane at
   `0.539%`. Retain the named-type display view: `922,353` stable repeats clear
   the CPU lane at `2.892%` once the API avoids both reconstruction and the
   result copy. Keep the broader source-owner pointer rejected after its
   independent balance score fell to `0.406`. Close snapshot interning: only
   29 of 6,746 snapshots were empty, and sharing the 2,903 repeated contents
   would save roughly 1.1 MiB through another broad cache. Reject overlay
   result caching: only 2,413 of 276,899 stable target/source pairs repeat.
   `107,924` calls have no relevant source names, but an early empty-source
   return improves instructions by only about `0.18%`; moving the guard to the
   wrapper or linearly probing the small exclusion set is worse.
10. Completed: measure the two narrow traversal families and retain neither
   candidate. Conversion-function group and name collection make 45,367 root
   traversals, visit at most four classes, and see no duplicate class or
   virtual-base insertion on the frozen workload. Reusing the inline visit-set
   abstraction nevertheless regresses instructions by `0.26%` to `0.39%`
   across full, capacity-sized, and binding-only forms. Itanium IR substitution
   lookup makes 1,979,226 small-state probes and 11,750,955 structural key
   comparisons. Lowering the vector/hash crossover from 28 to 8 or 16
   regresses instructions by more than `0.6%`; raising it to 40 or 64 remains
   slightly worse. Keep the current tree sets and 28-entry crossover.
11. Completed: refresh the release-binary sample and retain implicit
   inline-namespace visibility without eager binding copies. A census showed
   that 556 namespace imports replayed hundreds of thousands of accumulated
   bindings. The directive-only form initially exposed a qualified-definition
   ownership bug in PA7; resolving the prior value binding's real inline child
   fixed it without restoring copies. Conservative parent cache invalidation
   remains. Three final pairs improve instructions by `1.107%`, cycles by
   `1.591%`, and RSS by `1.035%`; strict and full reports pass.
12. Completed: compact parser lookup snapshots. A census found `6,746`
   immutable snapshots retaining `35,959` small lookup scopes. The final
   null-delimited form removes a hash-table allocation per retained scope and
   clears the balanced lane with a `0.204%` instruction gain, `0.602%`
   footprint gain, `0.846%` RSS gain, and `0.806` balance score. The two
   allocation-heavier vector forms remain rejected under the refined rubric.
13. Completed: refresh the release-binary sample at `9bcbc6fc6`. The exact
   20-second sample contains `15,651` main-thread samples. The dense
   template-angle zero-fill remains the top compiler leaf, but both current
   sparse retries are already closed by paired evidence. Two new
   template-dependence cache families were also rejected. The Analyzer path
   made `2,421,362` calls over `31,801` identities with no observed result
   change, but hashing every recursive call regressed instructions by
   `0.155%`; root-only caching was worse. The template-services path already
   serves `2,752,628` persistent hits for only `27,264` misses. A 16K direct
   front cache intercepted `1,780,998` probes, but retained too many otherwise
   temporary types and regressed to `112,512,270,234` instructions with a
   `529,502,208 B` footprint. A fused placeholder walk, a 1K-entry front cache
   for `top_level_scope_split`, symbol-sanitizer reservations, and metadata-only
   CallSem ranking copies also miss every lane after their required screens or
   paired batches. The CallSem form removed `186,767` child allocations and
   `30,606,928` requested bytes, but five pairs made user and wall time worse;
   this is the allocation lane working as intended rather than treating fewer
   allocations as sufficient proof.
14. Completed: borrow dependent-class template argument metadata for read-only
   consumers. The old accessor made about `382,984` outer-vector allocations
   and copied `244,588,352` requested bytes on one frozen census. The focused
   view conversion leaves `37,258` copies and `22,575,624` requested bytes,
   removing about `345,726` allocations and `222,012,728` bytes without a cache
   or invalidation state. Three interleaved pairs improve instructions by
   `0.863%`, cycles by `0.961%`, user time by `1.051%`, wall time by `1.178%`,
   and RSS by `0.467%`; every CPU and latency pair agrees. Direct strict is
   `1530/1530`, the full report is `4863/4863`, and the final committed record
   is `109,780,001,232` instructions. The cumulative improvement is `36.97%`,
   leaving `22,701,115,760` instructions to the halving target.
15. Completed: refresh the exact release profile and revisit the translation
   pipeline without changing its buffer protocol. The adjacent dependent-alias
   accessor has only `12,257` copying hits and `5,915,856` requested bytes, so
   it remains unchanged. Replacing the existing type-dependency root map with
   per-Type state also remains rejected: a model-pointer form scores only
   `0.396`, and a padding-only atomic epoch form loses the instruction gain. A
   generic tagged source dispatcher regresses instructions by about `4.4%`.
   The retained form instead gives only `UTF8Translator` and `FullTranslator`
   typed references to the same sources already owned by their base class and
   directly implements the existing pop-or-advance rule. The historical
   `?π` failure came from overload resolution selecting `BufferedIterator`'s
   copy constructor when the FullTranslator source became statically typed;
   an explicit `CodePointIterator&` cast selects the intended source-binding
   constructor. The exact UTF-8/trigraph reducer and all 53 PA1 tests pass.
   Three pairs improve instructions by `1.449%`, cycles by `1.412%`, user time
   by `1.141%`, and wall time by `1.085%`, with all four signals agreeing in
   every pair. Direct strict is `1530/1530`, the full report is `4863/4863`,
   and the committed record is `108,407,409,101` instructions. Cumulative
   improvement is `37.75%`; `21,328,523,629` instructions remain.
16. Completed: close the remaining translation-buffer devirtualization forms.
   A minimal outer wrapper was exact, but three interleaved pairs improved
   instructions by only `0.042%` and footprint by `0.025%`, regressed RSS by
   `0.881%`, and improved wall and user time in only one pair. It misses every
   lane. A broader specialization made all five hot buffer operations call
   `FullTranslator` directly and remained exact on the UTF-8/trigraph reducer,
   but its one-run screen regressed instructions by `0.491%` and footprint by
   `0.127%`. Both forms were restored. The retained typed inner sources remove
   the profitable virtual chains without duplicating the buffer implementation.
17. Completed: refresh attribution after closing preprocessing dispatch and
   reject a broad AST type-lookup result cache with current population evidence.
   The exact frame-pointer sample attributes `2,315` of `14,424` samples to
   `lookup_type_from_ast_node` inclusively, but the stability census shows that
   the inclusive cost is mostly distinct semantic work: `199,335` calls contain
   `173,241` first stable observations and only `21,803` stable repeats. There
   are `9,690` third-or-later observations that a conservative promotion cache
   could serve, `85` equal-value results with a different shared pointer, and
   no changed repeated result. A scope-, syntax-, and mutation-validated cache
   limited to non-template qualified nodes emits the frozen object but uses
   `108,712,229,770` instructions, a `0.281%` regression, while footprint also
   regresses `0.432%`. Restore direct lookup. The phase record confirms that the
   next work should remove common unique semantic work: output seed takes
   `6,597 ms`, instantiated-template output `3,451 ms`, and late synthesized
   output `4,058 ms`; LowIR collection is only about `1.4 s`.
18. Completed: retain the cohesive packed CallSem output representation. The
   earlier one-pointer child container missed the density gate at 72 bytes per
   node. Packing the adjacent kind and value-category fields reduces the full
   record from 88 to 64 bytes. The retained census shows `9,004,008 B` less
   CallSem storage. Three interleaved pairs improve median footprint by
   `1.042%` or `5,361,664 B`, keep instructions `0.031%` better, and agree on
   footprint in all pairs and instructions in two. The candidate clears the
   memory-density lane, emits exact bytes, and passes focused `878/878`, direct
   strict `1530/1530`, and full report `4863/4863`. The post-commit record is
   `37.70%` below the original baseline and leaves `21,423,313,674`
   instructions to the halving target.
19. In progress: attribute the common function-body semantic path inside output
   seed and late synthesized output, then census a construction or traversal
   that is paid once for each unique body. Do not reopen broad AST/type caches
   without a new reuse population.

Keep these families closed without new population evidence: broad AST/type
caches, text interner replacements, unrelated container swaps, pool allocators,
alias observation, and LowIR ownership. The packed CallSem exception required
a retained-size census and crossed the density lane as one cohesive record
change; byte or allocation removal alone still does not predict instruction
improvement on this host.

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
- Apply the multi-signal retention rubric. Treat the validation script's
  tolerances as regression guards, not as an instruction-only definition of a
  useful optimization.
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
| `01f875b8f` | remove net-negative dependent-resolution cache and tree-backed recursion guard | `173,508,264,894` | `-0.37%` | `743,952,384` | `557,821,952` | SHA-256 `4fc1303a...5c4` | `1530/1530` | `4863/4863` | `/tmp/cppgm-dependent-resolution-negative-cache-removal-final.json` |
| `4ab86136b` | reuse native temp and forwarded-parameter interval analyses | `166,898,155,701` | `-4.17%` | `745,816,064` | `557,416,448` | SHA-256 `4fc1303a...5c4` | `1530/1530` | `4863/4863` | `/tmp/cppgm-interval-reuse-final.json` |
| `325644977` | intern each visible template-body value name once per scope walk | `165,685,333,837` | `-4.86%` | `749,821,952` | `557,666,304` | SHA-256 `4fc1303a...5c4` | `1530/1530` | `4863/4863` | `/tmp/cppgm-template-body-single-atom-final.json` |
| `773cadc65` | trust existing exported runtime function identities before fallback ownership probes | `164,248,241,098` | `-5.69%` | `748,720,128` | `557,678,592` | SHA-256 `4fc1303a...5c4` | `1530/1530` | `4863/4863` | `/tmp/cppgm-runtime-symbol-identity-fast-path-final.json` |
| `10ab1b728` | avoid materializing empty sparse records in substitution and mangling AST clones | `163,459,605,743` | `-6.14%` | `738,054,144` | `553,398,272` | SHA-256 `4fc1303a...5c4` | `1530/1530` | `4863/4863` | `/tmp/cppgm-sparse-clone-materialization-final.json` |
| `dc49aaa16` | return concrete alias-template cache hits before constructing an instantiation scope | `160,251,233,762` | `-7.99%` | `740,306,944` | `552,783,872` | SHA-256 `4fc1303a...5c4` | `1530/1530` | `4863/4863` | `/tmp/cppgm-alias-concrete-hit-final.json` |
| `a1f5d1db1` | share populated AST side vectors until mutable access | `159,287,615,401` | `-8.54%` | `741,462,016` | `552,189,952` | SHA-256 `4fc1303a...5c4` | `1530/1530` | `4863/4863` | `/tmp/cppgm-ast-lazy-vector-cow-final.json` |
| `c0155750b` | build filtered AST roots without copying discarded children | `158,427,135,579` | `-9.03%` | `734,437,376` | `552,079,360` | SHA-256 `4fc1303a...5c4` | `1530/1530` | `4863/4863` | `/tmp/cppgm-ast-filter-shallow-copy-final.json` |
| `a28f2dda1` | index class reference declarations by member name | `157,041,590,005` | `-9.83%` | `750,534,656` | `555,393,024` | SHA-256 `4fc1303a...5c4` | `1530/1530` | `4863/4863` | `/tmp/cppgm-reference-member-name-index-final.json` |
| `80c3e4d47` | cache stable function-type keys for one LowIR generation | `152,441,499,735` | `-12.47%` | `747,036,672` | `555,773,952` | SHA-256 `4fc1303a...5c4` | `1530/1530` | `4863/4863` | `/tmp/cppgm-stable-function-type-key-cache-final.json` |
| `87f17bd97` | index normalized runtime symbols | `145,884,874,306` | `-16.23%` | `744,771,584` | `556,154,880` | SHA-256 `4fc1303a...5c4` | `1530/1530` | `4863/4863` | `/tmp/cppgm-runtime-symbol-index-final.json` |
| `faf3a29a6` | index function-local type overlays | `145,188,653,153` | `-16.63%` | `740,917,248` | `556,396,544` | SHA-256 `4fc1303a...5c4` | `1530/1530` | `4863/4863` | `/tmp/cppgm-local-type-overlay-index-final.json` |
| `70607afcd` | update function-symbol lookup index incrementally during runtime discovery | `143,082,634,066` | `-17.84%` | `754,479,104` | `556,146,688` | SHA-256 `4fc1303a...5c4` | `1530/1530` | `4863/4863` | `/tmp/cppgm-function-symbol-index-incremental-final.json` |
| `39d241018` | move overload CallSem trees between owners and make `ExprInfo` relocation non-throwing | `142,363,891,265` | `-18.26%` | `747,724,800` | `556,298,240` | SHA-256 `4fc1303a...5c4` | `1530/1530` | `4863/4863` | `/tmp/cppgm-callsem-ownership-moves-final.json` |
| `a268247dc` | guard the function-type pack probe before cloning template argument syntax | `140,749,490,447` | `-19.18%` | `744,112,128` | `556,294,144` | SHA-256 `4fc1303a...5c4` | `1530/1530` | `4863/4863` | `/tmp/cppgm-function-pack-clone-guard-final.json` |
| `4ed53c57e` | index LowIR function-symbol entries by exact name | `138,575,302,610` | `-20.43%` | `742,748,160` | `556,113,920` | SHA-256 `4fc1303a...5c4` | `1530/1530` | `4863/4863` | `/tmp/cppgm-function-symbol-entry-name-index-final.json` |
| `05b1eb38d` | compare elaborated named-type keys without stripped-string temporaries | `137,038,283,448` | `-21.31%` | `745,762,816` | `556,085,248` | SHA-256 `4fc1303a...5c4` | `1530/1530` | `4863/4863` | `/tmp/cppgm-named-key-view-final.json` |
| `1060e05df` | classify elaborated type prefixes by family and reuse temporary input buffers | `136,011,111,282` | `-21.90%` | `735,920,128` | `556,093,440` | SHA-256 `4fc1303a...5c4` | `1530/1530` | `4863/4863` | `/tmp/cppgm-elaborated-prefix-dispatch-rvalue-final.json` |
| `1a96ce861` | use reserved hash indexes for high-volume machine-layout probes | `135,861,805,665` | `-21.99%` | `736,321,536` | `555,786,240` | SHA-256 `4fc1303a...5c4` | `1530/1530` | `4863/4863` | `/tmp/cppgm-layout-lookup-hashes-final.json`; paired A/B: `-0.577%` instructions |
| `70fa6d879` | keep using-directive traversal visited sets inline | `133,973,655,610` | `-23.07%` | `747,077,632` | `555,909,120` | SHA-256 `4fc1303a...5c4` | `1530/1530` | `4863/4863` | `/tmp/cppgm-using-directive-inline-visited-final.json`; paired decision: `-1.103%` instructions |
| `af290029f` | store template-body validation name atoms contiguously | `132,942,123,779` | `-23.67%` | `749,297,664` | `556,949,504` | SHA-256 `4fc1303a...5c4` | `1530/1530` | `4863/4863` | `/tmp/cppgm-template-body-sorted-atoms-final.json`; paired A/B: `-0.909%` instructions |
| `650dc50cd` | reuse the first temp-interval analysis for register allocation | `132,299,680,071` | `-24.03%` | `695,853,056` | `556,654,592` | SHA-256 `4fc1303a...5c4` | `1530/1530` | `4863/4863` | `/tmp/cppgm-register-interval-reuse-final.json`; paired A/B: `-0.739%` instructions |
| `59505722b` | borrow immutable backend definitions and signatures; classify direct-call indexes in one pass | `130,886,247,307` | `-24.85%` | `735,780,864` | `556,519,424` | SHA-256 `4fc1303a...5c4` | `1530/1530` | `4863/4863` | `/tmp/cppgm-backend-borrowed-metadata-final.json`; paired A/B: `-0.849%` instructions |
| `8e46f9fc2` | keep zero-length lazy AST vector reservations allocation-free | `128,444,585,958` | `-26.25%` | `734,158,848` | `554,516,480` | SHA-256 `4fc1303a...5c4` | `1530/1530` | `4863/4863` | `/tmp/cppgm-lazy-reserve-zero-final.json`; paired A/B: `-2.040%` instructions |
| `a74749dcd` | reuse nonvirtual special-member base-entry bodies for complete-entry output | `127,129,199,627` | `-27.00%` | `737,443,840` | `551,108,608` | SHA-256 `4fc1303a...5c4` | `1530/1530` | `4863/4863` | `/tmp/cppgm-special-member-base-body-reuse-final.json`; removed 2,267 duplicate body analyses |
| `d5900e76d` | use inline dependent-name recursion state and defer placeholder scans until exact binding fails | `126,447,955,718` | `-27.39%` | `734,490,624` | `550,830,080` | SHA-256 `4fc1303a...5c4` | `1530/1530` | `4863/4863` | `/tmp/cppgm-dependent-name-fast-path-final.json`; paired A/B: `-0.568%` instructions; avoided 95,972 placeholder scans |
| `2b72a6b7b` | compare named types by interned key identity and keep local-name traversal scratch contiguous | `125,553,969,817` | `-27.91%` | `730,492,928` | `554,708,992` | SHA-256 `4fc1303a...5c4` | `1530/1530` | `4863/4863` | `/tmp/cppgm-named-key-identity-final.json`; paired A/B: `-0.641%` instructions |
| `cfc773417` | resolve concrete standard `enable_if` forwarding aliases without a general instantiation scope | `124,285,854,757` | `-28.64%` | `740,732,928` | `554,344,448` | SHA-256 `4fc1303a...5c4` | `1530/1530` | `4863/4863` | `/tmp/cppgm-enable-if-alias-final.json`; `3,052` concrete shortcuts; `-1.010%` from `2b72a6b7b` |
| `32f889b69` | omit frame pointers in optimized release builds while preserving them in debug builds | `120,162,630,879` | `-31.00%` | `736,526,336` | `554,426,368` | SHA-256 `4fc1303a...5c4` | `1530/1530` | `4863/4863` | `/tmp/cppgm-omit-frame-pointer-final.json`; `-3.318%` from `cfc773417` |
| `56128c65d` | canonicalize immutable template-parameter named types by semantic identity and source spelling | `119,571,899,320` | `-31.34%` | `711,860,224` | `524,099,584` | SHA-256 `4fc1303a...5c4` | `1530/1530` | `4863/4863` | `/tmp/cppgm-template-parameter-type-intern-final.json`; 61,675 hits in 62,264 probes; `-0.492%` from `32f889b69` |
| `3686d87b0` | supply final synthesized special-member linkage during registration instead of regenerating the symbol immediately | `118,245,739,070` | `-32.10%` | `700,035,072` | `524,410,880` | SHA-256 `4fc1303a...5c4` | `1530/1530` | `4863/4863` | `/tmp/cppgm-synthesized-linkage-registration-final.json`; `-1.109%` from `56128c65d` |
| `a3dc6e079` | return expected constructor and root standard `enable_if` probe failures as status | `117,674,519,926` | `-32.43%` | `712,704,000` | `524,689,408` | SHA-256 `4fc1303a...5c4` | `1530/1530` | `4863/4863` | `/tmp/cppgm-exception-status-composite-final.json`; 4,465 throws removed; paired A/B: `-0.974%` instructions |
| `2ba26c3f4` | cache template-body scope value views and bulk-sort visible names | `116,933,622,740` | `-32.86%` | `718,651,392` | `525,627,392` | SHA-256 `4fc1303a...5c4` | `1530/1530` | `4863/4863` | `/tmp/cppgm-template-visible-composite-final.json`; two paired batches: `-0.485%` and `-0.474%` instructions; balance scores `0.682` and `0.773` |
| `0e5b1af2d` | resolve template-parameter placeholder types by interned identity | `116,375,621,217` | `-33.18%` | `708,911,104` | `525,758,464` | SHA-256 `4fc1303a...5c4` | `1530/1530` | `4863/4863` | `/tmp/cppgm-template-parameter-identity-final.json`; 239,256 identity hits; paired batches `-0.197%` and `-0.828%` instructions |
| `4d0fddd3e` | compact `CppAstNode` by declaration-only field reordering | `116,167,632,347` | `-33.30%` | `701,468,672` | `516,448,256` | SHA-256 `4fc1303a...5c4` | `1530/1530` | `4863/4863` | `/tmp/cppgm-ast-layout-final.json`; paired footprint: `-1.776%`, or `-9,338,880 B`; instructions: `-0.080%` |
| `9ad60e6c6` | memoize each class instantiation's primary-placeholder argument shape | `115,576,376,842` | `-33.64%` | `707,428,352` | `516,362,240` | SHA-256 `4fc1303a...5c4` | `1530/1530` | `4863/4863` | `/tmp/cppgm-primary-placeholder-final.json`; 754,890 hits; paired instructions: `-0.539%` |
| `8d58a3d6a` | reconstruct compacted named-type displays once and return an owned-lifetime view | `112,101,667,646` | `-35.63%` | `703,864,832` | `517,873,664` | SHA-256 `4fc1303a...5c4` | `1530/1530` | `4863/4863` | `/tmp/cppgm-display-view-final.json`; 922,353 stable repeats; paired instructions: `-2.892%` |
| `589b40ac8` | represent inline-namespace visibility with its implicit using-directive instead of copying accumulated bindings | `111,031,651,654` | `-36.25%` | `712,437,760` | `517,394,432` | SHA-256 `4fc1303a...5c4` | `1530/1530` | `4863/4863` | `/tmp/cppgm-inline-namespace-directive-final.json`; paired instructions: `-1.107%`; paired RSS: `-1.035%` |
| `9bcbc6fc6` | store immutable parser lookup snapshots in null-delimited contiguous atom buffers | `110,890,016,889` | `-36.33%` | `696,147,968` | `514,641,920` | SHA-256 `4fc1303a...5c4` | `1530/1530` | `4863/4863` | `/tmp/cppgm-lookup-snapshot-final.json`; paired instructions: `-0.204%`; footprint: `-0.602%`; balance score: `0.806` |
| `ee4ad64b0` | borrow dependent-class template argument vectors for read-only semantic queries | `109,780,001,232` | `-36.97%` | `693,858,304` | `514,256,896` | SHA-256 `4fc1303a...5c4` | `1530/1530` | `4863/4863` | `/tmp/cppgm-dependent-class-argument-view-final.json`; removed about 345,726 vector allocations and 222,012,728 requested bytes; paired instructions: `-0.863%`; user time: `-1.051%` |
| `d94a9aa4a` | call the concrete UTF-8 and full-translation sources directly while preserving each lookahead buffer | `108,407,409,101` | `-37.75%` | `699,043,840` | `514,293,760` | SHA-256 `4fc1303a...5c4` | `1530/1530` | `4863/4863` | `/tmp/cppgm-concrete-translation-source-final.json`; paired instructions: `-1.449%`; cycles: `-1.412%`; user time: `-1.141%`; exact PA1 UTF-8/trigraph reducer |
| `d4fe39d7c` | pack CallSem kind/category flags and store recursive children in one-pointer, single-allocation arrays | `108,502,199,146` | `-37.70%` | `696,795,136` | `509,206,528` | SHA-256 `4fc1303a...5c4` | `1530/1530` | `4863/4863` | `/tmp/cppgm-callsem-packed-children-final.json`; paired footprint: `-1.042%`, or `-5,361,664 B`; paired instructions: `-0.031%`; retained CallSem storage: `-9,004,008 B` |

## Rejected work ledger

Record the code shape, measured result, and reason for rejection. Remove the
experiment before starting the next candidate.

| Experiment | Result | Decision | Evidence |
| --- | --- | --- | --- |
| reason-armed semantic output queues | only 352 pending outcomes; late output had zero pending outcomes | remove the temporary census and keep current indexed worklists | `/tmp/cppgm-output-retry-instrumented.err` |
| change only the dependent-resolution cache from tree to hash storage | `-0.03%` instructions in the one-run screen | fold the finding into cache removal; key construction dominated lookup structure | `/tmp/cppgm-dependent-resolution-hash-screen.json` |
| replace the transient dependent-resolution map with a vector | `166,808,551,179` instructions, only `-0.05%` from the retained interval checkpoint | below the retention threshold; remove the experiment | `/tmp/cppgm-dependent-resolution-vector-screen.json` |
| return `CppAst::node_text` by reference | `166,956,248,006` instructions, slightly worse than the retained checkpoint | copies were not the measured cost | `/tmp/cppgm-node-text-reference-screen.json` |
| pack rare `CppAst` flags into bitfields | reduced the record from 192 to 176 bytes but increased instructions to `167,128,922,568` | reject the instruction regression despite lower memory | `/tmp/cppgm-cppast-bitfields-screen.json` |
| reorder `CppAst` booleans without bitfields | reduced the record from 192 to 184 bytes but increased instructions to `166,982,395,055` | the memory win did not pay for the access layout | `/tmp/cppgm-cppast-bool-layout-screen.json` |
| return `strip_top_level_cv` results by reference | `166,984,183,167` instructions | flat to slightly worse; keep value semantics | `/tmp/cppgm-strip-cv-reference-screen.json` |
| disable the nested dependent-resolution cache | `166,734,130,520` instructions, only `-0.10%` from the retained interval checkpoint | the inner cache remains useful for cycles and duplicate child work; the gain was too small | `/tmp/cppgm-dependent-nested-cache-off-screen.json` |
| use `make_shared` at all 14 type factory allocation sites | instructions stayed flat at `166,867,247,562`, while RSS rose to `766,730,240 B` and footprint to `572,997,632 B` | weak cache entries retain coallocated control blocks; keep separate allocation | `/tmp/cppgm-type-make-shared-screen.json` |
| replace text interning with an open-addressed table | instructions regressed to `167,555,357,053` | reject the `0.39%` regression from the retained interval checkpoint | `/tmp/cppgm-open-addressed-text-intern-screen.json` |
| front the text atom pool with an 8,192-slot direct cache | the pool census measured 2,958,455 lvalue calls, 169,277 rvalue calls, and 786,766 span calls for only 28,538 atoms. Caching all overloads hit 98.4% but screened at `137,102,871,458` instructions, `0.047%` above retained. Caching only spans hit 782,905 calls, 99.5%, but screened at `137,256,264,376`, `0.16%` above retained | hashing the input for the front cache costs as much as libc++'s node-table probe, even when it avoids span-string construction. Restore the original pool and close this family with the earlier open-addressed rejection | `/tmp/cppgm-text-intern-census.stderr`, `/tmp/cppgm-text-intern-front-cache-census.stderr`, `/tmp/cppgm-text-intern-front-cache-screen.json`, `/tmp/cppgm-text-intern-span-cache-census.stderr`, `/tmp/cppgm-text-intern-span-cache-screen.json` |
| convert every lookup-only `FunctionLayout` map to hash storage | extending the retained four-index candidate to forwarded parameters, promoted slots, alias maps, and elided branch loads screened at `135,770,906,462` instructions, worse than the scoped form's `135,455,438,572` screen | the small maps do not repay hash setup and reservation. Keep only storage offsets, storage types, floating-register assignments, and temporary definitions hashed | `/tmp/cppgm-layout-lookup-hashes-screen.json`, `/tmp/cppgm-layout-all-lookups-screen.json` |
| add a broad type-id parse cache | census found 10,110 calls, 2,544 exact repeats, but only 1,456 repeated results and 921 dependent cases | the safe hit population is too small for the proposed key and invalidation cost | `/tmp/cppgm-type-id-parse-stats.err` |
| prune the existing wrapper and function type caches | wrapper cache had 303,441 hits in 341,674 calls; function cache had 65,438 hits in 112,465 calls | both caches are healthy; pruning would discard substantial reuse | `/tmp/cppgm-type-cache-census.stderr` |
| intern the remaining array and member-pointer factories | the frozen compile constructed 3,277 arrays and 37 member pointers | the uncached factory population cannot repay another interning table; optimize the measured named-key comparison work instead | `/tmp/cppgm-type-factory-census-2.stderr` |
| dispatch elaborated named-type prefixes by their first character | `137,113,388,388` instructions, `0.055%` above the retained named-key checkpoint | the branch-heavy form did not beat the compact six-entry table; keep the allocation-free offset comparison and close this refinement | `/tmp/cppgm-named-key-prefix-dispatch-screen.json` |
| reuse temporary buffers in `semantic_utils::trim_space` | an rvalue overload screened at `136,311,068,727` instructions and confirmed at `136,446,018,555`; a by-value form screened at `136,243,586,386` and confirmed at `136,474,424,030`. Both emitted exact frozen bytes and passed the full direct gates. Extending the overload to the mangler weakened the screen to `136,400,724,667` | the two three-run results improve only `0.43%` and `0.41%` from the retained named-key checkpoint, below the `0.5%` retention threshold. Commits `098a3b522` and `6e8654ec8` record the final form and its removal | `/tmp/cppgm-trim-rvalue-screen.json`, `/tmp/cppgm-trim-rvalue-final.json`, `/tmp/cppgm-trim-rvalue-shared-and-mangle-screen.json`, `/tmp/cppgm-trim-by-value-screen.json`, `/tmp/cppgm-trim-by-value-test-strict.log`, `/tmp/cppgm-trim-by-value-test-report.log`, `/tmp/cppgm-trim-by-value-final.json` |
| share one atom lookup across repeated identifier-set membership checks | `165,712,971,270` instructions | flat to slightly worse than the retained template-body checkpoint | `/tmp/cppgm-template-body-contains-atom-screen.json` |
| borrow the top-level-CV child inside `class_info_for_type` | `166,119,709,124` instructions | shared-owner traffic was not the 2.14M-probe bottleneck; reject the regression | `/tmp/cppgm-class-info-borrowed-cv-screen.json` |
| probe argument identifiers directly when overlaying function-local types | `165,959,328,141` instructions | local named-type maps are smaller than the argument token sets; iteration wins | `/tmp/cppgm-local-type-direct-probes-screen.json` |
| make the unused normal-compile `FunctionBinding` source-anchor cache lazy | footprint fell by about 2.1 MiB but instructions rose to `166,344,947,939` | reject the instruction regression; a larger cohesive record slice is required | `/tmp/cppgm-function-anchor-side-record-screen.json` |
| replace recursive `CallSemNode` child vectors with a zero-allocation indexed view | LowIR-only screen was `165,749,705,323`; all walkers were `165,670,523,261` | operation removal was real but instruction results were flat; remove it from the retained symbol fast path | `/tmp/cppgm-callsem-zero-allocation-walk-screen.json`, `/tmp/cppgm-callsem-zero-allocation-walk-all-screen.json` |
| hold a local reference to the runtime-reference node symbol | three-run median regressed to `164,574,311,869` instructions | repeated inline accessors produced better code than the local alias | `/tmp/cppgm-runtime-symbol-local-ref-final.json` |
| hash the template-argument identifier membership set | `164,510,368,737` instructions | the identifier sets are too small to repay hash-table overhead | `/tmp/cppgm-template-argument-identifiers-hash-screen.json` |
| store template-argument identifiers as interned atoms | `164,238,424,503` instructions, only `-0.006%` from the retained runtime-symbol checkpoint | pointer membership avoids string ownership but does not move the workload; remove the added global interning traffic | `/tmp/cppgm-template-argument-atoms-screen.json` |
| make five rarely populated `Scope` sets lazy | `163,441,532,001` instructions, only `-0.01%` from the retained sparse-clone checkpoint; footprint fell by about 3.4 MiB | the pointer-backed headers save memory but do not advance compile throughput; keep the direct ordered-set layout | `/tmp/cppgm-scope-lazy-rare-sets-screen.json` |
| raw-pointer fast path for `class_info_for_type` | `163,528,784,054` instructions | the 1.85M-call counter is dominated by the existing embedded `named_class_info` return; avoiding one shared-owner copy was flat to worse | `/tmp/cppgm-class-info-raw-fast-path-screen.json` |
| return dependent alias-template cache hits before scope construction | `160,065,541,131` instructions, only `-0.12%` from the retained concrete-hit checkpoint | the concrete fast path captures the useful population; keep dependent results on the scope-sensitive redirect and repair path | `/tmp/cppgm-alias-dependent-hit-screen.json` |
| move enum-underlying and host-ABI chunk storage into the named-type side record | footprint fell by about 6.9 MiB, but instructions rose to `161,042,270,068` (`+0.49%` from the retained concrete-hit checkpoint) | the smaller common allocation does not repay the extra named-record access; reject the primary-signal regression | `/tmp/cppgm-type-named-side-record-screen.json` |
| filter the service-layer local-type overlay by template-argument names | `160,725,467,668` instructions (`+0.30%` from the retained concrete-hit checkpoint) | identifier-set membership costs more than scanning the small local type maps; keep the direct marker scan | `/tmp/cppgm-local-type-overlay-name-filter-screen.json` |
| return early from type-pack AST substitution when no replacement name occurs | `160,918,097,342` instructions (`+0.42%` from the retained concrete-hit checkpoint) | the recursive mention predicate turns affected trees toward repeated subtree scans; keep the single-pass clone/substitute recursion | `/tmp/cppgm-noop-pack-substitution-copy-screen.json` |
| validate persistent type-dependency memo hits with ownership identity instead of `weak_ptr::lock()` | `160,149,979,516` instructions, only `-0.06%` from the retained concrete-hit checkpoint | three million locks disappear, but the node-based hash lookup remains the cost; do not retain an isolated policy change below threshold | `/tmp/cppgm-type-dependency-owner-identity-screen.json` |
| replace the persistent type-dependency memo with a flat pointer table and ownership comparisons | `160,385,002,634` instructions (`+0.08%` from the retained concrete-hit checkpoint) | contiguous probing and lock removal do not improve the full compile; keep the healthy node-based memo and stop this family | `/tmp/cppgm-type-dependency-flat-root-screen.json` |
| bypass the persistent type-dependency memo for fundamental and semantically dependent named types | `160,208,856,972` instructions, only `-0.03%` from the retained concrete-hit checkpoint | 476,754 fundamental hits and 2.14 million named-type hits do not translate into a useful full-compile gain; retain the uniform memo path and close this family | `/tmp/cppgm-type-dependency-immediate-screen.json`, `/tmp/cppgm-type-dependency-kind-diagnostic.err` |
| disable the qualified-type lookup cache after a frozen census reported 9,283 misses and no hits | `159,108,436,598` instructions, only `-0.11%` from the retained AST-vector checkpoint | the frozen workload's failed lookups do not justify removing a general cache for a sub-threshold gain | `/tmp/cppgm-qualified-type-cache-off-screen.json`, `/tmp/cppgm-dc49-diagnostic.err` |
| reuse the first identifier-mention result instead of rescanning a type-pack substitution node | `159,716,690,143` instructions (`+0.27%` from the retained AST-vector checkpoint) | the source cleanup did not improve the primary metric; restore the original code | `/tmp/cppgm-template-pack-duplicate-scan-screen.json` |
| return canonical completed class types before the general completion path | `152,469,438,108` instructions, slightly worse than the stable-key checkpoint | the final lookup and self-synchronization path is not the class-materialization cost seen in the profile; remove the shortcut | `/tmp/cppgm-complete-class-direct-fast-path-screen.json` |
| canonicalize template-parameter types across scopes | 62,303 hits in 62,892 constructions and about 40 MiB lower memory, but the lower-overhead raw-key version used `152,499,369,751` instructions | the cache lookup offsets the saved construction work and does not advance compile throughput; do not expand partial type interning from this representation | `/tmp/cppgm-template-parameter-type-cache-diagnostic.stderr`, `/tmp/cppgm-template-parameter-type-cache-screen.json`, `/tmp/cppgm-template-parameter-type-cache-raw-key-screen.json` |
| cache finalized semantic identities by `Type` address | 127,257 requests touched 5,250 addresses and repeated the same identity 122,004 times, but three named identities changed legitimately; the guarded cache admitted 86,843 requests with 84,830 hits and screened at `152,459,023,788` and `152,537,112,353` instructions | pointer recurrence is high, but proving stability and hashing costs as much as spelling the safe subset; never cache the changing dependent-name population by address alone | `/tmp/cppgm-template-type-identity-census.json`, `/tmp/cppgm-template-type-identity-census-2.stderr`, `/tmp/cppgm-stable-template-type-identity-cache-screen.json`, `/tmp/cppgm-stable-template-type-identity-cache-fast-screen.json` |
| skip a second structured type lookup when direct alias/class lookup found no candidate | 1,968 redundant retries removed, but the screen used `152,399,237,254` instructions, only `-0.03%` from the stable-key checkpoint | the failed retries are cheap and too sparse to justify another lookup-state branch | `/tmp/cppgm-structured-type-lookup-retry-diagnostic.stderr`, `/tmp/cppgm-structured-type-lookup-retry-skip-diagnostic.stderr`, `/tmp/cppgm-structured-type-lookup-retry-skip-screen.json` |
| retain one callback bundle of each kind per analyzer | the frozen compile constructed 26,084 type-trait, 38,734 nothrow, 81,834 function-registry, 7,735 type-registry, and 40,589 constant-value bundles; exact output and both full gates passed, but the three-run median was `152,221,222,688` instructions, only `-0.14%` from the stable-key checkpoint | closure construction is frequent but cheap enough that analyzer-owned caching does not meet the retention threshold | `/tmp/cppgm-callback-factory-census.stderr`, `/tmp/cppgm-reuse-callback-bundles-screen.json`, `/tmp/cppgm-reuse-callback-bundles-screen-2.json`, `/tmp/cppgm-reuse-callback-bundles-final.json` |
| store stable type fingerprints in the function-template deduction cacheability table | 108,758 fingerprint requests covered 2,168 cacheable type graphs, with 106,590 repeats and no changed fingerprint, but the screen used `152,346,961,071` instructions, only `-0.06%` from the stable-key checkpoint | these eligible type graphs are shallow; avoid more work on deduction-key mechanics without a profile showing a larger consumer | `/tmp/cppgm-deduction-fingerprint-census.stderr`, `/tmp/cppgm-deduction-stable-fingerprint-screen.json` |
| replace the dominant scope template-bound type-name tree with a sorted flat set | 33,387 scopes had a nonempty set and footprint fell by about 1.7 MiB, but instructions rose to `152,675,579,014` (`+0.15%` from the stable-key checkpoint) | linear insertion shifts and lookup outweigh the removed tree nodes; a useful scope change must remove duplicate ownership rather than swap containers | `/tmp/cppgm-flat-template-bound-type-names-screen.json` |
| cache AST type lookup results by node and scope identity | 247,063 requests made 246,451 node lookups; raw node recurrence included 109,628 same results and 69,824 changing results, while the scope-aware analyzer population was 178,667 calls, 139,008 distinct keys, 20,262 same-result repeats, and 19,397 changed-result repeats | scope identity removes most apparent reuse, and results legitimately change as semantic state advances; the remaining stable population cannot repay a generation-aware cache and its invalidation machinery | `/tmp/cppgm-ast-type-lookup-census.stderr`, `/tmp/cppgm-ast-type-lookup-census-2.stderr`, `/tmp/cppgm-ast-scope-type-lookup-census.stderr` |
| retain compacted class-template display strings instead of reconstructing them | `145,833,753,997` instructions, only `-0.035%` from the runtime-symbol index checkpoint; one-run RSS was about 8 MiB higher | keep this eager, by-value form rejected. The later borrowed lazy view preserves compaction, removes reconstruction and return copies, and is retained in `8d58a3d6a` | `/tmp/cppgm-retain-named-display-screen.json` and `/tmp/cppgm-display-view-{parent,candidate}-{1,2,3}.time` |
| consume normalized input with one streambuf fetch per byte, then extend it with an 8 KiB `sgetn` buffer | the single-fetch form used `145,373,405,816` instructions (`-0.35%` from the runtime-symbol checkpoint), while buffering regressed to `146,013,046,659` (`+0.09%`). The current retry measured parent and candidate medians of `117,050,516,745` and `117,262,068,602`, a `0.181%` regression; RSS also regressed `0.686%` | the historical balanced screen did not survive the current binary. Restore separate `sbumpc` and `sgetc` calls; keep the 8 KiB form closed | `/tmp/cppgm-normalizer-single-fetch-screen.json`, `/tmp/cppgm-normalizer-buffered-input-screen.json`, `/tmp/cppgm-normalizer-one-fetch-current-screen.json`, and `/tmp/cppgm-normalizer-current-{parent,candidate}-{1,2,3}.time` |
| cache source-body output across constructor/destructor ABI entry-point variants | 6,505 emitted bindings included 1,296 constructors and 974 destructors, but variant expansion duplicated only 643 source-statement visits versus 8,578 ordinary-function visits | most special-member variant cost is distinct generated lifecycle work, so shared source-body caching is not a large enough lever | `/tmp/cppgm-function-variant-census.stderr` |
| store function-local overlay index entries as owned `(name,type)` pairs or raw map-entry pointers | the owned-pair screen used `145,288,273,365` instructions and the pointer screen used `145,502,550,768`, both worse than the name-only index | keep cached names: they have the best measured code shape and revalidate the source map before use | `/tmp/cppgm-local-type-overlay-binding-index-screen.json`, `/tmp/cppgm-local-type-overlay-pointer-index-screen.json` |
| index primary vtable definitions before LowIR collection | 19 lookups revisited 2,675,957 nodes and made 145 candidate comparisons, but the indexed screen used `145,197,179,467` instructions (`+0.006%` from the local-type-overlay checkpoint) | the repeated visits are cheap and the replacement prepass plus map operations cancel their removal; keep the depth-first search | `/tmp/cppgm-primary-vtable-census.stderr`, `/tmp/cppgm-primary-vtable-index-screen.json` |
| cache positive class-template source-owner results per instantiation | 10,275 requests covered 3,577 classes; 2,294 repeated the same non-null owner, while one libc++ `basic_string` result legitimately transitioned from null to its primary placeholder owner; the guarded positive-only screen used `142,956,838,220` instructions, only about `-0.09%` from the incremental-symbol-index checkpoint | leave negative results uncached for semantic correctness and reject the positive cache as sub-threshold | `/tmp/cppgm-source-owner-census-2.stderr`, `/tmp/cppgm-source-owner-census-5.stderr`, `/tmp/cppgm-source-owner-positive-cache-screen.json` |
| build `sizeof...(pack)` substitution children once instead of cloning each subtree before its recursive rewrite | `142,321,245,777` instructions, only `-0.03%` from the retained CallSem ownership checkpoint; an earlier screen was slightly worse at `142,655,056,753` | the removed main-child copies are real but too small and the recursive side-syntax clones still dominate this family; restore the measured code shape and do not widen the clone rewrite without attribution by syntax owner | `/tmp/cppgm-sizeof-pack-single-copy-screen.json` |
| borrow an unchanged source syntax during type-pack expansion instead of cloning it before producing element copies | the clone census attributed about 100,000 nodes to the eager source copy, but the screen used `140,651,109,802` instructions, only `-0.07%` from the retained guarded function-pack checkpoint | removing this clone does not move full-compile throughput; keep the simpler owned expansion source because nested-pack rewriting already requires it | `/tmp/cppgm-template-clone-origin-census.stderr`, `/tmp/cppgm-pack-expansion-borrow-source-screen.json` |
| share template-body validation name sets and visible-value maps until mutation | 16,469 name-set copies covered 582,942 entries but only 8,997 detached; 1,672 value-map copies covered 177,063 entries but only 270 detached. Despite that avoided copying, the screen used `140,538,019,761` instructions, only `-0.15%` from the retained guarded function-pack checkpoint, and footprint was about 1.5 MiB higher | container copies are not the dominant cost inside the recursive lookup validator; remove the bespoke ownership layer and follow the semantic lookup work in the profile | `/tmp/cppgm-template-body-cow-census.stderr`, `/tmp/cppgm-template-body-cow-screen.json` |
| cache an interned contiguous view of each scope's value bindings for template-body validation | 44,552 scope visits scanned 1,825,503 entries; 35,202 visits repeated an unchanged scope. An eager screen used `140,136,214,096` instructions, but the second-visit-only form used `140,739,486,236`, within `0.01%` of the retained checkpoint and with higher memory | destination name and type tables account for the visible allocation cost; replacing source-map traversal does not remove it, so restore the direct scan | `/tmp/cppgm-template-visible-values-census.stderr`, `/tmp/cppgm-template-visible-value-index-screen.json`, `/tmp/cppgm-template-visible-value-index-screen-2.json` |
| build a contiguous hash-bucket index for large function and function-template dedupe calls | function dedupe made 68,134,228 comparisons but only 96,935 hash-pair matches; template dedupe made 5,425,171 comparisons. Indexing combined sizes of eight used `138,144,700,790` instructions, only `-0.31%` from the retained name-index checkpoint, and added about 0.9 MiB of footprint. A size-32 threshold regressed to `139,030,640,266` | per-call scratch construction consumes the comparison savings; restore the cached-key linear loop and close this family after two measured forms | `/tmp/cppgm-append-unique-census.stderr`, `/tmp/cppgm-append-unique-bucket-index-screen.json`, `/tmp/cppgm-append-unique-bucket-index-screen-32.json` |
| index the 512-entry fast template-argument cache by its existing hash | 28,866 lookups scanned 12,282,565 slots; 9,956 hash matches were all cache hits. A node-based hash-to-slot index regressed to `140,188,174,364` instructions, while a fixed 1,024-bucket intrusive slot index regressed to `139,634,155,672` | ring overwrite maintenance and indexed lookup cost more than scanning the compact slot array; restore the linear probe and close this family | `/tmp/cppgm-resolve-template-fast-cache-census.stderr`, `/tmp/cppgm-resolve-template-fast-cache-slot-index-screen.json`, `/tmp/cppgm-resolve-template-fast-cache-fixed-index-screen.json` |
| route paired template-dependency text checks through the analyzer's existing mention caches | existing analyzer queries reported 29,159 placeholder-cache hits and 22,075 dependent-binding-cache hits, but routing the specialization helper through both caches used `138,571,733,932` instructions, within `0.003%` of the retained checkpoint, and raised footprint slightly | cache-key construction and separate miss tokenization consume the reused checks; keep the paired one-tokenization helper | `/tmp/cppgm-semantic-stats-f78e34bc4.stderr`, `/tmp/cppgm-template-dependency-flags-cache-route-screen.json` |
| bypass witness-anchor setup for simple unqualified AST type lookups | the branch census measured 247,063 AST lookup calls. The node hook resolved 240,029; the legacy spelling fallback ran 3,025 times and had no hits. A guarded fast path applied to 155,407 of 202,868 instrumented hook calls, but its three-run median was `135,548,474,216` instructions versus a contemporaneous retained median of `135,639,792,208`, only `-0.067%` | normal-mode witness guards and empty anchors are cheap. Restore the common path and follow the dependent-resolution work below it | `/tmp/cppgm-ast-type-lookup-branches.stderr`, `/tmp/cppgm-simple-type-lookup-fast-path-census.stderr`, `/tmp/cppgm-simple-type-lookup-fast-path-screen.json`, `/tmp/cppgm-simple-type-lookup-fast-path-decision.json`, `/tmp/cppgm-simple-type-fast-retained-ab.json` |
| extend dependent-type resolution reuse across root calls | a frozen census measured 307,516 roots, 177,324 distinct `(scope instance, binding fingerprint, type)` keys, and 130,192 repeated results with no observed transitions. Of the repeats, 128,935 remained unresolved; their outer kinds were led by 108,361 named types, including 48,700 template parameters and 39,321 dependent types. Returning immediately after an unbound template-parameter lookup screened at `135,673,819,299` instructions. An operation-scoped tree cache found 46,926 hits against 261,857 misses across 208,697 operations, with at most 39 entries, and screened at `135,927,876,353`. A single-entry last-root memo retained 45,005 hits. Its three-run median was `135,359,414,885`; a same-host alternating comparison measured retained `135,580,233,289` versus candidate `135,154,528,753`, a `0.314%` reduction. The current census found 45,220 same-key hits among 307,202 roots; all recomputed results matched, including 45,145 unresolved and 75 resolved hits. The current three-pair medians were `116,134,832,336` for the parent and `116,026,368,226` for the candidate, a `0.093%` reduction. Footprint improved `0.012%`, while RSS regressed `0.704%`; every object had the frozen SHA-256 | the retained resolver fast paths reduced the cost behind the stable hit population. The current balance score is `0.106`, so the memo misses every lane. Keep the direct resolver and service interface | `/tmp/cppgm-dependent-resolution-roots-2.stderr`, `/tmp/cppgm-dependent-operation-last-cache-census.stderr`, `/tmp/cppgm-operation-ab-{retained,candidate}-{1,2,3}.time`, `/tmp/cppgm-dependent-root-current-stability.stderr`, and `/tmp/cppgm-dependent-root-{parent,candidate}-{1,2,3}.time` |
| skip alias source-occurrence materialization when witness capture is disabled | the frozen compile built 19,532 alias source-occurrence vectors containing 31,604 copied arguments and 31,604 deep-copied syntax records; every call had source capture disabled. Guarding only those records screened at `135,680,956,488` instructions. Also borrowing the caller's argument-text vector instead of copying and rewriting witness spellings improved the screen to `135,082,147,793`; the three-run median was `135,200,957,915`, about `0.28%` below the immediately preceding retained A/B median of `135,580,233,289`. The frozen object remained exact | the allocations are avoidable but do not meet the `0.5%` full-compile retention threshold. Keep the uniform source-observation path and seek a larger allocation owner | `/tmp/cppgm-alias-source-arguments-census.stderr`, `/tmp/cppgm-alias-source-arguments-screen.json`, `/tmp/cppgm-alias-source-materialization-screen.json`, `/tmp/cppgm-alias-source-materialization-decision.json` |
| borrow named-type strings while finding a template parameter | `find_template_parameter(TypePtr)` copied the named key, semantic payload, and source name into a local candidate array even though it only compared them. Replacing the copies with pointers produced a three-run median of `135,050,779,339` instructions versus `135,434,571,754` for the interleaved retained build, a `0.283%` reduction. Candidate and retained median RSS were `727,531,520` and `735,023,104`; footprints were `555,859,968` and `555,630,592` | the isolated borrow remained rejected. Commit `0e5b1af2d` later paired it with the measured placeholder-identity fast path; the cohesive lookup change met the balanced and CPU lanes in independent batches | `/tmp/cppgm-template-parameter-candidate-refs-{screen,candidate-2runs,retained-screen,retained-2runs}.json` and `/tmp/cppgm-template-parameter-identity-guarded-{parent,candidate}-{1,2,3}.time` |
| remove tree and `std::function` scratch from value lookup through using-directives | the fresh profile attributed 56 top-stack samples and 21 direct allocations to `Analyzer::lookup_value_at_token`. A direct recursive helper used the retained inline scope visit set and ancestor comparison in place of two tree sets and a recursive `std::function`. The initial three-run candidate median was `133,162,716,462`, but its post-commit absolute median moved to `133,581,240,724`. A contemporaneous three-pair binary A/B measured parent `133,535,105,679` versus candidate `133,254,277,052`, only `-0.210%`. Candidate and parent median RSS were `752,021,504` and `749,436,928`; footprints were `556,036,096` and `555,831,296`. The candidate emitted the exact frozen object and passed direct strict `1530/1530` and full report `4863/4863` | the allocations are real, but the paired instruction reduction is below the `0.5%` threshold and secondary memory signals are slightly worse. Commit `ddd19fd3a` records the tested form and `709061c3f` removes it | `/tmp/cppgm-value-using-directive-scratch-{screen,decision,final}.json`, `/tmp/cppgm-value-ab-{parent,candidate}-{1,2,3}.json`, and `/tmp/cppgm-using-directive-profile.sample.txt` |
| store template-body visible value types in a sorted contiguous map | the companion value map supported only lookup, overwrite, copy, and const iteration, so a private sorted vector could preserve its interface. Its one-run screen used `132,755,079,558` instructions, but a contemporaneous three-pair binary A/B measured parent median `132,781,984,174` versus candidate `133,060,302,274`, a `0.210%` regression | the smaller value-map population does not repay ordered insertion and binary lookup. Restore the hash map and stop extending the template-body container-rewrite family | `/tmp/cppgm-template-body-sorted-value-map-screen.json` and `/tmp/cppgm-value-map-ab-{parent,candidate}-{1,2,3}.json` |
| inline or borrow the result of `strip_top_level_cv` | exposing the three-branch body in the header screened at `132,748,141,512` instructions, only about `0.03%` below its contemporaneous parent median. A later profile attributed 202 leaf samples to the helper across 1,692 source uses. Returning a borrowed `const TypePtr&` removed refcount pairs from inspection-only callers but screened at `128,509,370,739`, effectively flat against the retained `128,444,585,958` median | neither call dispatch nor result ownership explains enough of the profile attribution. Restore the value-returning out-of-line API and close this helper family after two measured forms | `/tmp/cppgm-inline-strip-cv-screen.json`, `/tmp/cppgm-using-directive-profile.sample.txt`, `/tmp/cppgm-lazy-reserve-profile.sample.txt`, and `/tmp/cppgm-borrow-strip-cv-screen.json` |
| devirtualize the concrete UTF-8 and full-translation source chain | the fresh preprocessing sample showed 251 `Normalizer` increment leaves and 144 buffered-iterator increment leaves. The historical typed form decoded `?π` as `?Ï€`; a second refill/pop form produced `ππ`. The current trace proved that `BufferedIterator(source)` had selected the implicit base copy constructor after `source` became a `UTF8Translator&`, so FullTranslator read the copied base's raw Normalizer. An explicit `CodePointIterator&` cast preserves source binding. The corrected three-pair candidate improves instructions by `1.449%`, cycles by `1.412%`, user time by `1.141%`, and wall time by `1.085%`; every pair agrees and every object is exact | the broken historical forms remain rejected, but their shared constructor bug is resolved. The corrected concrete-reference form is retained in `d94a9aa4a` after the exact reducer, PA1 `53/53`, strict `1530/1530`, and full `4863/4863` | `/tmp/cppgm-register-interval-profile.sample.txt`, `/tmp/cppgm-translator-typed-source-{screen,decision}.json`, `/tmp/cppgm-concrete-source-{parent,candidate}-{1,2,3}.time`, `/tmp/cppgm-concrete-translation-source-final.json`, and `pa1/tests/100-trigraph-lookahead-utf8.t` |
| retry function-symbol generation only when function-value rehydration changes its pointer | the frozen compile emitted 69,797 function-symbol trace events, including 29,900 exact repeated results. A focused census found 19,000 linkage-upgrade calls: 9,388 left the symbol unchanged, and one binding was retried 118 times. Suppressing a retry after the same function pointer had already been installed emitted exact frozen bytes but screened at `132,026,096,185` instructions, only `0.21%` below the retained checkpoint | the redundant upgrades are measurable, but the safe local guard does not meet the `0.5%` retention threshold. Restore the retry behavior; do not add a broad symbol-state cache without an explicit mutation contract | `/tmp/cppgm-symbol-trace.log`, `/tmp/cppgm-symbol-upgrade-census.log`, and `/tmp/cppgm-rehydrate-on-change-screen.json` |
| defer class-member symbol construction until a provisional binding survives merge lookup | class-member registration mangled and reserved every provisional binding before checking the existing overload slot. Moving that work to the insertion path screened at `131,679,982,602` instructions and emitted exact frozen bytes. A three-pair binary comparison measured a parent median of `132,071,823,847` and candidate median of `131,862,510,228`, a `0.158%` reduction | the ordering cleanup removes real discarded work but does not meet the `0.5%` retention threshold. Restore eager construction and reservation rather than changing registration side-effect order for a sub-threshold gain | `/tmp/cppgm-deferred-class-symbol-screen.json` and `/tmp/cppgm-deferred-class-symbol-ab-{parent,candidate}-{1,2,3}.json` |
| extend inline visited storage to the remaining using-directive lookup APIs | the generic lookup helpers already use the retained eight-pointer visit set, but callable, function-template, value, and qualified-namespace helpers still used tree sets. Converting that whole family preserved the frozen object and screened at `132,072,257,170` instructions, about `0.17%` below the retained interval-reuse checkpoint | the remaining tree allocations are too small a share of the compile to justify changing a public lookup interface. Restore the tree-set API and keep the proven inline representation only on the high-volume generic paths | `/tmp/cppgm-remaining-using-directive-inline-screen.json` |
| bypass recursive argument-combination enumeration when every template call argument has one interpretation | the late profile attributed 593 inclusive samples to the combination runner. Moving each sole option into the deduction input once and calling its terminal step directly preserved the frozen object, but screened at `132,208,159,899` instructions, about `0.07%` below the retained interval-reuse checkpoint | the profile attribution belongs to deduction and binding acquisition below the runner; its recursion and move bookkeeping are negligible. Restore the uniform enumerator and investigate the work inside deduction instead | `/tmp/cppgm-single-template-combination-screen.json` |
| add an exact-pointer front cache before structural function-template deduction keys | 45,199 structural cache hits included 28,168 hits with identical type pointers, all with at most four arguments. A fixed direct cache served 25,400 hits with 4K slots and 27,222 with 16K slots. The best one-run screen used `131,710,486,668` instructions, but an interleaved three-pair binary comparison measured parent median `132,052,142,360` versus candidate median `132,161,188,380`, a `0.083%` regression | repeated type fingerprints are real, but hashing, scope validation, and the second cache probe consume the savings. Restore the single structural cache; do not add another deduction cache layer | `/tmp/cppgm-deduction-pointer-census.stderr`, `/tmp/cppgm-deduction-fast-cache-screen{,-2,-16k}.json`, and `/tmp/cppgm-deduction-fast-ab-{parent,candidate}-{1,2,3}.json` |
| bulk-insert visible scope names during template-body validation | the fresh post-backend profile attributed 95 leaf samples to visible-scope value collection. One form sorted each batch and merged it into the retained contiguous name set; a second appended the batch and sorted in place. They used `130,646,281,297` and `130,576,934,916` instructions, about `0.18%` and `0.24%` below the retained three-run median | the isolated destination forms remained rejected. Commit `2ba26c3f4` later combined one final sort with the source-scope view on the same collector path and met the balanced lane twice | `/tmp/cppgm-backend-metadata-profile.sample.txt`, `/tmp/cppgm-template-visible-bulk-insert-screen.json`, and `/tmp/cppgm-template-visible-bulk-sort-screen.json` |
| trim temporary strings in place through an rvalue overload | the post-lazy-reserve profile attributed 73 leaf samples to the shared trim helper and 27 to the mangler-local copy. Rvalue overloads reused temporary `substr` and generated buffers while preserving the lvalue API, but screened at `128,511,249,469` instructions, effectively flat against the retained `128,444,585,958` median | temporary-buffer allocation is too small and dispersed to justify a second trim API. Restore the single const-reference helper | `/tmp/cppgm-lazy-reserve-profile.sample.txt` and `/tmp/cppgm-trim-rvalue-screen.json` |
| memoize the immediately preceding AST type lookup within one hook bundle | the cache stayed inside one parse operation and therefore needed no scope epoch, but its exact-output screen used `128,590,908,638` instructions, slightly above the retained `128,444,585,958` median | even the allocation-free one-entry form is flat. Restore direct lookup and close AST result caching together with the earlier scope-aware census | `/tmp/cppgm-ast-last-node-screen.json` |
| replace the diagnostic-context frame vector with intrusive RAII frames | normal semantic work no longer pushed or popped `Frame` objects in a heap-capable vector, while exception-time realization retained the same outer-to-inner stack, but the exact-output screen used `128,353,586,846` instructions, only about `0.07%` below the retained median | diagnostic frame bookkeeping is visible but not material to the frozen compile. Restore the simpler vector and leave exception diagnostics unchanged | `/tmp/cppgm-diagnostic-intrusive-stack-screen.json` |
| replace `Scope::values` ordered maps with hash maps | the frozen memory census found 14,102 entries across 36,331 scopes, with only 4,988 nonempty maps. The candidate emitted exact frozen bytes and used `128,340,237,385` instructions, about `0.08%` below the retained `128,444,585,958` median | most value maps are too small to repay hashing and bucket storage. Restore ordered maps and avoid an iteration-order contract change for a sub-threshold result | `/tmp/cppgm-memory-census-current.stderr` and `/tmp/cppgm-scope-values-unordered-screen.json` |
| move the exception and alignment AST node vectors into the existing sparse record | removing two pointers from each common `CppAstNode` reduced peak footprint from the retained `554,516,480 B` median to `549,568,512 B`, and the frozen object stayed exact. The screen used `128,937,061,931` instructions, about `0.38%` above the retained median | memory density alone does not repay sparse-record access on these traversals. Restore the inline lazy handles and do not move the hotter qualifier vectors behind the same pointer | `/tmp/cppgm-ast-rare-node-vectors-screen.json` |
| replace each recursive `CallSemNode` child vector with a one-pointer, single-allocation container | the original 72-byte form emitted exact objects but improved footprint by only `0.828%` and `0.780%` in two batches, so it remained rejected. The current adjacent-field audit packs `CallSemKind` and `CallValueCategory` into existing flag storage, producing one cohesive 64-byte record. Across `201,463` nodes the composite removes `9,004,008 B` of retained CallSem storage. Three current pairs improve median footprint by `1.042%` or `5,361,664 B` and instructions by `0.031%`; all footprint pairs and two instruction pairs agree | reopen and retain the composite in `d4fe39d7c`. It now clears the memory-density lane without RSS, keeps vector-like ownership and invalidation rules explicit, uses no pool or cache, emits the frozen object, and passes focused `878/878`, direct strict `1530/1530`, and full report `4863/4863`. The 72-byte child-only form remains rejected as an isolated edit | `/tmp/cppgm-callsem-duplicate-census.stderr`, `/tmp/cppgm-callsem-compact-{parent,candidate}-{1,2,3,4,5,6}.time`, `/tmp/cppgm-callsem-kind-flags-screen.json`, `/tmp/cppgm-callsem-packed-{parent,candidate}-pair{1,2,3}.json`, and `/tmp/cppgm-callsem-packed-children-final.json` |
| pool recursive `CallSemNode` child-vector storage in reusable slabs | the construction census counted 276,542 node constructions, and a pooled-allocator census measured 1,686,253 child-buffer allocation requests with 1,570,427 free-list reuse hits. The instrumented form used `128,117,455,767` instructions; removing normal-mode census increments and thread-local pool access improved that to `128,051,791,175`, only `0.31%` below the retained median. The best form emitted the exact frozen object, reduced one-run RSS to `695,099,392 B`, and raised footprint to `560,300,032 B` | macOS's small-object allocator handles this traffic efficiently enough that slab lookup and lifetime bookkeeping consume most of the saved work. The result remains below the `0.5%` threshold after two forms and increases footprint, so restore the default allocator | `/tmp/cppgm-callsem-construction.stderr`, `/tmp/cppgm-callsem-child-pool-census.stderr`, `/tmp/cppgm-callsem-child-pool-screen.json`, and `/tmp/cppgm-callsem-child-pool-screen-2.json` |
| extend special-member body reuse to virtual-base and deleting variants | the frozen census found two virtual-base constructor pairs, one virtual-base destructor triplet, and 13 nonvirtual deleting destructors. Those entry-specific paths contain 398 CallSem nodes; the frozen object remained exact | the residual population cannot repay a partial-tree reuse mechanism. Keep virtual-base setup, virtual-base teardown, and deleting deallocation on their existing paths | `/tmp/cppgm-special-member-variant-census.stderr` |
| store the persistent type-dependency memo result in each `Type` | the existing memo served 2,981,946 persistent hits with 56,590 misses and 17,709 stale-address detections. Disabling it used `128,801,711,184` instructions. Replacing its pointer hash and `weak_ptr` check with a byte in existing `Type` padding emitted exact frozen bytes and used `127,139,069,875` instructions, flat against the retained `127,129,199,627` median | the current memo earns its cost, while its lookup representation does not consume enough of the full compile to meet the retention floor. Restore the ownership-safe map and close this representation family with the earlier flat-table and fast-path trials | `/tmp/cppgm-type-dependency-memo.stderr`, `/tmp/cppgm-type-dependency-memo-off-screen.json`, and `/tmp/cppgm-type-dependency-inline-state-screen.json` |
| reuse the emitted base-entry `SymbolIdentity` when attaching a complete-entry object alias | the candidate removed one full Itanium remangle for each of 2,267 reused bodies and emitted the exact frozen object. A three-pair binary comparison measured parent median `126,883,104,116` and candidate median `126,616,060,859`, a `0.210%` reduction | the measured gain falls below the `0.5%` retention floor. Restore local symbol regeneration and avoid widening the special-member state path for this gain | `/tmp/cppgm-special-member-symbol-reuse-screen.json` and `/tmp/cppgm-symbol-ab-{parent,candidate}-{1,2,3}.time` |
| test the exact template-parameter bound before scanning its type spelling for local dependent placeholders | the reordered path emitted the exact frozen object and screened at `126,699,852,401` instructions, `0.338%` below the retained `127,129,199,627` median | the isolated result misses the `0.5%` floor. It was restored for the individual decision, then retained as part of the combined dependent-name fast path in `d5900e76d` | `/tmp/cppgm-template-parameter-exact-bound-first-screen.json` |
| replace the tree-backed dependent-name recursion guard with an inline LIFO stack | the candidate removed recursion-key copies, pointer-to-text formatting, and tree allocation while preserving the old scope-and-name equivalence. A three-pair binary comparison measured parent median `127,121,119,307` and candidate median `126,514,114,084`, a `0.478%` reduction. Parent and candidate median RSS were `735,195,136 B` and `734,056,448 B`; footprints were `550,821,888 B` and `550,887,424 B`. All six outputs had SHA-256 `4fc1303a...5c4` | the isolated result misses the `0.5%` floor. It was restored for the individual decision, then retained with the exact-binding reorder in `d5900e76d` after the combined slice cleared the floor | `/tmp/cppgm-dependent-named-inline-recursion-screen.json` and `/tmp/cppgm-named-guard-ab-{parent,candidate}-{1,2,3}.time` |
| return a fixed void alias target before general instantiation | the census found 925 concrete `__void_t` uses. The exact-output shortcut screened at `125,523,296,465` instructions, only `0.024%` below the retained `125,553,969,817` median | the scope and key work removed from this population is too small to meet the retention floor. Restore the uniform path and target the larger concrete `enable_if` population | `/tmp/cppgm-alias-fast-path-census.stderr` and `/tmp/cppgm-void-alias-fast-path-screen.json` |
| move concrete builtin-transform aliases ahead of general instantiation | the all-alias census found 18,410 successfully resolved aliases, including 1,764 concrete uses of recognized remove/add/decay/identity transforms. Reusing the existing typed transform before scope and key construction emitted exact output and screened at `124,269,784,538` instructions, effectively flat against the retained `124,285,854,757` median | the existing concrete alias cache already captures the useful recurrence. Restore the established transform path and stop moving small alias families across the cache | `/tmp/cppgm-alias-name-census.stderr` and `/tmp/cppgm-builtin-alias-front-fast-path-screen.json` |
| return cached dependent standard `enable_if` aliases before scope construction | the scope census found 3,910 dependent `__enable_if_t` base-key hits that still built an instantiation scope. A witness-disabled early return emitted exact output and screened at `124,069,418,372` instructions, about `0.17%` below the retained checkpoint | the remaining dependent scope overlay is cheap enough that this specialized cache branch misses the retention floor. Restore the uniform dependent-cache path | `/tmp/cppgm-alias-scope-census.stderr` and `/tmp/cppgm-enable-if-dependent-cache-hit-screen.json` |
| bypass template/witness anchor setup for simple unqualified type nodes | normal-compile nodes without qualified names, template syntax, builtin transforms, trace state, or witness state can call the same token-aware fallback directly. The exact-output candidate screened at `124,058,457,734` instructions, about `0.18%` below the retained checkpoint | the sampled inclusive cost belongs to semantic lookup below the dispatcher. Restore the uniform node path rather than retain another branch | `/tmp/cppgm-simple-type-node-lookup-screen.json` |
| remove the unused `trimmed_name` local from type lookup | the value has no readers, but removing its declaration shifted the release layout and screened at `124,732,294,752` instructions, a `0.36%` regression | reject the primary-signal regression. The optimizer already removes the unused construction | `/tmp/cppgm-dead-type-lookup-trim-screen.json` |
| borrow already-normalized type lookup spellings | a guarded reference path preserved trimming, `typename`, elaborated-prefix, `::template`, and signed/unsigned normalization but screened at `124,388,097,231` instructions, about `0.08%` above the retained checkpoint | string ownership is not the semantic lookup bottleneck. Restore the direct normalized string and close this lookup-text branch | `/tmp/cppgm-borrow-normalized-type-lookup-name-screen.json` |
| reuse structured AST semantic types before node lookup | a focused frozen census observed zero entries into the structured semantic-type refresh branch | this workload's hot node lookups do not carry reusable structured semantic types. Remove the census without implementing a cache | `/tmp/cppgm-structured-semantic-type-census.stderr` |
| retain reconstructed class-template display text on named types | disabling display compaction kept the generated spelling but used `124,284,558,137` instructions, flat against the retained `124,285,854,757` median | keep the eager-retention form rejected. The retained `8d58a3d6a` form restores text only on demand and borrows it after that first request | `/tmp/cppgm-retain-template-display-screen.json` and `/tmp/cppgm-display-view-final.json` |
| allocate `Scope::named_type_access` on first use | 2,614 of 36,327 scopes populated the map. The historical candidate used `124,074,856,078` instructions, a `0.17%` reduction; footprint improved `0.058%` and one-run RSS improved `1.237%`. The current form reused the existing `LazyMap` with a one-line field change. Three current pairs measured parent and candidate instruction medians of `116,007,142,756` and `116,028,961,523`, a `0.019%` regression. RSS regressed `1.685%`, footprint improved `0.102%`, and every object had the frozen SHA-256 | the historical RSS result did not repeat. The current score is `0.083`, and only one instruction pair improved. Keep the direct map | `/tmp/cppgm-lazy-named-type-access-screen.json` and `/tmp/cppgm-lazy-named-access-{parent,candidate}-{1,2,3}.time` |
| allocate the three core scope maps on first use | 4,988 scopes populated `values`, 4,266 populated `function_sets`, and 2,614 populated `named_type_access`. The combined candidate used `124,768,251,577` instructions, a `0.39%` regression | pointer checks and allocations on populated scopes cost more than the common-record reduction. Restore direct map storage and close this scope family | `/tmp/cppgm-lazy-scope-core-maps-screen.json` |
| allocate source-declaration anchor payloads on first use | moving two strings behind a copied side record reduced one-run RSS to `718,831,616 B`, but instructions rose to `124,464,108,891` | the memory reduction does not repay allocation and pointer access across more than 65,000 owning records | `/tmp/cppgm-lazy-source-anchor-screen.json` |
| reorder `CppAstNode` fields for a 176-byte record | the historical screen reduced the record from 192 to 176 bytes but missed the former instruction-only floor. Three current pairs measured a `0.080%` instruction improvement, `1.286%` lower RSS, and `1.776%` or `9,338,880 B` lower footprint. Every object had the frozen SHA-256 | reopened under the refined rubric and retained in `4d0fddd3e`; the declaration-only reorder independently meets the memory-density lane without adding ownership or access indirection | `/tmp/cppgm-ast-layout-compaction-screen.json`, `/tmp/cppgm-ast-compact-record-layouts.txt`, `/tmp/cppgm-ast-layout-{parent,candidate}-{1,2,3}.time`, and `/tmp/cppgm-ast-layout-final.json` |
| cache general semantic-validation function bodies for output reuse | an environment-gated census logged 6,522 output emissions and zero calls through `analyze_function_body_semantics_impl` on the frozen workload | the two paths do not overlap on this compile, so a broader cache cannot remove work | `/tmp/cppgm-function-body-census.stderr` |
| return fundamental non-type template parameter types before dependent resolution | a census found 11,125 fundamental inputs among 14,183 calls. The exact-output shortcut used `124,267,237,736` instructions, a `0.015%` reduction | the dependency memo makes these high-count calls cheap. Restore the uniform resolver | `/tmp/cppgm-nttp-type-census.stderr` and `/tmp/cppgm-nttp-fundamental-fast-path-screen.json` |
| resolve exact bound named non-type parameter types before the general resolver | the census found 1,029 concrete named template-parameter results. Adding a direct lexical binding probe to the fundamental shortcut emitted exact output and used `124,283,660,973` instructions | the direct path is flat, so the sampled cost belongs to the remaining dependent named cases | `/tmp/cppgm-nttp-type-census.stderr` and `/tmp/cppgm-nttp-concrete-type-fast-path-screen.json` |
| build the compiler at `-O2` | an isolated build produced a 16,124,240-byte executable and emitted the frozen object, but used `126,873,871,810` instructions, a `2.08%` regression from the retained checkpoint | restore `-O3`. The smaller executable does not offset weaker optimization on this workload | `/tmp/cppgm-build-o2-screen.json` and `/tmp/cppgm-o2-screen-binary` |
| promote an overloaded binary-operator probe instead of analyzing the call twice | a frozen census found 994 viable builtin probes and 1,730 overloads selected through the direct path. No builtin probe also found an overloaded candidate, so the probe-then-materialize branch ran zero times | the suspected duplicate analysis does not occur on this workload. Remove the census and optimize the direct overload path instead | `/tmp/cppgm-binary-probe-census.stderr` and `/tmp/cppgm-binary-probe-census.o` |
| retain type fingerprints in the function-template deduction cacheability table | 106,124 of 108,321 cacheability checks hit the existing type-address table. Storing the structural fingerprint beside each positive entry avoided recomputing it while building deduction keys, but the exact-output screen used `124,569,178,846` instructions, a `0.23%` regression | the larger hash entry and added result plumbing cost more than the shallow fingerprint walks. Restore the set and its on-demand fingerprint calculation | `/tmp/cppgm-deduction-fingerprint-screen.json` |
| add a direct exact-type cache for finalized function-template bindings | a 16K-slot census covered 16,375 of 19,023 acquisitions and found 14,711 exact declaration, owner, and type-argument repeats. Of those, 14,708 held a live finalized binding, and every exact repeat returned the same binding through the full path. A guarded cache preserved the frozen object and used `123,965,783,988` instructions, a `0.26%` reduction | finalized binding reacquisition is redundant but not large enough to retain another cache and scope-independence contract. Restore the established instantiation cache | `/tmp/cppgm-function-template-acquire-census.stderr` and `/tmp/cppgm-function-template-acquire-fast-screen.json` |
| avoid binary builtin-probe operand copies when no class conversion can run | the original path copied both `ExprInfo` trees before discovering scalar operands or a preserving comma or logical operator. Branching before those copies emitted exact output but used `124,179,016,816` instructions, a `0.09%` reduction | the copied trees are small enough that the new branch misses the retention floor. Restore the uniform probe path | `/tmp/cppgm-binary-probe-copy-screen.json` |
| devirtualize direct and negative `class_info_for_type` results | the frozen metrics recorded 1,764,871 general calls plus 282,995 definite-negative skips. An inline wrapper handled cv stripping, non-class types, and named types carrying a direct class pointer before calling a virtual slow path. It preserved metrics mode and exact output but used `124,110,706,806` instructions, a `0.14%` reduction | the high-count common cases are cheap. Restore the single virtual interface and target work below class lookup | `/tmp/cppgm-class-info-devirtualize-screen.json` |
| replace caught false standard `enable_if` results during non-type parameter resolution with a status | all 1,145 caught substitution failures came from the concrete standard `enable_if` fast path. A broad status used `123,612,231,381` instructions, a `0.542%` reduction, but changed the frozen object by selecting different libc++ `std::pair` and `std::__find` overloads. The status suppressed nested, recoverable SFINAE as well as the failure caught by the non-type parameter resolver. Per-probe state restored the `std::pair` choices, and restricting the status to the root alias-instantiation frame restored the frozen SHA-256 `4fc1303a...5c4`. Three semantics-preserving forms used `123,790,741,936`, `123,901,375,414`, and `123,944,605,544` instructions, reductions of `0.398%`, `0.309%`, and `0.275%` | the isolated forms remained rejected. Commit `a3dc6e079` later retained the root-only status as one cohesive expected-failure boundary with constructor probes; the combined paired result clears the CPU lane while nested SFINAE still throws | `/tmp/cppgm-nttp-failure-census.stderr`, `/tmp/cppgm-nttp-enable-if-status-screen.json`, `/tmp/cppgm-nttp-enable-if-root-status-screen.json`, `/tmp/cppgm-nttp-enable-if-root-active-screen.json`, and `/tmp/cppgm-nttp-enable-if-explicit-status-screen.json` |
| build the compiler for the local CPU with `-march=native` | an isolated `-O3` build targeted the host's Skylake instruction set, emitted the exact frozen object, and used `123,933,837,451` instructions, a `0.28%` reduction from the retained checkpoint. Adding `-mtune=native` produced a byte-identical `callsemantic.o`, confirming that explicit native scheduling does not add to the target-CPU choice | the small machine-specific gain misses the retention floor and would make the normal compiler binary less portable. Keep the generic `-O3` developer build and return to semantic work removal | `/tmp/cppgm-march-native-screen.json`, `/tmp/cppgm-march-native`, and `/tmp/callsemantic-march-only.o` |
| replace class-output temporary pointer maps and sets with hash tables | the one-file candidate changed the member-node index and emitted-binding membership set, neither of which exposes iteration order. It emitted the exact frozen object but used `124,377,323,752` instructions, a `0.07%` regression from the retained checkpoint | hash setup costs more than tree lookup for these per-class populations. Restore the ordered temporary containers | `/tmp/cppgm-class-output-hash-temporaries-screen.json` |
| disable release stack protection after omitting frame pointers | the optimized binary contains 2,291 references to stack-canary support. An isolated `-fno-stack-protector` build emitted the exact frozen object and used `119,842,623,172` instructions, only `0.27%` below the retained `120,162,630,879` median | the gain misses the retention floor and does not justify removing input-processing hardening. Keep the host compiler's default stack protection | `/tmp/cppgm-no-stack-protector-screen.json` and `/tmp/cppgm-no-stack-protector` |
| dead-strip unused Mach-O sections from `cppgm++` | a link-only `-Wl,-dead_strip` candidate reduced the executable from 17,116,168 to 16,417,864 bytes and emitted the exact frozen object, but used `120,173,818,108` instructions with a slightly higher footprint | the existing reduced frontend source set already captures the useful locality reduction. Reject the instruction-flat link flag | `/tmp/cppgm-dead-strip-screen.json` and `/tmp/cppgm-dead-strip` |
| disable the qualified-type lookup cache after a zero-hit census | metrics reported 8,744 misses, 8,744 stored entries, and no hits. Using the existing disable switch avoided all key and table work and emitted the exact frozen object, but the screen rose to `120,448,363,838` instructions | the unused cache population is too small for removal to produce a stable full-compile gain. Keep the existing path and require measured improvement even for apparently redundant work | `/tmp/cppgm-current-phases.stderr` and `/tmp/cppgm-qualified-type-cache-off-screen.json` |
| keep class, type, and base-traversal cycle guards inline | replacing tree-backed pointer sets in semantic lookup, conversion, and overload resolution emitted the exact frozen object and used `119,841,579,416` instructions, a `0.267%` reduction. Extending the same representation to class-model recursion reduced the gain to `0.130%`, at `120,005,993,148` instructions | both forms miss the `0.5%` retention floor, and the larger form shows that linear scans and inline-state setup offset allocation savings outside the narrowest traversals. Restore the ordered sets and close the broad traversal-set family | `/tmp/cppgm-inline-traversal-sets-screen.json` and `/tmp/cppgm-inline-traversal-sets-full-screen.json` |
| cache `AstDeclHooks` by scope and reference-only mode | the cache reused all eleven scope-bound callback wrappers for repeated type parsing and preserved the frozen object, but used `120,216,058,374` instructions and raised the one-run footprint to `563,654,656 B` | the inclusive profile cost belongs below the callbacks. Retaining hundreds of bytes of hooks per participating scope adds memory without removing material semantic work, so restore on-demand hook construction | `/tmp/cppgm-decl-hooks-cache-screen.json` |
| borrow the hot exact-template lookup anchor instead of copying it into thread-local storage | a pointer-stack variant kept owned anchors for guards that outlive their source and borrowed the local anchor in `lookup_type_node`. It emitted the exact frozen object but used `120,170,127,684` instructions, effectively flat against the retained checkpoint | anchor copying is not the lookup long pole. Restore the simpler owned stack and focus on template/type resolution below it | `/tmp/cppgm-borrowed-exact-anchor-screen.json` |
| bypass spelling normalization on common template-parameter cache probes | the guarded direct-spelling path preserved the frozen object but used `119,625,165,717` instructions, worse than the simple cache's `119,459,124,931` one-run screen | two empty scratch strings, indirect spelling pointers, and new branches outweighed the avoided string normalization. Keep the uniform normalized probe | `/tmp/cppgm-template-parameter-type-intern-payload-screen.json` and `/tmp/cppgm-template-parameter-type-intern-direct-screen.json` |
| reserve 1,024 buckets for the template-parameter type cache | the reserved cache emitted exact output but screened at `119,629,005,329` instructions, again worse than the unreserved form | libc++ growth for 589 entries is cheaper than carrying the larger bucket table through 62,264 probes. Keep default growth and stop refining this cache | `/tmp/cppgm-template-parameter-type-intern-reserved-screen.json` |
| canonicalize partial-order placeholder named types | three partial-order construction sites shared placeholders by normalized display and semantic key. The frozen object remained exact, but the candidate used `120,114,179,799` instructions, about `0.45%` more than the retained template-parameter checkpoint | these placeholders are short-lived deduction scratch and do not provide the persistent reuse that made template-parameter canonicalization effective. Restore direct construction and keep the canonical cache limited to the measured immutable population | `/tmp/cppgm-partial-order-type-intern-screen.json` |
| allocate `FunctionBinding` records from reusable 64-object slabs | the post-canonicalization census counted 33,713 bindings retaining `50,912,982 B`. A class-specific pool preserved ordinary destruction and recycled discarded slots. It emitted exact output but used `119,500,478,279` instructions, only `0.06%` below the retained median, with effectively unchanged memory | macOS already handles these large, mostly translation-unit-lifetime allocations efficiently. Restore ordinary allocation and avoid custom allocator complexity for a flat result | `/tmp/cppgm-post-template-parameter-intern-memory-census.stderr` and `/tmp/cppgm-function-binding-pool-screen.json` |
| canonicalize function types by structural child-type equality | an opt-in census observed 110,793 function-type requests. The existing pointer-keyed cache handled 66,602 requests; only 926 of its 44,191 misses found a structurally equal live function type. The census output remained byte-identical to the frozen object | recursive structural hashing on every pointer miss would serve less than 1% of all requests and add a second equality path. Remove the census and keep the existing pointer-keyed cache | `/tmp/cppgm-function-type-census.stderr` and `/tmp/cppgm-function-type-census.o` |
| replace absolute-index template-angle heuristic vectors | a current full-run sample attributed 203 leaf samples to zero-fill beneath `can_open_nested_template_angle_at`. The cache census measured 77,602 lookups, 201 hits, 77,401 resizes, and 13,435,475,718 cumulatively zero-filled bytes. A 16-entry sparse cache emitted exact output and screened at `119,186,547,133` instructions, about `0.32%` below the retained median. Keeping only active recursion keys used `119,421,010,995`; split key/state storage used `119,451,627,822`; a generation-tagged dense table used `119,641,995,761` and added about 9 MiB of footprint. The current 16-entry retry improved instructions by `0.202%` and footprint by `0.241%`, for a `0.442` score. Packing state into the index word produced only a `0.105%` instruction gain and a `0.284` score | neither current form meets a retention lane. Restore the dense vectors and close the family | `/tmp/cppgm-post-template-type-profile.sample.txt`, `/tmp/cppgm-template-angle-cache-census.stderr`, `/tmp/cppgm-template-angle-sparse-screen.json`, `/tmp/cppgm-template-angle-active-screen.json`, `/tmp/cppgm-template-angle-sparse-split-screen.json`, `/tmp/cppgm-template-angle-generation-screen.json`, `/tmp/cppgm-template-angle-current-{parent,candidate}-{1,2,3}.time`, and `/tmp/cppgm-template-angle-packed-{parent,candidate}-{1,2,3}.time` |
| add an exact front cache to function-template deduction | a 16K direct-mapped census found 14,250 safe exact hits among 57,679 structural-key builds; a 64K table raised that to only 14,604. Every direct hit was already a structural-cache hit with unchanged use and declaring scope identities. A production form stored pointers to authoritative structural-cache entries, validated current scope fingerprints, and invalidated with the structural cache, but used `119,647,133,207` instructions | hashing every probe and carrying a 64K side table cost more than bypassing 14,604 type fingerprints and temporary argument keys. Restore the single structural cache and close exact front caches around template deduction | `/tmp/cppgm-deduction-exact-census.stderr`, `/tmp/cppgm-deduction-exact-64k-census.stderr`, and `/tmp/cppgm-deduction-exact-cache-screen.json` |
| probe the resolved-template-argument fast cache before the simple resolver | current semantic counters showed 28,453 unsupported simple probes followed by 9,637 exact fast-cache hits. Reordering the existing operations emitted exact output but used `119,570,712,880` instructions, effectively identical to the retained median | the avoided simple probes are cheap and are offset by cache-key work on 1,019 simple successes. Restore the established order and avoid an additional admission cache | `/tmp/cppgm-current-semantic-stats.stderr` and `/tmp/cppgm-resolve-fast-cache-first-screen.json` |
| reuse source-body analysis across the remaining special-member ABI variants | the frozen census counted 2,270 special-member bindings. Existing base-to-complete reuse already handled 2,267 complete variants. Only three complete variants and fourteen deleting variants still ran independently, repeating one top-level source statement in total | entry-specific destructor and virtual-base generation, not repeated user-body analysis, accounts for the remaining variants. Remove the census and avoid a second body-cache path for negligible work | `/tmp/cppgm-special-member-variant-census.stderr` and `/tmp/cppgm-special-member-variant-census.o` |
| bypass recursive function-template argument-combination traversal when every argument has one option | the caller-attributed profile placed 958 samples in the combination runner. A census measured 56,293 deduction leaves and 107,239 argument-option slots; every slot held exactly one value. An iterative move/restore path removed the recursive descent but used `119,875,208,908` instructions, about `0.25%` more than the retained median, while emitting the exact frozen object | recursion is not the cost represented by the inclusive stack. Restore the uniform runner and target deduction, lookup, or argument construction below it | `/tmp/cppgm-post-template-type-frame-profile-2.sample.txt`, `/tmp/cppgm-template-candidate-combination-census.stderr`, `/tmp/cppgm-template-candidate-combination-census.o`, and `/tmp/cppgm-template-candidate-single-option-screen.json` |
| memoize declaration-side source owners for class-template instantiations | 10,187 lookups covered 3,575 concrete class records. Results included 2,262 stable positive repeats, 4,349 stable null repeats, and one null-to-positive transition. A safe declaration-level positive memo would bypass 2,028 calls with no observed owner changes | the positive population is too small relative to the sampled stack to justify another semantic-model field. Caching the dominant null population would require generation tracking at every class-instantiation map mutation. Remove the census and keep the state-sensitive lookup | `/tmp/cppgm-source-owner-census.stderr`, `/tmp/cppgm-source-owner-transition-census.stderr`, and `/tmp/cppgm-source-owner-primary-census.stderr` |
| optimize local and external lookup in the constexpr evaluator | 7,868 evaluator lookups made no frame or scope probes: 4,416 external hits and 3,449 misses all delegated immediately to a hook. The semantic hook handled 5,120 of those calls, all with source capture disabled and no resolved source-use location. Combining inactive-provenance bypass with allocation-free whitespace comparison emitted the exact frozen object but used `119,596,613,209` instructions, effectively flat against the retained median | local scope storage is unused on this workload, and the external hook's visible setup is not the sampled long pole. Restore both paths and target template resolution below the hook | `/tmp/cppgm-constexpr-lookup-census.stderr`, `/tmp/cppgm-constexpr-external-lookup-census.stderr`, and `/tmp/cppgm-constexpr-external-lookup-screen.json` |
| return direct type-parameter alias patterns before general substitution | only 17 of 18,410 resolved aliases had an exact, unqualified type-parameter result pattern. All 17 results were usable, split across `_Result`, `_Select`, `__enable_hash_helper_imp`, and `__enable_if_tuple_size_imp` | this remaining population is negligible after the standard conditional and transform alias families already screened. Remove the census without adding another alias classifier or completion branch | `/tmp/cppgm-direct-parameter-alias-census.stderr` and `/tmp/cppgm-direct-parameter-alias-census.o` |
| inline `strip_top_level_cv` and return its selected `shared_ptr` by reference | the caller-attributed profile showed the out-of-line helper as a 183-sample leaf across 1,627 static call sites. The historical form reduced instructions by `0.165%` and one-run RSS by `1.980%`. Three current pairs measured parent and candidate instruction medians of `115,994,030,695` and `115,724,170,520`, a `0.233%` improvement, with all three pairs agreeing. RSS regressed `0.393%`, footprint regressed `0.008%`, and every object had the frozen SHA-256 | the current score is `0.225`, and RSS exceeds the balanced-lane regression limit. Keep the value-returning out-of-line API rather than widen borrowed ownership across the compiler | `/tmp/cppgm-post-template-type-frame-profile-2.sample.txt`, `/tmp/cppgm-inline-strip-top-cv-screen.json`, and `/tmp/cppgm-strip-top-cv-{parent,candidate}-{1,2,3}.time` |
| move completed LowIR records and emitted instruction buffers into their owners | moving completed functions, blocks, and parsed program records used `119,350,370,239` instructions. Transferring each generated instruction string into block storage improved that to `119,121,763,300`. Extending the same ownership rule to parameters, declarations, globals, aliases, and aggregate items remained flat at `119,114,461,155`, a `0.38%` reduction from the retained median. Every form emitted the exact frozen object | the useful part is instruction-buffer transfer, but the full ownership family still misses the `0.5%` floor and raises one-run RSS by about 11 MiB. Restore ordinary value handoffs and close this copy-removal slice | `/tmp/cppgm-lowir-move-handoffs-screen.json`, `/tmp/cppgm-lowir-move-emission-screen.json`, and `/tmp/cppgm-lowir-move-records-screen.json` |
| borrow persistent ABI type-IR cache entries while emitting mangled symbols | the built-in census measured 174,142 hits, 1,658 misses, and 1,658 entries. Streaming cached types avoided owning copies and preserved the exact frozen object. A first form eagerly constructed a fallback ABI type and used `118,764,457,797` instructions, `0.439%` above the retained checkpoint. Moving fallback construction off the hit path improved that to `118,542,358,680`, still a `0.251%` regression | cache-result copying is measurable, but direct streaming adds branches and separates parameter emission from the optimized aggregate ABI encoder. Restore the existing owning API and close this mangling representation family | `/tmp/cppgm-type-ir-cache-census.stderr`, `/tmp/cppgm-borrowed-type-ir-emission-screen.json`, and `/tmp/cppgm-borrowed-type-ir-emission-screen-2.json` |
| move completed type-pack and value-pack substitution children into their destination vectors | both recursive walkers copied a fully built local `CppAstNode` that died immediately after insertion. Moving those two children emitted exact frozen bytes but used `118,400,593,933` instructions, `0.131%` above the retained checkpoint | the copy removal is real but not material to the full compile. Restore the established handoff and avoid extending this below-floor ownership slice | `/tmp/cppgm-substitution-child-moves-screen.json` |
| bypass all alias source-observation setup when witness capture and template-resolution tracing are disabled | the broader form skipped source-occurrence argument construction, observation records, completion ownership probes, and parameterized-pattern completion setup. It emitted exact frozen bytes and used `118,273,002,736` instructions, effectively flat at `0.023%` above the retained checkpoint | the per-instantiation mode branch offsets the disabled setup. Restore the uniform caller path and close this family together with the earlier partial source-materialization result | `/tmp/cppgm-alias-source-observation-off-screen.json` |
| admit metadata-free negative `class_info_for_type` results before declaration collection completes | the existing registry epoch and metadata guards made the earlier admission exact. It converted 21,330 registry misses into named-key cache hits, reducing misses from 30,928 to 9,598, but used `118,388,459,077` instructions, `0.121%` above the retained checkpoint | the added negative-cache traffic costs as much as the second hash lookup it avoids. Restore post-declaration admission and keep the cache focused on stable negatives | `/tmp/cppgm-early-negative-class-cache-census.stderr` and `/tmp/cppgm-early-negative-class-cache-screen.json` |
| coallocate CallSem optional payloads with their shared control blocks | coallocating the common extra record, rare payloads, qualified names, and interned symbols emitted the exact frozen object but used `118,748,695,498` instructions, `0.425%` above the retained checkpoint. Restricting coallocation to the roughly 91K common extra records was worse at `118,923,591,013`, a `0.573%` regression | the larger allocation changes recover part of the common-record loss but do not make the family competitive. Restore separate object and control-block allocations and keep the existing copy-on-write representation | `/tmp/cppgm-callsem-make-shared-screen.json` and `/tmp/cppgm-callsem-extra-make-shared-screen.json` |
| move cold `FunctionBinding` strings and source anchors to a side record, then compact scalar layout | only 266 of 33,713 bindings stored an explicit `noexcept` expression and 50 stored an object-symbol override; source anchors are unused in normal compilation. Moving that union behind one pointer reduced footprint by about 2.7 MiB but used `118,374,356,578` instructions, `0.109%` above the retained checkpoint. Grouping scalar state removed alignment holes and reduced the record from 824 to 656 bytes, but used `118,496,295,717` instructions, a `0.212%` regression, while reducing footprint by about 5.8 MiB | density alone does not repay cold-access branches or the changed hot-field layout. Restore the original direct fields and close `FunctionBinding` compaction unless a future change removes work as well as bytes | `/tmp/cppgm-function-binding-rare-census.stderr`, `/tmp/cppgm-function-binding-cold-metadata-screen.json`, and `/tmp/cppgm-function-binding-compact-layout-screen.json` |
| retry the interned template-body scope-value snapshot after atom-set compaction | the safe cache stored `(Atom, ValueBinding*)` entries, rebuilt after erase/clear/swap, and preserved live mapped-value updates. The census recorded 44,552 scope visits, 35,194 hits, 9,358 builds, 68 invalidation rebuilds, 3,092 source entries scanned, and 1,825,503 cached entries replayed. A lean optional-vector form screened at `118,075,598,689` instructions. Three interleaved binary pairs measured retained and candidate medians of `118,654,162,513` and `118,365,277,484`, a `288,885,029` instruction or `0.243%` reduction. Median RSS changed from `710,828,032` to `712,835,072`; footprint changed from `524,652,544` to `525,099,008`. All six objects had the frozen SHA-256 | the isolated source view remained rejected. Commit `2ba26c3f4` later removed destination replay in the same collector and met the balanced lane in two independent batches | `/tmp/cppgm-template-body-value-snapshot-census.stderr`, `/tmp/cppgm-template-body-value-snapshot-{screen,lean-screen}.json`, and `/tmp/cppgm-template-body-value-{retained,candidate}-{1,2,3}.time` |
| skip template-bound overlay work for sources with no relevant names | a detailed census measured 276,899 source-scope requests, 166,963 unchanged results, and only 2,413 repeated stable target/source pairs. `115,809` requests made no insertion attempt; `107,924` of those had no relevant source names. An early return emitted the exact frozen object and screened at `111,894,594,757` instructions, about `0.18%` below the clean retained median. Moving the guard to the ancestor wrappers used `112,161,643,952`; combining it with linear lookup in the small exclusion sets regressed to `112,567,833,384` | a result cache has too little exact-pair reuse, while the simple empty work is below every retention lane and does not improve memory materially. Restore the uniform traversal and keep the census as evidence against adding invalidation state | `/tmp/cppgm-overlay-detailed-2.stderr`, `/tmp/cppgm-overlay-empty-screen.json`, `/tmp/cppgm-overlay-empty-wrapper-screen.json`, and `/tmp/cppgm-overlay-empty-excluded-screen.json` |
| use inline visit sets for conversion-function class, virtual-base, and direct-binding traversal | 23,399 conversion-group roots and 21,968 conversion-name roots visited at most four classes and one virtual base, with no duplicate class or virtual-base insertion. The per-class direct-binding set reached two entries and rejected 20,669 duplicate probes. Reusing the retained inline visit set for all three emitted exact bytes but screened at `112,486,953,373` instructions; a binding-only form used `112,540,821,783`, and capacity-sized sets used `112,392,376,486` | the tree allocations are real, but linear scans, larger stack state, and the overflow-vector branch cost more on this path. All three forms regress instructions and miss the allocation-and-latency lane. Restore the tree sets | `/tmp/cppgm-conversion-traversal-census.stderr`, `/tmp/cppgm-conversion-inline-screen.json`, `/tmp/cppgm-conversion-binding-inline-screen.json`, and `/tmp/cppgm-conversion-inline-sized-screen.json` |
| retune the Itanium IR substitution vector/hash crossover | 2,007,030 lookups included 1,979,226 small-state probes, 339,623 small hits, and 11,750,955 structural comparisons. The 28-entry cutoff materialized 1,594 indexes for 46,226 keys. Cutoffs of 8 and 16 used `112,802,228,864` and `112,838,296,308` instructions. Cutoffs of 40 and 64 were closer at `112,258,746,153` and `112,307,177,178`, but still worse than the clean retained median. Every form emitted the frozen object | the current crossover balances structural equality against hash construction better than either direction tested. Keep 28 and do not add another front index | `/tmp/cppgm-ir-substitution-census.stderr` and `/tmp/cppgm-ir-substitution-limit-{8,16,40,64}-screen.json` |
| use one insertion probe while eagerly copying inline-namespace maps | 556 import calls rescanned 61,162 named types, 7,442 values, 118,067 class templates, 46,257 alias templates, and 24,313 variable templates. Replacing `count` plus `operator[]` with `insert` emitted exact frozen bytes but used `112,236,996,211` instructions, slightly worse than the retained checkpoint | the duplicate probe was not the dominant cost. Commit `589b40ac8` removes the redundant copies and retains only the implicit using-directive plus conservative parent invalidation | `/tmp/cppgm-inline-namespace-import-census.stderr`, `/tmp/cppgm-inline-namespace-single-probe-screen.json`, and `/tmp/cppgm-inline-namespace-directive-final.json` |
| store one compact atom vector per parser lookup-snapshot scope | the screen emitted exact output and reduced RSS and footprint, but instructions regressed by about `0.14%`; footprint improved by only about `0.26%` | misses the memory-density lane, and the per-scope vector still retains one allocation per populated scope. Commit `9bcbc6fc6` instead removes those allocations with one delimited buffer per stack | `/tmp/cppgm-compact-lookup-snapshot-screen.json` and `/tmp/cppgm-compact-snapshot-census.stderr` |
| flatten parser lookup-snapshot atoms with separate scope-boundary vectors | three pairs improved instructions by `0.109%` and footprint by `0.424%`, while RSS regressed by `1.31%` | misses the balanced lane's `0.15%` instruction floor and `0.50` score. Commit `9bcbc6fc6` removes the nine boundary vectors and clears the balanced lane | `/tmp/cppgm-flat-lookup-snapshot-screen.json` and `/tmp/cppgm-flat-snapshot-{parent,candidate}-{1,2,3}.time` |
| memoize Analyzer template-dependence results by type identity | a frozen census counted `2,421,362` recursive calls over `31,801` type identities, `2,389,561` repeats, and no changed result. A broad cache used `111,062,135,186` instructions, a `0.155%` regression from the clean checkpoint; a root-only cache was worse at `111,355,980,603` | the repeated fundamental and shallow-wrapper classifications are cheaper than a shared-pointer hash lookup. Restore direct recursion; the zero-transition census does not override the measured loss | `/tmp/cppgm-type-dependence-census.stderr`, `/tmp/cppgm-type-dependence-cache-screen.json`, and `/tmp/cppgm-type-dependence-root-cache-screen.json` |
| add a direct-mapped front cache to template-services type-dependence memoization | the existing weak root memo handled `2,752,628` hits for `27,264` misses. Simulated direct tables hit `215,693`, `361,399`, `477,939`, `631,103`, `1,122,747`, and `1,780,998` times at 16, 64, 256, 1K, 4K, and 16K slots. The 16K production form retained strong type identities and screened at `112,512,270,234` instructions, with footprint rising from `514,641,920 B` to `529,502,208 B` | the front lookup and retained lifetimes cost more than the avoided hash and weak-pointer locks, and both CPU and memory regress far outside every lane. Restore the existing weak root map and close direct fronts | `/tmp/cppgm-template-type-dependency-memo-census.stderr`, `/tmp/cppgm-template-type-dependency-direct-census-2.stderr`, and `/tmp/cppgm-template-type-dependency-direct-screen.json` |
| fuse placeholder binding collection and traversal | the exact one-run candidate used `111,028,274,823` instructions, a regression from the `110,890,016,889` retained checkpoint | removing an intermediate traversal does not remove enough work to offset the fused control flow. Restore the separate walk and stop before paired testing | `/tmp/cppgm-placeholder-fused-scope-screen.json` |
| reserve symbol-sanitizer output capacity | a census found `68,507` calls, `180,279` growth events, `32,617,584` requested capacity bytes, and `12,517,113` input bytes. Five pairs improved median instructions by `0.222%` but regressed wall time by `0.954%`, user time by `0.969%`, and RSS by `0.321%`; only three latency pairs improved. Reserving only `size + 1` regressed wall time, user time, instructions, and cycles | the measured allocation reduction fails the allocation-and-latency lane because latency moves in the wrong direction. Restore ordinary string growth | `/tmp/cppgm-symbol-mangle-allocation-census.stderr`, `/tmp/cppgm-symbol-reserve-{parent,candidate}-{1,2,3,4,5}.time`, and `/tmp/cppgm-symbol-prefix-{parent,candidate}-{1,2,3,4,5}.time` |
| add a direct front cache to `top_level_scope_split` | `2,829,936` calls produced `2,808,299` existing map hits and only `21,637` misses. A simulated 1K direct table intercepted `1,934,256` calls, but the content-validated production form used `110,838,095,430` instructions, only about `0.047%` below the checkpoint, with flat memory | the existing content cache already removes the split work. A second lookup layer misses every lane, so restore the single map | `/tmp/cppgm-scope-split-census.stderr`, `/tmp/cppgm-scope-split-direct-census.stderr`, and `/tmp/cppgm-scope-split-direct-screen.json` |
| keep only CallSem ranking metadata in copied overload candidates | the change removed `68,178` full tree copies, `415,984` copied nodes, `186,767` child allocations, and `30,606,928` requested bytes. Five exact interleaved pairs improved instructions by `0.070%` and RSS by `0.082%`, but regressed wall time by `0.469%`, user time by `0.569%`, and footprint by `0.045%`; only one latency pair improved | this is a large allocation reduction, but it fails the allocation-and-latency lane and every CPU/memory lane. Restore full candidate copies until a representation change also reduces ranking latency | `/tmp/cppgm-call-ranking-metadata-census.stderr`, `/tmp/cppgm-call-ranking-metadata-screen-2.json`, and `/tmp/cppgm-call-ranking-{parent,candidate}-{1,2,3,4,5}.time` |
| borrow dependent-alias template argument vectors from their owning Type | a current census measured `362,565` accessor calls but only `12,257` copying hits, `19,986` arguments, `12,257` outer-vector allocations, and `5,915,856` requested bytes | this adjacent population is far below the allocation lane and much smaller than the retained dependent-class slice. Remove the census and avoid changing roughly thirty callers without stronger profile attribution | `/tmp/cppgm-dependent-alias-argument-copy-census.stderr` |
| store persistent type-dependency root results on each Type | the existing weak map serves about `2.75M` hits for `27,264` misses. A model-pointer field removed the map and emitted exact bytes at `109,634,424,717` instructions, a `0.133%` gain; footprint improved `0.264%`, RSS regressed `0.141%`, and the score was about `0.396`. A clone-safe atomic epoch packed into existing Type padding emitted exact bytes at `109,810,890,704` instructions, losing the CPU gain while RSS rose to `706,428,928 B` | both forms miss every lane. The hot lookup count does not justify adding state to every Type; restore the weak root map | `/tmp/cppgm-type-dependency-inline-cache-screen.json` and `/tmp/cppgm-type-dependency-epoch-cache-screen.json` |
| dispatch BufferedIterator sources through a concrete-source tag | the exact candidate preserved the buffer protocol and passed the focused UTF-8/trigraph reducer, but used `114,585,533,837` instructions, about `4.4%` more than the checkpoint | a branch on every source operation costs more than virtual dispatch. Restore the generic iterator; commit `d94a9aa4a` removes the two proven virtual chains with typed references and no per-character tag | `/tmp/cppgm-tagged-translation-source-screen.json` and `/tmp/cppgm-concrete-translation-source-final.json` |
| specialize only the outer translation buffer's dereference and increment | the exact screen was close enough to require paired evidence. Three interleaved pairs measured parent and candidate medians of `108,464,014,169` and `108,418,368,725` instructions, a `0.042%` improvement. Footprint improved `0.025%`, RSS regressed `0.881%`, wall time improved `0.461%`, and user time improved `0.448%`; only one of three wall and user pairs agreed | the candidate misses the CPU, balanced, memory-density, and allocation-and-latency lanes. Restore the generic outer buffer; the retained concrete inner sources already capture the useful dispatch removal | `/tmp/cppgm-translation-buffer-screen.json` and `/tmp/cppgm-translation-buffer-{parent,candidate}-{1,2,3}.time` |
| specialize all five hot outer translation-buffer operations | the candidate directly implemented dereference, increment, peek, next, pop, and extraction against `FullTranslator`, preserving exact output on the UTF-8/trigraph reducer. Its one-run screen used `108,940,165,427` instructions, a `0.491%` regression from the retained record, and raised footprint by `0.127%`; RSS improved by `0.796%` | this is an obvious CPU loss and fails every lane even before accounting for roughly 150 lines of duplicated buffer protocol. Restore the shared `BufferedIterator` implementation and close further preprocessing dispatch work without a different representation | `/tmp/cppgm-translation-buffer-full-screen.json` |
| promote stable qualified AST type-node lookup results after two observations | the current frame-pointer profile attributes `2,315` of `14,424` samples to `lookup_type_from_ast_node` inclusively. A frozen census measured `199,335` calls, `173,241` first stable observations, `21,803` stable repeats, and only `9,690` third-or-later observations; repeated results changed zero times. The candidate admitted only non-template qualified nodes and validated scope instance, syntax identity and shape, reference-only mode, and mutation state before reuse. It emitted the frozen object at `108,712,229,770` instructions, with RSS `697,638,912 B` and footprint `516,517,888 B` | compared with the retained `108,407,409,101`-instruction record, instructions regress `0.281%` and footprint regresses `0.432%`; the `0.201%` RSS improvement cannot qualify through any lane. Restore direct lookup and treat the inclusive stack as common unique semantic resolution rather than reusable AST results | `/tmp/cppgm-d94-frame-profile-2.sample.txt`, `/tmp/cppgm-type-node-lookup-census-3.stderr`, and `/tmp/cppgm-qualified-type-node-promotion-screen.json` |
