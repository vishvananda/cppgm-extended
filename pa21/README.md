## CPPGM Programming Assignment 21 (`cppgm++ --emit-lowir`)

### Overview

Write a C++ application called `cppgm++` that takes as input a set of C++
source files, executes translation phases 1 through 7, parses them as PA10/PA21
translation units, reuses the PA11-PA20 semantic foundation, builds on the
PA14-PA20 LowIR lowering path, and writes LowIR text.

PA21 is the first half of template completion. Its job is to finish the
template declaration and specialization model so the compiler knows:

- what template entities exist
- what specializations exist
- which specialization is selected
- which declarations/definitions own the selected specialization

PA21 still produces LowIR. It does not introduce a new output format.

### Prerequisites

You should complete Programming Assignment 20 before starting this assignment.

You will want to reuse:

- the preprocessing and tokenization pipeline from PA1-PA6
- the PA10 AST as the syntax boundary
- the PA11-PA12 semantic foundation
- the PA14-PA20 LowIR lowering path
- the PA13 LowIR contract
- the PA18-PA19 template and metaprogramming machinery
- the PA20 full constant-evaluation layer

### Starter Kit

The starter kit contains:

- a `cppgm++.cpp` assignment entry point, linked to the editable compiler source
  in `../dev/cppgm++.cpp`
- the standard assignment `Makefile` and harness scripts
- the PA21 specialization/entity test suite under `tests/`

In the starter kit, the editable `../dev/cppgm++.cpp` file is seeded from
the `cppgm++` scaffold and is the file you extend for this assignment.

Unlike PA1-PA9, there is no external reference binary for PA21. The checked-in
`.ref` files are the default oracle.

### Input / Command-Line Arguments

The PA21 invocation is the unoptimized LowIR mode:

    $ cppgm++ --emit-lowir -O0 -o <outfile> <srcfile1> <srcfile2> ... <srcfileN>

Behaviour is undefined unless the command-line arguments match that shape, with
the same source-file ordering and `-o` relaxations as the earlier source-to-LowIR
milestones. Other `--emit-*` modes, driver mode, and optimized LowIR output are
not part of PA21.

### Output Format

On success, `cppgm++` shall write LowIR text to `<outfile>` and exit
`EXIT_SUCCESS`.

The authoritative LowIR definition is `../pa13/lowir.md`. PA21 extends the
PA20 lowering surface only by making more of the C++ source language lower into
the already-defined LowIR family.

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

The test harness checks that the generated LowIR is well formed and matches the
checked-in `.ref` files after canonicalizing presentation details that are not
part of the assignment contract. Exact textual LowIR matching is not a PA21
grading requirement.

### Error Handling

If an error occurs during preprocessing, tokenization, parsing, semantic
analysis, or LowIR generation, `cppgm++` shall `EXIT_FAILURE`.

The output file is not required to be meaningful on failure.
Diagnostics are not part of the grading contract.

### Standard Output / Error

Standard output and standard error are ignored for automated testing of
`cppgm++`.

You are free to use them for debugging, tracing, or diagnostic messages.

### Testing

PA21 tests live under `tests/`. The suite is split by test role:

- `tests/spec/` contains N3485/spec-anchored specialization/entity tests. Each
  provided C++ language test in this directory starts with a leading comment of the
  form `// N3485 focus: 14.x.y [clause.name] ...` so a reviewer can find the
  governing text in `../doc/n3485.txt`.
- `tests/general/` contains broader cross-feature and realistic
  template-entity examples that are useful for PA21 but are not one-rule spec
  probes.

The `make test` target runs both directories through the LowIR validator. For
successful tests, the validator checks the reference LowIR and your generated
LowIR for basic structural correctness, then compares the canonicalized LowIR
against the checked-in reference. For rejected tests, the exit status is the
checked result; exact diagnostic text is not checked.

This split assignment intentionally focuses on the specialization/entity half of
template completion:

- class partial specialization and specialization selection
- alias and variable template entity modeling
- explicit specialization and explicit-instantiation ownership
- the declaration/instantiation behavior required to make that model coherent

### PA21 Syntax Boundary

PA21 does not ship a new grammar file. It inherits the PA19 source-language
syntax boundary and the PA20 constant-evaluation semantics. PA21 is a template
entity and specialization assignment: parsing a template construct does not by
itself make that construct required unless it is inside the PA21 boundary below.

### Optional Student Test Ideas

When adding your own tests, useful PA21 themes include alias/variable template
entities, class partial specialization selection,
explicit-instantiation ownership, constructor/member-template specialization
ownership, and partial ordering boundaries. Larger generic-library integration
cases can go in `tests/general/`.

### Assignment Boundary

PA21 owns the template declaration graph and specialization model over the
implemented language surface, including:

- alias templates
- variable templates
- class partial specialization
- partial-specialization ordering and specialization selection
- current-specialization identity in the supported class-template and
  specialization cases
- explicit-instantiation declarations and definitions over the supported surface
- integration with PA19 explicit specialization declarations/definitions when
  they interact with the PA21 specialization graph
- collection/ownership behavior for constructor/member-template specializations
- the dependent-name and instantiation behavior strictly required to make the
  specialization model work

### Out Of Scope

The following are explicitly out of scope for PA21:

- full function-template deduction over the intended language surface
- function-template partial ordering
- SFINAE and substitution-failure completion
- the remaining no-eager-instantiation / dependent-call timing work that is
  better framed as substitution behavior
- initializer-list template behavior
- hosted/vendor-only extensions that happen to use templates
- post-C++11 template-language features

Inputs that rely on those features have undefined behaviour for this milestone.

### Stage Handoff

The intended next stage is PA22, which finishes deduction, substitution, and
SFINAE over the now-complete specialization model.

So PA21 should leave behind:

- a stable template declaration/specialization graph
- deterministic specialization selection
- specialization ownership that lowers through the ordinary LowIR path
- no remaining "template entity model later" gap before full template
  completion

### Design Notes (Non-Normative)

The useful shape for PA21 is a canonical template-entity graph. Alias templates,
variable templates, primary class templates, partial specializations, explicit
instantiations, and PA19 explicit specializations should refer to the same
semantic entities instead of being tracked as unrelated source-text forms.

Useful intermediate representations include:

- canonical specialization keys built from typed template arguments
- an ordered partial-specialization candidate set with deterministic selection
- explicit ownership links from constructor/member-template specializations back
  to the class or namespace entity that owns the generated declaration
- reuse of PA20 constant values for value-dependent specialization keys
