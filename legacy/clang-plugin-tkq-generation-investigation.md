# Clang Plugin To `.tkq` Generation Investigation

This note evaluates what it would take to expand the experimental Clang plugin
from:

- a student-facing witness generator

into:

- a generator for the **whole** template-kernel `.tkq` surface

The relevant existing pieces are:

- plugin: [dev/clang_template_student_metrics_plugin.cpp](/private/tmp/cppgm-template-kernel-20260416/dev/clang_template_student_metrics_plugin.cpp)
- current kernel contract: [template-kernel/README.md](/private/tmp/cppgm-template-kernel-20260416/template-kernel/README.md)
- current plugin writeup: [clang-template-student-metrics-prototype.md](clang-template-student-metrics-prototype.md)

## Short Answer

The plugin can probably generate a **good seed** for `.tkq`.

It cannot reliably generate the **final current `.tkq`** for all interesting
cases from stock Clang plugin hooks alone.

The hard part is not just extracting template facts from Clang. It is reducing
real C++ into the much smaller kernel language while preserving:

- the decisive candidate set
- the binding story
- the loser and drop reason
- the minimal owned mechanism

That requires more than observation.

## What “Whole `.tkq`” Actually Means

To generate a full `.tkq` case from source, the tool would need to produce:

1. template declarations
2. specializations / partial specializations
3. explicit specializations where relevant
4. a query set corresponding to the observed template behavior
5. enough information for `tmplsolve` to reproduce the same student-visible
   result
6. for current function-deduction cases, the loser candidate and deterministic
   drop reason

For complex reductions, it would also need to choose a **smaller abstraction**
than the original source.

That final reduction step is the core difficulty.

## Current Plugin Surface

The current plugin already observes:

- `class_use`
- `variable_use`
- `alias_use`
- `function_call`
- selected declaration kind for class / variable templates
- final template arguments
- coarse binding source tags:
  - `explicit`
  - `deduced`
  - `defaulted`

That is enough to produce a source-level witness JSON.

It is not enough to produce the current full `.tkq` contract for many cases.

## Gap Analysis

### 1. Declaration Capture

This part is mostly feasible from AST.

Likely straightforward:

- `class_template`
- `variable_template`
- `alias_template`
- `partial_class`
- `partial_variable`
- `explicit_class`
- `explicit_variable`
- `function_template`

Clang AST already exposes:

- primary template declarations
- partial specialization declarations
- specialization kind
- template parameter lists
- default template arguments
- instantiated template arguments
- alias target type

Main issues:

- the current `.tkq` type grammar is much smaller than Clang `QualType`
- non-type template arguments in real C++ are much richer than current
  `.tkq` value support
- current `.tkq` has only a tiny requirement DSL (`when same(...)`)

So declaration capture is possible only if one of these is true:

- we greatly expand `.tkq`
- we accept a lossy seed format
- we add a reducer that maps richer C++ forms into kernel idioms

### 2. Query Capture

Also mostly feasible from AST.

The current plugin can already recover source-level queries for:

- class specialization uses
- variable template references
- alias expansions
- resolved function-template call sites

So it could emit a query seed roughly equivalent to:

- `query select_class ...`
- `query select_variable ...`
- `query expand_alias ...`
- `query deduce_call ...`

This is the strongest part of the current approach.

### 3. Final Bindings

This is partially feasible.

The plugin can already emit final bound template arguments for the winner.

But there are real gaps:

- function-template defaults are not always distinguishable cleanly from
  deduced / substituted results from stock AST alone
- alias default binding is not fully reconstructed from the current AST-only
  pass
- pack behavior is visible in Clang, but the current `.tkq` output model does
  not carry enough structure to lower it automatically in general

### 4. Losing Candidates And Drop Reasons

This is the biggest blocker.

The current `.tkq` contract frequently wants:

- the losing candidate
- a stable reason such as:
  - `requirement_not_satisfied`
  - no match
  - ambiguity
  - substitution failure

Stock AST after successful overload resolution does **not** retain that full
candidate story.

Evidence from the current prototype and case
[479-dependent-variable-template-empty-pack-enable-if-selection.t](/Users/vishvananda/cppgm/pa22/tests/spec/479-dependent-variable-template-empty-pack-enable-if-selection.t):

- Clang plugin sees the winning `traits<int>::construct`
- our compiler trace sees both candidates and the failed `enable_if` path
- kernel output expects the loser and the reason

That missing loser path is exactly what a generated `.tkq` needs for current
function-template reduction cases.

## What Public Clang APIs Actually Give Us

### Available To A Stock Plugin

Useful public surfaces exist:

- `ASTConsumer` / `SemaConsumer`
  - the plugin can receive `Sema` via `InitializeSema`
- AST declarations and specialization info
- public `Sema` lookup / overload helpers
  - `LookupName`
  - `LookupQualifiedName`
  - `CreateUnresolvedLookupExpr`
  - `buildOverloadedCallSet`
  - `BuildOverloadedCallExpr`
- public overload / deduction data structures
  - `OverloadCandidateSet`
  - `OverloadCandidate`
  - `DeductionFailureInfo`
  - `TemplateDeductionInfo`

This is enough to say:

- reconstructing more of the candidate story is not conceptually impossible

### But There Are Important Limits

The candidate set during successful resolution is mostly **transient**.

For a stock plugin, recovering it later would require re-running parts of Sema
using reconstructed lookup state.

Problems:

- many resolved `CallExpr` nodes no longer retain the original unresolved
  lookup set
- unqualified lookup depends on `Scope *`, which is not preserved in the final
  AST
- ADL reconstruction is nontrivial and context-sensitive
- member and operator cases add extra lookup machinery
- reproducing the exact candidate set after the fact risks divergence from what
  Clang actually used in the original pass

So while `OverloadCandidateSet` is publicly visible, the exact inputs needed to
rebuild it are not always retained in a plugin-friendly form.

### Template Instantiation Callbacks

Clang does have `TemplateInstantiationCallback` support in the public headers.

That is promising for:

- instantiation begin/end tracing
- richer witness output

But it does **not** by itself solve overload candidate capture.

Also, in the installed public headers inspected here:

- `Sema` stores `TemplateInstCallbacks`
- but there is no obvious public registration API exposed for a plugin to add
  one cleanly

So using that path from a stock plugin likely requires:

- a Clang patch, or
- a non-public hack that we should not build on

## Mapping Current `.tkq` Features To Feasibility

### Likely Auto-Generatable From Stock Plugin

- `class_template`
- `variable_template`
- `alias_template`
- `partial_class`
- `partial_variable`
- `explicit_class`
- `explicit_variable`
- basic `function_template` declarations
- `select_class`
- `select_variable`
- `expand_alias`
- simple `deduce_call` winner queries

### Auto-Generatable Only With Heuristics Or A Richer IR

- defaulted-vs-deduced classification for all function-template arguments
- pack-heavy signatures
- structural lowering into `arr<...>` and `fn<...>`
- conversion from real type spellings into the current bounded `.tkq` type
  grammar
- lowering alias / trait / `enable_if` scaffolding into smaller kernel
  declarations

### Not Reliably Auto-Generatable From Stock Plugin Alone

- losing candidates for successful calls
- stable drop reasons for those losers
- reduction into current `when same(...)` requirements from arbitrary C++
  boolean / trait / alias machinery
- faithful shrinking of a complex hosted/STL case into one or more small
  kernel cases

## Why Reduction Is Harder Than Observation

Even if the plugin captured every winning declaration and every loser
candidate, it would still not know how to produce the **right reduced `.tkq`**
without a reducer.

Example:

```cpp
template<class Tp, class... Args,
         enable_if_t<has_construct_v<Alloc, Tp*, Args...>, int> = 0>
static int construct(...);
```

Current kernel reduction:

```txt
function_template construct enabled
  <value bool Can, value int Gate = value 0>
  params (type constructible<Can>)
  return int
  when same(Can, true)
```

That transformation is not an AST dump.

It introduces:

- a latent boolean variable `Can`
- a normalized sentinel `Gate`
- a synthetic `constructible<Can>` shape
- a simplified requirement `when same(Can, true)`

Those are reduction choices.

A Clang plugin can witness the original source behavior. It cannot infer that
this is the uniquely correct pedagogical abstraction without a separate
reduction pass.

## What Would Have To Change To Make “Whole `.tkq`” Work

There are three viable directions.

### Option A: Keep Current `.tkq`, Add A Real Reducer

Pipeline:

- source C++
- Clang plugin emits rich witness JSON
- reducer chooses a minimal owned mechanism
- reducer emits final `.tkq`

What is needed:

- richer witness than the current plugin output
- candidate-set reconstruction or Clang patch for loser info
- a substantial normalization / reduction engine

This keeps the assignment language small, but the reducer becomes real
compiler-engineering work.

### Option B: Expand `.tkq` Into A Much Richer Structural IR

Instead of insisting on today’s tiny kernel surface, grow it to encode much
more of Clang’s directly observable world:

- richer types
- richer value arguments
- explicit boolean / trait expressions
- packs
- more direct requirement expressions
- maybe direct “candidate” records

Then the plugin could emit something much closer to source facts.

This makes generation easier, but it pushes `.tkq` toward a full template IR
rather than a small teaching kernel.

### Option C: Patch Clang

Add explicit callbacks / trace emission for:

- overload candidate enumeration
- template deduction result per candidate
- final chosen candidate
- substitution-failure detail
- constraint satisfaction / failure detail

Then the plugin or patched frontend could emit a richer witness that is close
enough for a reducer to be trustworthy.

This is the cleanest route for exactness, but it stops being a stock plugin
solution.

## Recommended Interpretation

The plugin should be viewed as a **witness and seed generator**, not as the
final `.tkq` generator.

Recommended staged plan:

1. expand the plugin from student-facing JSON into a richer witness format
   that includes:
   - declarations
   - uses
   - winner bindings
   - when possible, reconstructed candidate sets

2. introduce `tkq_seed_v1`
   - mechanical lowering from witness JSON
   - no claim of minimality

3. build a reducer from `tkq_seed_v1` to final `.tkq`
   - this is where pedagogical reduction happens

4. only if needed, patch Clang for exact loser / deduction data

## Recommendation For Current Work

The next practical step is **not** “generate whole `.tkq` directly”.

It is:

- add a richer witness schema
- add `tkq_seed` emission for the easy subset
- define a normalization comparison between:
  - source witness
  - `tkq_seed`
  - final reduced `.tkq`

That gives us a path to higher confidence without pretending the current plugin
can solve reduction by itself.

## Bottom Line

To expand the plugin to generate the whole current `.tkq`, we would need:

- more than the current AST-only witness
- some access to candidate and deduction failure data beyond what successful
  AST nodes preserve
- a real reducer from C++ facts to kernel abstractions

So:

- **plugin-only, stock Clang, direct final `.tkq`**: not realistic for the
  full interesting test set
- **plugin + richer witness + reducer**: realistic
- **patched Clang + richer witness + reducer**: the strongest route if we want
  high-fidelity automation for complex cases
