# Semantic-Side Template Boundary Consolidation Plan

## Goal

Keep the template/semantic boundary minimal while reducing repetition on the
semantic side of that boundary.

The current boundary rule is good:

- non-template semantic code crosses into the template subsystem through
  `template_api`
- `TemplateServices` remains template-side/shared implementation glue
- callback traffic from template code back into semantic code stays small and
  explicit

The next problem is semantic-side distribution of decision making. Several
semantic modules now call the clean boundary directly, but they still decide
locally when to deduce, instantiate, materialize, resolve dependent types,
mutate template binding state, or suppress output.

This plan consolidates those semantic decisions without widening the template
API and without moving template-owned logic back into semantic code.

## Non-Goals

- Do not add a broad `SemanticTemplateManager` that becomes a kitchen sink.
- Do not hide every `template_api` call just because it mentions templates.
- Do not reopen `TemplateServices` as a semantic-facing interface.
- Do not move template matching, specialization selection, or substitution
  ownership out of the template layer.
- Do not start by refactoring the mixed-ownership `callsemantic.cpp`
  exception; use it as a later migration target after cleaner semantic-side
  services exist.

## Current Audit Snapshot

The boundary cleanup left the structural include/call rules clean outside
`callsemantic.cpp`, but the semantic side still has repeated decision sites.

Observed repetition outside the mixed `callsemantic.cpp` exception:

- Function-template deduction/acquisition:
  - `TemplateFunctionDeductionRequest` / `deduce_function_template(...)`
    appears in `semantic_conversion.cpp`, `semantic_class_model.cpp`, and
    repeatedly in `semantic_overload.cpp`.
  - `TemplateFunctionInstantiationRequest` construction is repeated in
    semantic helper functions for conversion, class-model, and overload paths.
- Dependent type recovery:
  - `template_type_ops::resolve_type_argument_text(...)`,
    `resolve_type_lookup_text(...)`, and
    `resolve_instantiated_dependent_type(...)` are called from
    `semantic_builtins.cpp`, `semantic_class_model.cpp`,
    `semantic_conversion.cpp`, `semantic_expression.cpp`,
    `semantic_lookup.cpp`, `semantic_overload.cpp`, and
    `semantic_statement.cpp`.
- Template output/readiness policy:
  - `semantic_output.cpp` and `output_requirement_engine.cpp` both compose
    low-level template identity, explicit-instantiation, suppression, and
    readiness queries.
- Scope/template binding mutation:
  - scope fingerprint bumps and template named-type bindings are still invoked
    from several semantic modules instead of one semantic mutation path.

These are not boundary violations. They are consolidation targets.

## Decision Ownership Rules

### Semantic Side Owns

- deciding that a source construct is a function call, conversion, variable
  template use, class member use, output requirement, or declaration mutation
- choosing the semantic intent:
  - lookup only
  - track instantiation
  - require definition
  - require definition and export
- choosing restricted semantic analysis policy for a call site
- deciding when semantic state must be mutated and invalidated
- deciding whether output/materialization is required

### Template Side Owns

- binding template parameters to arguments
- deduction mechanics
- specialization selection
- instantiation mechanics
- template-owned structured type lookup beneath `TemplateTypeSystem`
- template witness lifecycle mechanics behind `template_api`

### Shared Rule

If a semantic caller needs to combine more than one `template_api` query to
make a policy decision, that combination should usually move behind a
semantic-owned facade.

If a semantic caller only asks a single factual question or performs a simple
template action, a direct `template_api` call is acceptable.

## Stage 1: Add Semantic-Side Audit Ratchets

Purpose:

- prevent the boundary from regressing while consolidation proceeds
- measure decision distribution before each slice

Add a lightweight audit script or documented `rg` target that reports counts
for:

- direct semantic construction of template request structs
- direct semantic calls to `template_type_ops`
- direct semantic calls to template binding/fingerprint mutation helpers
- direct output/readiness query composition outside the output policy owner
- direct construction of `TemplateServices` or `SemanticContextTemplateServices`
  outside template-owned implementation files

Initial audit commands can start from:

```sh
rg -n 'TemplateFunctionDeductionRequest|TemplateFunctionInstantiationRequest|TemplateVariableInstantiationRequest|deduce_function_template|acquire_.*instantiation' dev/src/semantic_*.cpp dev/src/output_requirement_engine.cpp dev/src/callsemantic_phase_bridge.cpp
rg -n 'template_type_ops::(resolve_type_argument_text|resolve_type_lookup_text|resolve_instantiated_dependent_type)' dev/src/semantic_*.cpp dev/src/output_requirement_engine.cpp dev/src/callsemantic_phase_bridge.cpp
rg -n 'template_api::bump_scope_template_binding_fingerprint_epoch|template_binding_ops::bind_named_type' dev/src/semantic_*.cpp dev/src/output_requirement_engine.cpp dev/src/callsemantic_phase_bridge.cpp
rg -n 'compute_instantiated_class_output_readiness|function_binding_instantiation_arguments|function_binding_output_suppressed|class_suppresses_implicit_instantiation_definition' dev/src/semantic_output.cpp dev/src/output_requirement_engine.cpp
```

The checked-in ratchet is:

```sh
python3 scripts/audit_semantic_template_boundary.py
```

When a consolidation slice moves a category behind its semantic owner, lower
`docs/semantic-template-boundary-audit-baseline.json` in the same commit.
The script intentionally excludes the planned semantic owner files for each
category so the tracked count measures unconsolidated direct call sites rather
than the facade implementation itself.

Completion criteria:

- the audit is easy to run before and after each consolidation slice
- new semantic-side direct calls are visible and intentional
- the audit does not fail on existing debt until the relevant stage owns it

## Stage 2: Centralize Semantic Template Function Use

Purpose:

- make function-template call/conversion/acquisition intent explicit once
- stop repeating request construction and deduction follow-up decisions

Create a semantic-owned facade, likely `semantic_template_function.*`, that
includes `template_api.h` and owns semantic function-template use cases.

The facade should expose request shapes in semantic terms, for example:

- ordinary function-template call candidate
- conversion-function-template candidate
- friend/member function-template candidate
- declaration-only or lookup-only acquisition
- definition-required acquisition

The facade should own:

- `TemplateFunctionDeductionRequest` construction
- `TemplateFunctionInstantiationRequest` construction
- `TemplateFunctionBindingAcquisitionRequest` construction
- default `TemplateInstantiationIntent`
- `include_body` / declaration-only policy
- use-scope and active-owner forwarding
- common deduction failure/drop result formatting where it is not witness-only

The facade should not own:

- template deduction mechanics
- overload ranking
- specialization selection
- candidate viability rules that belong to overload analysis

Initial migration targets:

- duplicate `acquire_function_template_binding(...)` helpers in
  `semantic_conversion.cpp`, `semantic_class_model.cpp`, and
  `semantic_overload.cpp`
- repeated `TemplateFunctionDeductionRequest` construction inside
  `semantic_overload.cpp`
- conversion-function-template acquisition in `semantic_conversion.cpp`
- friend function-template binding in `semantic_class_model.cpp`

Completion criteria:

- direct `TemplateFunctionDeductionRequest` construction is confined to the
  semantic function-template facade and template-owned code
- direct `TemplateFunctionInstantiationRequest` construction outside
  `callsemantic.cpp` is confined to the facade
- changing function-template acquisition intent requires auditing one semantic
  facade, not several semantic modules

Current status:

- function-template acquisition and binding acquisition request construction
  have moved behind `semantic_template_function.*`
- function-template deduction request construction has moved behind
  `semantic_template_function.*`
- explicit function-template argument resolution policy has moved behind
  `semantic_template_function.*`
- call-site explicit function-template argument resolution has moved behind
  `semantic_template_function.*`
- instantiation use-scope overlaying for function-template deduction/acquisition
  has moved behind `semantic_template_function.*`
- function-template partial-ordering placeholder substitution and transformed
  parameter deduction mechanics have moved behind `semantic_template_function.*`
- variable-template source-use argument resolution and instantiation request
  construction have moved behind `semantic_template_variable.*`
- the `function_template_requests` audit ratchet is zero
- semantic files no longer include template-internal headers directly

## Stage 3: Consolidate Function-Template Candidate Collection

Purpose:

- reduce the number of places that know how to gather and prefilter function
  templates
- keep overload ranking in overload code while making collection policy shared

Current duplication appears around:

- direct ordinary function-template lookup
- member function-template lookup
- associated friend function-template lookup
- operator function-template candidate collection
- argument-count prefiltering

The likely owner is `semantic_overload.cpp` or a small
`semantic_template_candidates.*` helper used by overload and expression code.

The consolidated path should answer:

- which lookup sources are active for this use?
- should ADL/associated friends participate?
- is this an operator, member, conversion, or ordinary call candidate search?
- what cheap prefilters are safe before deduction?

It should not decide:

- final overload ranking
- template argument deduction internals
- witness emission policy

Initial migration targets:

- operator template candidate collection in `semantic_expression.cpp`
- function-template arg-count filtering duplicated between expression and
  overload paths
- candidate gathering paths in `semantic_overload.cpp` that differ only by
  lookup source

Completion criteria:

- expression analysis asks for template candidates through one semantic helper
  instead of manually combining direct/member/associated-friend searches
- argument-count prefiltering has one implementation
- new operator-template fixes land in one candidate collection path

Current status:

- unary, binary, and compound-assignment nonmember operator candidate collection now
  use `semantic_overload::collect_nonmember_operator_candidates(...)`
- temporary lookup scope seeding for overloaded operators now goes through
  `semantic_overload::initialize_operator_candidate_scope(...)`
- low-level associated namespace/friend candidate gathering is shared by
  ordinary-call ADL and nonmember operator collection
- ordinary-call ADL temporary-scope assembly and hint rebinding are behind
  `append_ordinary_call_adl_candidates(...)`

## Stage 4: Centralize Dependent Type Recovery Policy

Purpose:

- stop distributing "try structured, then dependent recovery, then text
  fallback" decisions across semantic modules
- keep the template API clean while giving semantic code one typed recovery
  policy

Create a semantic-owned helper, likely `semantic_dependent_type.*`, that wraps
the template type operations in semantic terms.

The helper should expose use-case-level operations:

- resolve alias target after template substitution
- resolve base type with optional reference-only class-template lookup
- resolve builtin trait type argument
- resolve declaration type argument in dependent scope
- resolve member access object type after dependent substitution

It should own:

- whether unresolved lookup is allowed to defer
- whether class-template lookup is reference-only
- whether a failure is a hard semantic error or a recoverable dependent result
- diagnostic context for recovery failures
- common post-resolution normalization

It should not own:

- template-side type substitution
- AST parsing
- semantic expression analysis

Initial migration targets:

- `semantic_builtins.cpp` type-trait argument resolution
- `semantic_class_model.cpp` base/alias/deferred type recovery
- `semantic_lookup.cpp` dependent named-type recovery after lookup
- `semantic_expression.cpp` object/operand dependent type recovery
- `semantic_statement.cpp` local alias dependent recovery
- `semantic_conversion.cpp` conversion-function parameter recovery

Completion criteria:

- direct semantic calls to `template_type_ops::resolve_type_argument_text`,
  `resolve_type_lookup_text`, and `resolve_instantiated_dependent_type` are
  confined to the dependent-type helper and any explicitly justified local
  exception
- recovery failures are classified consistently
- adding a new dependent type recovery rule does not require auditing seven
  semantic modules

Current status:

- direct semantic dependent-type recovery calls, including type-text recovery
  without fragment fallback, are confined to `semantic_dependent_type.*`
- direct semantic type-argument text lookup calls are confined to
  `semantic_dependent_type.*`
- the `dependent_type_ops` audit ratchet is zero

## Stage 5: Centralize Scope Mutation And Template Binding Invalidations

Purpose:

- make semantic scope mutation atomic
- avoid forgetting template binding fingerprint invalidation

Create a semantic-owned mutation helper, likely `semantic_scope_mutation.*`,
that owns:

- adding named types to semantic scopes
- adding or merging function-template bindings where applicable
- invalidating template binding fingerprints
- setting access metadata paired with the binding

The helper should provide small operations rather than one broad mutation API:

- bind class alias type in member scope
- bind local alias type in block/namespace scope
- note scope binding mutation
- merge using-declaration function templates

Initial migration targets:

- `semantic_class_model.cpp` class alias binding sites
- `semantic_declaration.cpp` and `semantic_statement.cpp` mutation bumps
- `semantic_output.cpp` function-scope mutation bumps

Completion criteria:

- no caller needs to remember both "mutate scope" and "bump template binding
  fingerprint"
- class alias binding and access update happen through one helper
- scope mutation audit counts shrink to the helper plus justified exceptions

Current status:

- namespace, block, and statement alias type bindings that previously paired a
  direct `named_types` write with a mutation note now use
  `semantic_scope_mutation::bind_named_type(...)`
- template-bound named-type setup now uses
  `semantic_scope_mutation::ensure_template_named_type(...)` where callers need
  "bind if absent and mark template-bound" semantics
- using-declaration binding for namespaces, values, class/alias/variable
  templates, function sets, and function-template sets now routes through
  `semantic_scope_mutation.*`
- single local value bindings in statement analysis now use
  `semantic_scope_mutation::bind_value(...)`; multi-binding sites still
  intentionally share one invalidation
- function-output parameter alias, pack-size, and value-pack bindings now use
  grouped `semantic_scope_mutation.*` helpers that preserve one invalidation
  per grouped mutation
- namespace alias/definition binding, using-directive insertion, and inline
  namespace member import now route through `semantic_scope_mutation.*`
- anonymous-member value alias injection and template-parameter placeholder
  bindings in class-model helper scopes now route through
  `semantic_scope_mutation.*`
- lambda helper-scope value packs/parameters and range-for helper variables now
  route through grouped `semantic_scope_mutation.*` helpers

## Stage 6: Consolidate Template Output/Readiness Policy

Purpose:

- keep output/materialization decisions in the output policy layer rather than
  spread across output callers
- make `output_requirement_engine` the authoritative semantic output decision
  owner where possible

Today `semantic_output.cpp` still composes many low-level facts:

- template identity
- source-template identity
- explicit-instantiation suppression
- class output readiness
- instantiation argument completeness/dependence
- placeholder checks

Introduce or extend an output-facing semantic view, for example:

```cpp
struct SemanticTemplateOutputDecision {
  bool template_owned;
  bool source_template_owned;
  bool require_definition;
  bool suppress_implicit_definition;
  bool output_blocked_by_placeholders;
  bool declaration_only_template;
};
```

This shape should be computed in one output-policy owner from `template_api`
facts and semantic state.

Initial migration targets:

- duplicated checks between `output_requirement_engine.cpp` and
  `semantic_output.cpp`
- repeated calls to `compute_instantiated_class_output_readiness(...)`
- local explicit-instantiation suppression branches in `semantic_output.cpp`
- local placeholder/readiness combinations in function output decisions

Completion criteria:

- semantic output callers consume one decision object per function/class/value
  instead of recomposing template facts
- output requirement fixes land in the output policy owner
- `semantic_output.cpp` stops being a parallel policy engine for template
  output readiness

Current status:

- low-level class/function template output readiness queries are confined to
  `semantic_template_output_policy.cpp` and `output_requirement_engine.cpp`
- function explicit-instantiation suppression exception checks now go through
  `semantic_template_output_policy.*`
- instantiated class-method emission decisions, tracked-template-body checks,
  definition-acquisition checks, declaration-only template classification, and
  owner-class non-dependent wait checks now go through
  `semantic_template_output_policy.*`
- the `output_readiness_queries` audit ratchet is zero

## Stage 7: Normalize Witness/Lifecycle Touchpoints After Decisions Move

Purpose:

- avoid preserving duplicated decision paths only because they also emit
  witness/lifecycle rows

This stage should run after the relevant semantic decision owner exists.

Rules:

- semantic decision helpers may return enough structured state for witness
  emission, but they should not emit witness rows unless the witness event is
  intrinsic to the decision they own
- lifecycle events that are template-owned should still go through
  `template_api`
- source-use rows should continue moving toward structured semantic source-use
  capture, not renderer recovery or local text scanning

Initial migration targets:

- function-template call source decisions in overload/conversion paths
- class/base/source-use decisions in class model paths
- require/ensure-definition lifecycle events in `semantic_lifetime.cpp`

Current status:

- variable-template source-use location selection now lives in
  `semantic_template_variable.*` with the variable-template instantiation
  request construction
- function-template call source-use event construction, including the synthetic
  `declval<T>` source-use path, is shared through `semantic_template_function.*`;
  overload code still owns candidate drops and ranking decisions
- instantiated class-template source-use event construction is available through
  `semantic_template_class.*`; overload code only decides when an owner-class
  qualifier use should be emitted
- require/ensure-definition lifecycle witness event sequencing for semantic
  lifetime paths now lives in `semantic_template_function.*`

Completion criteria:

- witness output does not force duplicated semantic decision logic to remain
- semantic decision owners return structured facts that witness code can
  consume
- strict witness output remains green after each migration slice

## Execution Order

1. Land the audit ratchet first.
2. Consolidate function-template acquisition and deduction request
   construction.
3. Consolidate candidate collection for function templates.
4. Consolidate dependent type recovery policy.
5. Consolidate scope mutation and template binding invalidation.
6. Consolidate template output/readiness policy.
7. Normalize witness/lifecycle touchpoints that became simpler after the
   decision owners exist.

This order keeps high-churn overload/template work ahead of output cleanup, and
keeps mutation invalidation cleanup before broader output/readiness changes.

## Validation Strategy

For each small slice:

```sh
make -C dev cppgm++
```

For semantic/template boundary slices:

```sh
make test-strict-nobuild STRICT_PAS='pa18 pa19 pa21 pa22' STRICT_SUBTEST_JOBS=8
```

For output/readiness or witness-adjacent slices:

```sh
make test-strict-nobuild STRICT_PAS='pa18 pa19 pa21 pa22' STRICT_SUBTEST_JOBS=8
```

For lower-PA semantic changes that touch shared expression/declaration logic,
add focused owners for the touched PA before the strict run.

Always run the boundary audit after each consolidation stage and record whether
counts dropped, stayed flat, or intentionally added a new exception.

## Completion Criteria

This plan is complete when:

- semantic/template structural boundary audits remain clean outside
  `callsemantic.cpp`
- direct semantic template request construction is confined to semantic-owned
  facades
- function-template acquisition and candidate collection have one semantic
  decision path
- dependent type recovery policy has one semantic decision path
- scope mutation plus template binding invalidation is atomic
- template output/readiness policy is computed by one output owner
- adding a new semantic/template feature no longer requires auditing several
  semantic modules to find which one made the same policy decision differently

## Completion Status

Complete.

The strict semantic/template boundary audit reports zero direct violations for:

- function-template request construction
- dependent type operations
- scope/template mutation operations
- output readiness queries
- template services mentions
- template-internal header includes

The remaining owner-local exceptions are intentional:

- `semantic_template_function.*` and `semantic_template_variable.*` construct
  template API requests on behalf of semantic callers.
- `semantic_dependent_type.*` owns dependent type recovery calls into
  template type operations.
- `semantic_scope_mutation.*` owns scope mutation plus template binding
  fingerprint invalidation.
- `semantic_overload.cpp` owns temporary function/function-template slots for
  overload candidate scopes.
- `semantic_template_output_policy.*` and `output_requirement_engine.cpp` own
  output readiness, suppression, and materialization policy.
- Other direct template fact reads are trace/debug output or single factual
  questions, which the decision ownership rules allow.

## Follow-Up: Semantic Layer Logic Consolidation

The boundary cleanup exposed several higher-level semantic duplication seams.
These are not template-boundary violations; they are maintenance risks because
future semantic fixes still need to land in multiple places.

### Track A: Constructor Selection Consolidation

Problem:

- `semantic_overload.cpp` has separate constructor selection paths for
  pre-analyzed `ExprInfo` arguments and AST-node arguments.
- Both paths independently handle constructor candidate filtering, default
  arguments, constructor-template deduction, ranking, rejection tracking, and
  witness source events.

Goal:

- make one constructor candidate engine authoritative, with the argument source
  as the pluggable part.

Completion criteria:

- candidate filtering/default-argument/template-deduction code is shared
  between `select_constructor_from_exprs(...)` and `select_constructor(...)`
- constructor witness/rejection reporting is produced from the shared candidate
  state
- constructor policy still flows through `constructor_lifecycle_service`

Current status:

- constructor selection uses one shared `ConstructorSelectionState` for
  matches, built candidates, rejection text, witness drops, and conversion
  caching
- non-template constructor enumeration and constructor-template collection are
  shared between AST-node and pre-analyzed-argument entrypoints
- constructor default-argument analysis/conversion and constructor-template
  deduction/acquisition use shared helpers; only source-argument production
  remains entrypoint-specific because AST-node calls need target-aware analysis

### Track B: Declaration And Member Preparation Convergence

Problem:

- class collection, statement collection, and output still prepare declaration
  specifiers/declarators through overlapping local paths.
- output paths still reinterpret member declarations instead of consistently
  consuming prepared semantic declaration facts.

Goal:

- make a prepared declaration/member shape the single source for normalized
  specifiers, declarator shape, method syntax, and owner-sensitive parse facts.

Completion criteria:

- class/member collection and output share the same preparation helpers
- method-like classification is computed once per declaration path
- output-specific behavior remains downstream of semantic preparation, not a
  parallel declaration interpreter

Current status:

- class simple-declaration collection and output use
  `prepare_class_member_declaration_context(...)`
- class member function-definition collection and output use
  `prepare_class_member_function_definition(...)`
- output now consumes the prepared member-definition shape before applying
  emission policy, rather than reparsing member definitions independently

### Track C: Aggregate Initialization Helper Consolidation

Problem:

- aggregate initialization planning, anonymous-storage input-field selection,
  and default field expression construction are duplicated across lifetime,
  class-model, and target-aware expression paths.

Goal:

- move aggregate field/input/default planning behind one semantic helper used by
  constructor actions and target-aware initialization.

Completion criteria:

- aggregate initializer planning is shared where constructor actions and
  target-aware paths build aggregate constructor arguments
- anonymous-storage field selection has one owner
- aggregate constructor parameter-type derivation has one owner

Current status:

- anonymous-storage field selection and aggregate constructor parameter-type
  derivation live in `semantic_class_model.*`
- aggregate constructor source-argument planning now lives in
  `semantic_lifetime::build_aggregate_constructor_source_args(...)` and is
  shared by constructor actions and target-aware aggregate construction

### Track D: Structured Qualified-Name Binding Rewrites

Problem:

- using-declaration and inherited-constructor paths still have local helpers
  that rewrite qualified-name components through string-oriented binding
  rewrites.

Goal:

- replace local duplicated qualified-name rewrite helpers with one semantic
  helper, then remove any remaining text-based component rewrites when the
  needed structured binding state is available.

Completion criteria:

- semantic declaration/class-model paths use one shared helper for bound
  qualified-name handling
- any remaining text rewrite is isolated behind that helper and documented as a
  temporary bridge
- callers no longer duplicate component-by-component rewrite logic

Current status:

- using-declaration and inherited-constructor paths share
  `semantic_dependent_type::rewrite_bound_qualified_name_syntax(...)`
- the remaining component text rewrite is isolated in that helper as a bridge
  until qualified-name binding carries the needed structured type state

Validation remains:

```sh
make -C dev cppgm++
python3 scripts/audit_semantic_template_boundary.py --strict
python3 scripts/audit_text_reparse.py
make test-strict-nobuild STRICT_PAS='pa18 pa19 pa21 pa22' STRICT_SUBTEST_JOBS=8
```
