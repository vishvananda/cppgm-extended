# Callsemantic / Template / Output Convergence Follow-Up Plan

This plan is complete and archived.

The remaining duplicated caller logic that motivated this follow-up has now
been collapsed across the intended semantic service seams:

- `callsemantic.cpp` now routes its scattered template-acquisition paths
  through shared helper entry points instead of hand-building request state
- constructor-action follow-up now flows through
  `constructor_lifecycle_service.cpp` across the former duplicated caller
  paths, and constructor selection now applies `AnalysisPolicy` through one
  shared gate in `callsemantic.cpp`
- `callsemantic.cpp` and `semantic_class_model.cpp` now share prepared-method
  parse context and `MethodSyntaxInfo -> FunctionSemanticFlags` forwarding
  across the previously duplicated out-of-class, friend, special-member, and
  registration/classification paths
- the last residual hand-expanded special-member method-syntax site was moved
  onto the same prepared-method context shape
- `semantic_overload.cpp` now keeps structured option objects at the remaining
  rematerialization helper boundary instead of peeling policy back out into raw
  booleans

The historical rationale below is retained for context.

Several of the major seams from the original cleanup had already landed before
this follow-up:

- [`output_requirement_engine`](../dev/src/output_requirement_engine.cpp)
- [`template_instantiation_coordinator`](../dev/src/template_instantiation_coordinator.cpp)
- [`constructor_lifecycle_service`](../dev/src/constructor_lifecycle_service.cpp)
- [`AnalysisPolicy`](../dev/src/analysis_policy.h)
- [`FunctionSemanticFlags`](../dev/src/semantic_model.h)

So the remaining work is narrower. The goal now is to remove the places where
callsemantic/template/output fixes still have to be repeated across multiple
callers even though the service layer exists.

This follow-up intentionally does **not** try to reopen the completed broad
visibility/output work from
[implemented/VISIBILITY_OUTPUT_EXECUTION_PLAN.md](/Users/vishvananda/cppgm/docs/implemented/VISIBILITY_OUTPUT_EXECUTION_PLAN.md).
The remaining backend closure/export slice is tracked separately in
[visibility-output-followup-plan.md](/Users/vishvananda/cppgm/docs/visibility-output-followup-plan.md).

## Narrow Goal

Reduce the remaining duplicated-fix surface in semantic call analysis by making
the existing services more authoritative in the three places where duplication
still matters:

- template function acquisition
- constructor/callable follow-up after selection
- function/method registration and classification propagation

## What Is Already In Place

The following broad shifts have already happened:

- template acquisition has a shared coordinator
- constructor selection has a shared service
- output requirement decisions already flow through a central engine
- function semantic flags already exist as a canonical stored shape
- analysis policy already exists as a shared object

That means this plan is **not** about inventing new abstraction seams. It is
about finishing the migration onto the seams that already exist.

## Completed Work

The first concrete follow-up slices are now in:

- [`callsemantic.cpp`](../dev/src/callsemantic.cpp) no longer hand-builds
  `template_instantiation_coordinator::FunctionInstantiationRequest` at its
  old scattered acquisition sites; the remaining `callsemantic` template
  acquisition paths now route through one local helper
- [`constructor_lifecycle_service.cpp`](../dev/src/constructor_lifecycle_service.cpp)
  now has prepared constructor-action helpers, and the duplicated
  constructor-action follow-up in
  [`semantic_lifetime.cpp`](../dev/src/semantic_lifetime.cpp) was moved onto
  that service path
- the `semantic_lifetime.cpp` constructor-action helper boundary now takes a
  `ConstructorSelectionOptions` object instead of a raw
  `allow_explicit_constructors` boolean
- [`semantic_class_model.cpp`](../dev/src/semantic_class_model.cpp) and
  [`callsemantic.cpp`](../dev/src/callsemantic.cpp) now share a
  `MethodSyntaxInfo -> FunctionSemanticFlags` helper for the highest-value
  method-definition / special-member registration paths
- the function-template declaration merge/insert path in
  [`callsemantic.cpp`](../dev/src/callsemantic.cpp) now computes one local
  template-classification shape instead of expanding method syntax separately
  for matching, merge, and insert
- the remaining out-of-class member-owner and method-template parse paths in
  [`callsemantic.cpp`](../dev/src/callsemantic.cpp) now share one prepared
  method-parse helper instead of re-deriving filtered specifiers,
  filtered declarators, and cv/ref qualifiers at each owner/template site
- the remaining parse-heavy method collection paths in
  [`semantic_class_model.cpp`](../dev/src/semantic_class_model.cpp) now share
  the same prepared-method context shape across friend functions, class-member
  declarations, dependent class-member declarations, member definitions, and
  special-member registration
- the remaining constructor-action callers outside
  [`semantic_lifetime.cpp`](../dev/src/semantic_lifetime.cpp) now route
  through [`constructor_lifecycle_service.cpp`](../dev/src/constructor_lifecycle_service.cpp):
  `new-expression` uses `prepare_selected_constructor_action(...)`, and
  functional cast uses `prepare_lifecycle_call(...)` plus a side-effect-free
  resolved-call builder
- constructor selection now applies [`AnalysisPolicy`](../dev/src/analysis_policy.h)
  through one shared gate in [`callsemantic.cpp`](../dev/src/callsemantic.cpp)
  across expression, expr-list, and direct-braced-init entry paths instead of
  filtering those options ad hoc at only some call sites
- the functional-cast path in
  [`semantic_overload.cpp`](../dev/src/semantic_overload.cpp) now keeps
  `CallAnalysisOptions` intact through its helper boundary instead of
  collapsing back down to a raw `instantiate_bodies` boolean
- several remaining member-registration callers in
  [`semantic_class_model.cpp`](../dev/src/semantic_class_model.cpp) now route
  through `class_function_options(...)` instead of forwarding each method
  syntax field manually

The last remaining slices were:

- the residual special-member declaration path in
  [`callsemantic.cpp`](../dev/src/callsemantic.cpp), which now uses the same
  prepared-method context as the other declaration paths
- the last rematerialization helper boundary in
  [`semantic_overload.cpp`](../dev/src/semantic_overload.cpp), which now takes
  `ArgumentConversionOptions` instead of a raw `allow_user_defined` boolean

## Remaining Duplication Hotspots

### 1. Template Acquisition Is Partly Centralized, But Not Yet Authoritative

Positive change:

- [`template_instantiation_coordinator`](../dev/src/template_instantiation_coordinator.cpp)
  exists
- shared `acquire_function_template_binding(...)` wrappers already exist in
  [`semantic_overload.cpp`](../dev/src/semantic_overload.cpp) and
  [`semantic_class_model.cpp`](../dev/src/semantic_class_model.cpp)

What still remains:

- direct request construction still happens in multiple places, especially
  [`callsemantic.cpp`](../dev/src/callsemantic.cpp)
- there are still multiple caller-side ways to express:
  - lookup only
  - instantiate without body
  - upgrade to definition
  - require emitted output

That means a template acquisition fix still requires auditing more than one
call shape.

### 2. Constructor Lifecycle Is Centralized For Selection, Not For Action

Positive change:

- [`constructor_lifecycle_service`](../dev/src/constructor_lifecycle_service.cpp)
  already owns much of constructor selection
- it is used from:
  - [`semantic_lifetime.cpp`](../dev/src/semantic_lifetime.cpp)
  - [`semantic_expression.cpp`](../dev/src/semantic_expression.cpp)
  - [`semantic_overload.cpp`](../dev/src/semantic_overload.cpp)

What still remains:

- callers still separately decide when to:
  - require the selected constructor
  - create direct-call nodes
  - seed dependent runtime/special-member follow-up
  - treat explicit-only selection specially

So constructor selection is shared, but constructor action is still partly
reconstructed at call sites.

### 3. Function Classification Is Better, But Recomputed Too Often

Positive change:

- [`FunctionSemanticFlags`](../dev/src/semantic_model.h) exists
- `ClassFunctionOptions` is already an alias to that canonical shape in
  [`semantic_class_model.h`](../dev/src/semantic_class_model.h)

What still remains:

- repeated `analyze_method_syntax(...)` expansion in
  [`callsemantic.cpp`](../dev/src/callsemantic.cpp) and
  [`semantic_class_model.cpp`](../dev/src/semantic_class_model.cpp)
- overlapping registration paths still normalize and forward the same facts in
  slightly different ways

So the stored metadata is better than before, but the compute-and-forward path
still has unnecessary duplication.

### 4. Analysis Policy Exists, But Raw Booleans Still Leak Across Service Boundaries

Positive change:

- [`AnalysisPolicy`](../dev/src/analysis_policy.h) exists
- [`semantic_context.h`](../dev/src/semantic_context.h) exposes
  `current_analysis_policy()`

What still remains:

- service boundaries still thread individual booleans such as:
  - `allow_explicit`
  - `allow_user_defined`
  - `allow_partial_aggregate`
  - `instantiate_bodies`

That keeps call chains brittle and means policy changes still fan out across
many signatures.

## Near-Term Follow-Up Targets

### Target 1. One Authoritative Template Acquisition Entry Path

Do not invent a new template subsystem. Finish the migration onto the existing
coordinator by:

- routing remaining direct request construction through shared helper entry
  points
- making caller intent explicit in one request shape
- removing remaining direct caller combinations of instantiation + upgrade +
  output requirement where the coordinator can already express the intent

### Target 2. Upgrade Constructor Lifecycle From Selector To Action Service

The service should not stop at "here is the chosen constructor." The remaining
cleanup is to make it authoritative for:

- output/emission follow-up for the selected constructor
- dependent special-member/runtime seeding when needed
- explicit-only selection reporting

The caller should still build syntax-specific AST/IR nodes, but it should stop
re-deciding semantic lifecycle policy.

### Target 3. Make Registration And Classification Paths Compute Once

Focus on the compute-and-forward duplication rather than on inventing yet
another metadata type.

The remaining cleanup is:

- compute method syntax/classification once per declaration path
- minimize repeated `analyze_method_syntax(...)` calls
- consolidate overlapping registration helpers where they only differ in owner
  shape, not in semantic content

### Target 4. Stop Threading Raw Policy Booleans Once Inside Semantic Services

At semantic service boundaries, prefer:

- `const AnalysisPolicy &`
- or a small derived policy view

and stop extending new helper APIs with one more boolean each time a policy
rule changes.

## Suggested Migration Order

1. Finish template acquisition convergence first.
   This still has the highest duplicated-fix payoff.
2. Then tighten constructor lifecycle so selection follow-up stops being
   reconstructed at call sites.
3. Then reduce registration/classification recomputation.
4. Finally, collapse the remaining raw-boolean policy threading across service
   boundaries.

## Completion Criteria

This follow-up is complete only when:

- template acquisition intent is expressed through one authoritative service
  path
- constructor selection follow-up no longer has to be re-applied in several
  callers
- function/method classification is computed once and forwarded with minimal
  duplication
- new semantic policy changes no longer require extending multiple helper
  signatures with more raw booleans

## What This Plan No Longer Covers

To keep this plan narrow, the following are owned elsewhere:

- backend closure/export ownership:
  [visibility-output-followup-plan.md](/Users/vishvananda/cppgm/docs/visibility-output-followup-plan.md)
- hosted EH/runtime transition:
  [host-abi-runtime-followup-plan.md](/Users/vishvananda/cppgm/docs/implemented/host-abi-runtime-followup-plan.md)
- long-term LowIR design:
  [lowir-second-tranche-plan.md](/Users/vishvananda/cppgm/docs/implemented/lowir-second-tranche-plan.md)
