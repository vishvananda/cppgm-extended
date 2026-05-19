## CPPGM Programming Assignment 28 (`cppgm++ --emit-lowir`)

### Overview

Write a C++ application called `cppgm++` that takes as input a set of C++ Source
Files, executes translation phases 1 through 7, parses them as PA10/PA28 translation units,
reuses the PA11-PA12 semantic foundation, builds on the PA14-PA27 LowIR lowering path,
adds the PA28 multi-base object-model slice, and writes LowIR text.

PA28 completes the remaining non-virtual object-model work that still fits the current
single-vptr ABI:

- non-virtual multiple inheritance
- member lookup and access across multiple base subobjects
- constructor, copy, and destructor generation across multiple non-virtual bases
- `dynamic_cast<void*>` for the current polymorphic single-inheritance ABI

PA28 still produces LowIR. It does not introduce a new output format.

### Prerequisites

You should complete Programming Assignment 27 before starting this assignment.

You will want to reuse:

- the preprocessing and tokenization pipeline from PA1-PA6
- the PA10 AST as the syntax boundary
- the PA11-PA12 semantic foundation
- the PA14-PA27 LowIR lowering path
- the PA13 LowIR contract
- the PA23 native validation path
- the PA13 LowIR -> CY86 path as an optional secondary scaffold

### Starter Kit

The starter kit contains:

- `pa28/README.md`, `pa28/Makefile`, and the test scripts in `pa28/scripts/`
- a student-editable `dev/cppgm++.cpp` starter scaffold
- the `pa28/cppgm++.cpp` symlink back to `../dev/cppgm++.cpp`
- shared support sources and headers under `dev/src/`
- a local test suite under `pa28/tests/`
- the grammar for this assignment called `pa28.gram`
- an HTML grammar explorer of `pa28.gram` in the sub-directory `grammar/`
- a checked-in local test suite under `tests/`

Students should implement the assignment in `dev/cppgm++.cpp` and any reusable
student-owned helpers they add under `dev/src/`. The assignment directory, grammar files,
test fixtures, comparison scripts, and checked-in reference outputs are support
files, not implementation files to edit for normal solutions. The shared support files
provide reusable infrastructure and earlier assignment machinery; they do not implement the
new PA28 source-to-LowIR object-model slice for you.

Unlike PA1-PA9, there is no external reference binary for PA28. The checked-in `.ref`
files are the default oracle.

### Input / Command-Line Arguments

Behaviour is undefined unless the command-line arguments match:

    $ cppgm++ --emit-lowir -O0 -o <outfile> <srcfile1> <srcfile2> ... <srcfileN>

`-O0` is the PA28 test mode. Other optimization levels are later optimizer work and
are not required for this milestone.

### Output Format

`cppgm++` shall write LowIR text to `<outfile>`.

The authoritative LowIR definition is `../pa13/lowir.md`. PA28 extends the PA27 lowering
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

Standard output and standard error are ignored for automated testing of
`cppgm++`.

You are free to use them for debugging, tracing, or diagnostic messages.

### Testing

Testing uses checked-in golden outputs, not a reference binary. The `Makefile` invokes
`cppgm++` with `--emit-lowir -O0`.

The local checked-in tests live in `tests/general/`. That directory contains
PA28 source-to-LowIR tests over non-virtual multiple inheritance, multi-base
generated members, `dynamic_cast<void*>`, and ambiguity rejection. PA28 has no
`tests/spec/` directory because these tests focus on the combined
language-to-LowIR contract.

For each test case `x`:

- `cppgm++` is executed to produce `x.my`
- the exit status is recorded in `x.my.exit_status`
- `x.my` is compared against `x.ref`
- `x.my.exit_status` is compared against `x.ref.exit_status`

PA28 is tested against generated LowIR text using the relaxed LowIR comparator described
above. A useful manual validation path is:

- feed that LowIR into PA23 `lowir2native`
- optionally cross-check by feeding that same LowIR into PA13 `lowir2cy86`
- then feed the generated CY86 into PA9 `cy86 --target linux`

The shipped PA28 tests are the contract for this milestone.

### PA28 Syntax Spec

The authoritative source-language syntax boundary for PA28 is `pa28.gram`.

As in the earlier assignments, that grammar defines accepted input syntax only. The output
format for `cppgm++` is specified by this README, PA13 `lowir.md`, and the
checked-in `.ref` files.

PA28 inherits the PA27 syntax boundary and does not add a new source-language grammar file
format. It instead enables more of the already-accepted C++11 syntax to participate in
semantic analysis and lowering.

A checked-in HTML grammar explorer for that grammar lives in `grammar/`. Treat
`pa28.gram` as the source of truth.

`pa28.gram` uses the same token vocabulary and the same extended BNF operators as
`../pa6/pa6.gram`.

If this README and `pa28.gram` appear to disagree about source syntax, treat `pa28.gram`
as authoritative. If this README and PA13 `lowir.md` appear to disagree about LowIR syntax,
treat `lowir.md` as authoritative. If they disagree about the PA28 lowering slice, treat the
`Assignment Boundary` and `Out Of Scope` sections below as authoritative.

### Assignment Boundary

PA28 supports the following in addition to the PA27 subset:

- non-virtual multiple inheritance
- inherited field lookup and access across multiple non-virtual base subobjects
- inherited non-virtual method lookup and `this` adjustment across multiple non-virtual bases
- constructor, copy-constructor, copy-assignment, and destructor generation across multiple
  non-virtual bases
- `dynamic_cast<void*>` for the existing polymorphic single-inheritance ABI

Within this milestone, PA28 should produce valid LowIR for ordinary source programs over
that subset. That LowIR should be accepted by PA23 `lowir2native` for the supported cases.
PA13 `lowir2cy86` remains a secondary scaffold backend for cross-checking.

To complete PA28, implement these goals:

1. Multiple-base layout and field access.
   Distinct base subobjects should have deterministic offsets, and member access should lower
   through those offsets correctly.

2. Base-method lookup and `this` adjustment.
   Calling a method inherited from a later base must lower the implicit object argument to the
   correct base-subobject address.

3. Generated special members across multiple bases.
   Synthesized construction, copy, assignment, and destruction should sequence the supported
   non-virtual bases correctly.

4. Remaining single-vptr RTTI case.
   `dynamic_cast<void*>` should lower for the existing polymorphic single-inheritance ABI
   without introducing new LowIR operations.

5. Ambiguity handling.
   Ambiguous inherited member names must not silently resolve.

### Out Of Scope

The following are explicitly out of scope for PA28:

- virtual inheritance
- polymorphic multiple inheritance
- `dynamic_cast` reference forms
- `dynamic_cast` cases that depend on multiple or virtual polymorphic base layouts
- the remaining RTTI cases that require a broader multi-vptr or virtual-base ABI

Inputs that rely on those features have undefined behaviour for this milestone.

### Stage Handoff

The intended next stage is PA29, which completes the broader virtual / RTTI ABI that PA28
still deliberately avoids.

So PA28 should leave behind:

- a stable non-virtual multi-base object model over the existing LowIR family
- deterministic lowering for multiple-base field access, method calls, and generated special
  members
- explicit remaining deferrals only where PA29 needs to take over:
  - virtual inheritance
  - polymorphic multiple inheritance
  - the remaining `dynamic_cast` / RTTI cases that depend on that ABI

### Design Notes (Non-Normative)

PA28 should extend the existing semantic and lowering path, not replace it.

The same monotonic-extension rule applies here:

- PA28 should add its new behavior only when the source actually uses the supported PA28
  feature set
- it should not perturb PA27 outputs for programs that remain entirely within the PA27
  subset
- in practice, multiple-base offsets and lowered base-adjustment paths should stay on-demand
  rather than changing earlier single-base outputs unnecessarily
