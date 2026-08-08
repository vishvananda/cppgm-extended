# Class-Use Materialization Semantic Ownership Plan

## Status and handoff

This plan is a corrective follow-up to
`docs/witness-class-use-semantic-convergence-plan.md`. Class-use publication
has converged to one producer, one pre-publication occurrence collector, and
one final emitter, but the observer still reconstructs whether a dependent
source pattern materialized as a concrete type.

The correction that removed 55 late renderer decisions retained five concrete
dependent-pattern rows. Those rows agree with the patched-Clang witness, but
their CPPGM admission currently depends on three observer-side exemptions:

- variable-instantiation entry context;
- a source-text reconstruction of fixed class constants;
- a source-text reconstruction of fixed conversion-result aliases.

The last two are forbidden text reparses. This plan replaces all three
exemptions with one typed materialization result produced by the semantic
operation that materializes the source type.

- Repository worktree:
  `/Users/vishvananda/cppgm-alias-consolidation`
- Branch at plan creation:
  `experiment-witness-alias-path-consolidation`
- Evidence checkpoint:
  `f5d5eda3e` (`Document class-use materialization boundary evidence`)
- Parent class-use ledger:
  `docs/witness-class-use-semantic-convergence-ledger.md`
- Existing audit artifact:
  `/tmp/cppgm-class-materialization-boundary-audit.json`
- Execution ledger to create in Phase 0:
  `docs/witness-class-materialization-semantic-ownership-ledger.md`

Alias convergence is active in this worktree. Execution must start from its
clean final checkpoint, not from the dirty plan-creation tree. Phase 0 records
that exact starting commit and creates the post-diagnostic performance epoch.

## Problem statement

Clang's witness visitor emits a class use when it visits a concrete
`TemplateSpecializationTypeLoc`. CPPGM often reuses the dependent source AST
while instantiation resolves a concrete `TypePtr`; it does not always retain a
typed source-location result that distinguishes:

1. materializing a concrete type at a source occurrence; from
2. revisiting dependent syntax during lookup, SFINAE, constant evaluation, or
   another internal semantic query.

The class observer currently reconstructs that distinction in
`callsemantic.cpp`:

- `source_arguments_are_fixed_class_constants` renders initializer ASTs with
  `spaced_node_text`, scans argument strings, and matches identifiers against
  scope values;
- `source_arguments_are_fixed_class_aliases` scans argument strings, walks
  reference instantiations, and matches identifiers against named types;
- `text_mentions_template_parameter` supports both reconstructions with
  identifier-token matching;
- `observe_resolved_class_template_id` combines those answers with the current
  witness entry context to decide whether a concrete replay may publish.

This is semantic recovery from text after normal analysis already resolved the
type. It is not made acceptable by the small number of strict-corpus hits.

## Objective

Make normal semantic analysis publish one structured fact when it materializes
a concrete class-template type at a source occurrence. Make class-use
observation consume that fact without inspecting source spelling, enclosing
names, constant initializer text, conversion-function names, or ambient
witness context.

The final implementation must have:

- one typed source materialization result shared by function-body,
  declaration-type, static-member-initializer, and variable-template-
  initializer analysis;
- one parameterized dependent source result whose identity survives
  substitution without a location-string lookup;
- one admission rule for a concrete revisit of a dependent pattern: a valid
  typed materialization result exists for that exact source occurrence;
- no source-text, token, or identifier reconstruction in the class
  materialization decision;
- no fixture, template-name, source-mode, or conversion-function whitelist;
- the same five accepted rows and the same 55 rejected locations as patched
  Clang, with evidence for both groups in one self-contained audit;
- one source-table insertion per visible class occurrence and zero renderer
  visibility decisions;
- fewer production source lines and fewer retired instructions than the fixed
  Phase 0 checkpoint.

Intermediate phases may move slightly above the fixed performance checkpoint
while typed results are introduced. The final phase must recover that
temporary cost and finish below the checkpoint in source and instructions.

## Forbidden implementations

The following do not satisfy this plan:

- moving `source_arguments_are_fixed_class_constants` or
  `source_arguments_are_fixed_class_aliases` to a different file or helper;
- replacing those functions with another use of `node_text`,
  `spaced_node_text`, `contains_identifier_token`, source argument strings, or
  token scans;
- setting `materialized = true` solely from `ClassTemplateSourceUseMode` or
  `current_template_witness_entry_context`;
- inferring materialization by matching the current function name or result
  spelling;
- admitting the five known template names or source locations explicitly;
- publishing a speculative row and removing it in `WitnessBuilder` or the
  renderer;
- merging distinct source locations merely because they resolve to the same
  specialization;
- adding materialization storage to `Type`, `TemplateArgument`, `ClassInfo`, or
  another hot semantic structure without a measured plan amendment.

Source strings remain valid for final witness spelling. They may not determine
whether a semantic source occurrence exists.

## Starting evidence and the five retained rows

The existing audit records five concrete dependent-pattern materializations:

| Semantic owner | Fixture and source occurrence | Concrete result |
| --- | --- | --- |
| Instantiated function body | `pa20/tests/general/100-dependent-nontype-functional-cast-body-check.t:12:17` | `integral_constant<bool, true>` |
| Static-member initializer | `pa24/tests/general/100-dependent-reference-alias-default-nontype.t:29:37` | `Block<int>` |
| Instantiated conversion-function body | `pa24/tests/spec/100-out-of-class-conversion-operator-definition.t:17:10` | `sink<int>` |
| Instantiated declaration type | `pa24/tests/general/100-template-static-constant-nontype-argument.t:9:11` | `Box<9223372036854775807>` |
| Variable-template initializer | `pa24/tests/general/300-dependent-variable-template-empty-pack-enable-if-selection.t:21:30` | explicit `has_construct<int, int *, <> >` selection |

These are five source occurrences, not five emitters. Each currently reaches
`ClassTemplateReference02`, the class occurrence collector, and the single
`emit_class_use` call. They must remain separate from other locations that
resolve to the same specialization because Clang reports source occurrences,
not a set of referenced specializations.

The current artifact proves patched Clang emits zero rows at all 55 rejected
locations, but it does not store symmetrical patched-Clang payloads for the
five accepted locations. Phase 0 closes that evidence gap before behavior
changes.

## Target semantic model

### Parameterized source occurrence

When normal class-template-id resolution first sees a dependent source
pattern, create or retain a compact occurrence identity containing:

- stable `TemplateIdSyntax::source_location_id`;
- the originating `ClassTemplateDecl *`;
- structured argument syntax or existing parameterized argument handles;
- the declaring pattern scope or binding-frame identity;
- dependency state produced by template argument resolution;
- nested-source and selected-declaration facts already produced by resolution.

Substitution must propagate this occurrence identity into the concrete result.
The observer must not rediscover it through
`dependent_class_source_patterns_by_location` or a normalized location string.
Use the stable source ID and semantic origin as identity; use source strings
only when rendering the final row.

### Typed materialization result

Introduce a stack-scoped, non-owning result with a name such as
`ResolvedSourceTypeMaterialization`. The exact name is not normative. It must
carry or reference:

- the parameterized source occurrence or stable source ID;
- the resolved `TypePtr`;
- the completed `ResolvedClassTemplateIdView`;
- the semantic operation that owns the concrete source node;
- the resolved arguments and selected specialization already computed by that
  operation;
- an explicit fact that the operation materialized a source type node, rather
  than performing lookup-only analysis.

The owner kind is diagnostic provenance, not an admission whitelist. Expected
owners include instantiated function-body type, declaration type, static-
member initializer, and variable-template initializer. Admission depends on a
valid materialization result, regardless of owner kind.

Do not copy argument vectors for this result. Use stack views while the
operation is active and compact handles in the witness-session side store only
when observation is deferred.

### Structured dependency

Dependency must come from semantic data:

- `TemplateArgument::dependent` and its structured `TypePtr`;
- `ValueBinding::dependent_template_value`, constant-evaluation results, and
  structured `TemplateValueDependency` records;
- the resolved alias `TypePtr` and alias-resolution result;
- the parameterized source occurrence and active substitution frame.

If an owner does not currently retain sufficient structured dependency, fix
that owner to publish the missing fact when it evaluates or resolves the
expression. Do not recover it later by rendering or scanning the initializer.

### One observer rule

After migration, the relevant observer rule should be equivalent to:

```text
if this is a concrete revisit of a dependent source occurrence
  require a typed materialization result for that occurrence
otherwise
  apply the ordinary direct-source rules
```

The observer may format an already-selected result. It may not decide that a
constant is fixed, resolve an alias, match a conversion result, inspect the
current lifecycle context, or perform a second semantic query.

## Phase 0: Freeze evidence and the performance epoch

Start only after alias convergence has a clean correctness-complete commit.
Record the commit, worktree status, compiler defaults, and all relevant
artifact hashes in the new ledger.

1. Rebuild the ordinary and provenance compilers from that commit.
2. Regenerate the class materialization provenance corpus.
3. Extend the audit generator so `patched_clang` records exact rows for both:
   - all 55 rejected locations; and
   - all five accepted locations, including template, selection, bindings, and
     source anchor.
4. Add diagnostic-only route counters for materialization owners. Counters
   must distinguish observation, materialization production, collector merge,
   publication, and final visibility.
5. Record a three-run baseline after the counters are present:

```sh
scripts/validate_perf_regression.py record \
  --baseline /tmp/cppgm-class-materialization-ownership-fixed.json \
  --runs 3
```

Record the medians, individual samples, workload epoch, binary sizes,
`dev/src` line count, and these hot structure sizes:

- `Type`;
- `TemplateArgument`;
- `ClassInfo`;
- `ResolvedClassTemplateIdView`;
- `ResolvedAliasTemplateId`;
- the new materialization result, once introduced.

Phase 0 acceptance:

- strict witness/LowIR: `1305/1305`;
- PA1-PA38 report: `4860/4860`;
- accepted patched-Clang rows: exactly five at the audited locations;
- patched-Clang rows at rejected locations: zero;
- class source-table attempts equal insertions;
- class table rows equal public rows;
- renderer class removals remain zero.

## Phase 1: Add typed materialization in shadow mode

Add the parameterized occurrence and typed materialization result without
changing class visibility.

1. Create dependent occurrence identity during normal template-id resolution.
2. Propagate it through substitution and class-reference results.
3. Add explicit materialization scopes to ordinary source type analysis.
4. Have those scopes attach a typed materialization result to the resolved
   class-template-id view.
5. Keep the legacy exemptions temporarily, but record a shadow comparison
   between the legacy answer and the typed answer.

The shadow implementation must not call either forbidden text helper. A
typed result must originate at the operation that constructs the concrete
source type, not in `observe_resolved_class_template_id`.

Phase 1 acceptance:

- the typed result is present for all five accepted rows;
- it is absent for all 55 rejected locations;
- no accepted/rejected mismatch is hidden by renderer behavior;
- ordinary output is byte-for-byte unchanged;
- no hot semantic structure grows;
- provenance identifies the semantic owner for every typed result.

Do not advance while any mismatch remains. Diagnose whether the semantic owner
failed to publish a real materialization or incorrectly marked lookup-only
analysis.

## Phase 2: Migrate initializer owners

Move both initializer categories to explicit semantic ownership.

### Variable-template initializer

`acquire_variable_instantiation` already owns the operation that instantiates
a variable initializer. Make that operation pass a typed materialization scope
to initializer type and qualified-id analysis. Do not let the class observer
query `current_template_witness_entry_context` to infer this fact.

The expected retained row is `has_construct<int, int *>` in
`has_construct_v`'s initializer. An uninstantiated variable-template pattern
must not produce the row.

### Static-member initializer

The static-member definition analyzer owns whether `Box<T>::count` is
materialized. Propagate its typed initializer result directly. Do not use
`StaticMemberDefinitionValueUse` as a visibility decision.

The expected retained row is `Block<int>` in the instantiated `Box<int>::count`
initializer. The five prior static-owner candidates without a corresponding
variable materialization must remain unpublished.

Phase 2 acceptance:

- the two initializer rows are selected solely by typed materialization;
- ambient witness entry context is absent from class admission;
- source-use mode may describe role or spelling but cannot grant visibility;
- all initializer positive and negative fixtures match patched Clang.

## Phase 3: Migrate instantiated source type owners

Make ordinary instantiated source type analysis own the remaining three rows.

### Function-body type

When an instantiated function body analyzes the functional-cast type
`integral_constant<bool, _Rp != 0>`, carry the concrete resolved type and the
original parameterized occurrence into one typed materialization result.
Whether `_Rp` is fixed is already reflected in the resolved template argument;
the observer must not inspect `_Rp` or its initializer.

### Declaration type

When the instantiated `Limits<0>::type` declaration resolves `Box<max>`, carry
the resulting `Box<9223372036854775807>` type as the materialized source result.
The constant evaluator owns the structured value and dependency state. The
observer must not reconstruct the `nan -> min -> max` chain.

### Conversion-function body

When the instantiated body analyzes `sink<result_type>()`, the functional-cast
type resolver owns the concrete `sink<int>` result. Carry that result directly.
The observer must not recognize a conversion function or scan `result_type`
against named types.

Phase 3 acceptance:

- all three rows are selected solely by typed materialization;
- fixed-constant and fixed-alias shadow counters are no longer needed to
  explain any accepted row;
- a dependent constant or alias remains unmaterialized until its semantic
  owner actually constructs a concrete source type;
- repeated analysis of one source node produces one collected occurrence;
- separate source nodes resolving to the same specialization remain separate.

## Phase 4: Switch admission and delete forbidden recovery

Replace the legacy concrete-dependent-pattern gate with the single typed
materialization rule.

Delete:

- `text_mentions_template_parameter` if it has no unrelated structured
  semantic consumer;
- `source_arguments_are_fixed_class_constants`;
- `source_arguments_are_fixed_class_aliases`;
- `fixed_class_constant_source` and `fixed_conversion_alias_source` from the
  class admission decision;
- their diagnostic fields and accepted-reason taxonomy;
- materialization visibility checks based on
  `current_template_witness_entry_context`;
- location-string fallback maps used to rediscover dependent class patterns,
  once the parameterized occurrence handle owns that identity.

`class_template_use_matches_current_conversion_result` may remain only if it
has a non-materialization semantic or formatting consumer. It must not decide
whether a class row exists. Remove it if migration leaves no such consumer.

Add a static validation check that fails if the deleted helper names return.
The class materialization implementation must also be reviewed for new uses of
`node_text`, `spaced_node_text`, `contains_identifier_token`, token scans, or
source-argument string matching.

Phase 4 acceptance:

- both forbidden reconstruction helpers are absent from `dev/src`;
- no equivalent text reparse replaces them;
- a concrete dependent replay without a typed materialization result stops
  before `ClassUseEmitRequest` construction;
- all five accepted rows publish exactly once;
- all 55 rejected locations publish zero times;
- source-table attempts equal insertions and table rows equal public rows;
- renderer class visibility actions remain zero.

## Phase 5: Simplify the source model

Remove migration-only state and terminology.

1. Delete shadow counters and legacy materialization reason fields.
2. Narrow or remove `ClassTemplateSourceUseMode` values whose only purpose was
   to infer visibility.
3. Remove dependent-pattern location maps if all substitution paths carry the
   stable occurrence identity.
4. Remove unused witness entry-context plumbing from class resolution.
5. Keep one collector key based on source occurrence and semantic origin.
6. Confirm `observe_resolved_class_template_id` only validates and formats
   completed semantic results; it must not perform lookup, dependency
   analysis, constant evaluation, alias recovery, or source scans.

The result should delete more code than it adds. If the new typed result grows
into a general metadata container, stop and split it into a stack view plus a
compact retained occurrence handle.

## Validation matrix

### Focused correctness

Keep the five accepted fixtures as positive tests. Add or retain reduced
negative pairs covering:

- a class constant whose initializer depends directly on a class-template
  parameter;
- a class constant whose nested dependency chain is dependent;
- a fixed alias and an alias that remains dependent;
- a variable-template initializer that is never instantiated;
- a static-member definition whose owner specialization never materializes;
- SFINAE and constant-analysis queries that resolve a type without
  materializing a source type location;
- repeated semantic queries of one source occurrence;
- two source occurrences resolving to the same class specialization;
- an explicit specialization selected during variable initialization;
- empty and nonempty pack substitution in a materialized type.

Place a new regression in the earliest PA that owns its semantic behavior.
Generate witness references only with the patched-Clang path.

### Broad correctness

Every behavior-changing phase must pass:

```sh
CPPGM_LOWIR_DIRECT_TEXT_COMPARE=1 make test-strict
make test-report
```

The final phase must additionally pass:

```sh
make inception
```

### Provenance invariants

At every phase record:

- dependent source patterns observed;
- typed materializations produced, by semantic owner;
- early repeated observations;
- pre-publication occurrence merges;
- class source-table attempts and insertions;
- class source-table and public row counts;
- renderer class actions;
- accepted and rejected patched-Clang location comparisons.

Final required invariants:

```text
class source-table attempts == class source-table insertions
class source-table rows == class public rows
class renderer visibility actions == 0
accepted audited materializations == 5
rows at 55 rejected locations == 0
forbidden materialization text reparses == 0
```

The number five is a corpus observation, not a hard-coded compiler invariant.
New tests may add legitimate materializations when patched Clang supplies the
same typed source fact.

## Performance policy

Measure each correctness-clean phase with one three-run candidate batch and
compare it with both the fixed and rolling checkpoints.

- instruction advisory boundary: `0.5%`;
- footprint boundary: `1%`;
- RSS warning boundary: `3%`;
- an RSS warning triggers a second complete three-run gate;
- a second median increase above `3%` fails the phase.

An advisory regression must be investigated before rollback. Determine
whether it comes from allocation, copying, a repeated semantic operation, or
binary layout. A valid typed-result design may need a compact representation
or a moved ownership boundary rather than abandonment.

Intermediate phases do not require a strict reduction. Record and explain
their divergence. The final phase requires:

- fewer retired instructions than the Phase 0 fixed checkpoint;
- fewer net production source lines;
- peak footprint within `1%`;
- RSS passing the warning/confirmation rule;
- no growth in `Type`, `TemplateArgument`, or `ClassInfo`;
- no unexplained growth in the retained source-result side store.

Advance the rolling checkpoint only after correctness and the applicable
performance decision are recorded in the ledger.

## Final completion checklist

- [ ] One typed materialization result represents all concrete source type owners
- [ ] Dependent occurrence identity propagates through substitution
- [ ] The five audited rows are produced from typed semantic facts
- [ ] All 55 rejected locations remain absent
- [ ] `source_arguments_are_fixed_class_constants` is deleted
- [ ] `source_arguments_are_fixed_class_aliases` is deleted
- [ ] No equivalent source-text or token reconstruction exists
- [ ] Class admission does not inspect ambient witness lifecycle context
- [ ] Class admission does not recognize conversion functions or fixed names
- [ ] One class occurrence produces one source-table insertion
- [ ] No class row disappears after source-table publication
- [ ] Strict tests pass `1305/1305`
- [ ] PA1-PA38 report passes `4860/4860`
- [ ] Inception passes
- [ ] Final production source and instruction medians are below fixed
- [ ] Footprint, RSS, and hot-structure gates pass

