# Semantic Analysis Algorithmic Optimization Strategy

## Goal

The current semantic benchmark work showed that local caches help, but they do
not change the shape of the workload. The remaining large wins are likely to
come from avoiding whole classes of semantic work: unnecessary output seeding,
repeated fixpoint rescans, premature class-template materialization, full class
completion when only identity is needed, and overload work that is repeated for
candidate shapes that cannot win.

This document reviews the current semantic path and lays out an instrumentation
and implementation strategy for finding those cutoffs safely.

## Current Semantic Path

The source-driven compile path enters semantic analysis through
`analyze_cpp_sources` in `dev/src/cpp_toolchain.cpp`. For each source file it
preprocesses, tokenizes, parses a C++ AST inside `analyze_calls_translation_unit`,
then constructs `Analyzer` and calls `Analyzer::analyze`.

`Analyzer::analyze` currently performs these major phases:

1. Build the global scope and register builtins.
2. Record template source contexts for witness capture.
3. Collect all top-level declarations into semantic bindings.
4. Determine virtual ABI mode.
5. Run an output seed pass over every top-level AST child.
6. Run an output fixpoint loop until output and tracked-state sizes stop
   changing.
7. Validate required function definitions.
8. Optionally analyze unemitted member bodies for witness-only semantics.

The important detail is that phase 5 is not demand-driven. The declaration
collection pass builds the binding graph first, but the output seed pass still
walks every top-level AST declaration and recursively walks namespace bodies.
For simple declarations it reparses output-facing declaration types, emits
function declarations, checks namespace variable emission, evaluates constant
initializers, and invokes lifetime analysis for class variables. For class
specifiers it calls class output, which completes the class if needed.

The fixpoint loop is also only partly incremental. Some subpasses keep cursors,
but other subpasses rescan broad state:

- `analyze_instantiated_template_output` uses local `class_index` and
  `function_index` variables, so a later outer fixpoint iteration starts again
  from the beginning of `state.instantiated_classes` and
  `state.instantiated_functions`.
- `analyze_late_required_function_output` scans all deferred constexpr and all
  required function definitions each outer iteration.
- `analyze_late_required_synthesized_output` repeatedly walks every tracked
  class, every method bucket in each class, and every static member function
  until one full pass emits nothing.
- `expand_emitted_output_callee_closure` is already cursor-based over top-level
  output nodes.

For source-to-LowIR/object builds, semantic output is followed by LowIR
collection in `ProgramGenerator::collect`. That stage traverses the whole
`CallSemNode` tree multiple times for symbols, virtual runtime classes, virtual
base layouts, global references, function references, string literals,
exception support, and final scope emission. This is not semantic analysis
proper, but it is part of the same compile latency and should be measured
separately before optimizing semantic-only work.

## Current Evidence

The frozen `semantic_overload.cpp` benchmark on this branch improved from a
current-main timeout at `260.405s` to `190.042s` after the class-info named-key
cache refinement. That proves repeated lookup is material, but the post-cache
counters still show broad repeated semantic work:

- `class-info-for-type-calls=9885310`
- `class-info-for-type-pointer-cache-hits=8372031`
- `class-info-for-type-named-key-cache-hits=193772`
- `class-info-for-type-map-lookups=13153`

The map lookup problem is mostly contained, but the compiler still asks the same
semantic question almost ten million times. Similar earlier counter runs showed
hundreds of thousands of template-argument resolutions and overload candidate
attempts. The next target should be reducing the number of questions asked, not
only making each question cheaper.

## High-Leverage Cutoff Areas

### 1. Output Seed Pass

Today every top-level declaration reaches `analyze_declaration_output`, even
when it is a template declaration, a using declaration, an unused declaration, or
a declaration whose only needed effect was already handled during collection.
The simple-declaration path is especially expensive because it parses output
types and can evaluate initializers or lifetime actions before knowing whether a
declaration is an output root.

Strategy:

- Introduce an explicit output-root list populated during declaration
  collection and later semantic use tracking.
- Classify declarations as `collection_only`, `signature_output`,
  `definition_output`, `variable_output`, `forced_witness`, or `forced_debug`.
- First add shadow counters that record whether each output-seed visit appended
  output or changed output state. Do not change behavior in the first patch.
- Once measured, replace the full AST seed walk with a queue of output roots,
  keeping a fallback flag that can run the old pass for comparison.

Expected improvement:

- Avoid recursive namespace output scans through declarations that cannot emit.
- Avoid repeated type parsing for namespace declarations that were already
  collected and are not output roots.
- Avoid eager class completion for class declarations that are present only for
  lookup/type identity.

Risk:

- Namespace-scope function declarations may be part of earlier PA semantic
  output contracts, so the root policy must distinguish semantic-output mode
  from LowIR/object mode.
- Static data members, explicit instantiations, and witness-forced declarations
  need explicit roots.

### 2. Output Fixpoint Rescans

The output fixpoint has the clearest immediate repeated-work shape. Some lists
are cursor-based, but instantiated templates and late synthesized class methods
are rescanned from the beginning.

Strategy:

- Add counters for each fixpoint subpass:
  - items scanned,
  - items emitted,
  - items skipped because already emitted,
  - items skipped because not ready,
  - newly queued items.
- Convert `analyze_instantiated_template_output` to use persistent cursors in
  `OutputState` for instantiated classes and instantiated functions.
- Convert late required free-function output to a dirty queue keyed by
  `FunctionBinding *`.
- Convert late synthesized class method output to queues:
  - newly required class methods,
  - classes whose RTTI/vtable state changed,
  - classes whose output readiness changed.
- Keep a small pending-not-ready queue for items whose readiness can change after
  later output. Requeue only when a relevant dependency changes.

Expected improvement:

- Directly reduces repeated scans without changing semantic policy.
- Lower risk than demand-splitting class completion because it preserves the same
  emit decisions and only changes scheduling.

Risk:

- Some class method readiness is affected by class-level output decisions, so
  the dirty signals must be explicit rather than a one-shot cursor only.

### 3. Class Completion Demand Split

`complete_class_type` currently means several different things:

- is this type a class at all,
- is class info already known,
- are base/member declarations available,
- is object layout known,
- are implicit special members available,
- is a class-template specialization materialized for output.

Because one API does all of that, expression analysis, overload conversion,
type traits, lifetime analysis, ABI layout, and output all call the full
completion path defensively. The class-info cache made no-class lookup cheap,
but it did not remove the calls or prevent unnecessary materialization.

Strategy:

- Introduce a semantic demand enum, initially for instrumentation only:
  - `ClassIdentity`
  - `ClassReferenceMembers`
  - `ClassLayout`
  - `ClassImplicitSpecialMembers`
  - `ClassLifetime`
  - `ClassOutput`
  - `ClassRtti`
  - `ClassTypeTrait`
- Add wrappers around high-volume callers so counters report which demand caused
  `complete_class_type`.
- Split APIs after the data is clear:
  - `class_info_if_known(type)`
  - `ensure_class_reference_members(type)`
  - `ensure_class_layout(type)`
  - `ensure_class_lifetime_model(type)`
  - `ensure_class_output_materialized(type)`
- Make `complete_class_type` a compatibility wrapper during migration, then
  remove broad use from hot paths.

Expected improvement:

- Avoid materializing full libc++ template classes when only type identity or a
  trait answer is needed.
- Make future semantic code harder to accidentally over-complete.

Risk:

- C++ rules often require completeness indirectly. The migration must proceed by
  replacing one reason at a time with strict LowIR comparisons after each step.

### 4. Class Template Reference-Only Instantiations

`reference_class_template_instantiation_with_syntax` already has a raw reference
cache and lazy reference instantiation support, but a cache miss still resolves
arguments, builds canonical keys, selects specialization, creates a `ClassInfo`,
binds template arguments into a scope, records source uses, and may later be
promoted by `complete_class_type`.

Many STL references only need a stable type identity. They do not need member
collection or output unless a later expression asks for layout, members, or
lifetime actions.

Strategy:

- Add a `ClassTemplateReferenceDemand`:
  - `IdentityOnly`
  - `SourceUseOnly`
  - `Members`
  - `Layout`
  - `Output`
- Track every reference-only instantiation and report whether it later escalates
  to members/layout/output.
- For non-escalating identity references, create the minimal semantic identity:
  name, key, template origin, arguments, mangle metadata, and source-use data if
  witness capture requires it.
- Delay member-scope population and argument-scope binding until an escalation
  demand arrives.

Expected improvement:

- Reduce work for recurring library traits and helper templates that are used
  only as types or SFINAE gates.

Risk:

- Existing code may assume `member_scope` exists after any reference. The first
  implementation should keep a lightweight member scope shell and assert when a
  caller touches members without escalating demand.

### 5. Template Argument Resolution

`resolve_template_arguments` expands argument inputs, builds syntax
fingerprints, constructs a cache probe, scans the fast cache, builds a full key,
then does a hash lookup before actual resolution. The rejected recent-cache and
precomputed-key experiments show that changing cache size or moving key
construction does not produce the needed win.

Strategy:

- Add miss classification before another cache rewrite:
  - parameter list identity,
  - raw text vector hash,
  - syntax fingerprint hash,
  - use-scope key,
  - default-argument scope key,
  - success/failure,
  - dependent/non-dependent.
- Identify whether misses are genuinely unique or differ only by overly broad
  scope identity.
- Skip scope-sensitive key dimensions for arguments that are structured and
  already semantically resolved.
- Prefer semantic argument IDs for cache keys where possible, with text retained
  for output/mangling only.

Expected improvement:

- Avoid repeated text expansion and key construction for simple type/value
  arguments.
- Avoid cache misses caused by irrelevant caller-scope differences.

Risk:

- Template argument lookup is scope-sensitive. Scope elision must be proven per
  argument kind, not applied globally.

### 6. Overload Resolution Staging

`analyze_call_expression` already has some useful filters: function-template
argument-count checks, generic argument analysis caches, and per-call conversion
caches. The remaining repeated work is likely in candidate discovery,
deduction/acquisition for templates, per-candidate conversion checks, and
diagnostic string preparation on paths that eventually succeed.

Strategy:

- Add counters by call shape:
  - candidate count before/after ADL,
  - duplicate candidates removed,
  - rejection reason,
  - template candidates inspected,
  - template candidates instantiated,
  - selected candidate rank,
  - calls where one candidate was viable before template candidate expansion.
- Delay diagnostic strings for candidate rejection until a failure path needs
  them. Store compact rejection enums on success paths.
- Deduplicate ordinary/ADL candidates before conversion work, not only after
  viable matches.
- Add a single-candidate path for calls where lookup proves there is one
  non-template function candidate and no function-template candidates can enter.
- Only attempt exact-match early selection when C++ overload rules make template
  candidates irrelevant; otherwise keep the current full selection.

Expected improvement:

- Reduce repeated conversion and template deduction for large overload sets.
- Avoid paying diagnostic formatting cost on green calls.

Risk:

- Overload ranking is subtle. This should come after instrumentation shows
  exact high-frequency call shapes, and each shortcut should have direct tests.

### 7. LowIR Tree Traversal Fusion

`ProgramGenerator::collect` currently performs multiple full recursive passes
over the semantic output tree. This is not the first semantic target, but it can
hide semantic improvements in wall-clock measurements and may be a straightforward
compile-time win after semantic scheduling improves.

Strategy:

- Add LowIR collection phase timers and node-visit counters.
- Fuse independent recursive scans into one visitor where practical:
  symbols, virtual runtime class discovery, virtual base layout collection,
  runtime global/function references, string literals, and exception support.
- Move reachability earlier so output-on-use definitions that will be pruned do
  not pay full collection and lowering cost.

Expected improvement:

- Reduces repeated `CallSemNode` walks.
- Separates semantic-analysis wins from post-semantic LowIR work.

Risk:

- The current order seeds data used by later passes. Fusion should preserve
  explicit ordering or split into a small number of dependency-ordered passes.

## Instrumentation Plan

The first implementation phase should be behavior-preserving instrumentation.
The goal is to measure opportunities, not to guess.

Add `CPPGM_SEMANTIC_PHASE_STATS=1` with machine-readable output for:

- `semantic.collect_declarations`
- `semantic.output_seed`
- `semantic.fixpoint.required_definition_refresh`
- `semantic.fixpoint.instantiated_template_output`
- `semantic.fixpoint.synthetic_function_output`
- `semantic.fixpoint.late_required_function_output`
- `semantic.fixpoint.late_required_synthesized_output`
- `semantic.fixpoint.callee_closure`
- `semantic.validate_required_definitions`
- `semantic.witness_unemitted_bodies`
- LowIR collection subpasses

Add semantic counters for:

- output seed nodes by AST kind and whether they appended output,
- classes/functions scanned vs emitted by each fixpoint subpass,
- class completion calls by demand reason,
- class-template references by demand and final escalation,
- function body analysis/materialization by reason,
- overload candidate rejection enums and duplicate counts,
- template resolver cache misses by classified shape.

Add end-of-analysis shadow reports:

- declarations visited by output seed that produced no output and no state
  change,
- reference-only class-template instantiations that never escalated,
- function bodies materialized but not emitted, not constexpr-evaluated, and not
  witness-required,
- fixpoint subpass items rescanned after their output state was already final.

## Implementation Phases

### Phase A: Measurement

Implement phase timers, work counters, and shadow skip reports. Do not change
semantic behavior. Run the frozen `semantic_overload.cpp` benchmark and PA34
STL controls with stats enabled and update `semantic-analysis-performance-plan.md`
with the top costs.

### Phase B: Fixpoint Queues

Convert the output fixpoint rescans to persistent cursors and dirty queues. This
is the safest first behavior change because it preserves the same output policy
and changes only scheduling. Validate with strict LowIR direct text comparison
and the frozen benchmark.

### Phase C: Output Roots

Introduce output-root collection and run it in shadow mode beside the current
seed pass. Once the root list matches emitted output, switch LowIR/object modes
to root-driven seed output. Keep semantic-output assignment modes on the old
policy until their contracts are explicitly checked.

### Phase D: Class Demand API

Thread class completion reasons through hot callers, then replace broad
`complete_class_type` calls one reason at a time. Start with callers that only
need class identity or an already-known class-info check. Move layout, lifetime,
implicit-special-member, and output callers to explicit APIs.

### Phase E: Reference-Only Class Templates

Use the class demand API to make class-template references create only identity
state by default. Escalate to members/layout/output through explicit demand
calls. Keep witness/source-use data behind existing witness guards.

### Phase F: Resolver And Overload Cutoffs

Use the resolver miss classifier and overload counters to implement targeted
shortcuts. Avoid broad cache-size tuning. Prefer structured semantic keys and
candidate-stage cutoffs with direct tests.

### Phase G: LowIR Pass Fusion

After semantic wins are measurable, fuse LowIR collection passes and move
reachability earlier where possible. Keep this phase separate so semantic timing
data remains interpretable.

## Validation Strategy

Fast gate after behavior changes:

```sh
CPPGM_STRICT_SEMANTIC_FALLBACKS=1 make test-strict STRICT_PAS='pa18 pa19 pa21 pa22' STRICT_SUBTEST_JOBS=8 CXX=/usr/local/opt/llvm/bin/clang++ CPPGM_HOST_CXX=/usr/local/opt/llvm/bin/clang++
```

Current `main` has known strict failures in pa19 and pa22 in this worktree. Until
those are fixed on the integration baseline, treat an unchanged failure set as
the gate for local optimization patches.

Performance gates:

- Frozen `benchmarks/self_compile/stable/semantic_overload.cpp` timing without
  hotspot tracing.
- The same benchmark with semantic stats enabled for attribution.
- PA34 STL controls for regression checks.
- PA35 hosted-link controls after a meaningful behavior change.

Correctness gates for output-sensitive changes:

- Strict LowIR direct text comparison for pa18, pa19, pa21, and pa22.
- Targeted PA33/PA34 tests when changing symbol emission, mangling, overload, or
  hosted-library behavior.
- Full `make test-report` after the output-root and class-demand phases stabilize.

## Recommended Next Patch

Start with Phase A instrumentation and specifically include fixpoint scanned vs
changed counters. The code already shows likely redundant rescans, but counters
will quantify the payoff and make Phase B mechanically safer. The first
behavior-changing patch should be fixpoint queues, not class demand splitting:
it is lower risk and should produce measurable savings if the rescan suspicion is
correct.
