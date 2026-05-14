# Template Decision Ownership Plan

## Goal

Make template behavior in `cppgm` flow through a small, explicit boundary where:

- semantic code asks for template decisions through stable request/result APIs
- the template subsystem owns instantiation and specialization decisions
- witness events are emitted by the component that actually made the decision
- output lowering does not invent or reinterpret template decisions later

This plan is the bridge between:

- [template-semantic-boundary-refactor-plan.md](./template-semantic-boundary-refactor-plan.md)
- [template-direct-source-witness-plan.md](./template-direct-source-witness-plan.md)

The missing piece today is not just "fewer includes." It is explicit ownership.

## Main Risks In The Current Plan

The plan is directionally right, but there are still a few risks and holes.

### Risk 1. We preserve too much of the current witness contract

Right now the plan mostly assumes the current witness text should survive and
the implementation should become clean underneath it.

That is too conservative.

The repository already owns both sides of the student-facing contract:

- `cppgm` emission
- Clang-side oracle materialization

So if the current witness format includes fields that are:

- implementation-shaped
- fragile across valid implementations
- more useful for maintainer debugging than for students

we should simplify the contract first instead of hardening the wrong target.

### Risk 2. We conflate internal decision records with public witness text

The compiler needs richer internal records than the student-facing witness
should expose.

If we make the public witness mirror every internal distinction, we will bake
too much implementation detail into the assignment contract.

The cleaner model is:

- rich internal decision/lifecycle records
- a smaller stable public witness projection
- optional maintainer/debug-only projections for reduction work

### Risk 3. "One owner per family" is still a little too vague

The useful unit of ownership is not just event family, but semantic question.

For example:

- all of these may still render as `function-call`
  - ordinary call
  - constructor selection
  - overloaded operator selection
  - `operator()` selection
- but they should each still have exactly one canonical selection site

So the implementation rule should be:

- one owner per semantic question
- many owned questions may normalize into one public event family

### Risk 4. The plan does not yet define observability hard enough

A lot of the current strict churn exists because we emit helper/probe/internal
facts that are real in one implementation but not educationally observable.

We need an explicit rule for what is public witness versus hidden internal
machinery.

### Risk 5. Closure witness is still too implementation-shaped

Fields such as:

- `created-new=no`
- `trigger ...`
- `track-instantiation`

are useful for maintainers, but they are not the clearest student-facing way to
describe template semantics.

If the student goal is a correct and performant template engine, the witness
should emphasize semantic state changes, not cache behavior or output refresh
mechanics.

## Simplifying Principle

Before hardening the boundary, we should harden the contract.

The right target is:

- a small stable student-facing witness
- richer internal records
- optional hidden maintainer diagnostics

That makes the whole refactor more straightforward because we stop trying to
exactly preserve details that are not actually part of the intended teaching
surface.

## Current Problem

The codebase has a partially improved boundary, but decision ownership is still
blurred.

Good:

- structured template entry points exist in `template_api.h`
- `TemplateTypeSystem` and `TemplateRecursiveSemanticGateway` in
  `template_service_interfaces.h` are the right long-term shape
- direct source witness no longer depends on the old Python adapter

Bad:

- semantic and output code still inspect template internals directly:
  - `source_template`
  - `instantiation_key`
  - `instantiation_arguments`
  - owner-class template chains
- witness emission is still split across:
  - semantic selection code
  - output requirement expansion
  - class finalization/materialization paths
- output code still drives some closure/witness behavior through synthetic
  dependency refresh instead of consuming already-owned template decisions

This has two bad effects:

1. boundary regression
   - semantic/output code keeps learning more about template internals than it
     should

2. witness instability
   - the same semantic fact can be emitted at different times depending on
     output closure, causing the current bind/class-use/closure churn

## Final End State

The target model is:

### 0. Decision Records Are First-Class Objects

Every emitted template-facing fact should exist first as a structured decision
record owned by one subsystem.

That record should answer:

- who made this decision?
- what semantic/lifecycle question was being answered?
- which entity does it describe?
- what source location and declaration location are canonical?
- what cause chain led to it?
- is it observable source witness, lifecycle witness, or output-only state?

The important ownership rule is:

- a later layer may consume a decision record
- a later layer may not invent, reinterpret, or upgrade a decision record

The final internal shape does not need to use these exact type names, but it
should behave as if we had:

- `TemplateDecisionOwner`
  - `overload_resolution`
  - `constructor_selection`
  - `conversion_selection`
  - `class_use_selection`
  - `alias_use_selection`
  - `variable_use_selection`
  - `specialization_selection`
  - `instantiation_lifecycle`
  - `output_planner`
- `TemplateDecisionKind`
  - `function_call`
  - `class_use`
  - `alias_use`
  - `variable_use`
  - `bind`
  - `candidate_drop`
  - `require_definition`
  - `ensure_definition`
  - `function_instantiation`
  - `class_instantiation`
  - `class_finalization`
- `TemplateDecisionCause`
  - `implicit_use`
  - `explicit_instantiation_decl`
  - `explicit_instantiation_def`
  - `explicit_specialization`
  - `extern_template_suppressed`
  - `no_eager_instantiation_suppressed`
  - `class_finalization_member_materialization`

That gives us an explicit model for "this event exists because subsystem X made
decision Y for cause Z."

### 1. Source Decision Ownership

These decisions are owned at the real semantic decision sites:

- function template selection
- overload candidate drop reasons
- constructor template selection
- conversion function template selection
- class / alias / variable use selection
- bind provenance:
  - explicit
  - deduced
  - defaulted
  - specialized

Those sites emit structured source decision events directly.

No later layer is allowed to:

- synthesize a missing source event
- upgrade a guessed bind source
- reinterpret partial vs primary vs explicit selection
- rewrite a drop reason

### 2. Template Lifecycle Ownership

These decisions are owned by the template subsystem:

- implicit instantiation requested
- explicit instantiation declaration
- explicit instantiation definition
- explicit specialization selected
- extern-template suppression
- no-eager-instantiation suppression
- class finalization
- member materialization caused by class finalization

Those sites emit structured lifecycle events directly.

No later layer is allowed to:

- infer a lifecycle step from emitted LowIR
- infer class finalization from member output refresh
- convert "needed for output" into "template was instantiated"

### 3. Output Ownership

Output lowering owns only:

- what code/data must be emitted for the current translation unit
- backend/lowering refresh order
- non-template synthetic dependencies needed for emitted code

Output lowering must not own:

- template use decisions
- specialization selection
- template closure semantics
- witness ordering policy

### 4. Witness Ownership

Witness becomes a recording surface, not a recovery layer.

It should consume:

- source decisions from semantic/template selection code
- lifecycle decisions from template instantiation/finalization code

It should not recover facts from:

- trace text
- LowIR shape
- output refresh side effects
- ad hoc owner/template heuristics in semantic/output code

The public witness should be a projection of those recorded facts, not a dump
of every internal field.

## Boundary Rules

These are the rules the code should satisfy after the refactor.

### Allowed semantic -> template entry points

All semantic/template requests should go through `template_api`.

Examples:

- acquire function/class/variable instantiation
- finalize class instantiation
- deduce function template
- select specialization
- query template-owned metadata

### Allowed template -> semantic callbacks

The callback surface should stay limited to:

- leaf type-system queries
- explicitly audited recursive semantic gateways
- constant evaluation / builtin semantic trait evaluation where the template
  layer still cannot own the work

### Forbidden patterns

- direct semantic includes of `template_instantiation.h` or other concrete
  template implementation modules
- open-coded checks like:
  - `binding.source_template`
  - `!binding.template_instantiation_key.empty()`
  - owner-class template-chain walking
  unless wrapped in one template-owned query API
- witness event construction in `semantic_output.cpp` based on output refresh
- witness ordering fixed by renderer hacks

## Required Supporting Abstractions

The interface work needs a few explicit abstractions that do not exist cleanly
yet.

### A. Template Ownership Queries

One small API surface should answer questions like:

- is this binding template-owned?
- what template owner/lifecycle node owns this binding?
- what is the canonical witness entity/decl location for this template-owned
  symbol?

That removes repeated semantic-side heuristics.

### B. Source Decision Recorder

A dedicated recorder API should accept normalized source decisions:

- selected entity
- chosen specialization kind
- candidate counts
- candidate drops with canonical reasons
- binding provenance entries

This recorder should sit behind the witness session, but be fed only by the
selection sites.

Internally this recorder may keep richer metadata than the public witness
renders.

### C. Template Lifecycle Ledger

The template subsystem needs an explicit lifecycle ledger that records:

- selected
- suppressed
- required
- materialized
- finalized

with structured causes such as:

- explicit-instantiation-definition
- explicit-instantiation-declaration
- explicit-specialization
- implicit-use
- extern-template-suppressed
- no-eager-instantiation-suppressed
- class-finalization-member-materialization

Closure witness output should be rendered from this ledger.

### D. Output Closure Planner

If output still needs template-owned bodies/definitions, that should happen
through a dedicated planner surface that consumes the lifecycle ledger and
returns output requirements.

Output should stop creating lifecycle facts itself.

### E. Witness Projection Policy

We should explicitly separate:

1. internal decision/lifecycle records
2. public student-facing witness text
3. hidden maintainer/debug views

The public witness should prefer:

- winner-oriented source decisions
- stable specialization/binding provenance
- coarse loser/drop reasons only where educationally useful
- semantic lifecycle changes, not implementation cache details

The public witness should avoid exposing:

- exhaustive candidate inventories
- candidate counts unless a specific assignment truly needs them
- exact frontend-internal failure categories
- cache/creation bookkeeping like `created-new=no`
- output-refresh-specific details

If a fact is useful for reducer/debugger work but not for student buildout, it
belongs in a hidden maintainer surface, not the assignment contract.

## Final Decision Flow

The intended steady-state flow is:

1. semantic code asks a narrow question
   - examples:
     - "resolve this overload set"
     - "select the class template specialization for this use"
     - "materialize the class instantiation required by this explicit
       instantiation definition"
2. exactly one owner subsystem makes the decision
   - overload resolution owns function-call and candidate-drop decisions
   - specialization selection owns primary/partial/explicit selection and bind
     provenance
   - lifecycle management owns required/materialized/finalized decisions
3. that subsystem emits a structured decision record immediately
   - no text reconstruction
   - no output-side reinterpretation
   - no renderer-side guessing
4. the witness recorder stores the record unchanged except for stable
   normalization metadata
5. output planning consumes the lifecycle state and explicit materialization
   requests
   - output planning may request emission work
   - output planning may not create new semantic or lifecycle decisions
6. rendering turns the recorded decisions into text
   - path normalization
   - type spelling cleanup
   - deterministic ordering
   - very small dedupe for truly duplicate equivalent records

Between steps 4 and 6 there is one more conceptual step:

5.5. project internal records into the public witness contract
   - drop maintainer-only fields
   - normalize semantically equivalent internal paths
   - keep only the stable student-facing surface

The main architectural correction is step 5:

- today output refresh still creates some of the facts we want to report
- in the target model output refresh becomes a consumer, not an owner

## Explicit Ownership Rules

These rules should become true in code, not just in docs.

### One decision family, one owner

For each decision family there is exactly one canonical owner:

- `function-call` / `candidate-drop`:
  - `semantic_overload.cpp`
- `constructor-call`:
  - constructor selection path in `semantic_overload.cpp`
- `class-use` / `alias-use` / `variable-use`:
  - canonical template use/acquisition sites
- specialization kind and bind provenance:
  - template selection/deduction layer
- lifecycle / closure:
  - template instantiation and finalization layer

If a second site thinks it needs to emit the same family, that is a design
smell and should be treated as a bug unless it is obviously a different
decision.

### Output may request, not decide

`semantic_output.cpp` and related lowering code may:

- ask for already-owned template metadata
- ask the lifecycle planner what must be materialized
- consume finalized decisions

They may not:

- determine primary vs partial vs explicit selection
- determine bind provenance
- determine whether a lifecycle event "really happened"
- create witness events directly from output refresh work

### Witness may record, not recover

The witness layer may:

- record decision objects
- normalize filenames/spellings
- order and dedupe equivalent records

It may not:

- infer source decisions from traces
- infer lifecycle from LowIR/output closure
- reclassify drop reasons or bind provenance
- merge unrelated events to make diffs smaller

### Public witness should be educationally stable

When choosing whether to expose a field, prefer:

- "would a strong student need this to build the feature correctly?"

over:

- "can Clang expose this cheaply?"

That means the default public surface should likely be smaller than the current
prototype.

## Execution Plan

Each stage is intentionally large. The point is to move one big boundary chunk
at a time, not keep scattering micro-fixes.

For every stage below, the pass gate is:

- `make -j1 -C pa18 test CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++`
- `make -j1 -C pa19 test CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++`
- `make -j1 -C pa21 test CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++`
- `make -j1 -C pa22 test CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++`

During this refactor, `test-strict` is informative, not blocking. The strict
lane becomes blocking again only after `Stage 6`.

For each stage we should also save a short report artifact, for example:

- `/tmp/pa18.stageN.test.out`
- `/tmp/pa19.stageN.test.out`
- `/tmp/pa21.stageN.test.out`
- `/tmp/pa22.stageN.test.out`

The goal is to make every stage auditable: "boundary change landed, 4-PA
non-strict lane still green."

### Stage 0. Simplify The Witness Contract First

Goal:

- make the target contract smaller, clearer, and more student-facing before we
  harden the implementation boundary under it

Deliverables:

- split witness into:
  - public student-facing surface
  - hidden maintainer/debug surface
- audit every current witness field and classify it as:
  - keep public
  - move to hidden maintainer output
  - delete
- simplify the Clang-side materializer to emit the reduced public contract
- update the docs to describe the public contract as the assignment oracle

Likely simplifications:

- drop `candidates_built` and `candidates_viable` from the public witness
- keep only coarse loser/drop reasons where they teach something useful
- remove cache/bookkeeping details like `created-new=no`
- consider collapsing lifecycle output to semantic state transitions rather than
  `require/ensure` implementation traces if that better matches the intended
  assignment model

Expected effect:

- less fragile strict surface
- less pressure to imitate Clang implementation accidents
- more direct student-facing references

Concrete acceptance criteria:

- the public witness is short enough to read as assignment guidance
- hidden diagnostics still preserve enough detail for maintainers
- the 4-PA non-strict lane remains green after the oracle/materializer change

### Stage 1. Freeze And Audit The Boundary

Goal:

- stop the boundary from drifting further while we refactor it

Deliverables:

- add a single ownership-query helper surface in `template_api`
- move all semantic-side "is template-owned?" checks behind that surface
- remove direct semantic includes of concrete template implementation headers
  where an API wrapper can replace them
- add a boundary-audit checklist in comments/docs naming the only allowed
  semantic -> template and template -> semantic crossings
  - checklist: [template-boundary-audit-checklist.md](template-boundary-audit-checklist.md)

This stage should especially clean up:

- `semantic_output.cpp`
- `output_requirement_engine.cpp`
- `callsemantic.cpp`

Expected effect:

- fewer semantic-side heuristics
- cleaner setup for the later witness split

Concrete acceptance criteria:

- semantic/output files no longer poke at `source_template`,
  `instantiation_key`, or owner chains directly except inside one helper
  surface
- no new direct includes of concrete `template_*` implementation headers from
  semantic/output code
- `callsemantic.cpp`, `semantic_output.cpp`, and
  `output_requirement_engine.cpp` compile through `template_api` and query
  helpers only

### Stage 2. Pull Witness Construction Out Of Output

Goal:

- make output lowering consume decisions, not manufacture them

Deliverables:

- remove witness event construction from output-refresh driven code paths
- keep output-only synthetic dependency logic, but stop that logic from writing
  closure semantics directly
- introduce a dedicated witness recorder API for source/lifecycle events
- route all existing `note_template_witness_*` construction through recorder
  helpers rather than open-coded event assembly

Expected effect:

- no more LowIR regressions caused by witness-only edits
- no more renderer-side ordering bandaids

Concrete acceptance criteria:

- witness event constructors live in one recorder surface
- output code can no longer emit closure/source witness directly
- turning witness recording off avoids almost all event construction cost

### Stage 3. Centralize Source Decisions

Goal:

- make source witness facts come from exactly one semantic owner per decision
  family

Deliverables:

- function-call witness only from overload/selection code
- class/alias/variable use witness only from canonical lookup/use sites
- canonical bind provenance classifier
- canonical drop-reason mapper
- delete duplicate/secondary source event producers

This is the stage that should address the largest current strict buckets:

- missing/extra `bind`
- missing/extra `class-use`
- missing/extra `function-call`

Expected effect:

- the source witness should stop oscillating based on incidental control flow

Concrete acceptance criteria:

- each source event family has one owner file/function family
- duplicate source event producers are deleted, not just disabled
- bind provenance and drop-reason mapping have one implementation each

### Stage 4. Centralize Template Lifecycle Decisions

Goal:

- make closure/lifecycle witness reflect template semantics, not output refresh

Deliverables:

- add the lifecycle ledger
- move explicit-instantiation / explicit-specialization / extern-template /
  no-eager-instantiation ownership into that ledger
- record class finalization and member materialization there
- make closure witness render from lifecycle state, not output refresh order

This stage should address the other large strict bucket:

- missing/extra `require-definition`
- missing/extra `ensure-definition`
- missing/extra `function-instantiation`
- missing/extra `class-instantiation`
- missing/extra `class-finalization`

Expected effect:

- explicit-instantiation and finalization spec tests should stop reading like
  implementation traces

Concrete acceptance criteria:

- lifecycle transitions are stored as explicit state changes with causes
- class finalization is no longer reconstructed from member output refresh
- closure rendering reads the ledger rather than output requirements directly

### Stage 5. Replace Direct Template-Implementation Calls

Goal:

- make the boundary real rather than conventional

Deliverables:

- eliminate semantic-side direct calls into concrete template implementation
  modules
- wrap remaining needed operations in `template_api`
- move nested-member-class finalization and similar "special path" behavior
  behind explicit request/result APIs
- reduce semantic-side peeking at `ClassInfo` / `FunctionBinding` template
  internals to metadata queries only

Expected effect:

- future template changes stop requiring coordinated edits across semantic and
  output code

Concrete acceptance criteria:

- semantic/output code can be audited by searching for `template_instantiation`
  and finding only template-owned implementation files
- special-case operations like nested-member-class finalization are represented
  as explicit API requests/results

### Stage 6. Reintroduce Strict As The Boundary Oracle

Goal:

- after ownership is correct, make strict useful again

Deliverables:

- audit the remaining strict failures by bucket, not by filename
- fix only true semantic mismatches now that event ownership is stable
- restore `test-strict` as a blocking gate for the four PAs

This stage is where we should finally attack strict aggressively.
Before this point, strict noise is too entangled with boundary drift.

## Order Of Attack Inside Each Stage

Within each stage, work in this order:

1. `pa18`
   - smallest surface, quickest feedback
2. `pa19`
   - specialization/defaulting provenance
3. `pa21`
   - explicit instantiation and entity graph
4. `pa22`
   - deduction/substitution/SFINAE stress

Before `Stage 1`, do `Stage 0` across the same PA order so the simplified
public contract is validated on the easier surfaces first.

This keeps the simpler ownership mistakes from being rediscovered inside the
harder PA22 cases.

## Success Criteria

The boundary is solid enough when all of the following are true:

- semantic code reaches template operations only through audited APIs
- witness events are emitted only by the code that owns the decision
- output lowering no longer changes witness semantics
- the public witness is small, stable, and student-oriented
- strict failures mostly reduce to genuine semantic mismatches, not duplicate
  or re-timed events
- changing witness formatting no longer risks LowIR regressions

That is the point where "make strict pass" becomes a sensible main goal again.

## Suggested Commit Rhythm

To keep each chunk large enough to matter but still reviewable:

1. one commit to establish/refactor the API/query surface for the stage
2. one commit to migrate the main callers/producers for that stage
3. one commit only if needed for fallout cleanup or reference refresh

After each commit series:

- run the 4-PA non-strict gate
- record the reports
- optionally sample strict bucket counts to see whether the ownership change is
  paying off

That keeps us from sliding back into "many tiny witness patches with no
structural checkpoint."
