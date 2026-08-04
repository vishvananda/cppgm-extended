## CPPGM Programming Assignment 25 (`cppgm++ --emit-lowir`)

### Overview

Write a C++ application called `cppgm++` that takes as input a set of C++ Source
Files, executes translation phases 1 through 7, parses them as PA10/PA25 translation units,
reuses the PA11-PA12 semantic foundation, builds on the PA14-PA24 LowIR lowering path,
adds the PA25 advanced-language slice, and writes LowIR text.

PA25 finishes the deferred first-tier language features that sit on top of the existing
single-inheritance object model:

- capturing lambdas
- `std::initializer_list` semantic interoperation
- RTTI and `typeid`
- pointer-form `dynamic_cast`

PA25 still produces LowIR. It does not introduce a new output format.

### Prerequisites

You should complete Programming Assignment 24 before starting this assignment.

You will want to reuse:

- the preprocessing and tokenization pipeline from PA1-PA6
- the PA10 AST as the syntax boundary
- the PA11-PA12 semantic foundation
- the PA14-PA24 LowIR lowering path
- the PA13 LowIR contract
- the PA28 native validation path
- the PA13 LowIR -> CY86 path as an optional secondary scaffold

### Starter Kit

The starter kit contains:

- `pa25/README.md`, `pa25/Makefile`, and the test scripts in `pa25/scripts/`
- a student-editable `dev/cppgm++.cpp` starter scaffold
- the `pa25/cppgm++.cpp` symlink back to `../dev/cppgm++.cpp`
- shared support sources and headers under `dev/src/`
- a local test suite under `pa25/tests/`
- the grammar for this assignment called `pa25.gram`
- an HTML grammar explorer of `pa25.gram` in the sub-directory `grammar/`
- a checked-in local test suite under `tests/`

Students should implement the assignment in `dev/cppgm++.cpp` and any reusable
student-owned helpers they add under `dev/src/`. The assignment directory, grammar files,
test fixtures, comparison scripts, and checked-in reference outputs are support
files, not implementation files to edit for normal solutions. The shared support files
provide reusable infrastructure and earlier assignment machinery; they do not implement the
new PA25 source-to-LowIR language slice for you.

Unlike PA1-PA9, there is no external reference binary for PA25. The checked-in `.ref`
files are the default oracle.

### Input / Command-Line Arguments

Behaviour is undefined unless the command-line arguments match:

    $ cppgm++ --emit-lowir -O0 -o <outfile> <srcfile1> <srcfile2> ... <srcfileN>

`-O0` is the PA25 test mode. Other optimization levels are later optimizer work and
are not required for this milestone.

### Output Format

`cppgm++` shall write LowIR text to `<outfile>`.

The authoritative LowIR definition is `../pa13/lowir.md`. PA25 extends the PA24 lowering
surface only by making more of the C++ source language lower into the already-defined LowIR
family.

LowIR top-level declaration/definition order is a presentation convention, not
a dependency order. Reference outputs and canonical dumps use the order defined
in `../pa13/lowir.md`: `declare global`, `declare function`, `global`, then
`function`, but the relaxed LowIR comparison canonicalizes top-level entries
before comparison. Your output must still be repeatable for the same
inputs; `../pa13/lowir.md` defines the canonical reference presentation and
notes where internal LowIR symbol names are only a presentation tie-breaker.
Your output must also preserve order-sensitive LowIR regions when they are present: instruction order inside
blocks, item order inside structured globals, vtable slot order, and action
order inside generated initialization, finalization, constructor, destructor,
and cleanup bodies.

The generated LowIR must be well-formed and must match the checked-in `.ref` files under
the relaxed LowIR comparison used by the harness. That comparison still checks the
semantic LowIR shape and required IR facts, but it does not make helper metadata
presentation or other non-semantic text details part of the student contract.

### Error Handling

If an error occurs during preprocessing, tokenization, parsing, semantic analysis, or LowIR
generation, `cppgm++` shall `EXIT_FAILURE`.

The output file is not required to be meaningful on failure.

### Standard Output / Error

Standard output and standard error are ignored for automated testing of `cppgm++`.

You are free to use them for debugging, tracing, or diagnostic messages.

### Testing

Testing uses checked-in golden outputs, not a reference binary. The `Makefile` invokes
`cppgm++` with `--emit-lowir -O0`.

The local checked-in tests live in `tests/general/`. That directory contains
PA25 source-to-LowIR tests for capturing lambdas, initializer-list
interoperation, RTTI, `typeid`, `dynamic_cast`, and exception-source lowering
interactions. PA25 has no `tests/spec/` directory because these tests focus on
the combined language-to-LowIR contract.

For each test case `x`:

- `cppgm++` is executed to produce `x.my`
- the exit status is recorded in `x.my.exit_status`
- `x.my` is compared against `x.ref`
- `x.my.exit_status` is compared against `x.ref.exit_status`

PA25 is tested against generated LowIR text using the relaxed LowIR comparator described
above. A useful manual validation path is:

- feed that LowIR into PA28 `lowir2native`
- optionally cross-check by feeding that same LowIR into PA13 `lowir2cy86`
- then feed the generated CY86 into PA9 `cy86 --target linux`

The shipped PA25 tests are the contract for this milestone.

### PA25 Syntax Spec

The authoritative source syntax is the shared `cppgm++` source grammar, exposed
for this assignment as `pa25.gram`. The grammar defines accepted syntax only;
the PA25 semantic and lowering requirements are defined by the Assignment
Boundary and Out Of Scope sections below.

As in the earlier assignments, that grammar defines accepted input syntax only. The output
format for `cppgm++` is specified by this README, PA13 `lowir.md`, and the
checked-in `.ref` files.

PA25 does not add a new source-language grammar format. It instead enables more
of the already-accepted C++11 syntax to participate in semantic analysis and
lowering.

A checked-in HTML grammar explorer for that grammar lives in `grammar/`. Treat
`pa25.gram` as the source of truth.

`pa25.gram` uses the same token vocabulary and the same extended BNF operators as
`../pa6/pa6.gram`.

If this README and `pa25.gram` appear to disagree about source syntax, treat `pa25.gram`
as authoritative. If this README and PA13 `lowir.md` appear to disagree about LowIR syntax,
treat `lowir.md` as authoritative. If they disagree about the PA25 lowering slice, treat the
`Assignment Boundary` and `Out Of Scope` sections below as authoritative.

### Assignment Boundary

PA25 supports the following in addition to the PA24 subset:

- capturing lambdas with supported explicit by-copy and by-reference captures of local
  values, including class objects whose existing copy-construction path is supported
- default `[=]` and `[&]` captures over the same supported local-value and `this` subset
- explicit `this` capture for supported member-function cases
- `std::initializer_list<T>` interoperation for supported scalar elements and
  class elements whose construction, copy, and destruction stay within the
  PA15/PA16/PA23 object and template subset
- `typeid(type-id)`
- `typeid(expr)` for supported polymorphic lvalue expressions
- `dynamic_cast<T*>(expr)` for supported polymorphic single-inheritance pointer conversions

Within this milestone, PA25 should produce valid LowIR for ordinary source programs over
that subset. That LowIR should be accepted by PA28 `lowir2native` for the supported cases.
PA13 `lowir2cy86` remains a secondary scaffold backend for cross-checking.

To complete PA25, implement these goals:

1. Capturing lambda lowering.
   Explicit by-copy captures should materialize deterministic closure-object LowIR and the
   resulting closure object should be callable through the existing class/method lowering
   path. A catch parameter declared inside a lambda body is local to that body and is not an
   implicit capture.

2. `std::initializer_list` interoperation.
   Supported braced-list calls should materialize deterministic lowered storage and expose
   the expected `__begin` / `__size` semantics to range-for lowering.

3. RTTI and `typeid`.
   The compiler should emit deterministic RTTI globals and lower both static and dynamic
   `typeid` queries into ordinary LowIR address/load/branch operations.

4. Pointer-form `dynamic_cast`.
   The compiler should lower supported polymorphic single-inheritance pointer casts into
   ordinary LowIR control flow without introducing new IR operations.

5. Full-expression cleanup through condition control flow.
   Temporary-owning call arguments inside nested `&&` and `||` expressions
   should be destroyed exactly on evaluated paths, and every nested logical
   result used by an outer condition should retain a valid LowIR result slot.
   Guarded local-static initialization should destroy initializer temporaries
   on the initialization edge before that edge joins the already-initialized path.
   EH-bearing aggregate construction should invoke nontrivial member constructors
   instead of representation-copying those members, so cleanup state describes
   the subobjects that were constructed.
   Construction and destruction cleanup dependencies on class-template
   destructors should be demanded only after a recursively containing type is
   complete, and should retain that concrete owner in emitted cleanup calls.
   A caller-created copy for a destructible class value parameter transfers to
   the callee. The callee destroys that parameter, while the caller keeps only
   the unwind cleanup needed for objects it still owns.
   Once an exception object has been initialized, destroy the throw operand's
   temporaries and remove them from later unwind snapshots. A temporary from an
   untaken throw branch must not appear in a sibling call's cleanup path.
   If a conditional initializer arm throws before the destination object is
   constructed, do not schedule destruction of that destination on the unwind path.
   When a potentially throwing call is reached through a branch in an active
   handler, its unwind path must finish the handler and destroy objects that
   remain live from scopes outside the corresponding `try` statement.

### Out Of Scope

The following are explicitly out of scope for PA25:

- init-captures
- class captures that require unsupported copy construction, destruction, or object-model
  features
- `std::initializer_list` class elements that require unsupported construction,
  copy, destruction, or later object-model behavior
- `typeid` cases that require `bad_typeid`
- `dynamic_cast` reference forms
- `dynamic_cast<void*>`
- multiple inheritance and virtual inheritance
- any PA25 feature path that depends on unsupported later object-model or ABI work

Inputs that rely on those features have undefined behaviour for this milestone.

### Stage Handoff

The intended next stage is PA26, which completes the remaining non-virtual object-model work
that PA25 still deliberately avoids, especially non-virtual multiple inheritance and the
remaining single-vptr RTTI case `dynamic_cast<void*>`.

So PA25 should leave behind:

- a stable advanced-language semantic layer over the existing single-inheritance model
- LowIR lowering for the supported RTTI, lambda-capture, and initializer-list subset
- explicit remaining deferrals only where PA26 really needs to take over

Virtual inheritance and polymorphic multiple inheritance remain intentionally deferred beyond
PA26.

### Design Notes (Non-Normative)

PA25 should extend the existing semantic and lowering path, not replace it.

The same monotonic-extension rule applies here:

- PA25 should add its new behavior only when the source actually uses the supported PA25
  feature set
- it should not perturb PA24 outputs for programs that remain entirely within the PA24
  subset
- in practice, RTTI globals, closure helpers, and dynamic-cast support should stay
  on-demand rather than eagerly changing the behavior of ordinary earlier programs that do
  not use those features
