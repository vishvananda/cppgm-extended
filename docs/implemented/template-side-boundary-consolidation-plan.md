# Template-Side Boundary Consolidation Plan

## Goal

Consolidate the template-owned side of the semantic/template boundary while the
integration worktree consolidates semantic-side decision logic.

The boundary rule from
[`template-boundary-audit-checklist.md`](./template-boundary-audit-checklist.md)
still stands:

- non-template semantic and output code enters the template subsystem through
  `template_api.h`
- `TemplateServices` and `SemanticContextTemplateServices` remain
  implementation glue, not semantic-facing API
- template code calls semantic services only through `TemplateTypeSystem`,
  `TemplateRecursiveSemanticGateway`, or a narrow request/result API
- witness and source-location details stay behind `TemplateWitnessContext` or
  explicit template API requests

The remaining template-side problem is not just scattered entry points. Several
template modules still repeat the same internal decisions around argument
normalization, dependent-type recovery, binding, specialization keying,
instantiation acquisition, and witness/lifecycle metadata.

This plan makes those template decisions single-owner without widening the
semantic-facing API and without moving semantic policy into the template layer.

## Worktree Split

This worktree should primarily own template implementation files:

- `dev/src/template_argument_semantics.*`
- `dev/src/template_resolution.*`
- `dev/src/template_instantiation.*`
- `dev/src/template_specialization.*`
- `dev/src/template_selection.*`
- `dev/src/template_binding*`
- `dev/src/template_scope.*`
- `dev/src/template_decl_ast.*`
- `dev/src/template_api.cpp` internals and thin wrappers
- template-owned sections of `dev/src/callsemantic.cpp` after the internal
  owners exist

The integration worktree should primarily own semantic facades and policy files:

- `dev/src/semantic_*.cpp`
- `dev/src/output_requirement_engine.*`
- semantic-side wrappers around `template_api`

Crossing this split is acceptable when required for compilation, but broad
semantic policy changes should stay out of this worktree unless explicitly
coordinated.

## Non-Goals

- Do not create a monolithic `TemplateManager`.
- Do not expose template-internal helper APIs to semantic callers just to move
  code around.
- Do not reopen `TemplateServices` as a semantic-facing interface.
- Do not add a broad recursive semantic lookup callback.
- Do not make arbitrary synthesized text a new boundary contract.
- Do not start by splitting all of `callsemantic.cpp`; use it as a later
  migration target after clearer template owners exist.
- Do not retune the public witness text contract in this plan. This plan may
  reorganize witness plumbing, but not redefine what the assignment harness
  expects.
- Do not use the full PA34 or PA35 gates as routine correctness gates for this
  work. Use focused template suites and selected long-running probes only when
  the slice needs performance evaluation.

## Current Audit Snapshot

The structural boundary is much cleaner than before, but the template side still
has repeated internal ownership patterns.

Observed consolidation targets:

- Service acquisition and dispatch:
  - `SemanticContextTemplateServices` construction appears in `template_api.cpp`,
    thin ops wrappers, and several deeper template modules.
  - Some sites are legitimate semantic-facing wrapper entry points; others are
    internal shortcuts where a caller could pass `TemplateServices &` instead of
    reacquiring the adapter from `SemanticContext &`.
- Template argument normalization:
  - `template_argument_semantics.cpp`, `template_resolution.cpp`, and
    `template_decl_ast.cpp` all participate in turning raw text, structured
    syntax, type handles, source locations, packs, and defaults into resolved
    template arguments.
  - Structured input is often present, but text is still used for cache keys,
    matching, diagnostics, and fallback.
- Dependent type and text recovery:
  - Helpers such as `lookup_text_for_type_argument(...)`,
    `resolve_type_lookup_text(...)`, `resolve_type_argument_text(...)`,
    `resolve_instantiated_dependent_type(...)`, and
    `rewrite_bound_type_names_preserving_dependent_text(...)` are spread across
    template modules.
  - This makes it easy for new work to add another text fallback instead of
    routing through one audited bridge.
- Binding and scope overlays:
  - Template parameter binding, pack expansion, local named-type binding,
    default argument scope handling, and scope fingerprint invalidation are
    distributed across binding, resolution, and instantiation code.
- Specialization matching, selection, and keying:
  - Matching lives largely in `template_specialization.cpp`.
  - Best-specialization selection lives in `template_selection.cpp`.
  - Acquisition and key attachment live in `template_instantiation.cpp`,
    `template_api.cpp`, and template-owned `callsemantic.cpp` sections.
  - Key formatting and canonical argument text still appear in multiple layers.
- Instantiation acquisition and lifecycle:
  - Class, function, variable, and nested-member acquisition paths have similar
    create-or-hit, dependency, explicit-specialization, suppression, and
    readiness decisions, but do not yet share one internal result shape.
- Witness/source-location plumbing:
  - `template_api.cpp`, `template_resolution.cpp`,
    `template_instantiation.cpp`, `template_decl_ast.cpp`, and
    `callsemantic.cpp` all contain source-location prioritization or witness
    support logic.
  - Source metadata sometimes influences computation shape, for example cache
    eligibility, instead of being carried as adjunct metadata.
- Mixed `callsemantic.cpp` exception:
  - The checklist allows in-place template-owned implementation sections there
    for now.
  - That exception should shrink after the template-side owners above exist,
    not expand while semantic-side consolidation proceeds elsewhere.

These are mostly not boundary violations. They are places where template-side
decision ownership is still too distributed.

## Decision Ownership Rules

### Template Side Owns

- parsing and normalizing template arguments after semantic code has identified
  a template use
- binding template parameters to explicit, deduced, and defaulted arguments
- pack expansion and pack-size consistency
- type/value/template-template argument substitution mechanics
- dependent template placeholder and dependent type recovery mechanics
- class, function, variable, and alias specialization matching
- specialization selection and canonical template instantiation keys
- creation, lookup, and lifecycle metadata for template instantiations
- template-owned scope overlays and binding fingerprints
- template witness lifecycle mechanics behind `template_api`

### Semantic Side Owns

- deciding what source construct is being analyzed
- choosing semantic intent:
  - lookup only
  - track instantiation
  - require declaration
  - require definition
  - require definition and output
- overload ranking and semantic viability rules
- declaration, scope, and output policy
- choosing when semantic state must be mutated or invalidated
- the implementation of leaf semantic services exposed through
  `TemplateTypeSystem` and the recursive gateway

### Shared Rules

If template code needs to combine several semantic callbacks to answer a
semantic policy question, the policy belongs on the semantic side behind a
named request or semantic facade.

If multiple template modules repeat the same sequence of `TemplateServices`
calls plus template helper calls, the sequence belongs behind a template-owned
internal facade. Do not expose that facade through `template_api` unless a
semantic caller genuinely needs it.

If a path only works by synthesizing text and reparsing it, first decide whether
the caller already has structured syntax or type handles. If it does, migrate
toward the structured path and keep the text bridge as an explicit temporary
fallback.

## Stage 1: Add Template-Side Audit Ratchets

Purpose:

- make template-side consolidation measurable
- distinguish legitimate wrappers from internal shortcuts
- prevent new hidden semantic callbacks or text fallbacks while the work is in
  progress

Track counts for:

- construction of `SemanticContextTemplateServices`
- template modules that expose `SemanticContext &` overloads instead of
  accepting `TemplateServices &`
- direct uses of semantic callbacks through `services.type_system` and
  `services.recursive_semantic`
- text fallback and rewrite helpers
- canonical argument/key construction
- direct witness/source-location helper use
- direct template metadata reads and writes inside `callsemantic.cpp`

The runnable ratchet is:

```sh
make audit-template-boundary
```

It compares current line-based counts against
`docs/template-side-boundary-audit-baseline.json`. Use
`python3 scripts/audit_template_boundary.py --list-sites` when a count moves.

Initial audit commands:

```sh
rg -n 'SemanticContextTemplateServices|TemplateServices services|TemplateServices bundle' dev/src/template_*.cpp dev/src/template_*.h dev/src/template_api.cpp
rg -n 'services\.(type_system|recursive_semantic)|\.type_system\.|\.recursive_semantic\.' dev/src/template_*.cpp dev/src/template_*.h dev/src/template_api.cpp
rg -n 'resolve_type_lookup_text|resolve_type_argument_text|resolve_instantiated_dependent_type|lookup_text_for_type_argument|rewrite_bound_type|rewrite_bound_template|expand_bound_.*_pack_texts' dev/src/template_*.cpp dev/src/template_*.h dev/src/callsemantic.cpp
rg -n 'canonical_instantiation|instantiation_key|template_instantiation_key|instantiation_arg_texts|template_argument_key' dev/src/template_*.cpp dev/src/template_*.h dev/src/callsemantic.cpp
rg -n 'TemplateWitnessContext|ScopedTemplateArgumentSourceLocations|template_argument_source|normalize_template_witness_source_location|source_location_for_' dev/src/template_*.cpp dev/src/template_*.h dev/src/template_api.cpp dev/src/callsemantic.cpp
```

For `callsemantic.cpp`, review matches by ownership. The existing mixed file is
not automatically a failure, but new template-internal metadata access should
be intentional.

Completion criteria:

- baseline counts are recorded before each consolidation stage
- new broad semantic callbacks and text reparsing paths are visible in review
- the audit can show whether each stage shrank, held, or intentionally moved
  debt

## Stage 2: Normalize Internal Service Acquisition

Purpose:

- make it clear where `SemanticContext` is adapted into template services
- keep deep template implementation functions service-based
- reduce duplicate wrapper paths before larger behavioral moves

Target shape:

- semantic-facing wrappers in `template_api.cpp` and narrow ops shims may
  construct `SemanticContextTemplateServices`
- deeper template implementation functions accept `TemplateServices &`,
  `TemplateTypeSystem &`, or an explicit request object
- `SemanticContext &` overloads in template modules are thin compatibility
  wrappers, not primary implementation entry points

Initial migration targets:

- `template_type_ops.cpp`
- `template_signature_ops.cpp`
- `template_specialization_ops.cpp`
- `template_binding_ops.cpp`
- `template_resolution.cpp` helper overloads that reacquire services
- `template_argument_semantics.cpp` compatibility overloads
- `template_instantiation.cpp` selection/signature helper calls that reacquire
  services internally

Completion criteria:

- internal template code does not reacquire a service bundle when the caller
  already has one
- `SemanticContextTemplateServices` construction is concentrated in API/shim
  boundaries plus explicitly documented exceptions
- moving a template operation behind `template_api` no longer requires copying
  adapter setup boilerplate

## Stage 3: Consolidate Template Argument Normalization

Purpose:

- make one template-side owner responsible for turning explicit syntax, raw
  text, default arguments, packs, and source metadata into resolved template
  arguments
- keep structured arguments authoritative where they exist
- make canonical text a derived diagnostic/key projection, not the primary
  carrier

Introduce or extend an internal request/result shape along these lines:

```cpp
struct TemplateArgumentResolutionInput {
  TemplateServices *services;
  Scope *use_scope;
  const TemplateParameterList *parameters;
  std::vector<TemplateArgument> structured_arguments;
  std::vector<TemplateArgumentSyntax> syntaxes;
  std::vector<std::string> legacy_texts;
  std::vector<std::string> source_locations;
  TemplateArgumentResolutionMode mode;
};

struct TemplateArgumentResolutionResult {
  std::vector<TemplateArgument> resolved_arguments;
  std::vector<std::string> canonical_key_parts;
  bool dependent;
  bool used_legacy_text_fallback;
};
```

The exact type names can differ. The important property is one path for:

- explicit argument resolution
- default argument substitution
- pack expansion
- source-location frame attachment
- canonical key projection
- dependency classification
- cache-key construction

Initial migration targets:

- `resolve_template_arguments(...)` in `template_resolution.cpp`
- `resolve_template_argument(...)` overloads in `template_resolution.cpp` and
  `template_argument_semantics.cpp`
- `template_decl_ast.cpp` parse hooks that immediately convert structured type
  syntax back to lookup text
- call sites that provide both `instantiation_arguments` and
  `instantiation_arg_texts`

Completion criteria:

- a caller with structured `TemplateArgument` / `TemplateIdSyntax` input does
  not need to produce text before calling the hot resolution path
- source locations travel as metadata and do not define the semantic cache key
- canonical text/key parts are produced by one template-owned projection
- legacy text fallback is counted and isolated

## Stage 4: Consolidate Dependent Type And Text Recovery

Purpose:

- stop distributing "structured first, dependent recovery next, text fallback
  last" decisions across template modules
- make any remaining text reparsing bridge explicit and auditable
- avoid reintroducing broad recursive semantic lookup

Create one internal owner for these families:

- `lookup_text_for_type_argument(...)`
- `resolve_type_lookup_text(...)`
- `resolve_type_argument_text(...)`
- `resolve_instantiated_dependent_type(...)`
- `rewrite_bound_type_names_preserving_dependent_text(...)`
- `rewrite_bound_type_packs_in_text(...)`
- `expand_bound_type_pack_texts(...)`
- `expand_bound_expression_pack_texts(...)`

The owner should expose operations in template terms:

- normalize a resolved type for deduction/matching
- substitute a dependent type under a template binding
- resolve a structured dependent qualified-id when possible
- perform a legacy text fallback with a visible debt marker
- format a type argument for diagnostics/witness only

It should not expose:

- a general semantic type lookup callback
- arbitrary text-to-semantic reparsing as a normal path
- witness/source-location lookup as part of type computation

Initial migration targets:

- duplicate type text lookup helpers in `template_resolution.cpp`,
  `template_instantiation.cpp`, and `template_specialization.cpp`
- `template_decl_ast.cpp` branches that call `resolve_type_lookup_text(...)`
  directly
- local text rewrite chains in template-owned `callsemantic.cpp` sections

Completion criteria:

- text fallback audit matches point to one owner plus documented exceptions
- structured lookup failures do not silently become broad semantic lookup
- adding a new dependent type case requires editing the recovery owner, not
  several template modules

## Stage 5: Consolidate Binding And Scope Overlay Mechanics

Purpose:

- make template parameter binding and scope overlay behavior atomic
- prevent pack/default/local-type binding updates from drifting apart
- keep scope fingerprint invalidation paired with the mutation that requires it

The consolidated owner should cover:

- binding template parameters to resolved arguments
- binding and expanding type/value/expression packs
- applying default template arguments in the correct declaring scope
- binding argument-local named types into instantiation scopes
- preparing member scopes needed for dependent lookup
- bumping or reading template binding fingerprints

Likely implementation direction:

- extend `template_binding.*` / `template_binding_ops.*` for binding mechanics
- keep `template_scope.*` as the scope fingerprint/cache owner
- have instantiation and resolution code call one binding operation instead of
  mixing direct map mutation with fingerprint/cache decisions

Initial migration targets:

- `bind_argument_local_named_types(...)` style helpers in
  `template_instantiation.cpp`
- pack/default binding paths in `template_resolution.cpp`
- binding fingerprint calls near scope mutation
- compatibility wrappers in `template_binding_ops.cpp`

Completion criteria:

- no caller needs to remember both "update template binding" and "invalidate
  template scope/fingerprint"
- pack expansion errors and default argument scope decisions are reported from
  one binding path
- binding audit counts shrink to the binding/scope owners plus justified
  wrappers

## Stage 6: Consolidate Specialization Matching, Selection, And Keys

Purpose:

- make class/function/variable specialization decisions produce one structured
  result shape
- centralize canonical key construction
- keep selection mechanics template-owned while semantic overload policy stays
  semantic-owned

The consolidated result should answer:

- which primary or partial specialization was selected
- which canonical argument list/key was used
- whether the result is dependent or blocked
- whether explicit specialization or suppression metadata applies
- which bindings were deduced, defaulted, or transformed during matching
- which diagnostic/witness facts are attached to the decision

Initial migration targets:

- matching and partial ordering in `template_specialization.cpp`
- best partial selection in `template_selection.cpp`
- class/variable selection wrappers in `template_api.cpp`
- class/function/variable acquisition key construction in
  `template_instantiation.cpp`
- owner-prefixed key matching and canonical argument formatting in
  template-owned `callsemantic.cpp` sections

Completion criteria:

- there is one template-owned builder for canonical instantiation key parts
- class and variable partial selection use the same selection-result shape
  where practical
- function-template specialization matching shares key/binding projection logic
  with acquisition instead of reconstructing it locally
- semantic callers still see only `template_api` request/result types

## Stage 7: Consolidate Instantiation Acquisition And Lifecycle

Purpose:

- unify create-or-hit behavior across class, function, variable, and
  nested-member template instantiations
- keep lifecycle facts attached to the template decision that created them
- reduce direct metadata mutation in the mixed `callsemantic.cpp` exception

The owner, likely `template_instantiation.*`, should expose internal request
shapes that include:

- source template identity
- selected specialization result
- resolved/canonical arguments
- instantiation intent
- use scope and active owner
- definition requirement
- explicit instantiation/specialization cause
- output/materialization readiness metadata

The result should include:

- selected or created semantic entity
- created-new vs cache-hit state
- canonical key and argument projections
- dependency/readiness status
- lifecycle/witness event payload, if any
- any semantic materialization work that must be requested by the caller

Initial migration targets:

- class instantiation acquisition in `template_instantiation.cpp`
- function-template binding acquisition and signature creation paths
- variable template acquisition paths
- nested member materialization paths
- direct `source_template`, `template_instantiation_key`,
  `instantiation_arguments`, and suppression-flag updates in template-owned
  `callsemantic.cpp` sections

Completion criteria:

- instantiation metadata is written by the instantiation owner
- create-or-hit handling has one policy per entity family
- lifecycle events are emitted or returned by the owner that made the lifecycle
  decision
- `callsemantic.cpp` no longer has to match keys before deciding whether an
  instantiation already exists

## Stage 8: Normalize Template Witness And Source-Location Plumbing

Purpose:

- keep witness/source metadata from forcing duplicate template decisions
- make source locations adjunct data attached to decisions rather than inputs
  that change computation paths
- prepare for future source-use-table work without doing that semantic-side
  refactor here

Centralize template-side handling of:

- `ScopedTemplateArgumentSourceLocations`
- template argument source-location frames
- source-location normalization
- declaration/use/source-template location precedence
- lifecycle event source locations
- diagnostic/witness formatting of canonical type arguments

Initial migration targets:

- source-location helper clusters in `template_api.cpp`
- direct `TemplateWitnessContext` helper usage in `template_resolution.cpp`
  and `template_decl_ast.cpp`
- witness location decisions inside instantiation acquisition
- template-owned `callsemantic.cpp` helper blocks that choose lifecycle/use
  locations

Completion criteria:

- witness/source metadata does not disable semantic template caches unless the
  cache key genuinely depends on the captured data
- lifecycle witness rows consume template decision records instead of
  recomputing selection/acquisition facts
- source-location fallback logic has one owner per event family

## Stage 9: Shrink The `callsemantic.cpp` Template Exception

Purpose:

- move template-owned implementation sections out only after their destination
  owners exist
- leave semantic orchestration in `callsemantic.cpp` using `template_api`
- reduce the temporary exception from the audit checklist

Migration order:

1. key matching and canonical argument helpers
2. function-template instantiation/binding helpers
3. class and variable template acquisition helpers
4. nested-member materialization helpers
5. lifecycle/source-location helper blocks

Rules:

- classify each block as semantic orchestration or template-owned mechanics
  before moving it
- move template-owned mechanics into `template_*` files with `TemplateServices`
  inputs where possible
- keep semantic orchestration on the public `template_api` boundary
- do not move overload ranking, declaration policy, or output policy into the
  template layer as part of this cleanup

Completion criteria:

- the `callsemantic.cpp` audit exception is smaller and more clearly scoped
- new code does not add direct template metadata reads to semantic
  orchestration regions
- template-owned code in `callsemantic.cpp` becomes a migration queue rather
  than an expanding implementation area

## Execution Order

1. Land the template-side audit ratchet.
2. Normalize service acquisition so later moves do not copy adapter boilerplate.
3. Consolidate argument normalization and canonical key projection.
4. Consolidate dependent type/text recovery.
5. Consolidate binding and scope overlay mechanics.
6. Consolidate specialization matching, selection, and keys.
7. Consolidate instantiation acquisition and lifecycle.
8. Normalize witness/source-location plumbing after decisions have owners.
9. Shrink the `callsemantic.cpp` template exception in small family-based
   slices.

This order keeps high-churn structured argument and selection work inside the
template layer before touching semantic orchestration, and it avoids competing
with integration's semantic-side consolidation.

## Validation Strategy

For every small slice:

```sh
make -C dev cppgm++
```

For template/semantic boundary slices:

```sh
make test-strict-nobuild STRICT_PAS='pa18 pa19 pa21 pa22' STRICT_SUBTEST_JOBS=8
```

For parser, declaration AST, or template argument syntax changes, add:

```sh
make test-strict-nobuild STRICT_PAS='pa10 pa11 pa12 pa14 pa15 pa16' STRICT_SUBTEST_JOBS=8
```

For specialization, instantiation, and lifecycle changes, prefer:

```sh
make test-strict-nobuild STRICT_PAS='pa14 pa15 pa16 pa18 pa19 pa21 pa22' STRICT_SUBTEST_JOBS=8
```

For witness/source-location-adjacent slices, run the strict template suites and
record any drift before continuing. Do not hide drift behind larger refactors;
the point of this work is to reduce duplicated decision paths while keeping the
observable template behavior stable.

For performance-sensitive slices, record before/after hotspot counters on the
current focused gates and a few selected long PA34 probes. Do not use the full
PA34 or PA35 gates as routine correctness gates.

After each stage:

- rerun the template-side audit commands
- record whether each count dropped, stayed flat, or moved to a documented
  owner
- record any temporary exceptions with the stage that should remove them
- avoid advancing to a broad migration stage while the previous stage has
  unexplained witness drift

## Completion Criteria

This plan is complete when:

- template-side service acquisition is concentrated at API/shim boundaries
- structured template arguments are the hot-path authority, with text as a
  derived projection or isolated fallback
- dependent type/text recovery has one audited owner
- template binding and scope fingerprint invalidation are paired atomically
- specialization matching, selection, and key construction have one
  template-owned decision path per family
- instantiation metadata and lifecycle decisions are written by instantiation
  owners
- witness/source-location plumbing consumes template decisions instead of
  recomputing them
- the `callsemantic.cpp` mixed-ownership exception shrinks instead of growing
- semantic-facing callers still cross the boundary through `template_api`

## Completion Status

Complete for the boundary-consolidation tranche.

The completed implementation leaves semantic callers on the public
`template_api` surface, keeps `TemplateServices` as template-side glue, and
narrows semantic callbacks to the explicit type-system / recursive-gateway
service interfaces. The recent final cleanup removed broad declaration
collector callback bags in favor of focused parsing, source, out-of-class
member, and function-template declaration services, so adding a template
declaration feature no longer requires threading another ad hoc callback
through the collector.

Remaining template work should be tracked as concrete correctness or PA34/PA35
convergence failures rather than as this broad boundary-consolidation plan.
