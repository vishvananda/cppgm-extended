# Compiler Modularization Plan

## Purpose

This plan describes how to make the semantic and template-analysis pipeline more contained, more consistent, and easier to evolve without continuing to accumulate cross-file lifecycle bugs.

The immediate goal is not a large rewrite. The goal is to introduce explicit module boundaries so that:

- instantiation work happens through a single lifecycle entry point,
- output/visibility requirements are recorded consistently,
- diagnostics are classified instead of inferred from thrown strings,
- mode/suppression decisions are passed explicitly instead of as scattered booleans and depth counters,
- constructor/destructor selection is separated from output side effects,
- caches become inspectable and invalidation-aware,
- the `Analyzer` in `callsemantic.cpp` stops acting as the only place where every concern meets.

## Current Structural Problems

### 1. Template instantiation is a distributed lifecycle

The current lifecycle is spread across:

- [`template_scope.cpp`](/Users/vishvananda/cppgm/dev/src/template_scope.cpp)
- [`template_resolution.cpp`](/Users/vishvananda/cppgm/dev/src/template_resolution.cpp)
- [`template_instantiation.cpp`](/Users/vishvananda/cppgm/dev/src/template_instantiation.cpp)
- [`template_specialization.cpp`](/Users/vishvananda/cppgm/dev/src/template_specialization.cpp)
- [`template_selection.cpp`](/Users/vishvananda/cppgm/dev/src/template_selection.cpp)
- [`callsemantic.cpp`](/Users/vishvananda/cppgm/dev/src/callsemantic.cpp)
- plus downstream output consumers in [`semantic_output.cpp`](/Users/vishvananda/cppgm/dev/src/semantic_output.cpp)

Representative evidence:

- `instantiate_function_template(...)` is exposed directly through [`semantic_context.h`](/Users/vishvananda/cppgm/dev/src/semantic_context.h) and called from many semantic sites.
- `require_function_definition(...)` in [`callsemantic.cpp`](/Users/vishvananda/cppgm/dev/src/callsemantic.cpp) also upgrades template definitions via `ensure_function_template_definition(...)`.
- `note_instantiated_function_output(...)`, `track_instantiated_class(...)`, and `require_function_definition(...)` overlap semantically but are separate calls that callers must remember to combine correctly.

This is the biggest source of partially-completed work: selection, instantiation, definition upgrade, and output registration are not one operation.

### 2. `Analyzer` is a god object with mixed responsibilities

The concrete analyzer in [`callsemantic.cpp`](/Users/vishvananda/cppgm/dev/src/callsemantic.cpp) is currently carrying:

- semantic model ownership,
- template-instantiation bookkeeping,
- output-closure state,
- policy flags,
- runtime ABI toggles,
- caches,
- diagnostic source-location helpers,
- synthetic entity allocation,
- many service methods forwarded through [`semantic_context.h`](/Users/vishvananda/cppgm/dev/src/semantic_context.h).

This concentrates power, but it also means module boundaries are mostly social rather than enforced.

### 3. Diagnostics are string throws, not a system

Current state:

- [`semantic_errors.h`](/Users/vishvananda/cppgm/dev/src/semantic_errors.h) defines only `TemplateSubstitutionFailure`.
- `callsemantic.cpp` alone contains roughly `140` direct throws.
- other semantic files add many more:
  - `semantic_expression.cpp`: about `111`
  - `semantic_overload.cpp`: about `77`
  - `semantic_class_model.cpp`: about `77`
  - `semantic_statement.cpp`: about `72`
  - `semantic_lifetime.cpp`: about `53`

This makes it difficult to answer basic questions:

- is this a user error, SFINAE failure, or internal compiler bug?
- should this propagate, be collected, or be downgraded?
- what source location should be attached?

### 4. Mode/suppression state is implicit and fragile

Relevant current state in [`callsemantic.cpp`](/Users/vishvananda/cppgm/dev/src/callsemantic.cpp):

- `suppress_function_body_instantiation_depth`
- `ScopedFunctionBodyInstantiationSuppression`
- `expand_output_closure_`
- `use_extended_virtual_abi`

And separate boolean threading in interfaces such as:

- `instantiate_bodies`
- `mark_output_required`
- `materialize_user_defined_output`
- `include_body`

These decisions are part of one policy space, but they are represented as independent parameters and counters.

### 5. Constructor/lifetime work still mixes selection with side effects

Constructor/destructor behavior crosses:

- [`semantic_overload.cpp`](/Users/vishvananda/cppgm/dev/src/semantic_overload.cpp)
- [`semantic_lifetime.cpp`](/Users/vishvananda/cppgm/dev/src/semantic_lifetime.cpp)
- [`semantic_conversion.cpp`](/Users/vishvananda/cppgm/dev/src/semantic_conversion.cpp)
- [`semantic_expression.cpp`](/Users/vishvananda/cppgm/dev/src/semantic_expression.cpp)
- [`semantic_statement.cpp`](/Users/vishvananda/cppgm/dev/src/semantic_statement.cpp)
- [`callsemantic.cpp`](/Users/vishvananda/cppgm/dev/src/callsemantic.cpp)

The useful partial abstraction already exists:

- selection functions
- lifetime-analysis functions
- constructor-conversion expression creation

But the interfaces still mix:

- overload selection,
- argument conversion,
- body instantiation choice,
- output marking.

### 6. Caches are real subsystems, but unmanaged

The main semantic analyzer currently owns many mutable caches directly in [`callsemantic.cpp`](/Users/vishvananda/cppgm/dev/src/callsemantic.cpp), including:

- `identifier_token_cache`
- `tokenized_text_cache`
- `qualified_name_cache`
- `unscoped_template_id_cache`
- `template_placeholder_mentions_cache`
- `non_namespace_binding_mentions_cache`
- `dependent_non_namespace_binding_mentions_cache`
- `qualified_type_lookup_cache`
- `expression_fragment_cache`
- `type_fragment_cache`
- `dependent_type_resolution_cache`
- plus `captured_local_scope_cache`

The problem is not just size. The problem is lack of cache policy:

- no grouped ownership,
- no invalidation model,
- no instrumentation,
- no phase guarantees,
- no distinction between caches safe for the whole translation unit and caches that depend on template-instantiation growth.

## Design Principles

Any cleanup should follow these rules:

1. No big-bang rewrite.
   Each phase should leave the compiler shippable.

2. Separate decision from side effect.
   Selection functions should return chosen entities; registration/output code should be separate.

3. Push lifecycle into explicit request/result APIs.
   Callers should not have to remember a 3-step or 4-step ritual.

4. Narrow context interfaces over time.
   We should move away from a single giant `SemanticContext` toward focused service interfaces.

5. Prefer wrappers before structural moves.
   First centralize existing behavior behind one API; then move implementation.

6. Introduce observability before changing semantics.
   Caches and diagnostics need instrumentation before aggressive refactors.

## Recommended Target Modules

## 1. `AnalysisSession` and `AnalyzerState`

Create an internal session/state split:

- `AnalysisSession`
  - owns long-lived services and policy
  - owns diagnostic sink
  - owns cache manager
  - exposes focused service facades

- `AnalyzerState`
  - owns translation-unit mutable semantic state
  - semantic graph ownership
  - instantiated-function/class tracking
  - required-definition queues
  - synthetic entity registries

This separates:

- "what services exist"
- from
- "what this analysis run has accumulated"

That makes it easier to isolate output closure, template closure, and caches.

## 2. `TemplateInstantiationCoordinator`

Introduce one coordinator for the full function/class instantiation lifecycle.

Suggested API shape:

```cpp
struct FunctionInstantiationRequest {
  FunctionTemplateDecl& decl;
  std::vector<TemplateArgument> arguments;
  Scope* use_scope = nullptr;
  const CppAstNode* body_override = nullptr;
  const std::map<std::string, std::size_t>* pack_sizes = nullptr;
  bool explicit_specialization = false;
  bool include_body = true;
  bool prefer_overload_suffix = false;
  InstantiationIntent intent = InstantiationIntent::LookupOnly;
};

struct FunctionInstantiationResult {
  FunctionBinding* binding = nullptr;
  bool created_new_binding = false;
  bool definition_materialized = false;
  bool output_tracked = false;
  bool definition_required = false;
};
```

The key change is not the exact type names. The key change is that callers stop doing this manually:

- instantiate
- maybe track
- maybe upgrade definition
- maybe require output
- maybe track owning class

Instead they call one coordinator entry point with an intent.

Recommended intents:

- `LookupOnly`
- `TrackInstantiation`
- `RequireDefinition`
- `RequireDefinitionAndExport`

This coordinator should internally delegate to the current implementation files:

- scope binding
- argument resolution
- specialization matching
- body instantiation
- output tracking

But callers should only see the coordinator.

## 3. `OutputRequirementEngine`

`require_function_definition(...)` is a good start, but the plan should finish that direction.

Create an explicit engine responsible for:

- recording declaration/definition/export requirements,
- upgrading bindings when a required definition is not yet materialized,
- maintaining queues/sets for required definitions,
- late-required class method handling,
- output-closure expansion bookkeeping.

This logic currently lives mostly in [`callsemantic.cpp`](/Users/vishvananda/cppgm/dev/src/callsemantic.cpp) plus consumers in [`semantic_output.cpp`](/Users/vishvananda/cppgm/dev/src/semantic_output.cpp).

The important separation is:

- template instantiation decides what binding exists,
- output engine decides what must be emitted,
- semantic output phase consumes the already-computed requirement set.

## 4. `DiagnosticContext`

Introduce a real diagnostic object and sink, even if throws remain the transport for a while.

Suggested minimal model:

```cpp
enum class DiagnosticKind {
  UserError,
  SubstitutionFailure,
  InternalError,
  Warning,
  Note,
};

struct Diagnostic {
  DiagnosticKind kind;
  std::string message;
  std::string location;
  std::string phase;
};
```

Initial goals:

- central helper to construct diagnostics with `source_location_for_node(...)`,
- explicit conversion helpers:
  - `throw_user_error(...)`
  - `throw_internal_error(...)`
  - `throw_substitution_failure(...)`
- keep `TemplateSubstitutionFailure`, but make it carry a structured `Diagnostic`

This does not require replacing every `throw logic_error(...)` at once.

First convert the most structurally important areas:

- template resolution
- template instantiation
- overload resolution
- output requirement/definition upgrade

## 5. `AnalysisPolicy`

Bundle suppression and mode flags into one immutable policy object, with scoped overrides.

Candidate fields:

- `instantiate_function_bodies`
- `expand_output_closure`
- `materialize_direct_call_output`
- `materialize_user_defined_output`
- `use_extended_virtual_abi`
- `allow_user_defined_conversions`

Then replace counter/boolean threading with:

- `AnalysisPolicy`
- `ScopedAnalysisPolicyOverride`

This should subsume:

- `suppress_function_body_instantiation_depth`
- `instantiate_bodies`
- `mark_output_required`
- `materialize_user_defined_output`

Not every field has to be moved immediately, but the policy container should exist early.

## 6. `ConstructorLifecycleService`

Create a service layer that makes constructor/destructor behavior explicit in three steps:

1. select candidate and converted args
2. choose lifetime actions
3. separately request output/materialization if needed

Suggested result types:

```cpp
struct ConstructorSelectionResult {
  FunctionBinding* ctor = nullptr;
  std::vector<ExprInfo> converted_args;
  std::vector<ConversionRank> ranks;
};

struct LifetimeActionPlan {
  std::vector<CallSemNode> actions;
  std::vector<FunctionBinding*> required_special_members;
};
```

The point is to stop interfaces like:

- `select_constructor_from_exprs(..., bool mark_output_required)`

Selection should not also decide emission side effects.

## 7. `SemanticCache`

Group caches behind a dedicated owner.

At minimum, split into:

- text/token caches
- parsed-fragment caches
- dependent-name/type caches
- scope-capture caches

Then add:

- stats counters
- optional debug dump
- targeted invalidation hooks

Important note: do **not** start with aggressive invalidation logic. Start with:

- inventory,
- ownership,
- instrumentation,
- documented phase assumptions.

Only after that should we decide which caches need invalidation versus which are safe because the semantic world only grows monotonically for a given session.

## 8. Break up `SemanticContext`

[`semantic_context.h`](/Users/vishvananda/cppgm/dev/src/semantic_context.h) is currently too broad to preserve clean module boundaries.

After the service objects above exist, split the interface into narrower facets such as:

- `TypeParsingContext`
- `TemplateInstantiationContext`
- `OutputRequirementContext`
- `OverloadContext`
- `LifetimeContext`
- `DiagnosticContext`
- `CacheContext`

The concrete analyzer can still implement all of them initially.

The payoff is that helper modules stop depending on the entire compiler surface area.

## Recommended Order of Attack

## Phase 0: Baseline and instrumentation

Goal:

- add measurement and tracing before structural change.

Tasks:

- add counters for template instantiations, definition upgrades, and required-definition upgrades
- add cache hit/miss counters
- add optional diagnostic phase tags in traces

Deliverable:

- no behavior change
- clearer profiling and regression investigation

## Phase 1: Introduce `AnalysisPolicy`

Goal:

- stop threading raw booleans and depth counters as independent decisions.

Tasks:

- define `AnalysisPolicy`
- add scoped override helper
- migrate `suppress_function_body_instantiation_depth`
- migrate `materialize_user_defined_output`
- migrate `instantiate_bodies` in the most central overload/call paths

Deliverable:

- function-call/constructor code stops depending on multiple independent mode parameters

## Phase 2: Introduce `DiagnosticContext`

Goal:

- classify semantic failures and attach locations.

Tasks:

- add `Diagnostic` and helper constructors
- update `TemplateSubstitutionFailure` to carry structured data
- convert template resolution/instantiation/overload code first
- keep old `logic_error` throws elsewhere temporarily

Deliverable:

- SFINAE and fatal errors are distinguishable without string inspection

## Phase 3: Build `OutputRequirementEngine`

Goal:

- make output/definition/export requirements one subsystem.

Tasks:

- move `require_function_definition(...)` related state/queues behind a dedicated engine
- move `note_instantiated_function_output(...)`
- move late-required class method tracking
- expose a small API:
  - `require_declaration(...)`
  - `require_definition(...)`
  - `require_export(...)`
  - `note_instantiation(...)`

Deliverable:

- output closure and template definition upgrade no longer live as scattered side effects

## Phase 4: Build `TemplateInstantiationCoordinator`

Goal:

- callers request an instantiation lifecycle by intent, not by manually combining steps.

Tasks:

- add request/result types
- wrap current free-function instantiation path first
- then wrap class instantiation and specialization matching
- redirect `ensure_function_template_definition(...)` through the coordinator instead of duplicating upgrade logic

Deliverable:

- one place owns the contract for “instantiate, register, maybe require definition”

## Phase 5: Constructor/lifetime cleanup

Goal:

- separate overload selection from lifetime planning and from output side effects.

Tasks:

- create `ConstructorSelectionResult`
- remove `mark_output_required` from selection interfaces
- make lifetime analysis return required special members explicitly
- call output engine from the caller or a coordinating service, not from the selector

Deliverable:

- constructor behavior is easier to test and reason about independently

## Phase 6: Cache modularization

Goal:

- make caches explicit, measurable, and safe to evolve.

Tasks:

- move cache members out of the main analyzer body into a `SemanticCache`
- add per-cache stats
- add invalidation hooks only where evidence justifies them
- document which caches are safe across the whole TU and which depend on scope/template growth

Deliverable:

- cache behavior becomes inspectable instead of implicit

## Phase 7: Split `SemanticContext`

Goal:

- enforce module boundaries in the type system.

Tasks:

- define narrower interfaces
- switch helper modules to the smallest required interface
- keep a compatibility layer while callers migrate

Deliverable:

- future cleanup work stops broadening the god interface

## What Not To Do

1. Do not rewrite template machinery and diagnostics simultaneously.
   The failure modes will be impossible to separate.

2. Do not start by deleting existing helper functions.
   Wrap them first; move implementation later.

3. Do not let caches invent their own invalidation rules independently.
   Centralize ownership before changing semantics.

4. Do not keep `mark_output_required`-style booleans on selection APIs long term.
   Those are exactly the cross-cutting decisions this plan is trying to remove.

5. Do not move all `throw logic_error(...)` sites at once.
   Convert the template and overload subsystems first, where classification matters most.

## Success Criteria

This cleanup is succeeding when the following become true:

- callers no longer manually combine instantiation, tracking, and output marking,
- `ensure_function_template_definition(...)` becomes an implementation detail of a coordinator or output engine,
- constructor-selection APIs return decisions without mutating output state,
- diagnostics distinguish user error, substitution failure, and internal error,
- caches are grouped and measurable,
- new subsystems depend on focused context facets instead of the full `SemanticContext`,
- `callsemantic.cpp` shrinks in responsibility even if it remains large during the transition.

## Recommended First Concrete Refactor

The best first real refactor is:

1. introduce `AnalysisPolicy`,
2. introduce a small `OutputRequirementEngine` wrapper around the current `require_function_definition(...)` behavior,
3. then introduce `TemplateInstantiationCoordinator` on top of the existing free functions.

That sequence attacks the highest-value bug class first:

- partially-completed instantiation/output lifecycle work.

