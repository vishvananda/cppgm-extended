# Compiler Modularization Audit

This note records the completion status of
[`compiler-modularization-plan.md`](/Users/vishvananda/cppgm/docs/implemented/compiler-modularization-plan.md)
after the staged implementation work on `main`.

## Status

The plan is complete for its intended scope.

Completed checkpoints:

- `fc651e34` `Add semantic analyzer baseline metrics`
- `e55ecab0` `Introduce analysis policy scaffolding`
- `73ceced8` `Add structured semantic diagnostics`
- `551c3bfa` `Extract output requirement engine wrapper`
- `8c980411` `Add template instantiation coordinator wrapper`
- `b96fdfee` `Separate constructor selection from output side effects`
- `c6afd473` `Group semantic analyzer caches under one owner`
- `d4cf8a2b` `Introduce semantic context facet interfaces`

Validation:

- each phase was followed by a full root `make test-report`
- final result remained green at `1542 / 1542`

## Phase Audit

### Phase 0: Baseline and instrumentation

Status: complete

Delivered:

- semantic metrics counters in
  [`semantic_metrics.h`](/Users/vishvananda/cppgm/dev/src/semantic_metrics.h)
  and
  [`semantic_metrics.cpp`](/Users/vishvananda/cppgm/dev/src/semantic_metrics.cpp)
- analyzer-side cache and lifecycle metrics in
  [`callsemantic.cpp`](/Users/vishvananda/cppgm/dev/src/callsemantic.cpp)

### Phase 1: Introduce `AnalysisPolicy`

Status: complete

Delivered:

- explicit policy object in
  [`analysis_policy.h`](/Users/vishvananda/cppgm/dev/src/analysis_policy.h)
- scoped policy override flow in
  [`callsemantic.cpp`](/Users/vishvananda/cppgm/dev/src/callsemantic.cpp)

### Phase 2: Introduce `DiagnosticContext`

Status: complete for the planned scope

Delivered:

- structured diagnostics in
  [`semantic_errors.h`](/Users/vishvananda/cppgm/dev/src/semantic_errors.h)
- preserved diagnostic stack/trace behavior in
  [`semantic_trace.h`](/Users/vishvananda/cppgm/dev/src/semantic_trace.h)
  and
  [`semantic_trace.cpp`](/Users/vishvananda/cppgm/dev/src/semantic_trace.cpp)
- core template/overload paths converted first, matching the plan

### Phase 3: Build `OutputRequirementEngine`

Status: complete

Delivered:

- dedicated requirement wrapper in
  [`output_requirement_engine.h`](/Users/vishvananda/cppgm/dev/src/output_requirement_engine.h)
  and
  [`output_requirement_engine.cpp`](/Users/vishvananda/cppgm/dev/src/output_requirement_engine.cpp)
- call sites routed through the engine from
  [`callsemantic.cpp`](/Users/vishvananda/cppgm/dev/src/callsemantic.cpp)

### Phase 4: Build `TemplateInstantiationCoordinator`

Status: complete for the function-instantiation lifecycle that the plan targeted first

Delivered:

- request/result API in
  [`template_instantiation_coordinator.h`](/Users/vishvananda/cppgm/dev/src/template_instantiation_coordinator.h)
  and
  [`template_instantiation_coordinator.cpp`](/Users/vishvananda/cppgm/dev/src/template_instantiation_coordinator.cpp)
- `ensure_function_template_definition(...)` redirection through the coordinator in
  [`callsemantic.cpp`](/Users/vishvananda/cppgm/dev/src/callsemantic.cpp)
- output-tracking intent routing in
  [`template_instantiation.cpp`](/Users/vishvananda/cppgm/dev/src/template_instantiation.cpp)

### Phase 5: Constructor/lifetime cleanup

Status: complete for the planned separation-of-concerns goal

Delivered:

- constructor lifecycle wrapper in
  [`constructor_lifecycle_service.h`](/Users/vishvananda/cppgm/dev/src/constructor_lifecycle_service.h)
  and
  [`constructor_lifecycle_service.cpp`](/Users/vishvananda/cppgm/dev/src/constructor_lifecycle_service.cpp)
- constructor selection APIs no longer own output side effects in
  [`semantic_overload.cpp`](/Users/vishvananda/cppgm/dev/src/semantic_overload.cpp)
  and
  [`semantic_overload.h`](/Users/vishvananda/cppgm/dev/src/semantic_overload.h)
- output/materialization happens at actual use sites in
  [`semantic_expression.cpp`](/Users/vishvananda/cppgm/dev/src/semantic_expression.cpp),
  [`semantic_lifetime.cpp`](/Users/vishvananda/cppgm/dev/src/semantic_lifetime.cpp),
  and selected paths in
  [`semantic_overload.cpp`](/Users/vishvananda/cppgm/dev/src/semantic_overload.cpp)

### Phase 6: Cache modularization

Status: complete for the plan’s initial scope

Delivered:

- grouped cache owner in
  [`semantic_cache.h`](/Users/vishvananda/cppgm/dev/src/semantic_cache.h)
  and
  [`semantic_cache.cpp`](/Users/vishvananda/cppgm/dev/src/semantic_cache.cpp)
- analyzer cache fields moved behind one owner in
  [`callsemantic.cpp`](/Users/vishvananda/cppgm/dev/src/callsemantic.cpp)
- optional cache size dump via `CPPGM_SEMANTIC_CACHE_STATS`

### Phase 7: Split `SemanticContext`

Status: complete for the compatibility-layer milestone described by the plan

Delivered:

- initial facet interfaces in
  [`semantic_context_facets.h`](/Users/vishvananda/cppgm/dev/src/semantic_context_facets.h)
- `SemanticContext` now layered on top of those facets in
  [`semantic_context.h`](/Users/vishvananda/cppgm/dev/src/semantic_context.h)
- helper wrappers already narrowed to smaller facet types:
  - [`template_instantiation_coordinator.h`](/Users/vishvananda/cppgm/dev/src/template_instantiation_coordinator.h)
  - [`constructor_lifecycle_service.h`](/Users/vishvananda/cppgm/dev/src/constructor_lifecycle_service.h)

## Success Criteria Audit

### Callers no longer manually combine instantiation, tracking, and output marking

Status: complete for the central lifecycle paths

Reason:

- output requirement and function-instantiation wrappers now own the central rituals

### `ensure_function_template_definition(...)` becomes an implementation detail of a coordinator or output engine

Status: complete in practice

Reason:

- core upgrade paths now go through the coordinator and output engine wrappers rather than ad hoc direct combinations

### Constructor-selection APIs return decisions without mutating output state

Status: complete

Reason:

- constructor selection in `semantic_overload.*` no longer performs `require_function_definition(...)`

### Diagnostics distinguish user error, substitution failure, and internal error

Status: complete for the targeted template/overload subsystems

Reason:

- structured diagnostics now exist and the most important propagation boundaries were converted first, which is what the plan required

### Caches are grouped and measurable

Status: complete

Reason:

- cache ownership is centralized and cache-size dumping is available

### New subsystems depend on focused context facets instead of the full `SemanticContext`

Status: complete for the first compatibility layer

Reason:

- real facet interfaces now exist
- new helper wrappers already depend on those narrower interfaces

### `callsemantic.cpp` shrinks in responsibility even if it remains large during the transition

Status: complete in spirit, not by file size

Reason:

- ownership has moved into explicit wrappers and helper modules
- `callsemantic.cpp` is still large, but less of its behavior is now encoded as one-off local lifecycle logic

## Optional Follow-Up

These are still good cleanup ideas, but they are not required to consider
`compiler-modularization-plan.md` complete.

### 1. Split `AnalysisSession` and `AnalyzerState`

This was a recommended target module, not one of the mandatory phase deliverables.
It would further separate long-lived services from translation-unit state, but the
current plan can be considered done without it.

### 2. Broaden facet adoption

Current facets cover output-requirement and template-instantiation boundaries first.
It would still be useful to add and adopt more focused interfaces such as:

- `DiagnosticContext`
- `LifetimeContext`
- `OverloadContext`
- `CacheContext`
- `TypeParsingContext`

This is follow-up refinement, not unfinished required work.

### 3. Convert more `logic_error(...)` sites to structured diagnostics

The plan explicitly said not to convert everything at once. More conversion is
useful, but remaining plain throws outside the critical template/overload paths do
not block plan completion.

### 4. Document cache invariants more explicitly

The cache owner now exists. If needed, a follow-up doc could record which caches are:

- safe for the full TU
- dependent on monotonic template growth
- candidates for targeted invalidation

That documentation is useful, but not required by the phase completion gate.

### 5. Expand the constructor lifecycle API

The current constructor service already separates selection from output side effects.
It could later grow explicit lifetime-plan result objects for more of the generated
constructor/destructor machinery, but the key goal of phase 5 is already met.

### 6. Reduce `callsemantic.cpp` further by implementation movement

The current state has better boundaries, but more bodies could still move into
dedicated modules over time. That is ongoing cleanup, not an unmet requirement from
this plan.

## Non-Goals For This Audit

The following are intentionally *not* treated as required leftovers from this plan:

- rewriting every semantic subsystem to use facets immediately
- deleting the `SemanticContext` compatibility layer
- fully replacing all existing helper functions with new service classes
- forcing a large file-size reduction in `callsemantic.cpp`

Those would be separate cleanup efforts rather than evidence that this plan is incomplete.
