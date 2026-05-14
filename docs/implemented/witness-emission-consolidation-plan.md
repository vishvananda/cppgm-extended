# Witness Emission Consolidation Plan

## Goal

Keep witness output changes constrained to a small number of owners.

The source witness path is now mostly structured through
`SemanticSourceUseTable`, but many semantic sites still construct witness rows
by hand. The immediate target is to consolidate those producer paths so witness
format or policy changes do not require editing the same decision-building
logic in several unrelated semantic functions.

This plan only covers witness emission and source-row construction. It does not
try to change template semantics, patched-Clang references, or lifecycle
materialization policy.

## Current Shape

### Output Entrypoints

These are already reasonably constrained:

- `dev/cppgm++.cpp`
  - collects one `TemplateWitnessSession` per input when `--witness` or
    `--witness-debug` is requested
  - writes through `render_witness_sessions` or
    `render_witness_debug_sessions`
- `dev/src/template_text_output.cpp`
  - combines source witness text with closure lifecycle text
  - owns public/debug closure event formatting
- `dev/src/template_witness_renderer.cpp`
  - projects `SemanticSourceUseTable` rows into source witness text
  - still contains substantial normalization, dedupe, and source-text recovery
    logic

The file-write surface is not the main duplication problem.

### Session Storage

`TemplateWitnessSession` has two independent streams:

- `lifecycle_events`
  - temporal template activity
  - recorded through `note_template_witness_log_event` and closure-context
    helpers
- `source_use_table`
  - source-facing rows
  - recorded through `witness::record_*_source_use`

This split is correct and should stay. Source witness cleanup should not merge
source rows and lifecycle events into one generic event type.

### Source-Use API

`dev/src/witness_api.*` is the current structured bridge from semantic
decisions to `SemanticSourceUse` rows:

- `ClassUseSourceDecision`
- `AliasUseSourceDecision`
- `VariableUseSourceDecision`
- `FunctionCallSourceDecision`
- `record_class_use_source_use`
- `record_alias_use_source_use`
- `record_variable_use_source_use`
- `record_function_call_source_use`

The weakness is that these structs are still low-level payload structs. Callers
populate anchors, selected-declaration locations, selection kind, bindings, and
ownership manually.

## Duplication Inventory

### Class-Use Producers

Class-use rows are the largest duplicated cluster. The same pattern appears in
many `dev/src/callsemantic.cpp` sites, plus smaller copies in
`dev/src/semantic_overload.cpp` and `dev/src/template_instantiation.cpp`:

- choose a use location
- decide whether the source spelling is exact enough for a use anchor
- find selected declaration anchor
- select primary, partial, or explicit specialization
- append primary bindings
- append specialization bindings
- record with `Direct`, `NestedDerived`, or `SourceOwned`
- sometimes emit nested class-use rows from the same source location

Notable direct producers:

- class template reference and instantiated type lookup paths in
  `callsemantic.cpp`
- `build_class_use_source_decision_from_template_text` and nested-template-id
  fanout in `callsemantic.cpp`
- qualified owner use emission in `callsemantic.cpp`
- out-of-class owner class-use emission in `callsemantic.cpp`
- parameter-clause template use emission in `callsemantic.cpp`
- owner-class use from overload selection in `semantic_overload.cpp`
- out-of-class applied definition owner use in `template_instantiation.cpp`

This is the highest-value consolidation area.

### Alias-Use Producers

Alias-use rows repeat the same construction shape across:

- direct alias-template use in `template_argument_semantics.cpp`
- alias pattern source use in `template_specialization.cpp`
- alias pattern source use from texts in `template_specialization.cpp`
- alias instantiation source use in `callsemantic.cpp`

Each site builds the same selected declaration anchor and binding list, but each
site has its own local binding helper or selected-anchor logic.

### Binding Generation

Template witness binding generation is duplicated across several files:

- generic class/function helpers in `template_api.cpp`
- alias-template source bindings in `template_argument_semantics.cpp`
- alias-pattern source bindings in `template_specialization.cpp`
- variable-template source bindings in `template_instantiation.cpp`
- owner-class binding canonicalization in `callsemantic.cpp`

Most copies implement the same pack splitting, defaulted versus explicit source
selection, and unnamed-parameter fallback. The differences are real but should
be represented as policy options, not copy-pasted loops.

### Function-Call Producers

Ordinary function-template call witness emission is mostly centralized in
`semantic_overload.cpp::note_function_call_source_event`.

The main duplication is around special paths:

- `declval` witness row construction appears in both `callsemantic.cpp` and
  `semantic_overload.cpp`
- owner-class witness emission from chosen function bindings duplicates the
  class-use construction logic
- candidate drop creation has useful helpers, but the final decision payload is
  still built field-by-field

Function calls are less fragmented than class uses, so they should be cleaned
after class/alias/binding consolidation.

### Variable-Use Producer

Variable-template source use is mostly one producer in
`template_instantiation.cpp`, but it has its own binding builder and selected
declaration anchor construction. It should reuse the common binding and anchor
helpers once those exist.

### Renderer Recovery

`template_witness_renderer.cpp` mirrors `SemanticSourceUse` into a local
`WitnessEvent` model and then performs substantial recovery:

- nested template-id source scanning
- event synthesis from source text
- several class/function/variable grouping and dedupe passes
- type spelling and binding normalization
- source-defined function-call suppression

Some deterministic formatting belongs in the renderer, but semantic source-row
discovery does not. Once producers emit complete rows, the renderer should be
mostly mechanical.

There is also normalization duplication between:

- `template_witness.h`
- `template_witness_renderer.cpp`
- `template_text_output.cpp`

This should move into one witness text normalization utility once behavior is
stable.

## Drift-First Assessment

After refreshing the active strict witness set for `pa18 pa19 pa21 pa22`, the
current mismatch floor is:

- `pa18`: 27 witness mismatches
- `pa19`: 32 witness mismatches
- `pa21`: 13 witness mismatches
- `pa22`: 126 witness mismatches

The drift is not one single producer duplication bug. Representative failures
fall into these buckets:

- Missing source rows
  - example: `pa18/tests/general/182-template-forward-definition-parameter-rename.t`
    is missing a deduced `Box<int>` class-use row
  - producer consolidation helps only if the new owner records the missing
    semantic occurrence

- Extra source rows
  - example: `pa18/tests/general/200-constructor-template-const-ref-conversion.t`
    emits an extra template-pattern `duration<Rep2, Period2>` class-use
  - example: `pa22/tests/general/253-void-t-decltype-function-template-call-partial-specialization.t`
    emits function-call rows from the `void_t<decltype(...)>` probe where the
    reference expects the alias/class selection row instead
  - consolidation helps if the new API has explicit source-use policy, but a
    pure mechanical consolidation would preserve the wrong rows

- Binding text canonicalization
  - examples include `decltype (g (...))` versus `decltype(g(...))`, `int&`
    versus `int &`, and inline-namespace-qualified argument text such as
    `s::c::nano` versus `nano`
  - these are usually better fixed once in the witness binding/text
    normalizer, not by touching every producer

- Wrong template source text
  - example: `pa22/tests/general/479-dependent-variable-template-empty-pack-enable-if-selection.t`
    rewrites `Alloc` to `int` inside the explicit `enable_if_t` argument
  - this is missing source-pattern text preservation, not just duplicated
    emission code

- Lifecycle/entity drift
  - example: inline namespace entities appear as `s::i::c::...` where the
    reference expects `s::c::...`
  - source-use producer consolidation will not fix lifecycle entity naming;
    that needs lifecycle/entity canonicalization

- Candidate-drop reason drift
  - example: a partial-ordering drop reports `bad_conversion` where the
    reference expects `better_candidate_selected`
  - this belongs near overload candidate ranking, not in generic witness
    emission

Therefore, producer consolidation is necessary for maintainability, but it is
not the simplest first step to reduce the current drift floor.

## Simpler First Pass

Start with the high-leverage policy and canonicalization fixes that attack many
current diffs without changing semantic control flow:

1. Add one witness text/binding canonicalization owner.
   - Move common normalization out of scattered renderer/helper code.
   - Normalize type spacing, `decltype` formatting, reference spacing,
     integral non-type spelling, and inline namespace presentation in one place.
   - Validate against alias/function binding diffs before touching producers.

2. Add explicit source-capture policy for probe contexts.
   - Function-call rows inside SFINAE/`decltype` probes should not leak just
     because overload resolution happened while computing an alias/class
     selection.
   - Use a scoped source-capture policy rather than ad hoc renderer drops.
   - This targets the large `void_t`, `declval`, detected-or, and enable-if
     families.

3. Preserve source-pattern argument text before substituting concrete template
   arguments.
   - When witness expects `Alloc`/`Tp`/`Args...`, do not render the already
     instantiated `int`/`int *` values into explicit source argument text.
   - This should be represented in the binding helper policy, not patched per
     alias/variable producer.

4. Centralize lifecycle entity canonicalization separately from source-use
   emission.
   - Inline namespace elision and default-template-argument elision affect
     closure events as much as source rows.
   - Do not route this through class-use source emitters.

5. Only then consolidate source-use producers.
   - Once the policy is explicit, converting field-by-field producers to
     intent-level helpers becomes safer and should reduce missing/extra rows
     without freezing current drift into a new API.

This ordering is simpler because it fixes broad classes of drift first and
keeps producer consolidation from becoming a blind refactor over incorrect
behavior.

## Producer Consolidation Plan

### Phase 1: Add Intent-Level Source-Use Emitters

Add a small source-use emission layer above the raw decision structs.

Proposed owner:

- `dev/src/witness_api.h`
- `dev/src/witness_api.cpp`

Representative helpers:

```cpp
struct ClassTemplateSourceUseRequest {
  SemanticContext * ctx;
  Scope * scope;
  ClassTemplateDecl * template_decl;
  ClassInfo * selected_info;
  TemplateWitnessSelectionKind selection;
  std::string use_location;
  std::string source_anchor_identifier;
  SourceUseOwnership ownership;
  SourceUseRole role;
  std::vector<TemplateArgument> primary_arguments;
  std::vector<std::string> explicit_argument_texts;
  const std::vector<TemplateParameterInfo> * specialization_parameters;
  std::vector<TemplateArgument> specialization_arguments;
  bool emit_nested_uses;
};

bool record_class_template_source_use(const ClassTemplateSourceUseRequest & request);
```

The exact shape can be simpler than this, but the helper should own:

- use-anchor normalization
- selected-declaration anchor selection
- selection-kind mapping
- binding generation
- `record_*` call
- optional nested-use fanout policy

Do not make call sites construct `ClassUseSourceDecision` directly unless they
are temporary migration code.

### Phase 2: Centralize Binding Construction

Move pack/default/explicit binding construction behind one configurable helper.

Inputs:

- parameter list
- resolved arguments
- optional source explicit argument texts
- source policy for explicit, defaulted, and deduced bindings
- type-rendering callback
- pack rendering policy

Required policies:

- ordinary class/alias/variable bindings with pack groups as `<...>`
- function-template call bindings with explicit count and call-deduced/defaulted
  source selection
- alias-pattern bindings that may initially preserve explicit source text
- special owner-class canonicalization can remain as a policy callback until it
  is eliminated structurally

After this phase, local helpers such as alias-pattern and variable binding loops
should disappear.

### Phase 3: Consolidate Class-Use Producers

Replace class-use call sites in slices:

1. Nested template-id text path
   - update `build_class_use_source_decision_from_template_text`
   - keep current cache behavior, but cache request-independent semantic facts
     instead of a mostly complete witness decision

2. Direct type lookup and class-template reference paths
   - replace repeated selected-decl anchor and binding code
   - preserve current exact-spelling guards at the call site when they are
     semantic policy, not formatting policy

3. Qualified owner paths
   - consolidate out-of-class owner use, overload owner use, and applied
     definition owner use into one helper

4. Parameter and specifier replay paths
   - convert remaining source-owned class-use rows to the same helper

Validation after each slice:

- build `dev/cppgm++`
- run targeted witness tests for the touched owner
- run active strict witness set for `pa18 pa19 pa21 pa22`
- accept only known witness floor changes when they are intentional and
  understood

### Phase 4: Consolidate Alias And Variable Producers

Add an alias-template source-use request helper that owns:

- selected declaration anchor
- use anchor
- expanded-to text
- binding construction

Then migrate:

- `template_argument_semantics.cpp`
- `template_specialization.cpp`
- `callsemantic.cpp`

After the alias helper is stable, update variable-template source-use emission
to use the same binding and selected-anchor utilities.

### Phase 5: Consolidate Special Function-Call Producers

Keep `semantic_overload.cpp::note_function_call_source_event` as the main
ordinary function-template call owner.

Cleanups:

- replace the duplicate `declval` source-use builders with one helper
- route owner-class side emission through the class-use helper
- keep candidate-drop construction near overload resolution, because that data
  is naturally owned there

### Phase 6: Reduce Renderer Recovery

Once semantic producers emit complete rows, remove renderer recovery in this
order:

1. source-text nested-template-id synthesis
2. grouping-based class/variable/function reconstruction
3. source-defined template-call suppression that should instead be controlled
   by producer policy
4. duplicate normalization helpers that belong in a shared witness text utility

The renderer should still own:

- final deterministic ordering
- final public versus debug formatting
- small dedupe of byte-identical rows
- path normalization

## Guardrails

- Keep source witness rows table-backed.
- Keep lifecycle witness event-backed.
- Do not add more renderer source scanning to compensate for missing semantic
  rows.
- Do not add new field-by-field `ClassUseSourceDecision` construction sites.
- Prefer adding missing structured request data over parsing source text inside
  witness formatting.
- Preserve witness-off cost by keeping capture behind the existing session/table
  gate.

## First Implementation Slice

Start with drift reducers, not producer consolidation. The current failures show
too much text/policy drift for a blind consolidation to be safe.

Recommended first patch:

1. add a shared witness text/binding canonicalization owner
2. route alias, function, and variable binding text through it
3. add a scoped source-capture policy for SFINAE/`decltype` probe contexts
4. validate the active strict witness set and classify the remaining diffs

Expected effect:

- reduce broad formatting-only churn in alias/function bindings
- remove leaked function-call rows from probe-only contexts
- make remaining missing/extra class-use rows easier to attribute to semantic
  producers rather than renderer normalization

After this patch, move to the class-use selected-declaration and binding
consolidation slice from the producer plan.
