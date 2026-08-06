# Witness Semantic-Path Consolidation Experiment

The branch implements this plan. The experiment ledger records the final
correctness, provenance, irreducibility, and performance evidence:
`docs/witness-semantic-path-consolidation-experiment-ledger.md`.

The unique-output boundary in that ledger closes the deletion-only experiment.
The follow-up `docs/witness-class-use-semantic-convergence-plan.md` replaces the
remaining recovery analyzers with typed source-resolution results and requires
a final instruction reduction.

## Handoff

- Repository: `/Users/vishvananda/cppgm-extended`
- Branch: `experiment-witness-semantic-path-consolidation`
- Parent: `c3a2fd4f4347e5fb6edb349e0d9faf17a85c1767`
- Standard compiler on this host: `/usr/local/opt/llvm/bin/clang++`
- Patched-Clang comparison checkout:
  `/Users/vishvananda/llvm-project-template-metrics-20260416`
- Patched-Clang comparison head: `59c5d9c70`

The branch was created for an experiment. Keep each successful semantic-path
removal in its own commit so a useful subset can land even if the full endpoint
does not.

## Objective

Use witness producer duplication to find and remove duplicated semantic work.
For each witness event kind, reduce the number of semantic owners one verified
step at a time. Class-use is the first and largest target.

A successful reduction deletes or merges a semantic traversal, lookup, replay,
instantiation route, or special-case analyzer. Moving the existing calls into a
shared witness helper does not count. A helper may be useful after the semantic
paths have converged, but the experiment measures semantic owners and upstream
entry points as well as literal `emit_*` calls.

The intended result is:

- fewer independent ways to resolve the same source construct;
- fewer raw witness collisions and replacements;
- fewer renderer suppression rules;
- no witness or LowIR regression;
- bounded performance impact: three-run median instructions within 0.5% and
  peak footprint within 1% of the accepted baseline; median maximum RSS below
  the 3% warning threshold, or below it on one confirmation batch.

## Hypothesis

CPPGM often discovers the same source use through several phases: initial type
lookup, nested-template-id scanning, dependent-pattern handling, instantiation
replay, constant evaluation, declaration collection, and owner recovery. The
source-use table and renderer then collapse the results. Those collisions are
evidence that semantic analysis does not have a single durable result for the
source construct.

The patched Clang implementation provides a useful contrast. Source-use output
is collected by one `RecursiveASTVisitor` over the completed typed AST in
`clang/lib/Frontend/FrontendAction.cpp`. Its relevant visitor surface is:

- `VisitTemplateSpecializationTypeLoc`
- `VisitDeducedTemplateSpecializationTypeLoc`
- `VisitVarDecl`
- `VisitVarTemplateSpecializationDecl`
- `VisitDeclRefExpr`
- `VisitCallExpr`
- `VisitCXXConstructExpr`
- `VisitCXXDeductionGuideDecl`

There is one `emitClassUse` implementation and nine `pushEvent` calls in that
visitor. Clang retains Sema hooks for facts that the completed AST cannot
reconstruct: overload candidate metrics and drops, require/ensure-definition
transitions, and class/function/variable instantiation lifecycle transitions.

This comparison does not justify adding a witness-only CPPGM postpass that
repeats semantic resolution. It suggests a target architecture in which normal
semantic analysis resolves and stores each source construct once, and witness
capture observes that canonical result. A later observer is acceptable only
after the alternate resolution and replay routes have been removed. It must not
reparse source text or perform a second lookup.

## Hard Rules

1. Do not claim a reduction from a shared wrapper while the original semantic
   paths remain.
2. Do not add renderer suppression, dedupe, or source-text recovery to make a
   removed producer appear safe.
3. Do not regenerate witness references during this experiment. Witness output
   must remain identical.
4. Do not accept arbitrary LowIR reference churn. If a semantic consolidation
   causes a principled LowIR change, document the cause and verify direct text
   comparison before deciding whether a ref update is legitimate.
5. Put any instrumentation cost behind witness/debug capture guards. Normal
   compilation must not allocate provenance tables, fingerprint events, or walk
   syntax for this experiment.
6. Do not remeasure a baseline to improve a comparison. Record each baseline
   once and promote accepted candidate measurements without rerunning them.
7. Use three runs for each baseline and candidate, and gate on their medians.
   Allow at most 0.5% more instructions and 1% more peak footprint. Treat a
   maximum RSS increase of 3% or more as a warning and run one more three-run
   batch. Fail the candidate when the confirmation batch also reaches or
   exceeds 3%. Wall time is informational and never a gate.
8. Investigate a performance failure before reverting the semantic change.
   A failed measurement rejects the current implementation, while the semantic
   consolidation remains viable if an in-scope change removes the added work
   or memory.
   Use the recorded counters, allocation diagnostics, profiles, and executable
   layout to locate added work or memory. Amend the candidate when an in-scope
   correction exists, then rerun correctness and measure the changed commit.
   Do not rerun an unchanged failed commit. Revert only after the investigation
   finds no maintainable correction within the slice.
9. A failed `make test-strict` after removing a site means the semantic merge is
   incomplete. Restore the missing responsibility in the canonical path; do
   not repair the output downstream.

## Baseline Inventory

The counts below are from parent `c3a2fd4f4`. API definitions in
`dev/src/witness_api.cpp` are not counted as semantic producers.

| Event kind | Direct semantic sites | Current table behavior |
| --- | ---: | --- |
| Class use | 24 | Prefers spelling anchors and source-owned rows, replaces nested/direct rows, merges richer occurrence and pack data, then applies more renderer drops. |
| Alias use | 7 | Prefers direct/source-owned rows over nested rows and merges richer occurrence and pack data. |
| Function call | 4 | Dedupes rows while ignoring binding spacing. |
| Variable use | 1 | May replace an equivalent row while ignoring its location. |
| Lifecycle log | 17 | Records temporal events separately from source uses; later lifecycle handling also dedupes. |

The literal count is only the first measurement. There are also forwarding and
replay entry points such as `record_class_use_for_resolved_type_node`,
`emit_nested_class_use_source_events_from_*`, and declaration/instantiation
callbacks. These must fall with the producer count.

The API also exposes path-specific capture policy that should shrink as the
routes converge:

- five `ClassUseEmissionOrigin` values decide whether a class use survives
  function-call speculation;
- six `AliasUseEmissionOrigin` values decide whether an alias use survives a
  source-capture pause;
- `DeclvalCall` has a separate function-call capture bypass;
- variable use has both append and location-insensitive replacement modes.

Do not remove these distinctions before their callers converge. After a merge,
an origin or bypass with no callers is evidence that its policy can disappear.

### Class-use sites

All 24 current sites are listed here. Line numbers identify the parent snapshot
and will move during the experiment.

| Family | Sites | Current responsibility |
| --- | --- | --- |
| Base and template-parameter syntax scans | `dev/src/callsemantic.cpp:7402`, `:18918`, `:21775` | Rewalks base clauses, template parameter clauses, and parameter declarations to build class-use decisions. |
| Type lookup and resolved-type recovery | `dev/src/callsemantic.cpp:8002`, `:9153`, `:20801`, `:20905`, `:20961`, `:21448` | Emits from alias-to-class recovery, ordinary type lookup, dependent patterns, selected partials, deduced aliases, and resolved type nodes. |
| Nested source fanout | `dev/src/callsemantic.cpp:13015`, `:13083`, `:13158` | Scans locations, source ranges, and nested template-id syntax after another semantic operation has already resolved the enclosing construct. |
| Member and definition owner recovery | `dev/src/callsemantic.cpp:27871`, `:30467`; `dev/src/callsemantic/template_declaration_collector.cpp:2144`; `dev/src/template_instantiation.cpp:2141` | Reconstructs owner specializations for qualified values, function definitions, collected declarations, and applied static-member definitions. |
| Canonical class-template reference path and special branches | `dev/src/callsemantic/class_template_reference.cpp:2506`, `:3251` | Emits selected class-template references, with a separate dependent partial-specialization branch. |
| Constant-value and constexpr lookup | `dev/src/callsemantic/constant_value_lookup.cpp:684`, `:783`, `:1216`, `:1862`, `:2060` | Emits qualifier, call-owner, template-member-value, owner-chain, and fallback qualified-value uses during constant evaluation. |
| Instantiated type helper | `dev/src/semantic_template_class.cpp:107` | Emits an already-instantiated class type from callers such as overload and conversion analysis. |

The upstream class-use surface includes these additional public or callback
routes and must be counted in every slice:

- `SemanticContext::record_class_use_for_resolved_type_node`
- `SemanticContext::record_declaration_type_class_use_for_resolved_type_node`
- `SemanticContext::record_deduced_class_use_for_resolved_alias_type`
- `SemanticContext::emit_nested_class_use_source_events_from_location`
- `SemanticContext::emit_nested_class_use_source_events_from_ast_node`
- `SemanticContext::emit_nested_class_use_source_events_from_template_arguments`
- the location, AST, syntax-list, and template-id overloads around
  `dev/src/callsemantic.cpp:12963-13322`
- the class-template reference, constant-value lookup, and declaration
  collector callback adapters

### Alias-use sites

| Family | Sites |
| --- | --- |
| Base clause, resolved alias, and template-parameter replay | `dev/src/callsemantic.cpp:7369`, `:14279`, `:18795` |
| Nested and direct source template-id handling | `dev/src/template_argument_semantics.cpp:20239`, `:20529` |
| Structured and text-backed pattern deduction | `dev/src/template_specialization.cpp:1841`, `:1893` |

The nested alias traversal also calls class-use fanout for template arguments.
Treat that coupling as semantic work to remove, not as an emission utility to
preserve.

### Function-call and variable-use sites

| Kind | Sites | Observation |
| --- | --- | --- |
| Function call | `dev/src/semantic_template_function.cpp:570` | Main selected function-template call path. |
| Function call | `dev/src/callsemantic/constant_value_lookup.cpp:919` | Constexpr direct-call path. |
| Function call | `dev/src/semantic_overload.cpp:2265`, `dev/src/callsemantic.cpp:33265` | Two implementations of `declval` recognition and emission. This is a small, concrete duplicate semantic route. |
| Variable use | `dev/src/template_instantiation.cpp:12621` | Variable-template instantiation replay. The single direct site still relies on location-insensitive replacement and nested class fanout. |

### Lifecycle sites

There are 17 direct calls to `note_template_witness_log_event`:

- variable instantiation:
  `dev/src/semantic_class_model.cpp:12131`,
  `dev/src/template_argument_semantics.cpp:6314`, `:7843`,
  `dev/src/template_api.cpp:4248`, `:6730`,
  and `dev/src/callsemantic/constant_value_lookup.cpp:2294`, `:2376`, `:2432`;
- class instantiation and finalization:
  `dev/src/template_api.cpp:4339`, `:4361`, `:4386`, `:4410`, `:4482`,
  and `dev/src/callsemantic.cpp:29577`;
- function closure wrapper and direct instantiation:
  `dev/src/template_api.cpp:2894` and `dev/src/callsemantic.cpp:29606`;
- generic value-binding closure wrapper: `dev/src/template_api.cpp:4512`.

Refresh the category labels from the arguments before editing because some
`template_api.cpp` calls are generic wrappers. Lifecycle events cannot all come
from a completed source tree. The correct endpoint is one owning semantic
transition per lifecycle kind, reached through the real instantiation or
closure scheduler. Replacing 17 calls with one generic logging wrapper would
not improve the semantic architecture.

### Table and renderer consolidation that currently hides producer overlap

`semantic_source_use::record_source_use` in
`dev/src/semantic_source_use.h` performs semantic-looking conflict resolution:

- source-owned and direct alias rows remove nested-derived rows;
- class rows prefer a spelling anchor, then prefer source-owned over direct and
  direct over nested-derived ownership;
- alias and class rows adopt the more concrete template-id occurrence;
- alias and class rows adopt richer pack binding data;
- function rows compare equal after binding whitespace normalization;
- variable replay can replace a row at a different location.

`collect_rendered_source_events` in
`dev/src/template_witness_renderer.cpp` then runs, in order:

1. location canonicalization and dedupe;
2. name and binding normalization;
3. anonymous-namespace class-name preference;
4. uninstantiated static-member owner-use removal;
5. drop-order normalization;
6. member-alias qualification and placeholder-owner repair;
7. template-header pattern removal;
8. source-spelled alias preference;
9. source-defined call normalization;
10. redundant nested-event removal;
11. explicit class-specialization preference;
12. less-specific class-binding duplicate removal;
13. same-line deduced class-use removal;
14. final visible-event dedupe and sorting.

Not every pass is wrong. The experiment must identify which passes compensate
for multiple semantic routes. A pass becomes a deletion candidate only after
its hit count reaches zero across strict and full reports following a semantic
path removal.

## Phase 0: Freeze the Experiment Baseline

The original experiment started with a one-run, zero-tolerance parent
baseline. After the provenance checkpoint showed that a single memory sample
was too noisy for useful semantic-slice decisions, the performance method was
revised on 2026-08-05. The accepted, compile-time-guarded diagnostic checkpoint
`ba6b1070c` is the fixed comparison point so every semantic candidate includes
the same diagnostic code. The original parent and one-run reports remain audit
artifacts but no longer gate acceptance.

At the accepted diagnostic checkpoint `ba6b1070c`, build with the standard
Homebrew Clang and record three runs of the frozen semantic-overload workload:

```sh
make CXX=/usr/local/opt/llvm/bin/clang++ \
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++

scripts/validate_perf_regression.py record \
  --baseline /tmp/cppgm-witness-consolidation-diagnostic-3run.json \
  --runs 3

cp /tmp/cppgm-witness-consolidation-diagnostic-3run.json \
  /tmp/cppgm-witness-consolidation-rolling.json
```

The performance workload and its 51-header closure are frozen by
`benchmarks/self_compile/stable/PERF_EPOCH.json`. Do not edit or substitute the
source or headers. Record the three gated values in the experiment ledger:
instructions retired, maximum resident set size, and peak memory footprint.

Do not record the diagnostic baseline again. If the baseline files are lost,
recover the original file or restart the performance epoch explicitly; do not
silently create a more favorable comparison.

## Phase 1: Add Witness-Only Provenance Instrumentation

The first code change should make hidden overlap measurable. It is diagnostic,
not the consolidation itself.

### Producer identity

Assign a stable `WitnessProducerSite` identifier to every site in the inventory.
Capture it only when witness producer tracing is enabled. Do not add strings,
hashing, vectors, or source scanning to normal compilation.

Record an emission attempt before `record_source_use` changes the table. Each
attempt needs:

- producer site;
- event kind, role, and ownership;
- source location and spelling anchor;
- selected declaration/entity;
- template name, selection, and bindings;
- action taken by the table: inserted, exact duplicate, rejected, replaced, or
  enriched;
- producer identities already associated with the collided row.

Keep diagnostic provenance outside `SemanticSourceUse`. It must not become
permanent semantic metadata merely to support this experiment.

### Renderer attribution

While tracing, carry diagnostic event IDs through the renderer and report which
pass removes, replaces, or materially rewrites each event. The normal renderer
representation and output must stay unchanged when tracing is off.

### Reports

Add a small analyzer that emits these views for a test corpus:

1. site coverage: attempts, inserted rows, surviving rows, and final visible
   rows per producer;
2. collision matrix: producer pairs that describe the same table row;
3. replacement matrix: which producer supplies the retained anchor,
   occurrence, binding, or location;
4. renderer ownership: rows removed or rewritten by each cleanup pass and the
   producer sites responsible;
5. unique-output ownership: final output rows that only one producer can
   supply;
6. upstream route count: calls through each public replay/fanout entry point.

Run the instrumented strict set first, then the full report corpus. Store the
generated reports outside the repository or in an explicitly ignored scratch
directory. Commit only the analyzer and temporary guarded instrumentation, not
machine-generated reports.

The inventory is complete only when every static producer is either exercised
or listed with a targeted test that reaches it. Add a minimal test in the
earliest owning PA only if no existing test reaches a producer.

## Phase 2: Reduce Class-Use Semantic Routes

Class use is the primary experiment. Start at 24 direct sites and the full
upstream route count. Do not predetermine which single owner must survive; use
the collision and unique-ownership reports to choose each next merge.

Maintain this ledger in the branch while working:

| Slice | Direct sites before/after | Upstream routes before/after | Semantic route removed | Responsibility moved to | Strict | Full report | Instructions | Max RSS | Footprint | Renderer passes made idle |
| --- | --- | --- | --- | --- | --- | --- | ---: | ---: | ---: | --- |
| Parent | 24/24 | record after instrumentation | none | none | clean | clean | record | record | record | none |

The likely route collapses are ordered below. Instrumentation may change the
order, but it must provide the reason.

### 2A. Collapse nested syntax and location replay

The family around `callsemantic.cpp:12963-13322` discovers nested class uses by
scanning a location, an AST subtree, template arguments, syntax lists, or one
template-id. It is called after class reference, alias, constant-value,
declaration, and instantiation paths have already performed semantic work.

Target:

- make the canonical resolution of a structured template-id resolve and retain
  the semantic results for its nested template-id arguments exactly once;
- make consumers use those retained child results;
- delete location-based and AST-rewalk recovery overloads as their callers
  disappear;
- remove the related `NestedDerived` conflict policy when its hit count reaches
  zero.

Do not replace the overload family with one recursive witness scanner. The
semantic template-argument resolver must become the owner of child results.

### 2B. Collapse type lookup, dependent pattern, and declaration replay

Type uses currently emit during `lookup_type_impl`, dependent pattern handling,
alias-to-class deduction, resolved-type-node handling, parameter parsing, and
template-parameter replay.

Target:

- attach a canonical resolved template-id result to the source type construct;
- make dependent and instantiated forms use the same selection result model;
- make declaration, `sizeof`, conversion, and statement consumers retain or
  reference that result instead of asking `SemanticContext` to rediscover the
  class use;
- delete lookup recovery and parameter-clause witness scans after their unique
  responsibilities have moved into ordinary type semantics.

The result must preserve source spelling, selected declaration, primary versus
partial/explicit selection, and both primary and specialization bindings.

### 2C. Collapse constant-value and qualified-owner resolution

Constant evaluation has five class-use sites and separately reconstructs owner
chains, selected specializations, source argument text, and nested uses.

Target:

- make qualified-id/member lookup return one typed resolution object containing
  the selected value/function and its resolved owner chain;
- have normal expression semantics and constant evaluation consume that same
  object;
- remove constexpr-only class-use and qualifier replay;
- preserve overload candidate/drop facts at the overload-selection owner.

Constant evaluation must not become a second source-use producer merely because
it materializes a value.

### 2D. Collapse out-of-class and static-member owner recovery

Function definition collection, template declaration collection, and template
instantiation all reconstruct the class-template owner of an out-of-class or
static-member definition.

Target:

- carry the selected owner specialization on the canonical member declaration
  or binding;
- make declaration collection and definition application refer to the same
  owner result;
- remove static-member witness replay and separate definition-owner emitters;
- remove location-insensitive replacement rules that only exist because replay
  discovers the source use later at a better location.

### 2E. Converge on one class-use observation owner

After the previous collapses, one semantic representation should own each
resolved source template-id and its source anchor. Observe class use once from
that representation. Delete remaining special branches one at a time, including
the dependent partial-specialization branch, only after instrumentation shows
that the canonical result has all of its unique data.

The end condition is not merely one `emit_class_use` call. It is:

- one semantic resolution route for a source class template-id;
- one observation owner;
- no nested/source-owned replacement lattice needed for class uses;
- no class-use renderer pass hiding producer disagreement;
- no source-location scan or AST replay used to reconstruct semantics.

## Phase 3: Reduce the Other Source-Use Kinds

### Function calls

Use the duplicate `declval` analyzers in `semantic_overload.cpp` and
`callsemantic.cpp` as the smallest proof of the method. Choose the canonical
expression analyzer, route both clients through it, and delete the other
recognition and emission path. Then fold constexpr direct-call handling into
the same selected-call result used by ordinary overload resolution. Keep
candidate metrics and drops at the overload-selection transition because those
facts may not survive in the final expression node.

Target: four direct sites to one selected-call source-use owner, without moving
four calls behind a wrapper.

### Alias uses

Unify structured and text-backed alias pattern deduction first. Then make base
clauses, template parameters, and nested template arguments consume the same
resolved alias-template-id result as a direct source use. Remove alias ownership
preference and richer-data merging only when their trace counts reach zero.

Target: seven direct sites to one resolved alias-template-id observation owner.

### Variable uses

There is one direct producer, but its replacement policy and nested class fanout
show that the semantic result is still completed in stages. Retain the final
source anchor and owner results on the canonical variable-template
instantiation. Delete location-insensitive replacement and nested replay when
they become idle.

Target: keep one producer and eliminate staged correction.

## Phase 4: Consolidate Lifecycle Transitions

Group the 17 lifecycle calls by event kind and semantic entity. For each group,
identify the actual state transition that creates, requires, ensures, or
finalizes the entity. Make that transition the owner and remove calls from
lookups and replay paths that only observe the same transition later.

Expected owners are the real closure/instantiation schedulers:

- require definition;
- ensure definition;
- function instantiation;
- class instantiation;
- class finalization;
- variable instantiation.

Keep separate hooks when the transitions are genuinely separate. The measure
of success is one owner per transition, no duplicated scheduler work, and fewer
dedupe collisions. A single logging wrapper is not a result.

## Per-Slice Validation and Performance Gate

Use the following sequence for every producer removal or inseparable semantic
merge. Run independent test work with the repository's normal parallel report
settings; do not serialize the suite without a specific debugging reason.

### 1. Targeted verification

Run the smallest tests covering both producers and every unique payload field
identified by instrumentation. Add a reducer only when existing coverage does
not isolate the route.

### 2. Strict witness and LowIR gate

```sh
CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 \
make test-strict \
  CXX=/usr/local/opt/llvm/bin/clang++ \
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
```

The current strict set is `pa19 pa20 pa22 pa23 pa24`. Any witness mismatch means
the path has not been consolidated. Do not update refs.

### 3. Full report gate

```sh
CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 ORDERED=false \
make test-report \
  CXX=/usr/local/opt/llvm/bin/clang++ \
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
```

This must complete cleanly. Compile, link, and object-roundtrip failures should
get minimal reducers in the earliest owning PA before the fix. The full report
is required because strict primarily proves witness stability, not the absence
of semantic or LowIR fallout.

### 4. Checkpoint the candidate

Commit the code and any required reducers so the performance report identifies
the exact candidate commit. Do not combine an unrelated semantic route in this
checkpoint.

### 5. Three-run median candidate performance gate

Build the candidate with the same Homebrew Clang configuration, then run:

```sh
scripts/validate_perf_regression.py check \
  --baseline /tmp/cppgm-witness-consolidation-rolling.json \
  --report /tmp/cppgm-witness-consolidation-candidate.json \
  --runs 3 \
  --instruction-tolerance 0.005 \
  --rss-warning-tolerance 0.03 \
  --footprint-tolerance 0.01
```

The candidate fails when median instructions retired exceed the last accepted
measurement by more than 0.5% or peak footprint exceeds it by more than 1%.
A maximum RSS increase of 3% or more starts one confirmation batch of three
runs. The validator fails the candidate when the confirmation median also
reaches or exceeds 3%. Do not run a third batch under the same method. Wall,
user, system, and cycle time remain informational.

After a hard failure or confirmed RSS failure, inspect the semantic work,
allocation behavior, counters, profiles, and executable layout before deciding
the slice cannot ship. Keep the consolidation when a targeted, maintainable
amendment removes the regression and the changed candidate clears correctness
and performance validation. Do not repeat the same commit. Revert when the
measured cost has no maintainable in-scope fix.

Do not promote the candidate yet.

### 6. Accept and promote

After all gates pass, mark the checkpoint accepted in the ledger. Promote the
already-recorded single candidate run without remeasurement:

```sh
jq 'if .confirmation_candidate then .confirmation_candidate else .candidate end' \
  /tmp/cppgm-witness-consolidation-candidate.json \
  > /tmp/cppgm-witness-consolidation-rolling.next.json
mv /tmp/cppgm-witness-consolidation-rolling.next.json \
  /tmp/cppgm-witness-consolidation-rolling.json
```

Keep the fixed diagnostic three-run JSON for the final comparison. The rolling
baseline advances to the already-recorded three-run candidate after every
accepted slice.

If a slice fails after a checkpoint commit, amend it while investigating or
revert it explicitly. Do not promote its performance report.

## Commit Boundaries

Use these boundaries where possible:

1. guarded producer and renderer attribution instrumentation plus analyzer;
2. one duplicated semantic route removed;
3. now-idle table or renderer policy removed;
4. reducers required by that route;
5. instrumentation removal after the final inventory.

A semantic merge may delete several emit calls when they are inseparable, but
the commit message and ledger must name the one duplicated semantic operation
that disappeared. Do not combine unrelated clean producers merely because all
tests pass.

## Completion Criteria

The experiment is ready to package when all of these are true:

- every original producer has an exercised test or an explicit targeted test;
- class use has one semantic resolution/observation route, or the ledger gives
  evidence for each irreducible remaining owner;
- alias and function-call routes have been reduced using the same method;
- variable use no longer relies on staged location correction;
- lifecycle calls correspond one-to-one with real semantic transitions;
- table conflict branches and renderer cleanup passes made idle by the work are
  deleted;
- no witness renderer source reparsing or recovery was added;
- `make test-strict` passes with direct LowIR comparison;
- full `make test-report` passes with direct LowIR comparison;
- the final median instructions are within 0.5% and peak footprint within 1%
  of both the fixed diagnostic checkpoint and the latest promoted rolling
  baseline; maximum RSS clears the 3% warning rule against both baselines;
- all experiment-only provenance instrumentation is removed or retained only
  if it has clear ongoing diagnostic value and zero normal-path cost;
- the final branch is split into reviewable commits and has no generated output
  or unrelated scratch files staged.

## Fresh-Agent Start Checklist

1. Read this document, `AGENTS.md`, `docs/performance-regression-validation.md`,
   `dev/src/witness_api.h`, `dev/src/semantic_source_use.h`, and
   `dev/src/template_witness_renderer.cpp`.
2. Confirm the branch and parent with `git status --short --branch` and
   `git merge-base HEAD main`.
3. Preserve untracked `HANDOFF.md` and `scratch_*.cpp` files. They are not part
   of this experiment.
4. Refresh the static counts with fixed-string `rg` searches for
   `emit_class_use(`, `emit_class_use_decision(`, `emit_alias_use(`,
   `emit_function_call(`, `emit_variable_use(`, and
   `note_template_witness_log_event(`. Exclude API definitions.
5. Inspect patched Clang at `59c5d9c70`, especially
   `clang/lib/Frontend/FrontendAction.cpp` and its small set of Sema hooks. Use
   it as architectural evidence, not as permission for a CPPGM recovery pass.
6. Record the one-run immutable parent performance baseline before production
   edits.
7. Implement Phase 1 instrumentation behind witness/debug guards.
8. Produce the coverage, collision, replacement, renderer, unique-ownership,
   and upstream-route reports.
9. Choose the smallest evidence-backed semantic merge, run the complete
   per-slice gates, commit it, and promote its single candidate measurement.
10. Continue one semantic route at a time. Stop and document evidence if a
    remaining owner is genuinely irreducible.
