# Visibility / Output Execution Plan

Status note:

- The main semantic/output parts of this plan landed:
  - centralized requirement marking through `require_function_definition(...)`
    and `output_requirement_engine`
  - explicit requirement kinds (`ORK_*`)
  - required-definition closure expansion
  - required-definition closure validation
- The remaining work is narrower than this original broad plan:
  the backend still derives part of its LowIR/object export behavior from local
  reachability/closure heuristics instead of purely from the semantic required-
  definition closure.

The remaining slice now lives in
[visibility-output-followup-plan.md](/Users/vishvananda/cppgm/docs/visibility-output-followup-plan.md).

## Goal

Stop the recurring late host-link failures caused by missing emitted definitions by turning
visibility/output selection into a single explicit maintainer-owned pipeline.

This plan is intentionally incremental. It is designed to:

1. stop the current bootstrap/link failures quickly
2. reduce future whack-a-mole fixes in semantic marking sites
3. provide a clean path to the full required-symbol closure model

## Problem Statement

The current failures are not primarily object-backend bugs. They are output-closure bugs.

The repeated pattern from recent bootstrap cycles is:

1. semantic analysis discovers a function/class method/template instantiation that is real
2. one site marks it as needed, but another required side effect is skipped
3. call-sem output never emits a usable definition
4. LowIR/object generation proceeds with partial semantic visibility
5. the problem appears late as an unresolved `__cppgm_*` symbol at host link

Recent fixes have patched slices of this:

- template definition upgrades
- late-required class methods
- polymorphic stdlib suppression

Those were valid fixes, but they are all symptoms of the same structural issue:

- requirement marking is scattered
- emission still discovers requirements opportunistically
- there is no hard validator between semantic output and link

## Current Working Theory

There are three separate responsibilities that should be unified:

1. requirement marking
2. requirement closure / upgrade
3. emission / export validation

Today those responsibilities are split across:

- [callsemantic.cpp](/Users/vishvananda/cppgm/dev/src/callsemantic.cpp)
- [semantic_expression.cpp](/Users/vishvananda/cppgm/dev/src/semantic_expression.cpp)
- [semantic_overload.cpp](/Users/vishvananda/cppgm/dev/src/semantic_overload.cpp)
- [semantic_output.cpp](/Users/vishvananda/cppgm/dev/src/semantic_output.cpp)
- [lowirgensemantic.cpp](/Users/vishvananda/cppgm/dev/src/lowirgensemantic.cpp)
- [lowir_object_backend.cpp](/Users/vishvananda/cppgm/dev/src/lowir_object_backend.cpp)

The first implementation priority is to fix the first two without trying to rewrite the entire
backend in one step.

## Recommendation

The right execution order is:

1. centralize requirement marking now
2. add a call-sem closure validator now
3. move to an explicit requirement-closure pass
4. only then split requirement kinds and simplify backend reachability

This keeps the near-term work aligned with the actual failures we are seeing in bootstrap.

## Phase 1: Centralize Requirement Marking

### Objective

Remove all direct ad hoc writes of:

- `binding.output_required = true`
- paired `note_late_required_class_method(...)`
- paired `track_instantiated_function(...)`

and replace them with one shared API.

### API

Add new `SemanticContext` helpers:

```cpp
enum class OutputReason
{
  DirectCall,
  ConstructorUse,
  FunctionIdUse,
  NewExpression,
  VTableSlot,
  TemplateUpgrade,
  DirectCallNode,
  SyntheticDependency,
  RuntimeDependency
};

void require_function_definition(semantic_model::FunctionBinding * binding,
                                 OutputReason reason,
                                 bool enabled = true);
```

Initial behavior:

1. return if `!enabled`
2. return if `!binding`
3. return if `binding->is_deleted`
4. set `binding->output_required = true`
5. call `note_late_required_class_method(binding)`
6. if `binding->source_template`, call `track_instantiated_function(binding)`
7. optionally record the reason behind a trace flag

Do not add the full requirement bitset yet. Keep this step mechanical and low risk.

### Replace Sites

Route all direct function-definition requirement writes through this API in:

- [callsemantic.cpp](/Users/vishvananda/cppgm/dev/src/callsemantic.cpp)
- [semantic_expression.cpp](/Users/vishvananda/cppgm/dev/src/semantic_expression.cpp)
- [semantic_overload.cpp](/Users/vishvananda/cppgm/dev/src/semantic_overload.cpp)
- [semantic_output.cpp](/Users/vishvananda/cppgm/dev/src/semantic_output.cpp)

Known categories to cover:

- direct call resolution
- constructor selection
- new-expression constructor use
- function-id expression use
- template-upgrade replacement
- vtable slot emission seeds
- synthesized/late-required class methods

### Why First

This is the lowest-risk structural change with the highest immediate value. It closes the exact
class of bugs where one site remembers to mark output but forgets to:

- track the instantiation
- record the late class-method dependency

## Phase 2: Add A Hard Call-Sem Closure Validator

### Objective

Fail before LowIR/object/link if semantic output is incomplete for the required definitions.

### Validator

Add a post-output validator in [semantic_output.cpp](/Users/vishvananda/cppgm/dev/src/semantic_output.cpp),
called from [callsemantic.cpp](/Users/vishvananda/cppgm/dev/src/callsemantic.cpp) after the output loop.

Checks:

1. every non-deleted function with required definition is either:
   - emitted as a definition, or
   - intentionally suppressed as a declaration-only standard-header case
2. every required class method has:
   - an owner class
   - a usable emit scope
3. every tracked instantiated function requiring a definition has a concrete upgraded binding
4. every emitted call target resolves back to a semantic owner

The validator should start as maintainer-facing and fail hard only for impossible states.
Detailed reason dumps can stay behind a trace flag.

### Suppression-Aware Exceptions

Do not flag:

- dependent template instantiations with incomplete arguments
- intentionally suppressed standard-header polymorphic methods/vtables
- declaration-only deferred stdlib functions whose definition is not required

### Why Second

This catches the exact problem class we have been discovering only at host link:

- unresolved `unordered_map` / `unordered_set` hash-table helpers
- missing special-member definitions
- missing stdlib-template helper definitions

## Phase 3: Introduce Explicit Requirement Seeds And Closure

### Objective

Make emission follow an explicit required-definition closure, not a collection of side effects.

### Step 3A: Seed Collection

Collect required definitions from:

- direct calls
- constructor/destructor/copy uses
- function-id/address-taken uses
- vtable slots
- RTTI uses
- global init / lifetime actions
- runtime helper actions

This can initially still set `output_required`, but the important change is that the seeding logic
is centralized and deterministic.

### Step 3B: Upgrade Required Templates Eagerly

For any required function/class method that is template-instantiated:

- call `ensure_function_template_definition(...)`
- replace the binding if it upgrades
- fail immediately if a definition is required but not producible

This step must happen before emission order is decided.

### Step 3C: Expand Transitive Dependencies

Add closure expansion for:

- special-member dependencies
- class-method dependencies
- vtable / RTTI requirements
- runtime helper dependencies
- function-body references that imply required emitted definitions

### Why Third

This is the first step that changes the architecture meaningfully, but by this point:

- requirement writes are centralized
- missing-output cases already fail early

That makes the closure rewrite tractable and debuggable.

## Phase 4: Split Requirement Kinds

### Objective

Replace the overloaded `output_required` bit with explicit requirement kinds.

### Model

Add requirement flags to `FunctionBinding` / `ValueBinding`, for example:

- declaration required
- definition required
- export required
- runtime/vtable/rtti seed

Migration path:

1. introduce the new bitset beside `output_required`
2. route the centralized APIs to both systems
3. switch validators to the new flags
4. remove direct semantic dependence on `output_required`

### Why Not Earlier

This is the right long-term model, but doing it first would create a large migration surface
before we have centralized marking and early validation. That increases risk without shortening
the bootstrap path.

## Phase 5: Make LowIR And Object Export Derived From The Closure

### Objective

Stop using backend reachability as the source of truth for what must exist.

### Invariant

If a symbol must exist at the object/link boundary, it must already be present in the semantic
required-definition closure.

### Consequences

- [lowirgensemantic.cpp](/Users/vishvananda/cppgm/dev/src/lowirgensemantic.cpp) should emit from
  the closure, not recover missing definitions
- [lowir_object_backend.cpp](/Users/vishvananda/cppgm/dev/src/lowir_object_backend.cpp) should
  derive exports from emitted LowIR symbols that came from the same closure

### Backend Validators

Add a LowIR/object closure validator:

- every referenced function/global resolves
- every export refers to an existing LowIR entity
- every relocation target resolves

## Testing Plan

### Always Run

For each closure boundary:

```sh
make test-report CXX=/usr/local/opt/llvm/bin/clang++ \
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++ \
  TEST_REPORT_ASSIGNMENT_JOBS=6 \
  TEST_REPORT_SUBTEST_JOBS=1
```

Perf sanity:

```sh
make build CXX=/usr/local/opt/llvm/bin/clang++ \
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
/usr/bin/time -p env CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++ \
  ./dev/cppgm++ -c dev/src/template_audit.cpp -o /tmp/template_audit.perf.o
```

Bootstrap frontier:

```sh
make bootstrap-frontier CXX=/usr/local/opt/llvm/bin/clang++ \
  CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
```

### Add Targeted Regression Coverage

Add or keep small owned regressions for:

- late-required class methods after class output
- template function use discovered through non-obvious semantic paths
- vtable slot visibility in instantiated classes
- stdlib/polymorphic declaration-only suppression
- PA31 compile/link smokes for hosted container/hash-table visibility

## Current Bootstrap Interpretation

As of the current bootstrap state, the active frontier is `host-link`, and the unresolved symbol
family is dominated by stdlib hash-table/container internals:

- `unordered_map<string, int>` path
- `unordered_set<X*>` path
- supporting `basic_string` / exception-pointer runtime helpers

That pattern strongly suggests the next real work item should be Phase 1 plus Phase 2, not
another narrow ad hoc emission patch first.

In other words:

- the current reduction is useful for owning tests
- but the next implementation should target the visibility pipeline itself

## Concrete Next Implementation Step

Do this next:

1. add `require_function_definition(...)` to `SemanticContext`
2. replace all direct function-definition requirement writes with it
3. add a post-output validator for required definitions
4. rerun `test-report`
5. rerun bootstrap frontier
6. only then reduce the remaining `host-link` failure further if it still survives

## Out Of Scope For This Step

Do not do these in the first patch:

- full replacement of `output_required` everywhere
- object-backend rewrite
- LowIR reachability rewrite
- new student-facing assignment semantics

Those belong after the centralized marking + validator boundary is stable.

## Success Criteria

This plan is working if:

1. new missing-definition cases fail during semantic-output validation rather than at host link
2. requirement marking sites collapse to the centralized API
3. bootstrap frontier moves from repeated visibility/link misses to a smaller number of real
   semantic/backend bugs
4. the backend stops having to "discover" missing symbols that semantic output should have made
   explicit
