## CPPGM Programming Assignment 11 (`cppgm++ --emit-lowir`)

### Overview

PA11 is the first object-model milestone. It extends the PA10 procedural compiler with the
basic non-polymorphic class machinery needed by ordinary C++ code:

- class layout and object size/alignment
- member lookup and access control
- `this`, `.` and `->`
- ordinary non-template operator overloading that stays within the PA11 object-model subset
- non-virtual methods
- constructors and destructors
- object lifetime for locals and namespace-scope objects
- single inheritance without virtual dispatch

### Prerequisites

You should complete Programming Assignment 10 before starting this assignment.

You will want to reuse:

- the preprocessing and tokenization pipeline from PA1-PA4
- the PA5 AST as the syntax boundary
- the PA6 declarator/type model and class syntax preservation
- the PA7/PA10 resolved procedural and LowIR lowering path
- the PA8 LowIR contract

The intended direction is:

- PA5 provides syntax
- PA6 provides scope/type lookup and complete type metadata
- PA7 resolves the procedural expression subset
- PA10 lowers that resolved procedural subset into LowIR
- PA11 extends that lowering path into a usable basic object model

Use PA5's recorded name categories and PA6's actual declarations to resolve
type names. Class names do not require a special spelling prefix.

### Starter Kit

The starter kit contains:

- the student-editable `../dev/cppgm++.cpp` entry point, initially seeded from the course
  `cppgm++` scaffold and reached from this directory through the `cppgm++.cpp` symlink
- shared `../dev/` and `../dev/src/` support code from the earlier compiler pipeline
- the grammar for this assignment called `pa11.gram`
- an HTML grammar explorer of `pa11.gram` in the sub-directory `grammar/`
- a checked-in local test suite under `tests/`

Extend the driver and frontend you implemented in earlier assignments with
the PA11 lowering behavior.

Use the supplied reference tools to inspect example output. Tests compare
your compiler with the checked-in contract references.

### Input / Command-Line Arguments

The same as PA10 `cppgm++ --emit-lowir`. The PA11 test mode is unoptimized LowIR
generation. `make test` passes `--emit-lowir -O0` through the harness, so individual test
files do not spell those flags themselves.

Behaviour is undefined unless the command-line arguments match:

    $ cppgm++ --emit-lowir -O0 -o <outfile> <srcfile1> <srcfile2> ... <srcfileN>

with the same relaxations as PA10.

Accepting `--emit-lowir` without an explicit `-O0` as the same unoptimized mode is fine,
but optimized LowIR output is not part of PA11.

### Output Format

`cppgm++` shall write LowIR text to `<outfile>`.

The authoritative LowIR definition is `../pa8/lowir.md`. PA11 extends the PA10 procedural
subset of that IR with the object-model lowering needed by this milestone.

When PA11 emits function-boundary metadata such as `unwind=no`, treat that as a
truthful emitted fact, not as a promise that every semantically equivalent C++
exception specification is normalized. The direct `noexcept` form on free
functions, member functions, constructors, and destructors is in scope for the
tested metadata path. Other explicit `noexcept(expr)` forms may lower
conservatively without `unwind=no`.

A call to `__builtin_unreachable()` lowers directly to the PA8
`unreachable` block terminator. It does not emit a synthetic function
declaration or call.

The backing array for each C++ string literal is emitted as an internal
structured global with `storage=readonly`.  This records the source array's
const element contract in serializable LowIR; an ordinary writable character
array must not gain that storage property merely because it has a static
initializer.

PA11 writes a single concatenated LowIR program consisting of:

- zero or more `global` definitions
- zero or more `function` definitions

LowIR top-level declaration/definition order is a presentation convention, not
a dependency order. Reference outputs and canonical dumps use the order defined
in `../pa8/lowir.md`: `declare global`, `declare function`, `global`, then
`function`, but the relaxed LowIR comparison canonicalizes top-level entries
before comparison. Your output must still be repeatable for the same
inputs; `../pa8/lowir.md` defines the canonical reference presentation and
notes where internal LowIR symbol names are only a presentation tie-breaker.
Your output must also preserve order-sensitive LowIR regions when they are present: instruction order inside
blocks, item order inside structured globals, vtable slot order, and action
order inside generated initialization, finalization, constructor, destructor,
and cleanup bodies.

For non-static member functions, call analysis should treat the object expression as an
implicit object argument. Member lookup gathers the candidate methods; overload resolution
then checks and ranks those candidates using the cv-qualification of the object expression.
After a non-static member function is selected, the generated LowIR uses an explicit hidden
first parameter for the object pointer (`this`).

Namespace-scope object lifetime is represented through synthetic startup/shutdown helpers when
needed:

- `@__cppgm_init`
- `@__cppgm_fini`

Synthesized constructors and destructors are part of the PA11 semantic model, but `cppgm++`
only needs to emit the helper definitions that the lowered program actually requires. Unused
implicit default constructors / destructors do not need to appear in the PA11 LowIR output.
In practice, PA11 only needs ctor/dtor helpers for the supported declaration-time, member-
initializer, recursive subobject, and namespace-scope lifetime paths. Copy/value helpers do
not belong in PA11 output.

The generated LowIR is intended to become input for the later PA24
`lowir2native` backend, which will execute these helpers around `@main`. That
future native path is not the PA11 grading contract.

The checked-in `.ref` files define the required LowIR facts for the tests. The
test harness checks exit status, LowIR well-formedness, and the
course-defined normalized LowIR output rather than requiring students to match every
non-semantic helper spelling or presentation choice.

### Error Handling

If an error occurs during preprocessing, tokenization, parsing, semantic analysis, or LowIR
generation, `cppgm++` shall `EXIT_FAILURE`.

The output file is not required to be meaningful on failure.

### Standard Output / Error

Standard output and standard error are ignored for automated testing of `cppgm++`.

You are free to use them for debugging, tracing, or diagnostic messages.

### Testing

Tests compare your output with the checked-in references using the LowIR
comparison described in [the format guide](../pa8/lowir.md).

For each test case `x`:

- `cppgm++` is executed to produce `x.my`
- the exit status is recorded in `x.my.exit_status`
- `x.my` is validated as LowIR and compared against `x.ref` using the normalized
  LowIR comparison
- `x.my.exit_status` is compared against `x.ref.exit_status`

`make test` runs the checked-in local suite under `tests/` and supplies
`--emit-lowir -O0` through the harness.

The PA11 suite is split by test role:

- `tests/general/`: the default PA11 LowIR oracle suite. These tests cover object-model
  lowering, class layout, lifetime, helper emission, and cross-feature cases
  whose primary contract is the generated LowIR plus exit status.
- `tests/spec/`: focused C++ language-contract cases that cite a specific N3485 clause.
  Each source test in this directory starts with a comment of the form:

    // N3485 focus: <clause> [<stable-name>] <short topic>

`tests/spec/` covers the PA11 class/object contract: class layout, access
control, nested names, static members, aggregate and reference initialization,
friends/ADL, single inheritance, lifetime, bit-fields, pseudo-destructors,
ordinary non-template operators, standard `alignas` / `alignof`, and inheriting
constructors. `tests/general/` covers object-model and LowIR-shape cases that
are not tied to one specific C++11 clause.

### PA11 Syntax Spec

The authoritative source syntax is the shared `cppgm++` source grammar, exposed
for this assignment as `pa11.gram`. The grammar defines accepted syntax only;
the PA11 semantic and lowering requirements are defined by the Assignment
Boundary and Out Of Scope sections below.

As in the earlier assignments, that grammar defines accepted input syntax only. The output
format for `cppgm++` is specified by this README, PA8 `lowir.md`, and the checked-in
`.ref` files.

Because PA11 extends PA10 rather than adding a new syntax layer, PA11 gives the
class/object subset described below semantic and lowering meaning.

Passing PA10 is necessary but not sufficient for passing PA11: an input may be syntactically
valid for PA5 and code-generation-valid for PA10 and still be outside the PA11 class/object
slice described below.

A checked-in HTML grammar explorer for that grammar lives in `grammar/`. Treat
`pa11.gram` as the source of truth.

`pa11.gram` uses the same token vocabulary and the same extended BNF operators as
`../shared/source.gram`.

If this README and `pa11.gram` appear to disagree about source syntax, treat `pa11.gram`
as authoritative. If this README and PA8 `lowir.md` appear to disagree about LowIR syntax,
treat `lowir.md` as authoritative. If they disagree about the PA11 lowering slice, treat the
`Assignment Boundary` and `Out Of Scope` sections below as authoritative.

### Assignment Boundary

PA11 supports the following in addition to the PA10 procedural subset:

- namespace-scope and nested-namespace class/struct definitions and forward declarations
- access control for classes, fields, methods, nested types, and static members
  in the current non-virtual class model
- nested class/type declarations and lookup
- static data members and static member functions over the supported scalar and
  class subset
- complete object layout for non-static data members in declaration order, including:
  - empty classes
  - alignment and padding, including preservation of a stronger requested
    class alignment while laying out a direct base and members
  - ordinary integral and enum bit-fields, including zero-width unnamed separators
  - self-referential pointer members
  - previously completed class-type members
- single inheritance with the direct base subobject at offset `0`
- member lookup for:
  - direct fields
  - inherited fields
  - direct methods
  - inherited methods
- `this`, implicit member lookup inside methods, and member access expressions `.` and `->`
- non-static member-function calls selected through overload resolution with the implicit
  object argument described above
- ordinary non-template operator overloading over the supported object-model subset, including:
  - member operators such as `operator[]`
  - hidden-friend and namespace-scope non-member operators found through ordinary lookup / ADL
  - chained reference-returning operators such as `operator<<`
  - rejection of a non-member overloaded operator unless at least one operand has class or
    enumeration type
- ordinary non-template non-member function calls found through associated-namespace lookup /
  hidden-friend ADL when the arguments stay within the supported class subset
- in-class member-function definitions
- out-of-class definitions for ordinary non-static member functions when the parser accepts
  them as ordinary qualified function definitions, including a leading return type that
  names a private nested type in the member's class context
- constructors and destructors defined inside the class body
- implicit default constructors and destructors when no user-declared one exists
- semantically trivial constructor/destructor actions may be omitted directly;
  if a retained helper must be substituted, use the ordinary mandatory inline
  policy rather than a separate lifecycle label, let `no_inline` take
  precedence for that helper, and decide `object_root` retention independently
- demand-driven LowIR emission of the ctor/dtor helpers required by the supported lifetime
  paths above
- constructor initializer lists for:
  - the single direct base
  - non-static data members
- non-static default member initializers for the supported scalar and supported
  class/aggregate subobject construction forms, with explicit constructor member-initializers
  taking precedence
- local-class default member initializers that use an enclosing integral
  constant expression without odr-using the enclosing automatic object
- aggregate initialization for the supported PA11 object subset, including namespace-scope
  aggregate arrays whose elements contain string-literal pointer members
- local and namespace-scope class object lifetime:
  - constructor execution at declaration time / program startup
  - destructor execution at block exit, `return`, loop exit, and program shutdown
  - a `goto` that leaves one or more active object scopes destroys those
    objects in reverse construction order before transferring control; a
    backward `goto` within one scope likewise destroys objects initialized
    after the target label before reconstructing them on the next pass
  - per-thread initialization for namespace-scope `thread_local` class objects,
    with collision-free internal wrapper, guard, and initializer symbols
- shared LowIR cleanup continuations for equal lexical destructor suffixes;
  return paths may converge on a continuation only after preserving the return
  value and only when the destructor sequence and enclosing control context are
  identical
- one `zeroinit` operation for value-initialization that already identifies an
  exact contiguous nonvolatile, non-union object or subobject span; explicit
  initializer actions and union, volatile, lifetime, or side-effect boundaries
  remain separate
- recursive member/base construction and destruction for supported class-type subobjects
- anonymous struct/union members, including injected member lookup and layout in
  the supported class subset
- bit-field member access, assignment, initializer, and built-in increment/decrement
  lowering; reads of explicitly signed integral and signed-underlying enum bit-fields
  preserve the represented negative value, and built-in address-of rejects bit-fields
- pseudo-destructor and explicit destructor-name syntax over supported scalar
  and class expressions
- standard `alignas` and `alignof`, including rejection of a requested class
  alignment weaker than its natural alignment
- inheriting constructors through `using Base::Base`
- use of complete class types in:
  - `sizeof(type-id)`
  - `sizeof(expr)`
  - local object declarations
  - namespace-scope object declarations

Within this milestone, PA11 should produce valid LowIR for ordinary
non-polymorphic class code over the supported procedural subset. That LowIR is
intended to be accepted by the later PA24 `lowir2native` backend for the
supported cases.
### Out Of Scope

The following are explicitly out of scope for PA11:

- virtual functions, virtual inheritance, vpointers, and vtables
- RTTI and `dynamic_cast`
- copy/move construction and assignment
- pass-by-value and return-by-value of class objects
- temporary class-object materialization beyond the supported declaration/constructor path
- eager emission of unused constructor/destructor helpers
- operator overloads that require later value semantics, especially by-value class transfer and
  copy/move assignment operators
- template-backed operator overloads
- multiple inheritance
- member pointers
- out-of-class constructor and destructor definitions
  - PA5 preserves their syntax; PA12 adds their semantics and lowering
- conversion operators
- static assertions and constexpr metaprogramming
- hosted/vendor-only attributes such as `[[no_unique_address]]`
- broader C++ object-model corners such as advanced special-member generation rules

Inputs that rely on those features have undefined behaviour for this milestone.

### Stage Handoff

The intended next stages are:

- PA12: add the non-polymorphic value-semantics layer that PA11 intentionally stops short
  of:
  - copy/move behavior in the common cases
  - pass-by-value and return-by-value of class objects
  - demand-driven copy/value helper emission when those source forms are actually used
  - the assignment-operator and by-value operator cases that depend on that value-semantics work
- PA13: add the polymorphic machinery that is still intentionally absent after PA11:
  - virtual dispatch
  - vtables and override/final behavior
- PA14: add template-backed overload participation, including templated operator overloads,
  on top of the PA11-PA13 non-template object model

So PA11 should leave behind a usable non-virtual object model and a clean extension point for
the later PA12 value-semantics work and the later PA13 polymorphic work, rather than mixing
those harder features into the basic class milestone.

### Design Notes (Non-Normative)

The cleanest reuse path is to keep the PA10 procedural LowIR lowering model and extend it
rather than building a second backend just for classes.

Useful intermediate representations include:

- complete named types with stable size/alignment metadata
- class metadata that preserves fields, direct base, access, nested names, static
  members, friends, and member-function bindings
- one shared layout service for ordinary fields, bit-fields, anonymous members,
  and alignment directives
- resolved member expressions and method calls over the same call-semantics IR shape used by
  PA7/PA10
- explicit constructor/destructor actions attached to declarations or generated function bodies
  so lifetime can be lowered incrementally instead of requiring a separate runtime model
- compact cleanup-state identities formed from an action, its tail, its terminal
  continuation, and its control context; interning those identities while
  lowering avoids copying or repeatedly comparing complete destructor sequences
- exact typed size/alignment and volatile/union-containment layout facts can
  select a contiguous `zeroinit` at the initialization site without rescanning
  emitted instructions
- demand-driven helper emission keyed by semantic entities rather than source
  spelling, so unused constructors/destructors do not perturb earlier outputs
