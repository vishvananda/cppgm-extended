# Witness Lifecycle Semantic Convergence Plan

## Status and handoff

This plan starts from the completed class-use semantic convergence endpoint.
The class-use work left one source-use observer and exposed lifecycle logging as
the next large source of repeated semantic work.

- Repository worktree: `/private/tmp/cppgm-witness-lifecycle-convergence`
- Branch: `experiment-witness-lifecycle-semantic-convergence`
- Parent and fixed semantic checkpoint:
  `0662098ba13097ae0ac059593443a311f769a59b`
- Patched-Clang comparison checkout:
  `/Users/vishvananda/llvm-project-template-metrics-20260416`
- Patched-Clang comparison head: `59c5d9c70`
- Execution ledger:
  `docs/witness-lifecycle-semantic-convergence-ledger.md`

The fixed checkpoint includes the class-use diagnostic counters and the final
ordinary-path performance cleanup. Its three-run confirmation batch supplies
the performance baseline for this work. The original checkout continues its
class-use inception run while this worktree proceeds.

## Objective

Move lifecycle observation into the semantic operations that own template
state changes. Delete the independent logging arms that infer the same change
from call context, source syntax, or repeated acquisition.

The final implementation must have:

- one typed lifecycle transition model shared by function, class, and value
  template operations;
- one lifecycle observer that converts completed transitions into witness
  events;
- one semantic owner for each state change: acquire, require definition,
  ensure definition, materialize definition, instantiate, and finalize;
- explicit instantiation routed through the same acquisition and finalization
  operations as implicit instantiation;
- constant evaluation consuming the selected or materialized value transition
  instead of manufacturing an `integral_constant::value` event;
- unnamed and nested classes reported from class completion transitions
  without recursive post-completion field scans;
- no witness-only template lookup, entity reconstruction, or declaration
  location recovery after a transition completes;
- no exact-duplicate or enrichment actions in the lifecycle recorder on the
  strict corpus;
- fewer production source lines and fewer retired instructions than the fixed
  checkpoint.

Memory reduction remains a target. The final footprint may not exceed the
fixed checkpoint by more than 1%. Maximum RSS uses the 3% warning and
confirmation rule.

## Starting evidence

The final class-use provenance run provides the starting lifecycle evidence:

- trace directory:
  `/tmp/cppgm-class-use-final-provenance.5eQcbQ`;
- report:
  `/tmp/cppgm-class-use-final-provenance-report.json`;
- report SHA-256:
  `e94d543d32cd24cbf43c36b8386de1a3570fd2842308246d781be68c7013485f`;
- 1,180 trace files and 59,412 provenance records;
- 34,383 lifecycle attempts;
- 24,072 exact duplicates, 267 enrichments, and 10,044 inserted events;
- 11 static producers, 10 exercised producers, and one unexercised producer.

The recorder retained these event counts:

| Kind | Retained events |
| --- | ---: |
| Ensure definition | 3,314 |
| Function instantiation | 3,226 |
| Require definition | 2,177 |
| Variable instantiation | 784 |
| Class instantiation | 487 |
| Class finalization | 56 |

### Producer inventory

| Producer | Attempts | Duplicates | Enrichments | Inserted | Current semantic route |
| --- | ---: | ---: | ---: | ---: | --- |
| `lifecycle.template_api.01` | 24,679 | 15,705 | 265 | 8,709 | Generic function binding wrapper called by acquisition, definition closure, and body materialization arms |
| `lifecycle.template_api.02` | 8,023 | 7,317 | 2 | 704 | Static member value materialization and retained dependency replay |
| `lifecycle.template_api.07` | 1,449 | 963 | 0 | 486 | Generic class acquisition and finalization wrapper |
| `lifecycle.constant_value_lookup.02` | 111 | 78 | 0 | 33 | Manual `integral_constant::value` reconstruction before specialization acquisition |
| `lifecycle.template_api.03` | 52 | 5 | 0 | 47 | Nested member class completion |
| `lifecycle.template_api.09` | 51 | 4 | 0 | 47 | Variable-template acquisition |
| `lifecycle.callsemantic.02` | 8 | 0 | 0 | 8 | Direct explicit function instantiation |
| `lifecycle.callsemantic.01` | 7 | 0 | 0 | 7 | Direct explicit class finalization |
| `lifecycle.template_api.04` | 2 | 0 | 0 | 2 | Source unnamed-class finalization |
| `lifecycle.template_api.05` | 1 | 0 | 0 | 1 | Source unnamed-class instantiation |
| `lifecycle.template_api.06` | 0 | 0 | 0 | 0 | Anonymous member class scan |

The largest producer combines three kinds:

| Kind from `lifecycle.template_api.01` | Attempts | Duplicates | Enrichments | Inserted |
| --- | ---: | ---: | ---: | ---: |
| Ensure definition | 10,580 | 7,141 | 125 | 3,314 |
| Function instantiation | 10,397 | 7,062 | 117 | 3,218 |
| Require definition | 3,702 | 1,502 | 23 | 2,177 |

These counts show repeated calls inside one wrapper as well as collisions
between producers. A wrapper-only consolidation would leave the 24,339
discarded or merged attempts intact.

## Architectural cause

Template operations expose final pointers and booleans, while lifecycle state
changes occur across several layers:

- `acquire_function_instantiation` measures the instantiation vector before
  and after a request;
- `acquire_function_binding_in_current_context` observes require-definition
  intent before it calls `ensure_function_template_definition`;
- definition-closure helpers emit require, ensure, and materialized events
  around separate calls in `semantic_template_function.cpp` and
  `callsemantic.cpp`;
- explicit-instantiation analysis rebuilds entity and declaration strings and
  emits its own function or class event;
- class acquisition, nested completion, finalization, and unnamed-class scans
  set or consult `ClassInfo::template_instantiation_log_emitted` at different
  layers;
- value handling uses both
  `ValueBinding::witness_member_value_instantiation_noted` and the lifecycle
  table to suppress repeated attempts;
- constant evaluation emits an `integral_constant::value` event before it
  performs normal class specialization selection and value materialization.

The witness session therefore serves as a second state machine. It decides
whether a repeated request represents a new transition, a duplicate, or a
richer copy of an old event. Semantic state should make that decision once.

## Target semantic model

### Template lifecycle transition

Introduce a compact value such as `TemplateLifecycleTransition`. It describes
one completed semantic state change and carries references to existing semantic
storage:

- entity kind: function, class, or value;
- transition kind: acquired, definition required, definition ensured,
  definition materialized, instantiated, or finalized;
- the selected `FunctionBinding`, `ClassInfo`, `ValueBinding`, or template
  declaration;
- old and new state bits needed to prove that a transition occurred;
- instantiation intent and semantic cause;
- source-use location and declaration anchor handles;
- the active closure entry context;
- created-new, explicit-specialization, unnamed-class, constexpr, and public
  source facts that the renderer consumes.

Keep rendered entity and location strings out of this result. The observer can
format them while witness capture runs.

### Operation results

Extend the existing `TemplateInstantiationResult` family with transition
results or a small transition span. The operation that changes semantic state
creates the transition:

- function acquisition reports a new binding;
- definition closure reports each state edge around the one canonical ensure
  operation;
- class acquisition reports a new `ClassInfo`;
- class completion and finalization report the false-to-true state edge;
- variable-template acquisition and member-value materialization report a new
  value binding or definition state.

Callers may inspect transitions for diagnostics, but they must not synthesize
new lifecycle transitions from the returned pointer.

### Transition identity

Use semantic entity identity plus transition kind and the owning instantiation
request. Avoid identity based on rendered names or locations. Store a compact
per-session state table in witness capture when the semantic object cannot
carry persistent state.

Do not add vectors, strings, or owning pointers to `FunctionBinding`,
`ClassInfo`, `ValueBinding`, `Type`, or `TemplateArgument`. Use nonowning views
and session side storage. Record hot structure sizes after each phase.

### One observation owner

Add one function such as `observe_template_lifecycle_transition`. It accepts a
completed transition, derives the witness payload, and calls the lifecycle
recorder. It may normalize locations and names. It may not perform lookup,
specialization selection, source scanning, template argument resolution, or a
second semantic-state test.

Function, class, and value operations can create transitions in separate
modules. The observer remains the only direct lifecycle recorder caller
outside the witness API.

## Performance policy

### Fixed and rolling files

Use these files throughout the work:

- `/tmp/cppgm-lifecycle-convergence-fixed.json` holds the confirmed class-use
  endpoint and does not change;
- `/tmp/cppgm-lifecycle-convergence-rolling.json` advances to each accepted
  phase checkpoint.

Phase 0 copied both files from
`/tmp/cppgm-class-use-final-confirmation.json`. Their SHA-256 is
`bc6c01464e356e2c4da69f0a63e3baec74e7322f15931030540afaabea566203`.

Verify the fixed file before each measurement series:

- head: `0662098ba13097ae0ac059593443a311f769a59b`;
- three-run median instructions: `176156071669`;
- three-run median maximum RSS: `763035648`;
- three-run median peak footprint: `592871424`;
- workload epoch: `9764b3835e3c6996b6b80803054f80e1cf50f98e`.

The baseline includes provenance support in the source tree, but the ordinary
benchmark build compiles the counters out. Do not record a replacement fixed
baseline. A lost file can be restored from the class-use confirmation
artifact.

### Phase measurements

Commit each correctness-clean phase, then record one three-run batch with an
isolated object root:

```sh
scripts/validate_perf_regression.py record \
  --baseline /tmp/cppgm-lifecycle-phase-N.json \
  --runs 3

scripts/validate_perf_regression.py compare \
  --baseline /tmp/cppgm-lifecycle-convergence-fixed.json \
  --candidate /tmp/cppgm-lifecycle-phase-N.json \
  --advisory \
  --instruction-tolerance 0.005 \
  --rss-warning-tolerance 0.03 \
  --footprint-tolerance 0.01

scripts/validate_perf_regression.py compare \
  --baseline /tmp/cppgm-lifecycle-convergence-rolling.json \
  --candidate /tmp/cppgm-lifecycle-phase-N.json \
  --advisory \
  --instruction-tolerance 0.005 \
  --rss-warning-tolerance 0.03 \
  --footprint-tolerance 0.01
```

Promote the recorded candidate after the ledger captures both comparisons:

```sh
cp /tmp/cppgm-lifecycle-phase-N.json \
  /tmp/cppgm-lifecycle-convergence-rolling.json
```

Intermediate phases may perturb performance while transition results coexist
with old calls. Investigate instruction growth at or above 0.5%, footprint
growth above 1%, and RSS growth at or above 3%. Keep the reason and removal
phase in the ledger. Do not rerun an unchanged commit.

An RSS warning triggers one more three-run batch. Two batches at or above 3%
reject the implementation checkpoint. A single warning records a cleanup
obligation.

Pause new metadata work if cumulative instructions exceed the fixed checkpoint
by 2% or footprint exceeds it by 3%. Remove copies, allocations, and dual-path
work before adding another result type.

### Final gate

The final checkpoint compares with the fixed baseline:

- instructions must decrease; a reduction below 0.5% needs a confirmation
  batch, and both medians must remain below the fixed median;
- peak footprint must stay within 1% of the fixed median;
- maximum RSS must clear the 3% warning and confirmation rule;
- production source must show a net line deletion.

Profile a failed checkpoint before reverting the semantic direction. Fix
avoidable work in the same slice, rerun correctness, and measure the changed
commit.

## Phase 0: Freeze evidence and route map

1. Copy and verify the fixed and rolling performance files.
2. Record the lifecycle provenance report digest and all 11 site counts.
3. Map each wrapper caller to the semantic operation and state edge it
   observes.
4. Record ordinary production source lines and hot structure sizes.
5. Add targeted fixtures for `lifecycle.template_api.06`, which the strict
   provenance run did not exercise.
6. Build with an isolated object root and confirm that the worktree does not
   read or write the original checkout's `obj/` tree.

This phase changes diagnostics, tests, and documentation only.

## Phase 1: Converge value lifecycle transitions

Start with values because the trace exposes a concrete duplicate algorithm.

1. Make variable-template acquisition return a typed value-instantiation
   transition when it creates or selects the binding for tracked output.
2. Make static member value materialization return the same transition shape.
3. Route retained template-member dependencies through the materialization
   operation instead of replaying the lifecycle request.
4. Replace the manual `integral_constant::value` entity construction in
   `constant_value_lookup.cpp` with the transition from normal specialization
   selection and value binding acquisition.
5. Remove `lifecycle.constant_value_lookup.02`.
6. Merge `lifecycle.template_api.02` and `.09` into the value transition
   observer.
7. Remove `witness_member_value_instantiation_noted` if semantic transition
   identity makes the field redundant. Keep static-definition source capture
   state separate until its source-use replay disappears.

Expected result: four value producers become one. The 7,399 starting exact
duplicates from value producers should reach zero.

Use the strict files containing `integral_constant<bool, true>::value` as the
first parity suite, including
`pa22/tests/general/100-partial-specialization-pack-expansion-value-pattern.t`.

## Phase 2: Converge function acquisition and explicit instantiation

1. Add transition facts to `acquire_function_instantiation` and
   `acquire_function_binding`.
2. Route explicit function instantiation through the acquisition result and
   delete the direct callsemantic emitter.
3. Replace the separate function-declaration-instantiated callback with the
   acquisition transition.
4. Preserve explicit-specialization cause, created-new state, declaration
   anchor, and closure context in the typed result.
5. Remove `lifecycle.callsemantic.02`.

This phase must not merge require, ensure, and materialize into one event.
Those events describe separate semantic state edges and keep their order.

## Phase 3: Make definition closure one state machine

1. Put require, ensure, and materialize transitions around the canonical
   definition acquisition operation.
2. Route `semantic_template_function.cpp`, `callsemantic.cpp`, and template
   acquisition through that operation.
3. Delete the separate
   `note_function_definition_ensure_requested`,
   `note_function_definition_materialized_by_closure`, and local
   require-definition arms.
4. Replace string-based duplicate identity with the semantic binding and state
   edge.
5. Preserve the current closure ordering rules through transition sequence
   numbers, then prove the recorder no longer needs duplicate enrichment.

Expected result: function lifecycle has one transition owner and one observer.
The 15,705 duplicate attempts and 265 enrichments from `.01` should reach zero.

## Phase 4: Converge class acquisition and explicit finalization

1. Make class acquisition and selected-specialization acquisition create the
   class-instantiation transition.
2. Make class finalization create a transition only on the incomplete-to-
   complete edge.
3. Route explicit class instantiation and finalization through these operation
   results, then delete `lifecycle.callsemantic.01`.
4. Replace `template_instantiation_log_emitted` checks that compensate for
   repeated acquisition with transition identity.
5. Preserve partial-specialization ownership and explicit-specialization
   cause in the transition.

Expected result: generic class acquisition and explicit instantiation share
one semantic owner.

## Phase 5: Fold nested and unnamed class completion into class transitions

1. Have nested class completion return its transition with the selected
   declaration anchor.
2. Create child transitions while semantic class population creates unnamed
   field and anonymous member classes.
3. Delete recursive field scans from `note_anonymous_member_class_events`.
4. Remove the forced lifecycle-pause reset in nested completion.
5. Remove producers `.03`, `.04`, `.05`, and `.06` after their targeted rows
   transfer to class completion transitions.

The new `.06` fixture guards the unexercised anonymous-member route. A zero-hit
producer cannot be deleted without a test that proves the canonical transition
owns its intended event.

## Phase 6: Install the final observer and simplify the recorder

1. Route all typed transitions through one lifecycle observer.
2. Remove direct lifecycle recorder calls outside the observer and witness API.
3. Run provenance across strict and broad suites.
4. Delete duplicate, enrichment, and string-identity branches that record zero
   actions after convergence.
5. Remove dead producer IDs, wrappers, flags, and closure adapters.
6. Compact transition storage and remove fields that no consumer reads.
7. Record final source lines, hot structure sizes, binary sections, provenance,
   correctness, inception, and performance.

The final provenance report should contain one lifecycle observer producer,
zero duplicate attempts, zero enrichments, and no unknown producer.

## Correctness gate for each phase

Run the migrated producer's targeted files first. Then run:

```sh
CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 \
make test-strict \
  OBJ=/tmp/cppgm-witness-lifecycle-obj \
  CXX=/usr/local/opt/llvm/bin/clang++ \
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++

CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 ORDERED=false \
make test-report \
  OBJ=/tmp/cppgm-witness-lifecycle-obj \
  CXX=/usr/local/opt/llvm/bin/clang++ \
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
```

Run a provenance build for the affected strict PAs before deleting each old
producer. Confirm that the transition observer supplies the same entity,
location, cause, detail, entry context, and ordering.

Do not update witness references. A mismatch means the transition result lost
a fact or fired at the wrong semantic state edge. Fix the operation result.

Run inception at the final checkpoint. Run it after an earlier phase that
changes persistent semantic layout or template scheduler ownership.

## Code-size and structure accounting

Use the fixed semantic checkpoint for cumulative production accounting:

```sh
git diff --numstat \
  0662098ba13097ae0ac059593443a311f769a59b..HEAD \
  -- dev/src dev/Makefile dev/frontend_source_sets.mk
```

Record after each phase:

- production additions, deletions, and net lines;
- direct lifecycle recorder sites and semantic transition owners;
- exact duplicates and enrichments by producer and event kind;
- sizes of `Type`, `TemplateArgument`, `ClassInfo`, `FunctionBinding`,
  `ValueBinding`, and each transition result;
- ordinary `cppgm++` file size and Mach-O text and data sections.

The starting `dev/src` count is 416,807 lines. The final production diff must
delete more lines than it adds.

## Alias and function source-use follow-up

The source-use audit also found convergence work outside lifecycle logging:

- alias use has one direct emitter but four feed paths plus a recursive
  template-pattern syntax walker; its final trace recorded 1,326 attempts, 387
  exact duplicates, 14 enrichments or replacements, and 564 visible rows;
- function call has one direct emitter but overload, conversion, declval, and
  constexpr-direct feeds; its final trace recorded 920 attempts, 133 exact
  duplicates, and 599 visible rows;
- variable source use has one feed, 31 attempts, and no duplicates.

Keep alias convergence next in the queue. Its template-pattern and expansion
routes match the class-use problem and should reuse the typed source results
from class convergence. Audit function calls after alias work. Leave variable
source use alone unless later provenance finds a second semantic route.

## Commit discipline

Keep each phase as a short series:

1. typed operation result and parity diagnostics;
2. callers moved to the result;
3. old semantic arm and producer removed;
4. zero-hit recorder policy removed.

Do not keep two complete state-change algorithms across more than one measured
checkpoint. Correctness failures require a fuller transition result.
Performance failures require a profile and an in-scope cleanup attempt.

## Completion criteria

The work completes after all checks pass:

- one lifecycle observer owns the direct recorder call;
- function, class, and value lifecycle changes originate in their canonical
  semantic operations;
- explicit instantiation uses the same transition path as implicit
  instantiation;
- constant evaluation performs no witness-only `integral_constant` lookup or
  event construction;
- nested and unnamed classes need no recursive lifecycle scan;
- lifecycle provenance reports zero duplicates and zero enrichments on strict
  and broad probes;
- retired duplicate recorder policies and witness-only flags leave the code;
- production source has a net line deletion from `0662098ba`;
- strict, full report, and inception pass;
- final instructions decrease from `176156071669` under the repeatability
  rule;
- peak footprint stays within 1% of `592871424` and RSS clears the 3% warning
  rule against `763035648`;
- the ledger has no open cleanup obligation.
