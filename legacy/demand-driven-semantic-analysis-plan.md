# Demand-Driven Semantic Analysis Plan

## Goal

Replace the current eager seed-pass + output-fixpoint architecture with a
demand-driven dependency DAG that pulls semantic work transitively from the
externally observable roots of a translation unit. The target is a 5-10x
wall-time reduction on `dev/src/callsemantic.cpp` self-compile (currently 20+
minutes), bringing it within striking distance of state-of-the-art compilers
without micro-optimizing further.

This plan supersedes the iterative per-slice strategy in
`pa33-540-compile-performance-plan.md` and
`semantic-analysis-performance-plan.md` for the cases where those plans have
plateaued. It does not replace them: the smaller wins they describe are
complementary and should continue. This document targets the architectural
ceiling those plans cannot cross.

## Problem Statement

### What the existing data already proves

Counters from `benchmarks/self_compile/stable/semantic_overload.cpp` (per
`semantic-analysis-performance-plan.md`):

- `class-info-for-type-calls=9,885,310`
- `class-info-for-type-pointer-cache-hits=8,372,031`
- `complete-class-type-calls=250,633`, of which `184,849` return no-class
- `resolve-template-argument-calls=306,251`
- `class-template-key-builds=176,554`

After every cache and key-build optimization, the compiler still asks the same
semantic question 8.4 million times on a single TU. The leaf is fast; the
caller is wrong.

### Why micro-optimization has plateaued

`pa33-540-compile-performance-plan.md` records ~30 measured experiments since
2026-04-30. Median wins per slice are -1% to -3%, several reverted as noisy or
regressing. The cumulative improvement against the integration baseline is
roughly -2% to -5% on `540`. This is what hitting an architectural ceiling
looks like.

### The architectural diagnosis

`Analyzer::analyze` in `dev/src/callsemantic.cpp` runs four expensive phases:

1. `semantic.collect_declarations` (line 276): cheap; registers names only.
2. `semantic.output_seed` (lines 304-326): walks every top-level declaration
   in `ast.children`, calling `analyze_declaration_output` which recurses
   through every namespace member.
3. The do-while fixpoint (lines 328-402): six subpasses, each rescanning
   `state.instantiated_classes`, `state.instantiated_functions`,
   `state.synthetic_functions`, `state.late_required_class_methods`, and
   pending queues until sizes stop changing.
4. `semantic.validate_required_definitions` and witness analysis.

`analyze_declaration_output_impl` in `dev/src/semantic_output.cpp:4325` does
not just emit. Per the existing `semantic-analysis-algorithmic-strategy.md`,
it "reparses output-facing declaration types, emits function declarations,
checks namespace variable emission, evaluates constant initializers, and
invokes lifetime analysis for class variables. For class specifiers it calls
class output, which completes the class if needed." The seed pass and the
fixpoint subpasses are therefore the **driver of semantic analysis**, not a
post-semantic emission step. Phases 2 and 3 together are where the 9.85M
class-info calls happen.

The work is super-linear because each fixpoint iteration that adds a new
template instantiation can force the next iteration to revisit prior queues.
With K iterations and N items, the effective work is O(N*K). On
`callsemantic.cpp` both N and K grow with header weight, producing the
observed 170x slowdown over `540`.

## Hypothesis

The fraction of declarations and template instantiations that are
transitively reachable from the actual emitted symbols of a TU is small. If
the compiler analyzed only that reachable subset, semantic counters would
drop by 5-20x and wall time would drop in proportion.

This hypothesis must be measured before implementation. A wrong measurement
result invalidates the architectural plan and redirects effort toward
node-size compaction or fixpoint flattening instead.

## Phase 0: Reachability Probe

Before any architectural change, prove or disprove the hypothesis cheaply.

### Probe Implementation

Add a post-analysis pass that runs after the existing fixpoint completes.
Inputs: the final `OutputState` and `out` `DumpNode`. Outputs: for one TU,
the ratio of reachable to scanned semantic objects.

Algorithm:

1. Define roots:
   - every direct child of `out` whose origin is the primary source file
   - every emitted RTTI/vtable symbol
   - every static initializer
   - every explicit instantiation
2. Walk the transitive closure over:
   - callees of emitted function bodies
   - types referenced by emitted signatures
   - base classes and direct member types of reached classes
   - template arguments of reached instantiations
3. Record:
   - `reached_classes / total_classes`
   - `reached_functions / total_functions`
   - `reached_template_instantiations / total`
   - per-fixpoint-subpass: items scanned vs items in reachable set

Gate behind `CPPGM_REACHABILITY_PROBE=1`. Do not change behavior. Run once
on `callsemantic.cpp` and once on `pa33-reference-wrapper-smoke`.

### Decision Rule

| Reached / Total | Implication |
|---|---|
| ≤ 20% | Demand-DAG is the right architectural fix. Proceed with this plan. |
| 20-50% | Demand-DAG helps but node compaction also matters. Run both. |
| ≥ 50% | Demand-DAG ceiling is small. Focus on node compaction and fixpoint flattening instead. |

The probe is a hard gate. Phase 1 does not start without a number.

### Exit Criteria

- Probe builds and runs without changing behavior on the strict suites.
- Reachability ratios reported for at least `callsemantic.cpp` and one
  hosted-header-heavy PA33 case.
- `benchmarks/self_compile/stable/callsemantic_frozen.cpp` is checked in,
  taken from the same integration commit as the Phase 0 baseline binary.
  One single-run baseline number recorded as the checkpoint anchor; this
  file is not run again until end of Phase 2.
- Phase 0 baseline numbers also recorded for `semantic_overload.cpp`
  (counters + 3-run median wall time). This is the per-slice baseline used
  through Phases 1 and 2.
- This plan's Phase 1 starts only if the data supports it.

## Phase 1: Demand-Level API

Introduce a single explicit API for "what does this caller actually need to
know?" without touching the existing fixpoint.

### Demand Levels

```cpp
enum class ClassDemand {
  Identity,     // Is this a class? What's its name? (T*, T&, decltype)
  Members,      // Member declarations (member access, name lookup)
  Layout,       // sizeof, alignof, offsetof, ABI placement
  Lifetime,     // Construction, destruction, copy/move semantics
  Output,       // Vtable, RTTI, definition emission
};
```

### Implementation

1. Add a memo table keyed by `(NamedType*, ClassDemand)`. Today
   `class_info_for_type` is a single function; this becomes a function
   parameterized by demand level.
2. Convert callers in tranches, starting with the cheapest:
   - `Identity`: pointer/reference type construction, decltype, name lookup
   - `Members`: member access (`.`/`->`), nested-name-specifier resolution
   - `Layout`: `sizeof`, `alignof`, `offsetof`, type-trait evaluation,
     parameter passing
   - `Lifetime`: object construction, destruction, copy elision decisions
   - `Output`: existing `complete_class_type` callers in `analyze_*_output`
3. The existing `complete_class_type` becomes a wrapper that requests
   `Output` to preserve current behavior. New code should never call the
   wrapper.
4. Add per-level counters. Run the same `semantic_overload.cpp` benchmark
   and confirm that calls shift toward `Identity` and away from `Output`.

### Negative-Class Fast Path

Per the existing data, 184,849 of 250,633 `complete_class_type` calls return
no-class. Set a `is_definitely_not_class` bit on `Type` at construction
time for fundamental types, function types, and pointer/reference/array
types. Make `class_info_for_type` an immediate `nullptr` return when that
bit is set. This is independently valuable and should land first within
Phase 1 because it is small and risk-free.

### Exit Criteria

- All hot callers of `class_info_for_type` and `complete_class_type` use
  the demand-level API.
- The negative-class bit eliminates the 184k no-class calls outright.
- Strict suites `pa18 pa19 pa21 pa22` are clean.
- `semantic_overload.cpp` counter for `class-info-for-type-calls` drops by
  at least 50% (from 9.85M to under 5M). If it does not drop, the demand
  threading is not reaching the right callers; do not proceed to Phase 2
  until it does.

## Phase 2: Output Roots And Fixpoint Removal

Replace the do-while fixpoint with a single demand-driven walk.

### Root Collection

During `collect_top_level_declarations`, build an explicit `OutputRoots`
list with classified entries:

- `signature_output`: namespace function declarations whose definitions are
  in this TU
- `definition_output`: function definitions in this TU
- `variable_output`: namespace-scope variable definitions in this TU
- `forced_instantiation`: explicit instantiations
- `forced_witness`: declarations the witness session marks as required
- `forced_debug`: declarations forced by `emit_all_source_function_definitions`

These roots are derived from the AST; they do not require semantic analysis
to compute.

### Demand Walk

Replace the seed pass at `callsemantic.cpp:304-326` and the do-while at
`callsemantic.cpp:328-402` with:

```
work_queue = OutputRoots
while not work_queue.empty():
  root = work_queue.pop()
  result = analyze_for_emission(root)  // pulls dependencies
  for new_dep in result.discovered_dependencies:
    if not already_processed(new_dep):
      work_queue.push(new_dep)
```

`analyze_for_emission` is the demand-DAG entry point. When it needs a class
member, it requests `Members`-level. When it needs a callee body, it
queues that callee. When it needs a template instantiation, it queues the
instantiation. There is no global rescan: each item is processed exactly
once, and dependencies flow only outward from roots.

### What Goes Away

- `do { ... } while (pending_output_changed)` at `callsemantic.cpp:328-402`
- `analyze_declaration_output` recursive walks of every namespace member
  for declarations not transitively referenced from a root
- The six fixpoint subpasses become a single dispatcher driven by the
  worklist's element type
- `pending_late_required_class_methods` and similar pending queues fold
  into the worklist

### What Stays

- `state.required_function_definitions` and similar maps stay as the
  "already processed" set
- `validate_required_function_definition_closure` runs once at the end
- Witness session and source-location capture are unchanged

### Exit Criteria

- Strict suites `pa14 pa15 pa16 pa18 pa19 pa21 pa22` are clean.
- `pa33` and `pa34` focused compile cases produce identical LowIR text.
- `semantic_overload.cpp` counter for `complete-class-type-calls` drops
  by at least 70% (from 250k to under 75k).
- `semantic_overload.cpp` wall time drops by at least 3x against the
  Phase 0 middle-tier baseline. This is the per-phase gate; the 5x claim
  on `callsemantic_frozen.cpp` is verified once at the end of Phase 2 as
  a checkpoint run, not per slice.
- Fixpoint iteration counter is removed (the loop no longer exists).

## Phase 3: Cross-TU Header Cache (Optional, Parallel)

Independently of Phases 0-2, eliminate redundant work across TU boundaries.

### Mechanism

After Phase 2 lands, the demand DAG for the libc++ portion of any TU is
deterministic given the include set and the configured demand-level
queries. Persist that DAG to disk:

- Hash key: `(include_set, host_compiler_id, language_mode_flags)`
- Stored data: serialized `Type *` graph, `ClassInfo` slots,
  template-instantiation key map, demand-level memo table for libc++ types
- Storage location: `obj/cppgm-hdrcache/<hash>.cppgmhdr`

On the next compile of any TU with the same include set, restore the cache
and skip header semantic analysis. This is conceptually a precompiled
header but at the post-demand-DAG granularity rather than at the AST
granularity.

### Why This Is Easier After Phase 2

Today there is no clean cut between "semantic state from headers" and
"semantic state from this TU's source" because everything was eagerly
analyzed together. After Phase 2, the demand DAG records exactly which
nodes were pulled by which roots; library nodes pulled by library roots
form a serializable subgraph keyed only by the include configuration.

### Exit Criteria

- A warm header cache reduces `callsemantic.cpp` self-compile by at least
  another 2x.
- Cache invalidation on include-set changes is correct on a focused test
  matrix.
- No correctness regression on the full PA test report.

## Validation Strategy

### Frozen Benchmark Inputs

A 20+ minute compile cannot drive iteration: a 3-run median is ~2 hours per
check and forces every architectural decision to wait that long. Worse, the
expected demand-DAG win (5x or more) is so far above noise that medians
aren't needed at that scale anyway — one run answers the question.

The plan uses a tiered ladder of **frozen** benchmark inputs, all immutable
for the duration of this work:

| Tier | Benchmark input | Wall time | Use |
|---|---|---|---|
| Inner | `pa33-reference-wrapper-smoke` (540) and a few PA18/PA21 LowIR cases | ~7s / <1s | per-edit correctness, parser/output sanity |
| Middle | `benchmarks/self_compile/stable/semantic_overload.cpp` | ~190s | **per-slice and per-phase counter + wall-time gate** |
| Checkpoint | `benchmarks/self_compile/stable/callsemantic_frozen.cpp` | 20+ min | **single-run capstone at major boundaries only** |

The middle tier carries essentially all iteration. `semantic_overload.cpp`
already exhibits the exact pathology this plan targets (the 9.85M
class-info, 250k complete-class, 306k template-resolve counters in
`semantic-analysis-performance-plan.md` came from it, not from
callsemantic.cpp). It is large enough to make counter shifts visible and
small enough to run in three minutes. If the demand-DAG fixes the
architectural problem on `semantic_overload.cpp`, it will fix it on
`callsemantic.cpp` — they share the pathology by construction.

The checkpoint tier is run only at:

1. **Phase 0**: one baseline run on `callsemantic_frozen.cpp` to anchor the
   "20+ minutes" claim with a real number.
2. **End of Phase 2**: one run to confirm the architectural claim. Expected
   shape: 5x or more improvement against the Phase 0 baseline. A single
   run is fine because anything below 3x at that magnitude is a clear
   negative result regardless of noise.
3. **End of Phase 3**: one run to confirm the cross-TU header cache
   compounds with the demand-DAG win.

That is 3 checkpoint runs over the whole plan, not per phase. If the middle
tier shows counter regressions or unexpected behavior, debug there first;
do not run the checkpoint to "see what's happening." The checkpoint is a
yes/no answer to "did the architectural claim hold," not a diagnostic
tool.

Phase 0 freezes both files. Take the `callsemantic_frozen.cpp` snapshot
from the same integration commit used as the Phase 0 baseline and check it
in beside `semantic_overload.cpp`. Do not modify it after freezing.

One residual confound to watch: a frozen `.cpp` is still compiled against
the **live** `dev/src/*.h` headers. If a phase changes a header that
materially alters what the frozen files parse to, the cross-phase
comparison breaks. Mitigation: scope public header changes tightly (only
the Phase 1 demand-level enum is on the slate), and when a header does
change, re-baseline the prior candidate against the new headers before
claiming a phase delta. This is the same constraint
`semantic_overload.cpp` operates under.

### Correctness Gates

After each Phase 1 sub-tranche and after Phase 2 lands:

```sh
make test-strict STRICT_PAS='pa18 pa19 pa21 pa22' STRICT_SUBTEST_JOBS=8 \
  CPPGM_STRICT_SEMANTIC_FALLBACKS=1 \
  CXX=/usr/local/opt/llvm/bin/clang++ \
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++

make test-report ACTIVE_TEST_REPORT_PAS='pa10 pa11 pa12 pa14 pa15 pa16'
make test-report ACTIVE_TEST_REPORT_PAS='pa18 pa19 pa21 pa22'
```

LowIR text comparison for `pa18 pa19 pa21 pa22` is the strictest check and
should be the gate before any merge.

### Performance Gates

Per-slice and per-phase gate: counter and wall time on the **middle tier**
(`semantic_overload.cpp`). A patch must move counters in the predicted
direction and not regress wall time outside noise. This is the only
performance gate run during normal iteration.

Checkpoint gate (Phase 0 baseline, end of Phase 2, end of Phase 3): single
run on the **checkpoint tier** (`callsemantic_frozen.cpp`). Phase 2 must
reduce it by at least 5x against the Phase 0 baseline. A single run is
sufficient at that magnitude — anything below 3x is a clear negative
regardless of run-to-run variance.

Cross-benchmark sanity: focused PA33 cases
(`pa33-reference-wrapper-smoke`, `pa33-long-unordered-map-find`,
`pa33-long-recursive-std-function-string`) should improve monotonically
across phases or at minimum not regress. If the middle tier improves but
PA33 cases regress, the demand walk is over-aggressive.

### Counter Gates

The plan succeeds only if these counter changes happen, regardless of wall
time:

- `class-info-for-type-calls` drops from 9.85M to under 1M
- `complete-class-type-calls` drops from 250k to under 50k
- `resolve-template-argument-calls` drops from 306k to under 100k
- Fixpoint iteration count drops from "more than one" to zero (no fixpoint)

If wall time improves but counters do not, the implementation is hiding
work in a different place and is not actually demand-driven.

## Risks And Open Questions

### Strictness Of "Reachable"

C++ rules sometimes require completeness for reasons that are not
syntactically visible. Examples: defining a function whose return type is a
class with a non-trivial destructor; type traits that require checking
private members; ADL adding candidates that bring in unrelated types.
The demand walk must be conservative: when in doubt, escalate the demand
level rather than skip work. Phase 1's per-level counters surface where
the conservative escalation happens; if escalation is the common case, the
ceiling drops.

### Witness And Diagnostic Output

The witness session records source-use occurrences across the whole TU.
A naive demand walk would skip declarations the witness still wants to see.
The Phase 2 root list explicitly includes `forced_witness` to handle this,
but the breadth of witness requirements needs an audit before Phase 2.

### Fallback Safety

The current fixpoint is forgiving: anything that should have been processed
will eventually be processed because everything gets rescanned. The demand
walk has no such safety net. The migration must run both schemes side by
side under `CPPGM_DEMAND_DRIVEN=1` and diff the resulting `OutputState`
contents on a wide test matrix before flipping the default.

### Phase 2 Is A Big Patch

Unlike the per-slice patches in `pa33-540-compile-performance-plan.md`,
Phase 2 is a single architectural change. It cannot be landed in tranches
without the seed pass and fixpoint coexisting. The intermediate state
should be: both paths exist, demand path is gated by env var, default is
unchanged, and the gate flips only after counter and LowIR equivalence are
proven on the full test matrix.

## What This Plan Does Not Do

- It does not shrink `CallSemNode`. That is a separate, complementary plan;
  it should run after Phase 2 because the demand walk is the larger win
  and node compaction risks are easier to evaluate against a stable
  semantic core.
- It does not address parser or preprocessor cost. Those are already at the
  wall-time floor that recent tranches established.
- It does not change witness, mangling, or LowIR. The plan is purely about
  when and whether semantic analysis runs.

## Sequencing Summary

| Phase | Expected Win | Gate (middle tier unless noted) |
|---|---|---|
| 0 - Reachability probe + freeze | none (measurement) | reached/total ratio + baselines recorded |
| 1a - Negative-class bit | -1.5M class-info calls | counter delta |
| 1b - Demand-level API | -50% class-info calls | counter delta + strict |
| 2 - Output roots + fixpoint removal | 3x+ middle, 5x+ checkpoint | counter delta + LowIR diff + 1 checkpoint run |
| 3 - Header cache | additional 2x on checkpoint | counter delta + 1 checkpoint run |

Phase 0 starts immediately. Phase 1a can run in parallel with the probe.
Phase 1b starts only with the probe number in hand. Phase 2 starts only
when Phase 1 counters confirm the demand-level threading reaches the right
callers. Phase 3 is independent and can run in parallel with Phase 2.
