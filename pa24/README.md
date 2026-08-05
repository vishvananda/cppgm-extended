## CPPGM Programming Assignment 24 (`cppgm++ --emit-lowir`)

### Overview

Write a C++ application called `cppgm++` that takes as input a set of C++
source files, executes translation phases 1 through 7, parses them as PA10/PA24
translation units, reuses the PA11-PA23 semantic foundation, builds on the
PA15-PA23 LowIR lowering path, and writes LowIR text.

PA24 is the template integration assignment. Now that you have implemented the
individual template features in PA19, PA20, PA22, and PA23, this PA checks that
they work together in realistic combinations. These tests are intentionally
complicated: they are meant to uncover edge cases where a feature works alone
but loses typed information, chooses the wrong specialization, instantiates too
early, or fails during lowering when combined with another template mechanism.

PA24 does not introduce a new isolated template feature. It is a composition
surface over the earlier template assignments.

PA24 still produces LowIR. It does not introduce a new output format.

### Prerequisites

You should complete Programming Assignment 23 before starting this assignment.

You will want to reuse:

- the preprocessing and tokenization pipeline from PA1-PA6
- the PA10 AST as the syntax boundary
- the PA11-PA12 semantic foundation
- the PA15-PA23 LowIR lowering path
- the PA13 LowIR contract
- the PA19-PA23 template machinery
- the PA21 constant-evaluation layer

### Starter Kit

The starter kit contains:

- a `cppgm++.cpp` assignment entry point, linked to the editable compiler source
  in `../dev/cppgm++.cpp`
- the standard assignment `Makefile` and harness scripts
- the PA24 template-integration test suite under `tests/`

In the starter kit, the editable `../dev/cppgm++.cpp` file is seeded from the
`cppgm++` scaffold and is the file you extend for this assignment.

Unlike PA1-PA9, there is no external reference binary for PA24. The checked-in
`.ref` files are the default oracle.

### Input / Command-Line Arguments

The PA24 invocation is the unoptimized LowIR mode:

    $ cppgm++ --emit-lowir -O0 -o <outfile> <srcfile1> <srcfile2> ... <srcfileN>

Behaviour is undefined unless the command-line arguments match that shape, with
the same source-file ordering and `-o` relaxations as the earlier source-to-LowIR
milestones. Other `--emit-*` modes, driver mode, and optimized LowIR output are
not part of PA24.

### Output Format

On success, `cppgm++` shall write LowIR text to `<outfile>` and exit
`EXIT_SUCCESS`.

The authoritative LowIR definition is `../pa13/lowir.md`. PA24 extends the
PA23 lowering surface only by requiring previously introduced template features
to compose through the already-defined LowIR family.

LowIR top-level declaration/definition order is a presentation convention, not
a dependency order. Reference outputs and canonical dumps use the order defined
in `../pa13/lowir.md`: `declare global`, `declare function`, `global`, then
`function`, but the relaxed LowIR comparison canonicalizes top-level entries
before comparison. Your output must still be repeatable for the same inputs;
`../pa13/lowir.md` defines the canonical reference presentation and notes where
internal LowIR symbol names are only a presentation tie-breaker.

Your output must also preserve order-sensitive LowIR regions when they are
present: instruction order inside blocks, item order inside structured globals,
vtable slot order, and action order inside generated initialization,
finalization, constructor, destructor, and cleanup bodies.

The test harness checks that the generated LowIR is well formed and matches the
checked-in `.ref` files after canonicalizing presentation details that are not
part of the assignment contract. Exact textual LowIR matching is not a PA24
grading requirement.

### Error Handling

If an error occurs during preprocessing, tokenization, parsing, semantic
analysis, or LowIR generation, `cppgm++` shall `EXIT_FAILURE`.

The output file is not required to be meaningful on failure. Diagnostics are not
part of the grading contract.

### Standard Output / Error

Standard output and standard error are ignored for automated testing of
`cppgm++`.

You are free to use them for debugging, tracing, or diagnostic messages.

### Testing

PA24 tests live under `tests/`. The suite is split by test role:

- `tests/spec/` contains N3485/spec-anchored integration cases whose important
  behavior is still tied to a specific standard template rule.
- `tests/general/` contains broader generic-program examples that combine
  multiple template mechanisms.

The `make test` target runs both directories through the LowIR validator. For
successful tests, the validator checks the reference LowIR and your generated
LowIR for basic structural correctness, then compares the canonicalized LowIR
against the checked-in reference. For rejected tests, the exit status is the
checked result; exact diagnostic text is not checked.

The clusters are organized by feature combination:

- `100`: dependent-name and entity interactions that do not fit a narrower
  later cluster
- `200`: deduction, partial ordering, non-deduced context, and braced-init
  deduction combinations
- `300`: SFINAE, substitution, detector idiom, and no-eager instantiation
  combinations
- `400`: pack, member-template, template-template-parameter, alias-template, and
  variable-template compositions
- `500`: library-shaped end-to-end reducers without hosted or builtin-trait
  dependencies

When working through PA24 failures, keep the earlier template assignments
passing. A PA24 fix should not relax, special-case, or regress the basic
functionality already covered by PA19, PA20, PA22, and PA23. Before treating an
integration fix as complete, run the earlier template PAs and PA24 through the
report harness:

    $ make test-report ACTIVE_TEST_REPORT_PAS='pa19 pa20 pa22 pa23 pa24'

### PA24 Syntax Boundary

The authoritative source syntax is the shared `cppgm++` source grammar, exposed
for this assignment as `pa24.gram`. The grammar defines accepted syntax only;
the PA24 semantic and lowering requirements are defined by the Assignment
Boundary and Out Of Scope sections below.

### Optional Student Test Ideas

When adding your own tests, useful PA24 themes include dependent alias expansion
inside deduction, member-template calls through dependent owners, pack expansion
through SFINAE helpers, detector idioms that rely on earlier alias or partial
specialization machinery, and function-template partial ordering where more than
one template feature is needed for the selected result.

### Assignment Boundary

PA24 owns integration among already-introduced template features, including:

- dependent names combined with alias, variable, partial-specialization, or
  deduction behavior
- imported constants retained as defaults in nested member templates when the
  enclosing template is instantiated
- function-template deduction combined with packs, non-deduced contexts,
  explicit arguments, conversion templates, or constructor templates
- SFINAE and substitution behavior combined with alias templates, partial
  specializations, member templates, packs, and detector idioms
- no-eager-instantiation timing in realistic dependent template bodies
- dependent function and constructor default arguments are materialized only after overload
  selection, so an invalid default on an unselected candidate does not reject the call
- library-shaped reductions that do not require hosted headers, vendor
  builtins, or later language features

### Out Of Scope

The following are explicitly out of scope for PA24:

- new isolated template features not already introduced by PA19, PA20, PA22, or
  PA23
- `std::initializer_list` library semantics and initializer-list overload
  machinery
- member-pointer template behavior that depends on later member-pointer support
- hosted/vendor-only extensions that happen to use templates
- post-C++11 template-language features
- backend/toolchain ownership that belongs to the later native and toolchain
  milestones

Inputs that rely on those features have undefined behaviour for this milestone.

### Stage Handoff

The intended next stage is PA25, which extends the source-to-LowIR path with
the first broad ordinary-language closure slice.

So PA24 should leave behind:

- a complete standard template semantic layer whose individual features compose
  cleanly
- instantiated declarations that lower through the ordinary LowIR path without
  template subset special-casing
- no remaining "template features work alone but not together" gap before later
  backend, ABI, and hosted toolchain work

### Design Notes (Non-Normative)

The useful shape for PA24 is not another special-case layer. The same typed
template declarations, template arguments, substitution results, deduction
bindings, and deferred-instantiation records from PA19-PA23 should be threaded
through the combined cases.

When a PA24 test fails, first look for data that was lost between those
subsystems: an alias expansion that discarded a dependent owner, a pack binding
that became text-only, a substitution failure promoted into a hard diagnostic,
or an instantiation record forced before the template arguments were known.
