# Clang Template Witness Touchpoints

This note summarizes how broad the Clang-side implementation became while
prototyping a student-facing `--emit-templates`-style witness.

## Student-Facing Surface

The current prototype emits these event kinds:

- `alias-use`
- `class-use`
- `variable-use`
- `function-call`

For those events, the intended public fields are:

- source location
- selected template / callee
- resolved specialization or expanded alias
- selected specialization kind (`primary`, `partial`, `explicit`, `instantiation`)
- selected declaration location
- bindings
- specialization bindings for partials
- dropped candidates with coarse reasons when Clang exposes them cheaply

## Files Touched

Broad student-surface coverage currently touches four Clang files:

1. `clang/lib/Frontend/FrontendAction.cpp`
2. `clang/lib/Sema/SemaOverload.cpp`
3. `clang/lib/Sema/SemaInit.cpp`
4. `clang/include/clang/Basic/TemplateWitness.h`

That file count is deceptively small. The real spread is in the number of
distinct semantic entry points inside `SemaOverload.cpp`.

## Centralized AST-Side Surface

These winner/selection events are concentrated in one place:

- `VisitTemplateSpecializationTypeLoc`
  - owns `class-use`
  - owns `alias-use`
- `VisitDeducedTemplateSpecializationTypeLoc`
  - owns CTAD `class-use`
- `VisitDeclRefExpr`
  - owns `variable-use`
- `VisitCallExpr`
  - owns final `function-call` winner / bindings
- `VisitCXXConstructExpr`
  - owns constructor-template `function-call`
- `VisitVarDecl`
  - owns CTAD fallback for declarations where the type syntax has no explicit
    template-id but the variable initializer is a class-template construction

This is the encouraging part. A large amount of the student-facing surface is
recoverable from one AST visitor layer.

## Distributed Sema-Side Surface

Drop reasons are not concentrated in the AST. They are only available while
Clang still has an `OverloadCandidateSet`.

The current prototype had to patch these overload-resolution entry points:

1. `Sema::BuildOverloadedCallExpr`
   - free-function / ordinary overload calls
2. `Sema::BuildCallToMemberFunction`
   - unresolved member-call overload resolution
3. `Sema::CreateOverloadedUnaryOp`
   - overloaded unary operators
4. `Sema::CreateOverloadedBinOp`
   - overloaded binary operators, including `operator=`
5. `Sema::CreateOverloadedArraySubscriptExpr`
   - overloaded `operator[]`
6. `Sema::BuildCallToObjectOfClassType`
   - overloaded `operator()`
7. `Sema::BuildOverloadedArrowExpr`
   - overloaded `operator->`
8. `Sema::BuildLiteralOperatorCall`
   - literal operator templates

And one additional initialization-time entry point:

9. `Sema::DeduceTemplateSpecializationFromInitializer`
   - class template argument deduction / deduction-guide selection

So the real answer is not “three files” but “one AST file plus roughly eight
separate semantic call/overload hooks.”

## Current Coverage Result

What works well now:

- `class-use`
- `alias-use`
- `variable-use`
- `function-call` winners
- `function-call` loser/drop reasons for the important template-owned overload
  paths we checked
  - confirmed on `479-dependent-variable-template-empty-pack-enable-if-selection.t`
  - confirmed on `542-local-functor-std-function-assignment.t`
- constructor-template selection
  - confirmed on `431-forwarding-reference-constructor-template.t`
- CTAD as `class-use` plus attached deduction-guide data
  - guide winner
  - guide decl
  - guide candidate counts
  - guide drops
  - confirmed on a synthetic `pair_like p(1, 2)` probe
- conversion-function template selection when the conversion itself is a
  function template
  - confirmed on a synthetic `Box<int> -> double` probe
- overloaded `operator()` template selection
  - confirmed on a synthetic call-operator probe

What still looks imperfect:

- ordinary non-template conversion operators are intentionally not surfaced as
  `function-call`
  - they are not template decisions, even when they live inside a class
    template specialization
- some variable-template uses are still best-effort because Clang does not
  always preserve a clean source-side use node once the specialization is
  realized

## Remaining Surface Classification

The useful design question is not “what else could Clang emit?” but “which of
those remaining facts belong in a student-facing template layer?”

### Bucket 1: Worth Exposing In `--emit-templates`

These are stable semantic decisions that a student template layer should own.

- `alias-use`
  - template name
  - resolved alias-id
  - declaration location
  - expanded target type
  - bindings
- `class-use`
  - template name
  - resolved specialization-id
  - selected kind (`primary`, `partial`, `explicit`)
  - declaration location
  - bindings
  - partial-specialization bindings
- `variable-use`
  - same shape as `class-use`
- `function-call`
  - selected callee
  - selected declaration location
  - final template arguments / bindings
  - coarse loser/drop reasons
- future normalized call-like events that should probably still print as
  `function-call`
  - constructor template selection
  - conversion-function template selection
  - deduction-guide selection, attached to CTAD `class-use`
  - overloaded operator selection
  - overloaded `operator()` / `operator[]`

For drop reasons, the public surface should stay coarse and portable:

- `substitution_failure`
- `constraints_not_satisfied`
- `deduced_mismatch`
- `too_many_arguments`
- `too_few_arguments`
- `bad_conversion`
- `worse_conversion`
- `ambiguous`

Those map to real student-owned behavior without freezing the exact frontend
internals.

### Bucket 2: Maintainer-Only / Hidden Oracle Surface

These are useful for validation and reduction work, but should not define the
assignment contract.

- exact Clang failure kinds
  - for example the full `OverloadFailureKind` / `TemplateDeductionResult`
    space
- exhaustive candidate inventories
- candidate counts
- all loser declaration locations even when the student contract only needs the
  winner and one or two meaningful drops
- symbol/file filters and other trace-control knobs
- JSON summary counters
- internal normalization metadata
- per-candidate conversion diagnostics
- full partial-ordering tie-break traces
- “why this was even in the candidate set” provenance

This information is valuable when validating reductions against real source
tests, but it is too detailed and too implementation-specific for a student
assignment.

### Bucket 3: Too Clang-Architecture-Specific

These should generally not leak into the student architecture at all.

- `BoundMemberTy` / `OverloadTy`
- `OverloadCandidateSet::CandidateSetKind`
- `FoundDecl` / `UsingShadowDecl` plumbing
- rewritten/reversed candidate machinery
- surrogate call candidates
- implicit-object placeholder details
- address-of-overload-set handling
- deferred-candidate bookkeeping used only to support Clang’s internal control
  flow
- `operator->` recursion details
- multiversion / module / CUDA / ObjC-specific overload failure mechanics

If one of these affects the final public result, the student-facing contract
should expose only the normalized semantic consequence, not the mechanism.

## Remaining Gaps By Bucket

### Public Gaps Worth Chasing

- deferred/dependent “not selected yet” states if PA22 needs them explicitly
- only if later assignments demand them:
  - conversion-function template loser sets in more exotic initialization paths
  - CTAD/deduction-guide edge cases beyond the current source-anchored class-use
    surface

### Useful But Probably Hidden

- exact mapping from a loser to the overload-resolution phase that rejected it
- candidate-order snapshots for reduction debugging
- richer disambiguation between “bad conversion” and “not even comparable”

### Probably Not Worth Modeling Publicly

- separate public event kinds for every syntactic expression form
  - `operator=` should still normalize to `function-call`
  - `operator()` should still normalize to `function-call`
  - `operator[]` should still normalize to `function-call`
- frontend-specific distinctions between member-reference resolution and call
  resolution

## Architectural Takeaway

This Clang result should not be read as “a student compiler must also scatter
template logic everywhere.”

What it actually shows is:

- winner-oriented template facts are fairly centralized
- loser/drop facts are centralized around *the concept* of overload candidate
  sets, but Clang reaches that concept through many expression-form-specific
  entry points
- deduction-guide / CTAD facts are a separate initialization-time seam, not an
  overload-call seam

So for `cppgm`, a dedicated `--emit-templates` layer is still a good idea.
The Clang patch count reflects Clang’s interleaved parser/Sema architecture
more than it argues for a fragmented student design.
