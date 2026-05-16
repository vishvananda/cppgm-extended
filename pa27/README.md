## CPPGM Programming Assignment 27 (`cppgm++ --emit-lowir`)

### Overview

Write a C++ application called `cppgm++` that takes as input a set of C++ Source
Files, executes translation phases 1 through 7, parses them as PA10/PA27 translation units,
reuses the PA11-PA12 semantic foundation, builds on the PA14-PA26 LowIR lowering path,
adds the PA27 advanced-language slice, and writes LowIR text.

PA27 finishes the deferred first-tier language features that sit on top of the existing
single-inheritance object model:

- capturing lambdas
- `std::initializer_list` semantic interoperation
- RTTI and `typeid`
- pointer-form `dynamic_cast`

PA27 still produces LowIR. It does not introduce a new output format.

### Prerequisites

You should complete Programming Assignment 26 before starting this assignment.

You will want to reuse:

- the preprocessing and tokenization pipeline from PA1-PA6
- the PA10 AST as the syntax boundary
- the PA11-PA12 semantic foundation
- the PA14-PA26 LowIR lowering path
- the PA13 LowIR contract
- the PA23 native validation path
- the PA13 LowIR -> CY86 path as an optional secondary scaffold

### Starter Kit

The starter kit contains:

- `pa27/README.md`, `pa27/Makefile`, and the test scripts in `pa27/scripts/`
- a student-editable `dev/cppgm++.cpp` starter scaffold
- the `pa27/cppgm++.cpp` symlink back to `../dev/cppgm++.cpp`
- shared support sources and headers under `dev/src/`
- a local test suite under `pa27/tests/`
- the grammar for this assignment called `pa27.gram`
- an HTML grammar explorer of `pa27.gram` in the sub-directory `grammar/`
- a checked-in local test suite under `tests/`

Students should implement the assignment in `dev/cppgm++.cpp` and any reusable
student-owned helpers they add under `dev/src/`. The assignment directory, grammar files,
test fixtures, comparison scripts, and checked-in reference outputs are support
files, not implementation files to edit for normal solutions. The shared support files
provide reusable infrastructure and earlier assignment machinery; they do not implement the
new PA27 source-to-LowIR language slice for you.

Unlike PA1-PA9, there is no external reference binary for PA27. The checked-in `.ref`
files are the default oracle.

### Input / Command-Line Arguments

Behaviour is undefined unless the command-line arguments match:

    $ cppgm++ --emit-lowir -O0 -o <outfile> <srcfile1> <srcfile2> ... <srcfileN>

`-O0` is the PA27 test mode. Other optimization levels are later optimizer work and
are not required for this milestone.

### Output Format

`cppgm++` shall write LowIR text to `<outfile>`.

The authoritative LowIR definition is `../pa13/lowir.md`. PA27 extends the PA26 lowering
surface only by making more of the C++ source language lower into the already-defined LowIR
family.

Canonical top-level order is part of the LowIR output contract and is defined
in `../pa13/lowir.md`: `declare global`, `declare function`, `global`, then
`function`. The same contract defines required generated-definition ordering,
including source order, demand-emission order, copy-before-move within a
special-member family, and constructor/destructor ABI entrypoint order.

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
PA27 source-to-LowIR tests for capturing lambdas, initializer-list
interoperation, RTTI, `typeid`, `dynamic_cast`, and exception-source lowering
interactions. PA27 has no `tests/spec/` directory because these tests focus on
the combined language-to-LowIR contract.

For each test case `x`:

- `cppgm++` is executed to produce `x.my`
- the exit status is recorded in `x.my.exit_status`
- `x.my` is compared against `x.ref`
- `x.my.exit_status` is compared against `x.ref.exit_status`

PA27 is tested against generated LowIR text using the relaxed LowIR comparator described
above. A useful manual validation path is:

- feed that LowIR into PA23 `lowir2native`
- optionally cross-check by feeding that same LowIR into PA13 `lowir2cy86`
- then feed the generated CY86 into PA9 `cy86 --target linux`

The shipped PA27 tests are the contract for this milestone.

### PA27 Syntax Spec

The authoritative source-language syntax boundary for PA27 is `pa27.gram`.

As in the earlier assignments, that grammar defines accepted input syntax only. The output
format for `cppgm++` is specified by this README, PA13 `lowir.md`, and the
checked-in `.ref` files.

PA27 inherits the PA26 syntax boundary and does not add a new source-language grammar file
format. It instead enables more of the already-accepted C++11 syntax to participate in
semantic analysis and lowering.

A checked-in HTML grammar explorer for that grammar lives in `grammar/`. Treat
`pa27.gram` as the source of truth.

`pa27.gram` uses the same token vocabulary and the same extended BNF operators as
`../pa6/pa6.gram`.

If this README and `pa27.gram` appear to disagree about source syntax, treat `pa27.gram`
as authoritative. If this README and PA13 `lowir.md` appear to disagree about LowIR syntax,
treat `lowir.md` as authoritative. If they disagree about the PA27 lowering slice, treat the
`Assignment Boundary` and `Out Of Scope` sections below as authoritative.

### Assignment Boundary

PA27 supports the following in addition to the PA26 subset:

- capturing lambdas with supported explicit by-copy and by-reference captures of local
  values, including class objects whose existing copy-construction path is supported
- default `[=]` and `[&]` captures over the same supported local-value and `this` subset
- explicit `this` capture for supported member-function cases
- `std::initializer_list<T>` interoperation for the supported non-class element subset
- `typeid(type-id)`
- `typeid(expr)` for supported polymorphic lvalue expressions
- `dynamic_cast<T*>(expr)` for supported polymorphic single-inheritance pointer conversions

Within this milestone, PA27 should produce valid LowIR for ordinary source programs over
that subset. That LowIR should be accepted by PA23 `lowir2native` for the supported cases.
PA13 `lowir2cy86` remains a secondary scaffold backend for cross-checking.

To complete PA27, implement these goals:

1. Capturing lambda lowering.
   Explicit by-copy captures should materialize deterministic closure-object LowIR and the
   resulting closure object should be callable through the existing class/method lowering
   path.

2. `std::initializer_list` interoperation.
   Supported braced-list calls should materialize deterministic lowered storage and expose
   the expected `__begin` / `__size` semantics to range-for lowering.

3. RTTI and `typeid`.
   The compiler should emit deterministic RTTI globals and lower both static and dynamic
   `typeid` queries into ordinary LowIR address/load/branch operations.

4. Pointer-form `dynamic_cast`.
   The compiler should lower supported polymorphic single-inheritance pointer casts into
   ordinary LowIR control flow without introducing new IR operations.

### Out Of Scope

The following are explicitly out of scope for PA27:

- init-captures
- class captures that require unsupported copy construction, destruction, or object-model
  features
- `std::initializer_list` with class element types
- `typeid` cases that require `bad_typeid`
- `dynamic_cast` reference forms
- `dynamic_cast<void*>`
- multiple inheritance and virtual inheritance
- any PA27 feature path that depends on unsupported later object-model or ABI work

Inputs that rely on those features have undefined behaviour for this milestone.

### Stage Handoff

The intended next stage is PA28, which completes the remaining non-virtual object-model work
that PA27 still deliberately avoids, especially non-virtual multiple inheritance and the
remaining single-vptr RTTI case `dynamic_cast<void*>`.

So PA27 should leave behind:

- a stable advanced-language semantic layer over the existing single-inheritance model
- LowIR lowering for the supported RTTI, lambda-capture, and initializer-list subset
- explicit remaining deferrals only where PA28 really needs to take over

Virtual inheritance and polymorphic multiple inheritance remain intentionally deferred beyond
PA28.

### Design Notes (Non-Normative)

PA27 should extend the existing semantic and lowering path, not replace it.

The same monotonic-extension rule applies here:

- PA27 should add its new behavior only when the source actually uses the supported PA27
  feature set
- it should not perturb PA26 outputs for programs that remain entirely within the PA26
  subset
- in practice, RTTI globals, closure helpers, and dynamic-cast support should stay
  on-demand rather than eagerly changing the behavior of ordinary earlier programs that do
  not use those features
