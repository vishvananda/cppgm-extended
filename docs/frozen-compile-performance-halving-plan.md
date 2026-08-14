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
| retain compacted class-template display strings instead of reconstructing them | `145,833,753,997` instructions, only `-0.035%` from the runtime-symbol index checkpoint; one-run RSS was about 8 MiB higher | recursive display reconstruction is visible in the profile but does not move the full compile enough to justify undoing the existing memory compaction | `/tmp/cppgm-retain-named-display-screen.json` |
| consume normalized input with one streambuf fetch per byte, then extend it with an 8 KiB `sgetn` buffer | the single-fetch form used `145,373,405,816` instructions (`-0.35%` from the runtime-symbol checkpoint), while buffering regressed to `146,013,046,659` (`+0.09%`) | neither form meets the retention threshold, and buffering adds 8 KiB to every `Normalizer`; restore the original streambuf iterator before pursuing tokenizer work further | `/tmp/cppgm-normalizer-single-fetch-screen.json`, `/tmp/cppgm-normalizer-buffered-input-screen.json` |
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
| extend dependent-type resolution reuse across root calls | a frozen census measured 307,516 roots, 177,324 distinct `(scope instance, binding fingerprint, type)` keys, and 130,192 repeated results with no observed transitions. Of the repeats, 128,935 remained unresolved; their outer kinds were led by 108,361 named types, including 48,700 template parameters and 39,321 dependent types. Returning immediately after an unbound template-parameter lookup screened at `135,673,819,299` instructions. An operation-scoped tree cache found 46,926 hits against 261,857 misses across 208,697 operations, with at most 39 entries, and screened at `135,927,876,353`. A single-entry last-root memo retained 45,005 hits. Its three-run median was `135,359,414,885`; a same-host alternating comparison measured retained `135,580,233,289` versus candidate `135,154,528,753`, a `0.314%` reduction. Retained and candidate median RSS were `730,443,776` and `727,146,496`; footprints were `555,524,096` and `555,024,384`. All six paired objects had SHA-256 `4fc1303a...5c4` | the only cache shape that repaid its lookup cost remains below the `0.5%` retention threshold. Restore the resolver and services interface, and do not add global semantic invalidation machinery for this population | `/tmp/cppgm-dependent-resolution-roots-2.stderr`, `/tmp/cppgm-dependent-resolution-kind-census.stderr`, `/tmp/cppgm-unbound-template-parameter-screen.json`, `/tmp/cppgm-operation-resolution-cache-screen.json`, `/tmp/cppgm-dependent-operation-cache-census.stderr`, `/tmp/cppgm-dependent-operation-last-cache-census.stderr`, `/tmp/cppgm-operation-last-resolution-screen.json`, `/tmp/cppgm-operation-last-resolution-decision.json`, `/tmp/cppgm-operation-ab-{retained,candidate}-{1,2,3}.time` |
| skip alias source-occurrence materialization when witness capture is disabled | the frozen compile built 19,532 alias source-occurrence vectors containing 31,604 copied arguments and 31,604 deep-copied syntax records; every call had source capture disabled. Guarding only those records screened at `135,680,956,488` instructions. Also borrowing the caller's argument-text vector instead of copying and rewriting witness spellings improved the screen to `135,082,147,793`; the three-run median was `135,200,957,915`, about `0.28%` below the immediately preceding retained A/B median of `135,580,233,289`. The frozen object remained exact | the allocations are avoidable but do not meet the `0.5%` full-compile retention threshold. Keep the uniform source-observation path and seek a larger allocation owner | `/tmp/cppgm-alias-source-arguments-census.stderr`, `/tmp/cppgm-alias-source-arguments-screen.json`, `/tmp/cppgm-alias-source-materialization-screen.json`, `/tmp/cppgm-alias-source-materialization-decision.json` |
| borrow named-type strings while finding a template parameter | `find_template_parameter(TypePtr)` copied the named key, semantic payload, and source name into a local candidate array even though it only compared them. Replacing the copies with pointers produced a three-run median of `135,050,779,339` instructions versus `135,434,571,754` for the interleaved retained build, a `0.283%` reduction. Candidate and retained median RSS were `727,531,520` and `735,023,104`; footprints were `555,859,968` and `555,630,592` | the ownership cleanup removes sampled string allocations, but the full-compile instruction change is below the `0.5%` retention threshold. Restore the value array and use the profile to find a larger allocation family | `/tmp/cppgm-template-parameter-candidate-refs-{screen,candidate-2runs,retained-screen,retained-2runs}.json` |
| remove tree and `std::function` scratch from value lookup through using-directives | the fresh profile attributed 56 top-stack samples and 21 direct allocations to `Analyzer::lookup_value_at_token`. A direct recursive helper used the retained inline scope visit set and ancestor comparison in place of two tree sets and a recursive `std::function`. The initial three-run candidate median was `133,162,716,462`, but its post-commit absolute median moved to `133,581,240,724`. A contemporaneous three-pair binary A/B measured parent `133,535,105,679` versus candidate `133,254,277,052`, only `-0.210%`. Candidate and parent median RSS were `752,021,504` and `749,436,928`; footprints were `556,036,096` and `555,831,296`. The candidate emitted the exact frozen object and passed direct strict `1530/1530` and full report `4863/4863` | the allocations are real, but the paired instruction reduction is below the `0.5%` threshold and secondary memory signals are slightly worse. Commit `ddd19fd3a` records the tested form and `709061c3f` removes it | `/tmp/cppgm-value-using-directive-scratch-{screen,decision,final}.json`, `/tmp/cppgm-value-ab-{parent,candidate}-{1,2,3}.json`, and `/tmp/cppgm-using-directive-profile.sample.txt` |
| store template-body visible value types in a sorted contiguous map | the companion value map supported only lookup, overwrite, copy, and const iteration, so a private sorted vector could preserve its interface. Its one-run screen used `132,755,079,558` instructions, but a contemporaneous three-pair binary A/B measured parent median `132,781,984,174` versus candidate `133,060,302,274`, a `0.210%` regression | the smaller value-map population does not repay ordered insertion and binary lookup. Restore the hash map and stop extending the template-body container-rewrite family | `/tmp/cppgm-template-body-sorted-value-map-screen.json` and `/tmp/cppgm-value-map-ab-{parent,candidate}-{1,2,3}.json` |
| inline or borrow the result of `strip_top_level_cv` | exposing the three-branch body in the header screened at `132,748,141,512` instructions, only about `0.03%` below its contemporaneous parent median. A later profile attributed 202 leaf samples to the helper across 1,692 source uses. Returning a borrowed `const TypePtr&` removed refcount pairs from inspection-only callers but screened at `128,509,370,739`, effectively flat against the retained `128,444,585,958` median | neither call dispatch nor result ownership explains enough of the profile attribution. Restore the value-returning out-of-line API and close this helper family after two measured forms | `/tmp/cppgm-inline-strip-cv-screen.json`, `/tmp/cppgm-using-directive-profile.sample.txt`, `/tmp/cppgm-lazy-reserve-profile.sample.txt`, and `/tmp/cppgm-borrow-strip-cv-screen.json` |
| devirtualize the concrete UTF-8 and full-translation source chain | the fresh preprocessing sample showed 251 `Normalizer` increment leaves and 144 buffered-iterator increment leaves. Giving both translators concrete source references screened at `129,167,163,822` instructions and produced a three-run median of `129,099,623,064`, but PA1 `100-trigraph-lookahead-utf8` decoded `?π` as `?Ï€`. A typed FullTranslator refill/pop variant remained incorrect and produced `ππ`. Restricting the change to the UTF-8-to-Normalizer layer passed all 53 PA1 tests but screened at `132,400,730,462`, about `0.08%` above the retained checkpoint | the apparent `2.42%` gain violates the UTF-8/trigraph lookahead contract, while the semantics-preserving subset is flat. Restore the original virtual buffer protocol and close this devirtualization family | `/tmp/cppgm-register-interval-profile.sample.txt`, `/tmp/cppgm-translator-typed-source-{screen,decision}.json`, `/tmp/cppgm-utf8-normalizer-typed-source-screen.json`, and `pa1/tests/100-trigraph-lookahead-utf8.t` |
| retry function-symbol generation only when function-value rehydration changes its pointer | the frozen compile emitted 69,797 function-symbol trace events, including 29,900 exact repeated results. A focused census found 19,000 linkage-upgrade calls: 9,388 left the symbol unchanged, and one binding was retried 118 times. Suppressing a retry after the same function pointer had already been installed emitted exact frozen bytes but screened at `132,026,096,185` instructions, only `0.21%` below the retained checkpoint | the redundant upgrades are measurable, but the safe local guard does not meet the `0.5%` retention threshold. Restore the retry behavior; do not add a broad symbol-state cache without an explicit mutation contract | `/tmp/cppgm-symbol-trace.log`, `/tmp/cppgm-symbol-upgrade-census.log`, and `/tmp/cppgm-rehydrate-on-change-screen.json` |
| defer class-member symbol construction until a provisional binding survives merge lookup | class-member registration mangled and reserved every provisional binding before checking the existing overload slot. Moving that work to the insertion path screened at `131,679,982,602` instructions and emitted exact frozen bytes. A three-pair binary comparison measured a parent median of `132,071,823,847` and candidate median of `131,862,510,228`, a `0.158%` reduction | the ordering cleanup removes real discarded work but does not meet the `0.5%` retention threshold. Restore eager construction and reservation rather than changing registration side-effect order for a sub-threshold gain | `/tmp/cppgm-deferred-class-symbol-screen.json` and `/tmp/cppgm-deferred-class-symbol-ab-{parent,candidate}-{1,2,3}.json` |
| extend inline visited storage to the remaining using-directive lookup APIs | the generic lookup helpers already use the retained eight-pointer visit set, but callable, function-template, value, and qualified-namespace helpers still used tree sets. Converting that whole family preserved the frozen object and screened at `132,072,257,170` instructions, about `0.17%` below the retained interval-reuse checkpoint | the remaining tree allocations are too small a share of the compile to justify changing a public lookup interface. Restore the tree-set API and keep the proven inline representation only on the high-volume generic paths | `/tmp/cppgm-remaining-using-directive-inline-screen.json` |
| bypass recursive argument-combination enumeration when every template call argument has one interpretation | the late profile attributed 593 inclusive samples to the combination runner. Moving each sole option into the deduction input once and calling its terminal step directly preserved the frozen object, but screened at `132,208,159,899` instructions, about `0.07%` below the retained interval-reuse checkpoint | the profile attribution belongs to deduction and binding acquisition below the runner; its recursion and move bookkeeping are negligible. Restore the uniform enumerator and investigate the work inside deduction instead | `/tmp/cppgm-single-template-combination-screen.json` |
| add an exact-pointer front cache before structural function-template deduction keys | 45,199 structural cache hits included 28,168 hits with identical type pointers, all with at most four arguments. A fixed direct cache served 25,400 hits with 4K slots and 27,222 with 16K slots. The best one-run screen used `131,710,486,668` instructions, but an interleaved three-pair binary comparison measured parent median `132,052,142,360` versus candidate median `132,161,188,380`, a `0.083%` regression | repeated type fingerprints are real, but hashing, scope validation, and the second cache probe consume the savings. Restore the single structural cache; do not add another deduction cache layer | `/tmp/cppgm-deduction-pointer-census.stderr`, `/tmp/cppgm-deduction-fast-cache-screen{,-2,-16k}.json`, and `/tmp/cppgm-deduction-fast-ab-{parent,candidate}-{1,2,3}.json` |
| bulk-insert visible scope names during template-body validation | the fresh post-backend profile attributed 95 leaf samples to visible-scope value collection. One form sorted each batch and merged it into the retained contiguous name set; a second appended the batch and sorted in place. They used `130,646,281,297` and `130,576,934,916` instructions, about `0.18%` and `0.24%` below the retained three-run median | batching removes per-name vector shifts but cannot meet the `0.5%` retention threshold. Restore individual insertion and close this destination-container family after two measured forms | `/tmp/cppgm-backend-metadata-profile.sample.txt`, `/tmp/cppgm-template-visible-bulk-insert-screen.json`, and `/tmp/cppgm-template-visible-bulk-sort-screen.json` |
| trim temporary strings in place through an rvalue overload | the post-lazy-reserve profile attributed 73 leaf samples to the shared trim helper and 27 to the mangler-local copy. Rvalue overloads reused temporary `substr` and generated buffers while preserving the lvalue API, but screened at `128,511,249,469` instructions, effectively flat against the retained `128,444,585,958` median | temporary-buffer allocation is too small and dispersed to justify a second trim API. Restore the single const-reference helper | `/tmp/cppgm-lazy-reserve-profile.sample.txt` and `/tmp/cppgm-trim-rvalue-screen.json` |
