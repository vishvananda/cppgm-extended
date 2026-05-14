# Template/Semantic Boundary Refactor Plan

## Goal

Refactor the compiler so the template engine becomes a self-contained subsystem
with a small, explicit interface to the rest of semantic analysis.

The end state should make this true:

- non-template semantic code calls into one template-facing API, not directly
  into many `template_*` implementation modules
- template code does not depend on the full `SemanticContext`
- template code does not mutate arbitrary semantic state through scattered
  helpers
- the remaining semantic-to-template touch points are explicit request/result
  APIs plus a very small callback/service surface
- a future rewrite of the template engine can happen behind that boundary
  without requiring broad edits across the semantic layer

This is a structural refactor, not a one-shot rewrite.

## Current Interface Checkpoint

The boundary implemented by this refactor now looks like this:

- non-template semantic code enters the template subsystem through
  `template_api.h`
- helper-style public operations are grouped under the `template_api`
  namespace instead of requiring direct includes of separate
  `template_*_ops.h` headers from semantic callers
- template code depends on the focused service bundle in
  `template_service_interfaces.h`, not the full `SemanticContext`
- the remaining live template-to-semantic callback surface is intentionally
  small:
  - `TemplateTypeSystem::prepare_named_type_member_scope(...)`
  - `TemplateTypeSystem::resolve_direct_type_lookup(...)`
  - `TemplateTypeSystem::resolve_selected_class_template_id(...)`
  - `TemplateRecursiveSemanticGateway::evaluate_dependent_type_expression(...)`
  - `TemplateRecursiveSemanticGateway::evaluate_semantic_builtin_type_trait(...)`
  - `TemplateRecursiveSemanticGateway::evaluate_initializer_constant_value(...)`
- witness/location policy is passed in through `TemplateWitnessContext`
- witness aggregation and final text dumping are translation-unit scoped via
  `TemplateWitnessSession`

The important current property is:

- `resolve_direct_type_lookup(...)` is now an honest leaf service again
- selected class-template-id resolution is explicit and structured through
  `resolve_selected_class_template_id(...)`
- there is no general recursive template-id type-lookup callback

So the leaf/recursive split is now auditable, and the recursive gateway is back
to semantic-heavy expression, trait, and constant-evaluation questions.

The still-future work is the direct witness implementation behind the session
interface:

- `Stage 7C` remains intentionally deferred
- the current `dump_template_witness_text(...)` path still uses the existing
  Python-backed canonicalizer behind the new boundary

## Next Boundary Goal

The next phase is not another facade cleanup.

The remaining work is to reach a stricter interface contract in both
directions:

1. semantic-to-template entry points should carry enough structured input that
   the template layer no longer needs arbitrary semantic-side text fallback
2. template-to-semantic callbacks should be either:
   - acyclic leaf services, or
   - an explicitly tiny, audited recursive gateway when a true leaf service is
     not yet practical

The key non-goal for this phase is important:

- the template layer may continue to parse and rewrite text internally for now
- arbitrary synthesized text must stop crossing the semantic/template boundary

## Minimal Recursive Contract

The optimal minimal recursive interface is smaller than the current one.

The recursive gateway should exist only for semantic questions over already
structured inputs that the template layer cannot practically own yet.

That means the minimum viable recursive surface is:

1. `evaluate_dependent_type_expression(...)`
   - parsed AST operand plus scope
   - used only for hard unevaluated-expression typing cases such as
     `decltype` / `typeof` forms that still require full semantic expression
     analysis, overload resolution, or user-defined conversions
2. `evaluate_semantic_builtin_type_trait(...)`
   - explicit trait enum plus resolved `TypePtr` inputs
   - used only for the semantic-heavy constructible / assignable /
     convertible / no-throw / triviality cases that still require real class
     semantics
3. `evaluate_initializer_constant_value(...)`
   - parsed AST plus scope and optional target type
   - used only for hard constexpr evaluation cases that still require the
     full semantic constexpr engine

Everything else should become non-recursive.

In particular, the recursive gateway must not own:

- type lookup by name
- any text-based fallback
- syntax/parser callbacks
- witness/location callbacks
- template deduction, specialization selection, or instantiation entry points
- any request that accepts arbitrary synthesized text

That milestone has been reached for the general template-id lookup callback:
future lookup holes should either be resolved inside the template layer or
added as explicitly named temporary debt with a smaller structured contract.

## Honest Leaf Lookup Plan

The current state is:

- `TemplateTypeSystem::resolve_direct_type_lookup(...)` is honest leaf lookup
- elaborated-name lookup now stays on the leaf side through a narrow semantic
  helper that can introduce class/struct/union declarations without template
  re-entry
- selected class-template-id lookup is structured through
  `TemplateTypeSystem::resolve_selected_class_template_id(...)`

That is the current boundary because the leaf/recursive split is explicit and
auditable.

If new lookup fallback cases appear, handle them by doing one of two things:

1. move the resolution fully into the template layer, or
2. replace it with an even smaller explicitly named recursive API method with
   a tighter structured contract

No hidden recursion inside a leaf adapter is allowed, and no overly broad
recursive type-lookup request should be reintroduced.

### Fallback Cases To Classify

The old explicit recursive `resolve_recursive_template_id_type_lookup(...)`
request has been removed. New fallback cases to classify are any future lookup
failures that would tempt a caller to reintroduce semantic text lookup from the
template layer.

Known rule:

1. names containing template-id components must stay structured
   - examples: `EnableIf<B, int>::type`
   - they must not fall back to `ctx_.lookup_type(...)` through a hidden leaf
     adapter because that can parse, resolve arguments, select
     specializations, and instantiate templates

### Preferred End State

The preferred outcome remains:

1. template-id-containing lookups recurse only inside the template layer
2. no general recursive template-id type-lookup callback exists

This keeps the recursive API at the minimum contract while making the direct
leaf contract real.

### If Full Internalization Is Not Yet Practical

If some type-name lookup cases still genuinely require semantic/template
re-entry, introduce a temporary explicitly named structured request for that
exact lookup class rather than restoring a general type-lookup callback.

Rules for such a request:

- it must be structured, never text-based
- it must describe exactly which lookup class it handles
- it must be named as recursive debt, not presented as a leaf service
- it must remain separate from `resolve_direct_type_lookup(...)`

That is the fallback option, not the preferred one.

### Execution Plan

#### Stage 9H. Audit the hidden `lookup_type` fallback

For every new path that would otherwise call broad semantic type lookup from
the template layer, classify it as one of:

- direct leaf lookup that can be implemented below the boundary now
- template-owned lookup that should move fully into template code
- residual recursive debt that must become an explicit recursive request

No unclassified recursive fallback should remain.

#### Stage 9I. Internalize template-id type lookup

Keep lookup of names with template-id components out of the semantic adapter
and in template-owned resolution code.

This should reuse the template layer's existing machinery for:

- template-id parsing
- template argument resolution
- specialization selection
- alias/class template instantiation

No explicit recursive type-lookup request should be used for these requests.

#### Stage 9J. Isolate elaborated-name behavior

Handle nondependent elaborated names through one of:

- a dedicated acyclic semantic helper that introduces or finds the named type
  without re-entering template orchestration, or
- an explicit narrow recursive request if no acyclic implementation is
  currently practical

This logic should not remain mixed into a broad recursive type-lookup request.

#### Stage 9K. Make the leaf contract enforceable

Once the fallback cases are split correctly:

- `resolve_direct_type_lookup(...)` must not call `SemanticContext::lookup_type`
- the recursive gateway must contain only any still-necessary recursive
  name-lookup requests explicitly
- comments and tests should reflect the honest split

#### Stage 9L. Re-measure the minimum recursive contract

After the hidden recursion is removed, re-evaluate the recursive surface:

- if no name lookup remains recursive, the contract stays at the current
  minimum of 3 methods
- if one small lookup class must remain recursive, document that exact method
  as the revised minimum contract and justify why it cannot yet be internalized

## Current Remaining Risk

The remaining callback count is small, but the current callback behavior is not
yet in the desired final shape.

### 1. General recursive type lookup must not return

Today the remaining type-lookup surface is:

- `TemplateTypeSystem::resolve_direct_type_lookup(...)`
- `TemplateTypeSystem::resolve_selected_class_template_id(...)`

That keeps lookup on the type-system side instead of the recursive semantic
gateway. The remaining job is to preserve that split and avoid reintroducing a
generic recursive lookup fallback.

### 2. Some remaining callbacks are not leaf operations

The remaining callbacks may still re-enter higher semantic machinery:

- `evaluate_dependent_type_expression(...)`
  - still intentionally recursive for the hard unevaluated-expression cases
- `evaluate_semantic_builtin_type_trait(...)`
  - acceptable in principle, but some traits still flow through constructor
    selection, conversions, or no-throw analysis
- `evaluate_initializer_constant_value(...)`
  - acceptable in principle, but current consteval hooks still use broad
    semantic lookup and expression analysis

So the next phase must not just rename callbacks.
It must classify them by cycle risk and move them to narrower services.

## Target Interface Shape For The Next Phase

### Incoming semantic-to-template requests

The public `template_api` surface should continue to move toward explicit
request/result APIs.

The next required improvement is that request objects should start carrying
structured syntax, not just raw strings, so that future template-side
algorithms can stop reparsing without another semantic-boundary redesign.

The long-term target is not to eliminate internal template parsing yet.
The target is to ensure the incoming interface already carries enough
information that internal parsing can be removed later without needing more
semantic-side callbacks.

Important examples:

- explicit template argument lists
- class template instantiation arguments
- dependent type fragments used by deduction or specialization matching
- source spans or witness anchors associated with those fragments

### Outgoing template-to-semantic services

The service surface should split conceptually into two layers.

#### A. Acyclic leaf services

These are semantic queries that must not call back into `template_api` or
template-owned orchestration.

This should become the default allowed service category.

Candidate leaf requests include:

```cpp
struct TemplateTypeLookupRequest;
struct TemplateSemanticBuiltinTraitRequest;
```

These requests must be narrow, structured, and implemented below the
template/semantic boundary rather than through broad semantic entry points.

#### B. Explicit bounded recursive services

If a callback cannot yet be made acyclic, it must not remain mixed in with the
leaf service set.

It should move into a separately named and separately reviewed gateway with
strong rules around what it may do.

Candidate bounded-recursive requests include:

```cpp
struct TemplateDependentTypeExprRequest;
struct TemplateConstantEvaluationRequest;
```

The existence of this gateway should be treated as transitional debt, not the
final boundary shape.

## Structured Request Shapes

The exact names can change, but the requests should look roughly like this.

### Leaf type-name lookup request

This replaces the current string-based lookup half of
`resolve_type_text_fallback(...)`.

```cpp
struct TemplateTypeLookupRequest
{
  TemplateEnvironmentHandle scope;
  bool rooted = false;
  bool has_typename = false;
  bool allow_class_templates = false;
  std::vector<std::string> qualifiers;
  std::string terminal_name;
};
```

Important property:

- the request carries identifier structure, not arbitrary generated type text

### Provisional dependent type-expression request

This is the natural structured replacement for the current decltype/typeof text
fallback, but it should not automatically be considered safe just because it is
structured.

```cpp
enum TemplateDependentTypeExprKind
{
  TDTEK_DECLTYPE,
  TDTEK_TYPEOF
};

struct TemplateDependentTypeExprRequest
{
  TemplateEnvironmentHandle scope;
  TemplateDependentTypeExprKind kind;
  bool operand_was_parenthesized = false;
  CppAstNode operand;
};
```

Important property:

- this request removes arbitrary text from the boundary
- it does not, by itself, guarantee acyclic behavior

### Semantic builtin-trait request

```cpp
struct TemplateSemanticBuiltinTraitRequest
{
  TemplateSemanticBuiltinTypeTrait trait;
  std::vector<cpp_decl::TypePtr> types;
};
```

Important property:

- trait classification stays explicit and typed
- the remaining question is whether the implementation is leaf or recursive

### Constant-evaluation request

```cpp
struct TemplateConstantEvaluationRequest
{
  TemplateEnvironmentHandle scope;
  CppAstNode initializer;
  cpp_decl::TypePtr target_type;
};
```

Important property:

- AST and target type cross the boundary, not text
- whether this can be leaf depends on the evaluator implementation, not the
  request shape alone

## Safety Rules For Any Remaining Recursive Gateway

If we cannot immediately remove all opportunities for cycles, the remaining
recursive flows must be minimized and explicitly constrained.

Any bounded-recursive semantic service should satisfy all of these rules:

1. It is declared on a separate interface from the acyclic leaf services.
2. It may not call `template_api` entry points directly.
3. It may not call template argument resolution, specialization selection, or
   template instantiation helpers as part of its normal operation.
4. It may not emit witness records or alter witness/session policy.
5. It may not perform semantic output/export tracking or other translation-unit
   side effects unrelated to answering the query.
6. It should operate only on already-parsed AST or already-resolved canonical
   type handles, never on arbitrary synthesized text.
7. It should be covered by explicit recursion-depth or phase guards so that the
   remaining re-entry surface is observable and auditable.

If a callback cannot satisfy these rules, it does not belong in the long-term
boundary.

## Success Criteria For The Next Phase

This next phase is complete when all of these are true:

1. No arbitrary synthesized type text crosses from the template layer back into
   semantic.
2. The public semantic-to-template request types carry enough structured syntax
   that future template-side parsing removal will not require another boundary
   redesign.
3. The remaining semantic callback surface is split into:
   - an acyclic leaf service set
   - a separately named bounded-recursive gateway, if any still exists
4. The acyclic leaf services are implemented without calling `template_api` or
   template-owned orchestration.
5. Any remaining recursive gateway is explicit, tiny, and documented as
   temporary debt.

## Hard Invariant

`test-strict` stays green throughout the refactor.

That is the main guardrail against changing template behavior while moving the
boundary. A stage is not complete until:

```sh
make test-strict ORDERED=false
```

passes from the repository root.

For smaller intermediate slices, targeted strict checks are allowed first, but
every completed stage must end with the full strict suite green.

## Current State

Today there is no meaningful semantic/template boundary.

### 1. Entry points are scattered

Template operations are invoked directly from many semantic files, especially:

- `dev/src/callsemantic.cpp`
- `dev/src/semantic_overload.cpp`
- `dev/src/semantic_class_model.cpp`
- `dev/src/semantic_conversion.cpp`
- `dev/src/semantic_expression.cpp`
- `dev/src/semantic_declaration.cpp`
- `dev/src/semantic_statement.cpp`
- `dev/src/semantic_output.cpp`

Those files call deeply into implementation modules such as:

- `template_resolution`
- `template_instantiation`
- `template_selection`
- `template_specialization`
- `template_argument_semantics`
- `template_scope`
- `template_function_signature`

So the template engine is currently treated as a bag of semantic utilities, not
as a subsystem.

### 2. Template code depends on the full semantic analyzer

The main template modules depend heavily on `SemanticContext`, especially:

- `template_argument_semantics.cpp`
- `template_resolution.cpp`
- `template_instantiation.cpp`
- `template_specialization.cpp`
- `template_selection.cpp`
- `template_function_signature.cpp`

Those modules use `SemanticContext` for many unrelated concerns:

- type lookup and qualified lookup
- type parsing and expression parsing from text
- constant evaluation and trait evaluation
- class completion and layout
- function/class/variable entity registration
- output requirement tracking
- diagnostics and source text reconstruction
- local and template binding overlays

That makes template code impossible to replace independently.

### 3. Raw semantic state is shared everywhere

The current public template helper APIs expose and mutate semantic internals
directly:

- `semantic_model::Scope`
- `semantic_model::*Decl`
- `semantic_model::ClassInfo`
- `FunctionBinding`
- `ValueBinding`
- `cpp_decl::TypePtr`
- `CppAstNode`

This means template logic is coupled to both the semantic graph shape and the
semantic execution model.

### 4. Query and side effect are mixed

Current template helpers often do some mixture of:

- argument deduction
- specialization selection
- instantiation
- scope binding
- entity registration
- output marking
- class finalization

in the same call path.

That makes it hard to tell which parts are real template reasoning and which
parts are semantic materialization side effects.

## Main Coupling Buckets

The current entanglement mostly falls into five buckets.

### A. Template environment and scope binding

This is the `template_scope` / scope-overlay / binding-fingerprint world:

- template parameter bindings
- local binding overlays
- dependent named-type lookup through bound names
- point-of-instantiation environment capture

This is currently represented as direct mutation of `semantic_model::Scope`.

### B. Deduction and argument resolution

This is mostly:

- `template_argument_semantics`
- `template_resolution`
- parts of `template_specialization`

It currently mixes real deduction logic with textual parsing/rewrite glue.

### C. Specialization matching and selection

This is mostly:

- `template_selection`
- `template_specialization`

It should be a template query subsystem, but it currently still depends on
semantic parsing and lookup services.

### D. Instantiation and replay

This is mostly:

- `template_instantiation`
- `template_instantiation_coordinator`

It currently reaches directly into semantic registration, class completion, and
output tracking.

### E. Syntax bridge and text normalization

This is the most transitional coupling:

- parsing template-id fragments
- reparsing type text
- rewriting bound names in text
- placeholder detection in fragments

Right now this glue lives inside core template modules, which makes the whole
template engine depend on semantic text machinery.

## Refactor Objective

Move from this:

- many semantic files call many template modules directly
- template modules call broadly back into `SemanticContext`

to this:

- semantic files call one template facade
- the facade accepts explicit request structs
- the template layer performs deduction / selection / instantiation planning
- the template layer uses a small service interface for the few semantic
  operations it cannot own yet
- semantic materialization happens through a narrow apply/host layer instead of
  arbitrary mutation from inside template code

## Target Boundary

The target boundary should have three parts:

### 1. A single semantic-to-template facade

Introduce one public template boundary, for example:

- `dev/src/template_api.h`
- `dev/src/template_api.cpp`

All non-template semantic code should eventually depend only on this facade.

After the refactor, files like:

- `callsemantic.cpp`
- `semantic_overload.cpp`
- `semantic_conversion.cpp`
- `semantic_class_model.cpp`
- `semantic_expression.cpp`

should no longer include `template_resolution.h`,
`template_instantiation.h`, `template_selection.h`, and similar
implementation headers directly.

They should include only the facade.

### 2. Explicit request/result types

The facade should not expose ad hoc parameter lists full of semantic internals.
It should expose explicit operations with explicit payloads.

Initial operations should be close to the current behavior:

```cpp
struct TemplateResolveExplicitArgsRequest;
struct TemplateFunctionDeductionRequest;
struct TemplateSpecializationSelectionRequest;
struct TemplateClassInstantiationRequest;
struct TemplateFunctionInstantiationRequest;
struct TemplateVariableInstantiationRequest;
```

and results such as:

```cpp
struct TemplateArgumentResolutionResult;
struct TemplateDeductionResult;
struct TemplateSelectionResult;
struct TemplateInstantiationResult;
struct TemplateInstantiationPlan;
```

The exact names can change. The important property is:

- inputs describe one template problem
- outputs describe one template decision or one instantiation plan
- callers do not manually orchestrate three or four separate helper calls

### 3. A small semantic callback surface

The template layer should not call back into `SemanticContext` wholesale.
Instead it should depend on a small set of focused services.

The allowed callback categories should be:

#### A. Type System Service

Operations the template layer may still need:

- canonical type comparison
- type substitution over canonical type handles
- function type reconstruction
- alias/cv/ref/array/function normalization
- current-specialization-aware type identity checks

This should be a narrow service, not full lookup.

#### B. Constant Evaluation Service

Operations the template layer may still need:

- evaluate non-type template arguments
- evaluate integral constant expressions
- evaluate builtin traits needed by SFINAE or dependent matching

This keeps constexpr/trait logic outside the template engine while giving it
the results it needs.

#### C. Entity Materialization Service

Operations the template layer may still need:

- register an instantiated function/class/variable entity
- request class completion/finalization
- request function definition materialization
- record output/export requirements

This is the primary place where semantic side effects should be allowed.

#### D. Transitional Syntax Service

This is temporary and should shrink over time.

Operations the template layer may still need during transition:

- parse a type fragment
- parse a template-id fragment
- classify whether a fragment is type / value / template
- rebuild normalized text for legacy paths

The long-term goal is to remove as much of this as possible by moving the
template layer to normalized IR instead of text-driven repair.

## Initial Interface Shape

The first usable boundary should look like a query-and-plan API.

### Function-template deduction

Semantic should ask:

```cpp
TemplateDeductionResult deduce_function_template(
    const TemplateFunctionDeductionRequest& request,
    TemplateServices& services);
```

The request should include:

- template or overload-set identity
- explicit template arguments if present
- call argument semantic summaries
- use-site environment handle
- target type if relevant
- deduction mode or intent

The result should include:

- viable candidates
- dropped candidates with reasons
- selected candidate if unique
- canonical bound template arguments
- any witness/audit payload needed by `--emit-templates`

### Class/variable specialization selection

Semantic should ask:

```cpp
TemplateSelectionResult select_specialization(
    const TemplateSpecializationSelectionRequest& request,
    TemplateServices& services);
```

The request should include:

- primary template identity
- explicit or deduced canonical template arguments
- use-site environment handle
- specialization kind

The result should include:

- selected entity identity
- whether the winner is primary / partial / explicit
- canonical bound arguments
- selection audit info

### Instantiation

Semantic should ask:

```cpp
TemplateInstantiationPlan plan_instantiation(
    const TemplateInstantiationRequest& request,
    TemplateServices& services);

TemplateInstantiationResult apply_instantiation_plan(
    const TemplateInstantiationPlan& plan,
    TemplateMaterializationHost& host);
```

This split is important.

The template layer should decide:

- what entity is being instantiated
- with which arguments
- under what instantiation intent
- which dependent substitutions are required

The semantic/materialization layer should perform:

- actual semantic-model registration
- class completion
- function body materialization
- output tracking

That separation is what will let us replace the template engine later.

## Witness Interface Shape

The witness path should become an explicit part of the boundary, but in two
separate directions:

- semantic passes witness/source-location context into the template layer
- the template layer owns witness aggregation and witness text dumping for one
  translation unit

This should not be modeled as a syntax callback. The remaining fragment
location hook is really witness/use-location policy, not parsing behavior.

### Incoming witness context

The template layer should receive explicit location context from the semantic
boundary instead of calling back out for it mid-parse.

The initial incoming shape should stay minimal:

```cpp
struct TemplateWitnessContext
{
  std::string public_use_location;
  // Temporary source-location mapping data for helper fragment reparses.
  // This may later become stable source-location handles instead.
  ...
};
```

Important properties:

- `public_use_location` is the outward source location for the semantic
  template operation
- the context carries enough source-location mapping information for helper
  reparses or helper lookups so they do not inherit the outward user site
- this context is passed in at the call boundary
- the template layer does not call back into semantic just to ask what
  location a helper parse should use

The concrete representation can later change from strings to source-location
handles, but the boundary direction should stay the same.

### Witness session lifecycle

The output side should also become explicit, but without requiring the final
direct witness implementation now.

The template layer should aggregate witness state in a per-translation-unit
session:

```cpp
class TemplateWitnessSession;

TemplateWitnessSession create_template_witness_session();
std::string dump_template_witness_text(const TemplateWitnessSession & session,
                                       const std::string & source_path);
```

Important properties:

- the session is translation-unit scoped
- semantic/front-end code creates the session once for `--emit-templates`
- template operations emit into that session during ordinary semantic analysis
- the final text dump happens at the end of translation-unit processing

This keeps aggregation aligned with the current `generate_template_translation_units`
flow instead of pretending witness output is a one-shot template query.

### Keep the current canonicalizer behind the interface at first

The first implementation should not replace the Python witness normalization
pipeline.

Instead:

- `dump_template_witness_text(...)` should temporarily delegate to the existing
  canonicalizer path
- the new interface should exist even if its implementation still uses the
  current trace/Python machinery under the hood
- the initial goal is to make the witness boundary explicit before changing the
  witness format or serializer

This means the immediate refactor can be:

- interface first
- implementation compatibility second
- final direct witness generation later

### What should stay internal for now

The template layer will eventually need structured witness records such as
events, bindings, drops, and sinks.

Those should remain internal until the direct-output migration is ready.

For now, the public or cross-layer witness-facing interface should stay small:

- incoming `TemplateWitnessContext`
- per-TU `TemplateWitnessSession`
- `dump_template_witness_text(...)`

That avoids locking in the final internal witness representation too early.

## Data Ownership Direction

The target ownership model should be:

- semantic owns the semantic graph
- template layer owns template reasoning and template environment logic
- semantic passes stable handles or small views into the template layer
- template layer returns decisions and plans, not arbitrary mutations

The important long-term direction is to stop exposing raw
`semantic_model::Scope` as the public template environment type.

Instead, introduce something like:

```cpp
struct TemplateEnvironmentHandle;
struct TemplateEntityHandle;
struct CanonicalTemplateArgument;
struct CanonicalTemplateArgumentList;
```

At first those handles may still wrap semantic structures internally. That is
acceptable as a transition. The boundary only becomes real once the rest of
semantic no longer depends on template implementation details.

## Success Criteria For The Boundary

This refactor is successful when all of these are true:

1. Non-template semantic files call only `template_api`, not individual
   `template_*` implementation modules.
2. Public template headers no longer require full `SemanticContext`.
3. Template implementation files depend only on focused service interfaces, not
   arbitrary analyzer methods.
4. Instantiation uses a plan/apply boundary instead of direct scattered
   semantic mutation.
5. `template_scope` behavior is exposed through a template-environment
   abstraction, not direct raw scope mutation by unrelated semantic files.
6. `test-strict` remains green throughout.

## Staged Refactor

### Stage 0: Freeze The Current Behavior

Purpose:

- establish the strict suite as the refactor gate
- document the current boundary before moving code

Deliverables:

- this plan file
- a simple inventory of direct semantic-to-template entry points
- a simple inventory of template modules that still depend on
  `SemanticContext`

Validation:

- `make test-strict ORDERED=false`

### Stage 1: Introduce A Facade Without Changing Behavior

Purpose:

- create one semantic-to-template boundary before moving logic

Changes:

- add `template_api.h/.cpp`
- add wrapper entry points for:
  - function-template deduction
  - class/variable specialization selection
  - class/function/variable instantiation
  - explicit argument resolution
- keep the wrappers forwarding to the current implementation

Rules:

- no semantic behavior change
- no public call-site should gain new direct knowledge of implementation
  modules

Success criteria:

- semantic callers begin switching from direct `template_*::...` calls to
  `template_api`
- strict suite stays green

### Stage 2: Route Semantic Callers Through The Facade

Purpose:

- remove direct template implementation dependencies from the rest of semantic

Changes:

- update `callsemantic.cpp`, `semantic_overload.cpp`,
  `semantic_conversion.cpp`, `semantic_class_model.cpp`,
  `semantic_expression.cpp`, and other semantic callers
- remove direct includes of implementation headers where possible

Success criteria:

- non-template semantic files no longer call implementation modules directly
- `template_api` becomes the only allowed public template entry point
- strict suite stays green

### Stage 3: Split `SemanticContext` Into Template Services

Purpose:

- shrink the callback surface

Changes:

- define service interfaces such as:
  - `TemplateTypeSystem`
  - `TemplateConsteval`
  - `TemplateMaterializationHost`
  - transitional `TemplateSyntaxBridge`
- make `template_api` build one service bundle from the current analyzer
- convert template modules to consume services instead of the full
  `SemanticContext`

Success criteria:

- `template_resolution`, `template_specialization`, `template_selection`,
  `template_argument_semantics`, and `template_instantiation` stop depending on
  the whole `SemanticContext`
- service boundaries are visible in headers
- strict suite stays green

### Stage 4: Isolate Template Environment State

Purpose:

- stop exposing raw scope mutation as the template boundary

Changes:

- introduce a `TemplateEnvironment` abstraction
- move `template_scope` operations behind that abstraction
- convert semantic callers so they pass environment handles instead of mutating
  template bindings directly

Success criteria:

- raw `semantic_model::Scope` is no longer the public environment type for
  template operations
- binding overlays and template-bound names are owned behind one abstraction
- strict suite stays green

### Stage 5: Separate Query From Mutation

Purpose:

- make deduction/selection pure-ish and make instantiation explicit

Changes:

- refactor selection and deduction entry points to return results instead of
  performing side effects
- refactor instantiation entry points to return plans
- move semantic-model registration and output marking to one apply/materialize
  host layer

Success criteria:

- template queries do not directly register entities
- template instantiation has a plan/apply boundary
- output tracking is not scattered across callers
- strict suite stays green

### Stage 6: Consolidate Instantiation Ownership

Purpose:

- make the template layer own the instantiation graph and decisions

Changes:

- move more of `template_instantiation_coordinator` into the boundary
- centralize:
  - lookup-only instantiation
  - require-definition instantiation
  - export/output intent
  - class replay/finalization triggers

Success criteria:

- callers stop manually combining lookup, instantiation, finalization, and
  output steps
- one API represents the instantiation lifecycle
- strict suite stays green

### Stage 7: Remove Text-Driven Core Dependencies

Purpose:

- stop making core template reasoning depend on text rewrite/parsing repair

Changes:

- quarantine text parsing/rewrite logic behind the transitional syntax bridge
- progressively replace text-based argument handling with normalized template
  IR or canonical argument records
- keep textual witness rendering as an output concern, not a deduction concern

Success criteria:

- text rewriting is no longer embedded in the main deduction/selection core
- new template logic can operate over normalized data instead of fragment text
- strict suite stays green

### Stage 7A: Introduce The Witness Boundary Without Changing Output

Purpose:

- make witness/location policy explicit before removing the final syntax
  callback

Changes:

- add `TemplateWitnessContext`
- add `TemplateWitnessSession`
- add `dump_template_witness_text(...)`
- route `--emit-templates` through the session/dump interface
- keep the existing Python canonicalizer behind `dump_template_witness_text(...)`

Success criteria:

- witness aggregation has an explicit translation-unit interface
- helper parse use-location policy is no longer modeled as syntax behavior
- witness output remains compatible with the current strict-suite expectations
- strict suite stays green

### Stage 7B: Remove The Final Syntax Callback Using Witness Context

Purpose:

- eliminate `preferred_fragment_use_location(...)` safely

Changes:

- thread `TemplateWitnessContext` through the fragment/type-text parse paths
- use witness-context-provided helper parse location data instead of the syntax
  callback in `template_decl_ast` and related fragment helpers
- keep witness output flowing through the existing dump/canonicalizer path

Success criteria:

- `TemplateSyntaxService` no longer needs the fragment use-location callback
- helper reparses preserve the old witness behavior on strict canaries
- strict suite stays green

### Stage 7C: Later Replace Trace/Text Reparse Behind The Session

Purpose:

- leave room for direct witness emission later without committing to it now

Changes:

- introduce internal structured witness records behind the session
- progressively replace trace-text emission behind the session implementation
- eventually stop depending on the Python reparse path

Success criteria:

- this work can proceed behind the session interface
- no semantic/template boundary redesign is required when the direct witness
  implementation starts
- strict suite stays green

### Stage 8: Make The Boundary Enforceable

Purpose:

- prevent future backsliding

Changes:

- reduce or remove public includes of template implementation headers from
  non-template semantic code
- keep only the facade and service headers public
- document the allowed dependency direction

Success criteria:

- the boundary is structural, not just conventional
- a future template-engine rewrite can happen behind `template_api`
- strict suite stays green

### Stage 9A: Audit Callback Cycle Risk

Purpose:

- classify the remaining callback paths by whether they are leaf queries or
  template-aware semantic re-entry paths

Changes:

- inventory each remaining callback implementation
- record whether it currently reaches:
  - `template_api`
  - template argument resolution
  - specialization selection
  - template instantiation
  - broad semantic expression analysis / overload resolution
- classify each remaining callback as one of:
  - leaf-safe target
  - recursive but acceptable temporarily
  - must be eliminated

Success criteria:

- the remaining callback set is documented with explicit cycle-risk labels
- there is no ambiguity about which callbacks are allowed to remain recursive
- strict suite stays green

### Stage 9B: Replace Text Fallback With Structured Requests

Purpose:

- remove arbitrary synthesized text from the template-to-semantic boundary

Changes:

- replace `resolve_type_text_fallback(...)` with structured requests
- split the current behavior into at least:
  - `TemplateTypeLookupRequest`
  - a provisional structured request for decltype/typeof-style dependent type
    expressions
- keep any necessary text parsing internal to the template layer

Success criteria:

- no callback on the semantic boundary accepts arbitrary synthesized type text
- type-name lookup crosses the boundary as structured data
- strict suite stays green

### Stage 9C: Upgrade Semantic-To-Template Input Requests

Purpose:

- ensure the incoming `template_api` request surface already carries enough
  structure for a future no-reparse template layer

Changes:

- add structured fragment-bearing forms for key request types
- start with the entry points that still rely most heavily on raw strings:
  - explicit template argument resolution
  - function-template explicit arguments
  - class template instantiation arguments
- keep string-based forms only as compatibility adapters that parse inside the
  template layer

Success criteria:

- new or primary `template_api` requests can carry parsed fragments or other
  structured syntax, not only strings
- future internal parsing removal would not require another semantic interface
  expansion
- strict suite stays green

### Stage 9D: Introduce Acyclic Leaf Semantic Services

Purpose:

- make the default callback contract truly non-recursive

Changes:

- split the current service surface into:
  - acyclic leaf services
  - temporary bounded-recursive services
- implement leaf services below the template boundary rather than on top of
  broad semantic entry points
- start with the most tractable leaf candidates:
  - structured type-name lookup
  - any builtin traits that can be answered from canonical types and class
    metadata alone

Success criteria:

- the leaf service interface exists separately and clearly
- its implementations do not call `template_api`
- its implementations do not orchestrate template deduction/instantiation
- strict suite stays green

### Stage 9E: Quarantine And Minimize Recursive Semantic Flows

Purpose:

- keep unavoidable recursive behavior explicit, tiny, and auditable

Current status:

- the service boundary now separates:
  - `TemplateTypeSystem` for structured type lookup
  - `TemplateRecursiveSemanticGateway` for the remaining recursive semantic
    services
- the recursive gateway now also uses explicit request objects rather than
  ad hoc argument lists
- the next work in this stage is reducing what still lives in the recursive
  gateway, not further mixing of service categories

Changes:

- move any still-recursive operations behind a separately named temporary
  gateway
- likely initial members are:
  - dependent type-expression evaluation if still needed
  - initializer constant evaluation if still backed by broad semantic
    expression analysis
- add explicit guards and documentation for those flows

Success criteria:

- recursive callbacks are no longer mixed into the normal service bundle
- each remaining recursive path has a documented justification and bounded
  call pattern
- strict suite stays green

### Stage 9F: Shrink Or Eliminate The Recursive Gateway

Purpose:

- drive the temporary recursive gateway toward zero, or leave only a clearly
  justified minimum

Changes:

- move more builtin traits into leaf/shared logic where feasible
- split constant evaluation into smaller leaf-capable operations where
  possible
- decide whether a general dependent type-expression callback can be made safe,
  or whether it must stay internal to the template layer longer

Success criteria:

- the remaining recursive gateway is either gone or reduced to a very small,
  explicitly justified set
- the boundary has a clear default rule: callbacks are leaf unless proven
  otherwise
- strict suite stays green

### Stage 9G: Enforce The No-Text / No-Unreviewed-Cycle Rules

Purpose:

- make the new boundary rules durable

Changes:

- add lightweight checks or grep-based audits for:
  - no arbitrary type-text fallback callback
  - no leaf-service implementation calling `template_api`
  - no unreviewed new recursive service additions
- document the allowed dependency rules near the service declarations

Success criteria:

- regressions toward text fallback or hidden recursive callbacks are easier to
  detect
- the boundary contract is enforceable, not only aspirational
- strict suite stays green

## Metrics To Track During The Refactor

Track these as the work proceeds:

1. Number of non-template semantic files that include template implementation
   headers.
2. Number of direct semantic call sites into `template_*` implementation
   modules.
3. Number of public template headers that mention `SemanticContext`.
4. Number of template implementation files that require the full
   `SemanticContext`.
5. Number of public template entry points that expose raw
   `semantic_model::Scope`.

Those counts should trend toward zero except for the small service/facade
surface.

## Non-Goals

This refactor does not require, at first:

- a new template algorithm
- a template semantics rewrite
- a new semantic graph
- immediate removal of all text-based helpers
- eliminating all callbacks in the first stage

The initial goal is boundary creation, not instant purity.

## Recommended First Implementation Slice

Start with Stage 1 and Stage 2:

- add `template_api`
- forward current operations through it
- switch semantic callers to the facade

That gives us a real seam before we begin shrinking `SemanticContext` and
extracting environment/materialization services.

That first seam is the foundation for every later stage.
