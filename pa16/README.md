## CPPGM Programming Assignment 16 (`cppgm++ --emit-lowir`)

### Overview

Write a C++ application called `cppgm++` that takes as input a set of C++ Source Files,
executes translation phases 1 through 7, parses them as PA10/PA16 translation units,
reuses the PA11-PA12 semantic foundation, builds on the PA14-PA15 LowIR lowering path, and
writes LowIR text.

PA16 finishes the non-polymorphic class model so ordinary user-defined value types work
cleanly before virtual dispatch is added. It extends PA15 with the common value-semantics
paths:

- copy construction/assignment and the common move-construction/move-assignment cases
  needed by those same value paths
- pass-by-value and return-by-value of class objects
- temporary materialization in the common call/return/initialization paths
- delegating constructors
- out-of-class constructor and destructor definitions
- the ordinary user-defined copy/move constructors and assignment operators directly
  needed by that value-semantics work

### Prerequisites

You should complete Programming Assignment 15 before starting this assignment.

You will want to reuse:

- the preprocessing and tokenization pipeline from PA1-PA6
- the PA10 AST as the syntax boundary
- the PA11 declarator/type model
- the PA12 call-resolution layer
- the PA14/PA15 LowIR lowering path
- the PA13 LowIR contract
- the PA13 LowIR -> CY86 path as an optional secondary scaffold
- the PA15 class metadata, constructor/destructor machinery, and lifetime lowering

The intended direction is:

- PA10 provides syntax
- PA11 provides scope/type lookup
- PA12 provides the procedural expression/call core
- PA14 lowers the procedural subset
- PA15 adds the basic non-virtual object model
- PA16 extends that same object model into usable value semantics

### Starter Kit

The starter kit contains:

- the student-editable `../dev/cppgm++.cpp` entry point, initially seeded from the course
  `cppgm++` scaffold and reached from this directory through the `cppgm++.cpp` symlink
- shared `../dev/` and `../dev/src/` support code from the earlier compiler pipeline
- a local test suite
- the grammar for this assignment called `pa16.gram`
- an HTML grammar explorer of `pa16.gram` in the sub-directory `grammar/`
- a checked-in local test suite under `tests/`

The provided scaffold and shared support files establish the driver shape and previous
frontend modes. They do not implement the PA16 value-semantics LowIR lowering work.

Unlike PA1-PA9, there is no external reference binary for PA16. The checked-in `.ref`
files are the default oracle.

### Input / Command-Line Arguments

The same as PA15 `cppgm++ --emit-lowir`. The PA16 test mode is unoptimized LowIR
generation. `make test` passes `--emit-lowir -O0` through the harness, so individual test
files do not spell those flags themselves.

Behaviour is undefined unless the command-line arguments match:

    $ cppgm++ --emit-lowir -O0 -o <outfile> <srcfile1> <srcfile2> ... <srcfileN>

with the same relaxations as PA15.

Accepting `--emit-lowir` without an explicit `-O0` as the same unoptimized mode is fine,
but optimized LowIR output is not part of PA16.

### Output Format

`cppgm++` shall write LowIR text to `<outfile>`.

The authoritative LowIR definition is `../pa13/lowir.md`. PA16 extends the PA15 object-model
subset of that IR with the value-semantics lowering needed by this milestone.

PA16 writes a single concatenated LowIR program consisting of:

- zero or more `global` definitions
- zero or more `function` definitions

Canonical top-level order is part of the LowIR output contract and is defined
in `../pa13/lowir.md`: `declare global`, `declare function`, `global`, then
`function`. The same contract defines required generated-definition ordering,
including source order, demand-emission order, copy-before-move within a
special-member family, and constructor/destructor ABI entrypoint order.

For supported class value types, PA16 extends the PA15 lowering convention by introducing:

- indirect LowIR parameters for pass-by-value class objects
- indirect LowIR return destinations for return-by-value class objects
- explicit LowIR-level materialization of supported copy/move/value transfers

Synthesized copy/move constructors, assignment helpers, and related
temporary-materialization support are part of the PA16 semantic model, but `cppgm++` only
needs to emit the helper definitions that the lowered program actually requires. Unused
copy/move/value helpers do not need to appear just because they are synthesizable.

For supported indirect return-by-value cases, PA16 may also lower an eligible top-level
named local directly in `%ret` instead of building a separate local object and then
copying or moving it into the return destination. That direct return-slot form is part of
the accepted PA16 output contract.

For supported synthesized copy/move special members, PA16 may lower a leading trivially
copyable storage prefix directly as `copyobj <span> <src>, <dst>` instead of spelling that
prefix as separate field operations or a `__builtin_memcpy` helper call in the emitted
LowIR. That direct storage-copy form is also part of the accepted PA16 output contract.

For supported trivially copy-constructible class value transfers, PA16 may also lower the
copy/move construction step itself directly as `copyobj <span> <src>, <dst>` instead of
spelling a call to a synthesized trivial copy/move constructor helper. That direct
value-transfer form is part of the accepted PA16 output contract.

Supported synthesized constructors, destructors, and copy/move assignment operators may
also carry LowIR boundary metadata such as `[unwind=no]` when the compiler can determine
that the synthesized body is semantically non-throwing. That metadata is part of the
accepted PA16 output contract when it appears in the checked-in `.ref` files.

For supported synthesized destructors, trivial union subobject destructor steps may be
omitted from enclosing synthesized destructors.

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

Testing uses checked-in golden outputs, not a reference binary.

For each test case `x`:

- `cppgm++` is executed to produce `x.my`
- the exit status is recorded in `x.my.exit_status`
- `x.my` is validated as LowIR and compared against `x.ref` using the normalized
  LowIR comparison
- `x.my.exit_status` is compared against `x.ref.exit_status`

`make test` runs the checked-in local suite under `tests/` and supplies
`--emit-lowir -O0` through the harness.

The PA16 suite is split by test role:

- `tests/general/`: the default PA16 LowIR oracle suite. These tests cover value-semantics
  lowering, copy/value helper emission, temporary materialization, ABI-shape
  cases, and cross-feature cases whose primary contract is generated LowIR plus
  exit status.
- `tests/spec/`: focused C++ language-contract cases that cite a specific N3485 clause.
  Each source test in this directory starts with a comment of the form:

    // N3485 focus: <clause> [<stable-name>] <short topic>

`tests/spec/` covers the PA16 value-semantics contract: defaulted/deleted
special members, copy/move construction and assignment, ref-qualified member
functions, delegating constructors, allocation expressions, unions, conversion
operators, and class value ABI behavior. `tests/general/` covers
value-semantics and LowIR-shape cases that are not tied to one specific C++11
clause.

PA16 is tested against the generated LowIR text.

### PA16 Syntax Spec

The authoritative source-language syntax boundary for PA16 is `pa16.gram`.

As in the earlier assignments, that grammar defines accepted input syntax only. The output
format for `cppgm++` is specified by this README, PA13 `lowir.md`, and the checked-in
`.ref` files.

PA16 mostly inherits the PA10-PA15 syntax boundary, but extends it with the class-syntax
forms needed to finish the common value-semantics work, especially out-of-class constructor
and destructor definitions.

Passing PA15 is necessary but not sufficient for passing PA16: an input may be syntactically
valid for PA10-PA15 and code-generation-valid for PA15 and still be outside the PA16
value-semantics slice described below.

A checked-in HTML grammar explorer for that grammar lives in `grammar/`. Treat
`pa16.gram` as the source of truth.

`pa16.gram` uses the same token vocabulary and the same extended BNF operators as
`../pa6/pa6.gram`.

If this README and `pa16.gram` appear to disagree about source syntax, treat `pa16.gram`
as authoritative. If this README and PA13 `lowir.md` appear to disagree about LowIR syntax,
treat `lowir.md` as authoritative. If they disagree about the PA16 lowering slice, treat the
`Assignment Boundary` and `Out Of Scope` sections below as authoritative.

### Assignment Boundary

PA16 supports the following in addition to the PA15 subset:

- implicit copy constructors in the common field-wise/base-wise cases
- implicit copy assignment in the common field-wise/base-wise cases
- implicit move constructors in the common field-wise/base-wise cases needed by the
  supported value-semantics paths
- implicit move assignment in the common field-wise/base-wise cases needed by the
  supported value-semantics paths
- user-declared copy/move constructors and copy/move assignment operators in the ordinary
  non-template class cases needed by the supported value-semantics paths
- ordinary defaulted/deleted move-constructor and move-assignment cases in the supported
  non-template class patterns used by this assignment
- value passing of complete class objects to supported functions
- return-by-value of complete class objects from supported functions
- demand-driven LowIR emission of the copy/move/value helpers required by those supported
  paths
- raw `copyobj` lowering of a supported leading trivial storage prefix inside synthesized
  copy/move special members when the remaining suffix still needs ordinary field-wise
  lowering
- direct `copyobj` lowering of supported trivial class copy/move construction at the call
  site instead of forcing a separate synthesized trivial constructor call
- temporary class-object materialization in the common cases required by:
  - copy initialization from function results
  - pass-by-value call arguments
  - return forwarding through the supported value paths
- direct reuse of the indirect return destination for supported `return local;` cases when
  the named local is the returned complete object
- ref-qualified member functions and out-of-class definitions of ref-qualified
  members
- delegating constructors
- out-of-class constructor definitions
- out-of-class destructor definitions
- scalar `new` / `delete` expressions over the supported object subset
- array `new` / `delete[]` expressions over the supported object subset
- union definitions and union object lifetime in the supported non-template
  class subset
- non-template conversion operators that participate in the existing overload
  and conversion machinery

Within this milestone, PA16 should produce valid LowIR for ordinary non-polymorphic value
types over the supported PA15 procedural/class subset. That LowIR is intended
to be accepted by the later PA23 `lowir2native` backend for the supported
cases. PA13 `lowir2cy86` remains an optional execution scaffold, not the
primary validation path.

### Out Of Scope

The following are explicitly out of scope for PA16:

- virtual functions, vpointers, and vtables
- RTTI and `dynamic_cast`
- multiple inheritance
- member pointers
- generalized operator overloading beyond the supported value-semantics paths
- copy-elision perfection and the full set of standard temporary-materialization rules
- advanced move-generation rules beyond the common supported field-wise/base-wise cases
  above, and the full standard move-semantics corner cases
- exception-aware cleanup during value transfers
- template-aware value semantics
- lambda expressions, range-for, and later general convenience syntax that is not
  needed by the PA16 value-semantics tests

Inputs that rely on those features have undefined behaviour for this milestone.

### Stage Handoff

The intended next stage is PA17, which adds the polymorphic machinery that PA16
intentionally leaves out:

- virtual dispatch
- virtual destructors
- vtables
- override/final behavior

So PA16 should leave behind a clean non-polymorphic value-semantics object model and LowIR
lowering path rather than mixing virtual dispatch into the same milestone.

### Design Notes (Non-Normative)

The important point is to extend the existing PA15 behavior rather than inventing a second,
incompatible model just for copy/value behavior. Whether that reuse happens through shared
code, shared data structures, or a careful reimplementation is up to you.

An important implementation rule for this milestone is monotonic extension:

- PA16 should add value-semantics behavior only when the source actually requires it
- it should not perturb PA15 outputs for programs that remain entirely inside the PA15
  subset
- in practice, that means copy constructor / copy-assignment support should only become
  semantically visible when the program actually needs it, rather than eagerly changing the
  behavior or emitted output for every class
- "PA15 would have treated this as out of scope" is not a sufficient reason to let PA16
  change the observable output of a still-valid PA15 program

Useful intermediate representations include:

- class metadata that distinguishes ordinary methods, constructors, destructors, and
  synthesized special members
- explicit constructor/destructor/copy actions attached to declarations and returns
- for supported indirect-value local objects, those attached destructor actions should remain
  the source of truth for scope cleanup during LowIR lowering rather than being recomputed
  later from the lowered storage type alone
- a calling-convention layer that can lower class values indirectly without changing the
  source-level semantic types
- a stable way to identify the supported temporary-materialization points without requiring
  a fully general temporary lifetime model yet
- allocation expressions lowered as ordinary construction/destruction actions
  over explicit storage, rather than as a separate object model
- conversion operators represented through the same typed overload-resolution
  and conversion machinery used for ordinary calls
