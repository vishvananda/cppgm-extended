## CPPGM Programming Assignment 19 (`cppgm++ --emit-lowir`)

### Overview

Write a C++ application called `cppgm++` that takes as input a set of C++ Source Files,
executes translation phases 1 through 7, parses them as PA10/PA19 translation units,
reuses the PA11-PA12 semantic foundation, builds on the PA14-PA18 LowIR lowering path,
adds the PA19 metaprogramming slice, and writes LowIR text.

PA19 extends PA18’s first-tier templates with the first practical compile-time
metaprogramming layer:

- integral non-type template parameters
- integral non-type template arguments
- explicit specialization of supported class templates and function templates
- integral constant-expression evaluation for template arguments
- `static_assert` over the supported integral constant-expression subset

### Prerequisites

You should complete Programming Assignment 18 before starting this assignment.

You will want to reuse:

- the preprocessing and tokenization pipeline from PA1-PA6
- the PA10 AST as the syntax boundary
- the PA11 declarator/type model
- the PA12 call-resolution layer
- the PA14-PA18 LowIR lowering path
- the PA13 LowIR contract
- the PA13 LowIR -> CY86 path as an optional secondary scaffold
- the PA18 template declaration, lookup, deduction, and instantiation machinery

The intended direction is:

- PA10 provides syntax
- PA11 provides scope/type lookup
- PA12 provides the procedural expression/call core
- PA14-PA18 lower the supported language subsets to LowIR
- PA19 extends the template layer with compile-time value arguments and explicit specialization

### Starter Kit

The starter kit contains:

- a `cppgm++.cpp` assignment entry point, linked to the editable compiler source
  in `../dev/cppgm++.cpp`
- the standard assignment `Makefile` and harness scripts
- the grammar for this assignment called `pa19.gram`
- an HTML grammar explorer of `pa19.gram` in the sub-directory `grammar/`
- a checked-in local test suite under `tests/`

In the starter kit, the editable `../dev/cppgm++.cpp` file is seeded from the
`cppgm++` scaffold and is the file you extend for this assignment.

Unlike PA1-PA9, there is no external reference binary for PA19. The checked-in `.ref`
files are the default oracle.

### Input / Command-Line Arguments

The PA19 invocation is the unoptimized LowIR mode:

    $ cppgm++ --emit-lowir -O0 -o <outfile> <srcfile1> <srcfile2> ... <srcfileN>

Behaviour is undefined unless the command-line arguments match that shape, with
the same source-file ordering and `-o` relaxations as PA18. Other `--emit-*`
modes, driver mode, and optimized LowIR output are not part of PA19.

### Output Format

On success, `cppgm++` shall write LowIR text to `<outfile>` and exit
`EXIT_SUCCESS`.

The authoritative LowIR definition is `../pa13/lowir.md`. PA19 extends the PA18 LowIR
subset only by making more of the source language lower into the already-defined LowIR
family. PA19 does not introduce a new output format.

Canonical top-level order is part of the LowIR output contract and is defined
in `../pa13/lowir.md`: `declare global`, `declare function`, `global`, then
`function`. The same contract defines required generated-definition ordering,
including source order, demand-emission order, copy-before-move within a
special-member family, and constructor/destructor ABI entrypoint order.

The test harness checks that the generated LowIR is well formed and matches the
checked-in `.ref` files after canonicalizing presentation details that are not
part of the assignment contract. Exact textual LowIR matching is not a PA19
grading requirement.

### Error Handling

If an error occurs during preprocessing, tokenization, parsing, semantic analysis, or LowIR
generation, `cppgm++` shall `EXIT_FAILURE`.

The output file is not required to be meaningful on failure.
Diagnostics are not part of the grading contract.

### Standard Output / Error

Standard output and standard error are ignored for automated testing of `cppgm++`.

You are free to use them for debugging, tracing, or diagnostic messages.

### Testing

Testing uses checked-in golden outputs, not a reference binary.

For each test case `x`:

- `cppgm++` is executed to produce `x.my`
- the exit status is recorded in `x.my.exit_status`
- `x.my` is compared against `x.ref`
- `x.my.exit_status` is compared against `x.ref.exit_status`

`make test` runs the checked-in local suite under `tests/`. The suite is split
by test role:

- `tests/spec/` contains N3485/spec-anchored PA19 metaprogramming tests. Each
  provided C++ language test in this directory starts with a leading comment of the
  form `// N3485 focus: 14.x.y [clause.name] ...` so a reviewer can find the
  governing text in `../doc/n3485.txt`.
- `tests/general/` contains broader cross-feature and realistic
  metaprogramming tests that are useful for PA19 but are not one-rule spec
  probes.

The `make test` target runs both directories through the LowIR validator. For
successful tests, the validator checks the reference LowIR and your generated
LowIR for basic structural correctness, then compares the canonicalized LowIR
against the checked-in reference. For rejected tests, the exit status is the
checked result; exact diagnostic text is not checked.

PA19 is tested against generated LowIR text. That LowIR is intended to become
input for the later PA23 `lowir2native` backend, but that future native path is
not the PA19 grading contract.

### Optional Student Test Ideas

When adding your own tests, useful PA19 themes include explicit specialization
ordering and visibility, integral non-type argument equivalence, dependent
non-type parameter types, and static data member specialization. Keep larger
metaprogramming integration cases under `tests/general/`.

### PA19 Syntax Spec

The authoritative source-language syntax boundary for PA19 is `pa19.gram`.

As in the earlier assignments, that grammar defines accepted input syntax only. The output
format for `cppgm++` is specified by this README, PA13 `lowir.md`, and the checked-in
`.ref` files.

PA19 inherits the PA18 syntax boundary and extends it with:

- integral non-type template parameters such as `template<int N>`
- explicit specialization syntax such as `template<> int f<int>(int)` and
  `template<> struct Box<int> { ... }`

Passing PA18 is necessary but not sufficient for passing PA19: an input may be syntactically
valid for PA10-PA19 and still be outside the supported PA19 metaprogramming slice described
below.

A checked-in HTML grammar explorer for that grammar lives in `grammar/`. Treat
`pa19.gram` as the source of truth.

`pa19.gram` uses the same token vocabulary and the same extended BNF operators as
`../pa6/pa6.gram`.

If this README and `pa19.gram` appear to disagree about source syntax, treat `pa19.gram`
as authoritative. If this README and PA13 `lowir.md` appear to disagree about LowIR syntax,
treat `lowir.md` as authoritative. If they disagree about the PA19 lowering slice, treat the
`Assignment Boundary` and `Out Of Scope` sections below as authoritative.

### Assignment Boundary

PA19 supports the following in addition to the PA18 subset:

- class templates whose parameters may now include integral non-type parameters and integral
  non-type parameter packs
- function templates whose parameters may now include integral non-type parameters and
  integral non-type parameter packs when the arguments are supplied explicitly
- integral constant-expression template arguments over the supported subset:
  - literals, including ordinary character literals
  - keyword literals `true` / `false`
  - id-expressions naming supported constant bindings
  - parenthesized expressions
  - unary `+`, unary `-`, `!`, `~`
  - binary arithmetic, shifts, comparisons, equality, bitwise, and logical operators
  - conditional `?:`
  - `sizeof...(parameter-pack)`
  - `sizeof(type-id)` and `alignof(type-id)`
  - supported cast expressions that fold to integral constant values
- explicit specialization of supported class templates
- explicit specialization of supported function templates
- late explicit-specialization visibility and stale-primary refresh in the
  supported class/function template cases
- constant-valued template bindings over the supported subset, including class-scope
  `static const` / `static constexpr` members and other ordinary metaprogramming helper
  bindings that feed lookup, template arguments, or `static_assert`
- dependent qualified type/value lookups at the practical level needed by the supported
  metaprogramming subset
- `static_assert` declarations whose condition is in the supported integral constant subset,
  including conditions that remain template-dependent until instantiation

Within this milestone, PA19 should produce valid LowIR for ordinary metaprogramming code
over the supported PA18 language subset. That LowIR is intended to be accepted
by the later PA23 `lowir2native` backend for the supported cases. PA13
`lowir2cy86` remains an optional execution scaffold.

### Out Of Scope

The following are explicitly out of scope for PA19:

- partial specialization
- pointer, reference, member-pointer, class-type, and other non-integral
  non-type template parameters
- SFINAE and substitution-failure candidate dropping
- full standard-conforming two-phase lookup
- constexpr function evaluation
- function-template deduction of non-type arguments
- full function-template deduction and partial ordering
- alias templates and variable templates
- hosted/vendor-only template traits and intrinsics
- template metaprogramming that depends on unsupported PA14-PA18 language features

Inputs that rely on those features have undefined behaviour for this milestone.

### Stage Handoff

The intended next stages are:

- PA20: complete the language-level constant-evaluation model over the existing LowIR path
- PA21 and PA22: finish the remaining template specialization, deduction, substitution, and
  SFINAE work on top of that constant-evaluation engine
- PA23: retarget the settled LowIR language surface to the real native backend

So PA19 should leave behind:

- a stable template/metaprogramming semantic layer
- ordinary instantiated declarations ready for LowIR lowering
- no PA19-specific output representation beyond LowIR itself

### Design Notes (Non-Normative)

PA19 should extend the existing template machinery, not replace it.

The same monotonic-extension rule applies here:

- PA19 should add metaprogramming behavior only when the source actually uses the supported
  PA19 feature set
- it should not perturb PA18 outputs for programs that remain entirely within the PA18
  subset
- in practice, non-type template arguments, explicit specialization, and `static_assert`
  should stay on-demand rather than eagerly changing the behavior of ordinary earlier
  programs that do not use those features

Useful intermediate representations include:

- template parameters that distinguish type, template-template, and integral value slots
- template arguments that carry canonical constant values rather than only source text
- explicit-specialization tables that plug into the existing instantiation machinery
- a specialization lookup step that runs before instantiation so late visible
  specializations replace stale primary-template instantiations in the supported
  cases
- compile-time constant bindings that can be reused by both `static_assert` and template
  argument resolution
