# Class-Use Semantic Convergence Plan

## Status and handoff

This plan follows the completed
`witness-semantic-path-consolidation-experiment-plan.md`. The first experiment
removed fifteen class-use sites and exposed the nine remaining owners. Its
final ownership audit treated unique output as an irreducibility test. Unique
output instead identifies data that the next implementation must move into a
canonical semantic result.

- Repository: `/Users/vishvananda/cppgm-extended`
- Branch at plan creation: `experiment-witness-semantic-path-consolidation`
- Plan-creation head: `50593a940`
- Fixed semantic-code checkpoint: `a42c915e3acd91ee56f9f8913d0c5695ad52ccf5`
- Patched-Clang comparison checkout:
  `/Users/vishvananda/llvm-project-template-metrics-20260416`
- Patched-Clang comparison head: `59c5d9c70`
- Execution ledger:
  `docs/witness-class-use-semantic-convergence-ledger.md`

The two commits after `a42c915e3` record documentation and renderer-cleanup
evidence. They do not change `dev/`, the frozen benchmark, or the performance
validator.
The promoted three-run result for `a42c915e3` therefore supplies the fixed
performance baseline for this work. It includes the diagnostic counters added
during the first experiment.

## Objective

Replace nine class-use analyzers with one typed source-resolution model and one
observation pass. The implementation must remove the semantic recovery work,
not move nine calls behind a helper.

The final implementation must have:

- one class-template-id resolution result that retains source syntax, resolved
  arguments, specialization selection, selected declaration, and nested uses;
- one observation pass that emits class uses from completed semantic results;
- one parameterized representation for dependent uses that later substitution
  can instantiate without source lookup or reparsing;
- no class-use AST rewalk, source-location scan, post-`TypePtr` template
  recovery, constexpr-specific class-template resolver, or static-definition
  witness replay;
- one direct class-use producer instead of nine;
- zero calls through the four remaining replay routes;
- fewer production source lines and fewer retired instructions than the fixed
  checkpoint.

Memory reduction is a target. The final memory gates still allow normal sample
variation: peak footprint may not exceed the fixed checkpoint by more than 1%,
and maximum RSS follows the 3% warning and confirmation rule.

## Starting inventory

The final trace from the first experiment lives at
`/tmp/cppgm-witness-provenance-final-post-cleanup.rt3Y14`; its report lives at
`/tmp/cppgm-witness-provenance-final-post-cleanup-report.json`.

| Producer | Attempts | Unique visible rows | Semantic work |
| --- | ---: | ---: | --- |
| `class.class_template_reference.02` | 3,578 | 1,118 | Canonical lookup, argument resolution, selection, and reference or instantiation |
| `class.callsemantic.06` | 32,046 | 35 | Recursive syntax and AST recovery with a second lookup, argument resolution, and selection |
| `class.callsemantic.07` | 151 | 75 | Template-parameter reparse and full template-declaration source-use walk |
| `class.callsemantic.08` | 4 | 4 | Dependent current-pattern argument resolution and specialization selection after type lookup |
| `class.callsemantic.10` | 80 | 30 | Class recovery and reselection from an alias-expanded `TypePtr` |
| `class.callsemantic.13` | 56 | 29 | Out-of-class owner recovery, selection, anchoring, and private binding canonicalization |
| `class.constant_value_lookup.02` | 9 | 2 | Owner reconstruction for an already-selected constexpr member call |
| `class.constant_value_lookup.03` | 356 | 199 | Independent class-template resolution for constant member lookup |
| `class.template_instantiation` | 55 | 24 | Static-member definition owner reconstruction and nested AST replay |

The four remaining public routes are:

| Route | Strict-corpus calls |
| --- | ---: |
| `nested_class_use.ast_node` | 9,064 |
| `nested_class_use.template_arguments` | 62 |
| `class_use.resolved_alias_type` | 1,185 |
| `class_use.static_member_definition_ast_node` | 25 |

The unique rows form the migration suite. They do not justify retaining their
producer.

## Architectural cause

The class-template reference path computes the required semantic facts, then
returns `ClassInfo *`. Type parsing follows the same pattern and returns
`TypePtr`. Qualified lookup returns a binding. Alias expansion returns its
expanded type. These return values discard the typed link between a source
occurrence and the semantic object it selected.

Later phases reconstruct the lost facts from syntax, source locations,
`TypePtr`, `ClassInfo`, or a selected binding. Clang avoids these recovery arms
because its typed AST retains `TemplateSpecializationTypeLoc`, typed nested-name
specifiers, selected declarations, and resolved template arguments.

CPPGM needs an equivalent typed source layer. A shared emission helper cannot
replace that layer.

## Target semantic model

### Resolved class-template-id

Introduce a semantic result with a name such as `ResolvedClassTemplateId`.
Keep witness rendering fields out of it. The result must retain:

- a stable reference to `TemplateIdSyntax` and its name anchor;
- the originating `ClassTemplateDecl`;
- the referenced or instantiated `ClassInfo`, when one exists;
- resolved primary arguments and default provenance;
- selection kind and selected declaration;
- partial-specialization arguments and pack sizes;
- dependency and current-specialization state;
- resolved source results for qualifier components and template arguments.

The result should reference argument and selection storage that semantic
analysis already owns. It should not copy argument vectors to serve witness
capture.

### Typed source tree

Add a source-side result such as `ResolvedTypeLoc` that pairs a `TypePtr` with
the semantic results for the syntax that formed it. Add `ResolvedQualifiedId`
for a selected value or function plus its resolved qualifier chain.

These objects provide the data that `TypePtr`, `ClassInfo *`, and bindings lose.
Normal type, expression, and qualified-id consumers use the canonical semantic
value. Witness observation uses the source-side tree.

### Dependent patterns

Represent a dependent class use as an immutable parameterized result. It must
retain the source syntax, template origin, argument expressions, pattern scope,
and any fixed partial or explicit selection. Instantiation substitutes a
binding frame into this result. It must not parse its source text or repeat
template lookup.

This representation covers dependent current-specialization uses and
out-of-class static-member owner patterns.

### Result ownership and allocation

Use value-scoped results for direct resolution. Retain a result only when a
template pattern, alias expansion, or delayed definition needs it after the
current operation.

Use an arena or compact side store owned by `SemanticContext` for retained
source results. Store integer handles in declaration records that need delayed
access. Do not add vectors or owning pointers to `Type`, `TemplateArgument`, or
`ClassInfo`. Record the `sizeof` of those hot structures before the first code
change and after each phase.

The retained representation must follow these rules:

- reuse `TemplateIdSyntax`, template arguments, and specialization storage;
- store source location IDs or token indices instead of rendered location
  strings;
- allocate child lists in one arena block or compact vector;
- avoid per-use `shared_ptr` allocation;
- create witness bindings and rendered names during observation under witness
  capture;
- compile diagnostic allocation and provenance counters out of ordinary
  builds.

### One observation owner

Semantic operations submit completed resolved-source handles to one source-use
collector. The collector owns identity for a source occurrence and its
instantiation key. Repeated consumers reuse the same handle instead of merging
different payloads after emission.

After semantic analysis and required template materialization finish, one
observer walks the collected resolved results and calls `emit_class_use`. The
observer may format bindings and source locations. It may not perform name
lookup, specialization selection, type recovery, source scanning, or syntax
reparsing.

The implementation can add this observer after the semantic paths converge.
Adding it before deleting the old analyzers does not count as a site reduction.

Three semantic operations can create a resolved class-use handle:

1. class-template-id resolution;
2. alias expansion that propagates a retained materialized class use;
3. substitution of a retained dependent class-use pattern.

All three feed the same collector and observer.

## Performance policy

### Fixed and rolling comparisons

Keep two baseline files throughout the work:

- `/tmp/cppgm-class-use-convergence-fixed.json` holds the promoted
  `a42c915e3` result and never changes;
- `/tmp/cppgm-class-use-convergence-rolling.json` starts as a copy of the fixed
  result and advances to each measured phase checkpoint.

Initialize them without rerunning the baseline:

```sh
cp /tmp/cppgm-witness-consolidation-rolling.json \
  /tmp/cppgm-class-use-convergence-fixed.json
cp /tmp/cppgm-class-use-convergence-fixed.json \
  /tmp/cppgm-class-use-convergence-rolling.json
```

Verify the fixed file before implementation:

- head: `a42c915e3acd91ee56f9f8913d0c5695ad52ccf5`;
- three-run median instructions: `176517676986`;
- three-run median maximum RSS: `762712064`;
- three-run median peak footprint: `590123008`;
- frozen workload epoch: `9764b3835e3c6996b6b80803054f80e1cf50f98e`.

If the artifact disappears, recover it from the experiment artifacts. Do not
record a replacement baseline. Starting a new performance epoch requires a
written plan amendment and user approval.

### Measure each phase once

Commit a correctness-clean phase before measuring it. Record one three-run
candidate batch:

```sh
scripts/validate_perf_regression.py record \
  --baseline /tmp/cppgm-class-use-phase-N.json \
  --runs 3
```

Phase 0 adds a read-only comparison mode to the validator so one recorded
candidate can be compared with both baselines without another execution:

```sh
scripts/validate_perf_regression.py compare \
  --baseline /tmp/cppgm-class-use-convergence-fixed.json \
  --candidate /tmp/cppgm-class-use-phase-N.json \
  --advisory \
  --instruction-tolerance 0.005 \
  --rss-warning-tolerance 0.03 \
  --footprint-tolerance 0.01

scripts/validate_perf_regression.py compare \
  --baseline /tmp/cppgm-class-use-convergence-rolling.json \
  --candidate /tmp/cppgm-class-use-phase-N.json \
  --advisory \
  --instruction-tolerance 0.005 \
  --rss-warning-tolerance 0.03 \
  --footprint-tolerance 0.01
```

The comparison mode must reuse the existing workload-identity and median
logic. `--advisory` changes the exit policy, not the calculations. Add tests in
`scripts/tests/test_validate_perf_regression.py` before semantic work starts.

After recording the ledger entry, promote the measured candidate without a
rerun:

```sh
cp /tmp/cppgm-class-use-phase-N.json \
  /tmp/cppgm-class-use-convergence-rolling.json
```

### Intermediate interpretation

Intermediate phases may add source-result storage before they delete a
recovery arm. The existing tolerances act as investigation thresholds during
these phases:

- instructions at or above `+0.5%`;
- peak footprint above `+1%`;
- maximum RSS at or above `+3%`.

Crossing a threshold does not reject an intermediate phase. Before starting
the next phase:

1. identify the added work or allocation with semantic counters, allocation
   diagnostics, object size, or a profile;
2. remove avoidable copies, allocations, and dual-path work;
3. record the cause and the phase that deletes the remaining scaffold in the
   ledger.

Do not rerun an unchanged checkpoint. A correction creates a new commit and a
new three-run candidate.

An RSS warning receives one confirmation batch of three runs. Record both
medians. A confirmed warning creates a cleanup obligation but does not force an
intermediate rollback when the allocation belongs to named temporary
scaffolding.

Pause further metadata expansion when cumulative instructions exceed the fixed
checkpoint by 2% or peak footprint exceeds it by 3%. Profile and reduce the
representation before stacking another phase. This pause protects the final
reduction goal; it does not change the final gates.

### Final performance gate

The final checkpoint compares against the fixed baseline, regardless of the
rolling history.

Instructions must show a repeatable reduction:

- a first-batch reduction of at least 0.5% passes the instruction goal;
- a reduction between 0% and 0.5% requires one confirmation batch;
- both batches must remain below the fixed median, and the ledger reports the
  higher median;
- a median at or above the fixed checkpoint fails.

Peak footprint must remain within `+1%` of the fixed median. A reduction is the
target. Maximum RSS keeps the 3% warning rule: one confirmation batch runs at
or above 3%, and a second result at or above 3% fails.

If the final instruction result does not decrease, continue the cleanup and
profile work. Do not accept the semantic endpoint on correctness and site count
alone.

## Phase 0: Freeze evidence and add measurement support

1. Copy and verify the fixed and rolling performance files.
2. Copy the final provenance report to a stable external artifact location if
   `/tmp` cleanup threatens it. Record its digest in the ledger.
3. Record `sizeof(Type)`, `sizeof(TemplateArgument)`, `sizeof(ClassInfo)`, and
   the relevant declaration records in a diagnostic test or report.
4. Record production source line counts for `dev/src`, excluding generated
   files.
5. Add the validator's read-only `compare` mode and tests.
6. Build the ordinary compiler and confirm that the measurement tooling does
   not alter it.

No semantic code changes belong in this phase.

## Phase 1: Build the typed class-template-id result

Create the compact semantic result and its storage without changing witness
output.

1. Extract the arguments, dependency state, selection, selected declaration,
   and source syntax already computed by
   `reference_class_template_instantiation_with_syntax`.
2. Return or fill `ResolvedClassTemplateId` beside the existing `ClassInfo *`
   result.
3. Make the existing canonical class-use path format its event from this
   result. Keep its producer ID during parity validation.
4. Add structural equality diagnostics between the old request and the new
   result under `CPPGM_WITNESS_PROVENANCE`.
5. Prove that ordinary builds do not allocate retained source results unless
   later semantic work needs them.

Do not delete a producer in this phase. Measure the metadata cost before other
changes can hide it.

Exit evidence:

- witness and LowIR output match;
- canonical `.02` rows match field for field;
- hot structure sizes do not grow without a ledger exception;
- the phase has fixed and rolling performance comparisons.

## Phase 2: Merge dependent pattern selection

Fold `class.callsemantic.08` into the canonical result.

1. Let `ResolvedClassTemplateId` represent dependent primary, partial, and
   explicit selections.
2. Preserve the selected pattern scope, selected declaration anchor, dependent
   argument facts, and canonical parameter bindings.
3. Make `lookup_type_node` return the resolved source result with its type so it
   does not call a second pattern analyzer after lookup.
4. Delete `record_dependent_pattern_class_use_for_template_id_syntax` and its
   producer.

Expected inventory: nine class producers to eight.

The four rows once owned by `.08` form the targeted gate. Start with
`pa22/tests/general/100-reference-shell-out-of-class-current-specialization-iterator.t`.

## Phase 3: Delete nested and template-declaration recovery

This phase removes the largest duplicated semantic work. Split it into two
measured checkpoints if the result-storage change and template-definition
change cannot form one reviewable commit.

### 3A. Retain nested argument and qualifier results

1. Make template-argument, type-id, and expression resolution return child
   resolved-source handles.
2. Route the class-template reference path, alias expansion, declaration type
   parsing, constructor initializers, and variable initializers through those
   children.
3. Remove callbacks that ask `SemanticContext` to scan template-argument or AST
   syntax for class uses.
4. Transfer each unique `.06` row to the resolved child that selected it.

### 3B. Analyze template patterns once

1. Make template declaration collection build a pattern scope once and retain
   semantic results for nondependent and dependent template-ids.
2. Store nondependent results at definition analysis time.
3. Store parameterized results for dependent syntax and substitute them during
   instantiation.
4. Move the alias observations owned by `alias.callsemantic.03` into the same
   typed pattern analysis. The full rewalk cannot disappear while its alias arm
   remains.
5. Delete `record_template_parameter_clause_source_uses`,
   `build_class_use_source_decision_from_template_syntax`, the recursive
   nested-class scanners, their caches, and their producer IDs.

Expected inventory after 3B: eight class producers to six. The AST and
template-argument replay routes must reach zero.

Targeted fixtures include:

- `.06`: `pa20/tests/general/100-local-variable-template-keeps-concrete-class-instantiation.t`;
- `.07`: `pa19/tests/general/100-nested-instantiation-template-parameter-scope-isolation.t`;
- nested alias and pack fixtures that exercise `alias.callsemantic.03`.

The provenance report must show no replacement row that borrows occurrence
data from a deleted producer.

## Phase 4: Unify qualified-id and constant evaluation

Add `ResolvedQualifiedId` to normal expression and member lookup. It contains
the selected entity and the resolved owner chain.

1. Route `Template<Args>::value` through the canonical qualified-id resolver.
2. Keep constant evaluation responsible for materializing the selected value.
3. Make the constexpr fast-call path consume the selected call's resolved
   owner chain.
4. Delete `record_constexpr_direct_call_owner_class_use`.
5. Delete the class-template lookup, argument resolution, specialization
   selection, and emission block from
   `lookup_constant_template_member_value`.
6. Remove `class.constant_value_lookup.02` and `.03`.

Expected inventory: six class producers to four.

Targeted fixtures include:

- `.02`: `pa23/tests/general/500-alias-pack-enable-if-constexpr-constructor.t`;
- `.03`: `pa20/tests/general/100-dependent-decltype-qualified-static-member-value.t`.

## Phase 5: Retain out-of-class owner patterns

Merge direct out-of-class owner recovery and delayed static-member replay.

1. Make method, special-member, and static-member binding resolution return a
   `ResolvedOwnerReference` with its syntax anchor and resolved or parameterized
   class-template-id.
2. Store the owner handle on `OutOfClassStaticMemberDecl`,
   `OutOfClassMemberFunctionDecl`, and related records that apply a definition
   after collection.
3. Substitute a retained owner pattern when template instantiation applies a
   definition.
4. Feed the concrete result to the canonical collector.
5. Delete `emit_out_of_class_owner_class_use_if_needed_impl`, its private text
   canonicalizer, `note_out_of_class_owner_class_use_for_applied_definition`,
   and static-member AST replay.
6. Remove `class.callsemantic.13` and `class.template_instantiation`.

Expected inventory: four class producers to two. The static-member-definition
AST route must reach zero.

Targeted fixtures include:

- `.13`: `pa22/tests/general/100-partial-specialization-nested-template-id-member-outdef.t`;
- instantiation: `pa19/tests/general/300-class-template-static-member-assignment-lvalue.t`.

## Phase 6: Preserve alias expansion provenance

Remove class recovery from an alias-expanded `TypePtr`.

1. Make alias instantiation return its canonical type plus a source expansion
   result.
2. Retain the expanded target class use, including its primary and
   specialization bindings.
3. Let declaration type analysis classify the retained child as a materialized
   type use at the alias spelling.
4. Delete token-range probing used to decide whether the declaration spelled a
   direct class template or an alias.
5. Delete `record_deduced_class_use_for_resolved_alias_type`, its public route,
   and `class.callsemantic.10`.

Expected inventory: two class producers to one. The resolved-alias-type route
must reach zero.

Use
`pa19/tests/general/100-function-template-explicit-specialization-address.t`
as the first materialized-use fixture.

## Phase 7: Install the final observer and remove scaffolding

1. Publish class uses once from the completed resolved-source collection.
2. Remove direct emission from the class-template reference implementation.
3. Delete dual-path parity checks, temporary adapters, unused source-use modes,
   dead `ClassUseEmissionOrigin` cases, replay interfaces, and empty caches.
4. Run provenance and identify class conflict-table branches or renderer passes
   that receive no action. Delete a policy after both strict and broad probes
   show zero hits.
5. Compact the retained result arena and remove fields that the migrations no
   longer read.
6. Record production source lines, hot structure sizes, object sections, and
   final performance.

The final code has one semantic observation owner and one direct
`emit_class_use` call outside the API implementation.

## Per-phase correctness gate

Use this sequence for every phase checkpoint:

1. Run the fixtures that cover the migrated producer's unique rows and payload
   fields.
2. Build with provenance and run the affected strict PAs. Confirm that the new
   owner supplies each row and that the removed producer and route make no
   attempt.
3. Run the full strict gate with direct LowIR comparison:

```sh
CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 \
make test-strict \
  CXX=/usr/local/opt/llvm/bin/clang++ \
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
```

4. Run the PA1-PA38 report:

```sh
CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 ORDERED=false \
make test-report \
  CXX=/usr/local/opt/llvm/bin/clang++ \
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
```

5. Commit the phase, record its three-run performance batch, compare it with
   both baselines, and update the ledger.

Do not update witness references. A witness mismatch means the new semantic
result lacks a responsibility from the removed path. Restore that fact at
resolution or substitution time.

Run `make inception` at the final checkpoint. Run it earlier when a phase
changes persistent semantic model layout or template instantiation ownership.

## Code-size accounting

Record source additions and deletions after each phase:

```sh
git diff --numstat \
  a42c915e3acd91ee56f9f8913d0c5695ad52ccf5..HEAD \
  -- dev/src dev/Makefile dev/frontend_source_sets.mk
```

The final production diff must delete more lines than it adds. Tests,
documentation, diagnostic instrumentation, and generated files do not count
toward that requirement. Also record:

- number of direct class-use sites;
- number of class replay routes;
- number of deleted recovery functions and callbacks;
- ordinary `cppgm++` text, data, and total file size.

Binary size is diagnostic because compiler and path metadata can perturb it.
Production source reduction and deleted semantic arms are hard requirements.

## Commit and rollback discipline

Keep each semantic route removal in a separate commit or short commit series:

1. result-model support;
2. consumers migrated to the result;
3. old analyzer and route deleted;
4. zero-hit policy cleanup.

Do not combine unrelated renderer or compiler cleanup with a measured phase.
Do not leave two complete resolution algorithms active across more than one
phase checkpoint.

A correctness failure requires completing the canonical result. A performance
increase requires investigation and a cleanup obligation. Revert a semantic
direction after evidence shows that the retained model cannot meet the final
instruction and code-size goals without an unmaintainable design.

## Completion criteria

The work finishes after all of these checks pass:

- one class producer remains and owns every visible class-use row;
- all four starting replay routes have zero callers and no interface;
- semantic analysis performs no second class-template lookup or selection for
  witness capture;
- template patterns retain dependent uses and substitute them without source
  reparsing;
- constant evaluation consumes normal qualified-id results;
- alias expansion carries materialized class-use provenance;
- out-of-class definitions carry their resolved owner result through delayed
  application;
- class-use table replacements and renderer drops caused by competing
  producers reach zero and their dead policies are removed;
- the production source diff has net line deletion;
- strict, full report, and inception pass;
- the final instruction result satisfies the repeatable-reduction rule against
  `176517676986`;
- peak footprint stays within 1% of `590123008` and maximum RSS clears the 3%
  warning rule against `762712064`;
- the ledger contains fixed and rolling deltas for every phase and no open
  cleanup obligation.
