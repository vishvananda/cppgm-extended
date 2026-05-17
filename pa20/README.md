## CPPGM Programming Assignment 20 (`cppgm++ --emit-lowir`)

### Overview

Write a C++ application called `cppgm++` that takes as input a set of C++ Source
Files, executes translation phases 1 through 7, parses them as PA10/PA20 translation
units, reuses the PA11-PA19 semantic foundation, builds on the PA14-PA19 LowIR lowering
path, adds the full `constexpr` / constant-evaluation layer, and writes LowIR text.

PA20 extends PA19's first practical metaprogramming slice into a full language-level
constant-evaluation milestone. Its job is to make `constexpr` semantics a first-class part
of the compiler rather than leaving constant evaluation as only the small pragmatic subset
needed by PA19 template arguments and `static_assert`.

To complete PA20, implement these goals:

- `constexpr` function evaluation
- `constexpr` constructors, member functions, and variables
- constant initialization and object-valued constant evaluation
- a reusable constant-expression engine for both ordinary source semantics and later
  template machinery

PA20 still produces LowIR. It does not introduce a new output format.

### Prerequisites

You should complete Programming Assignment 19 before starting this assignment.

You will want to reuse:

- the preprocessing and tokenization pipeline from PA1-PA6
- the PA10 AST as the syntax boundary
- the PA11-PA12 semantic foundation
- the PA14-PA19 LowIR lowering path
- the PA13 LowIR contract
- the PA19 metaprogramming and integral constant-expression machinery

### Starter Kit

The starter kit contains:

- a `cppgm++.cpp` assignment entry point, linked to the editable compiler source
  in `../dev/cppgm++.cpp`
- the standard assignment `Makefile` and harness scripts
- a checked-in local test suite under `tests/`

In the starter kit, the editable `../dev/cppgm++.cpp` file is seeded from
the `cppgm++` scaffold and is the file you extend for this assignment.

Unlike PA1-PA9, there is no external reference binary for PA20. The checked-in
`.ref` files are the default oracle.

### Input / Command-Line Arguments

The PA20 invocation is the unoptimized LowIR mode:

    $ cppgm++ --emit-lowir -O0 -o <outfile> <srcfile1> <srcfile2> ... <srcfileN>

Behaviour is undefined unless the command-line arguments match that shape, with
the same source-file ordering and `-o` relaxations as the earlier source-to-LowIR
milestones. Other `--emit-*` modes, driver mode, and optimized LowIR output are
not part of PA20.

### Output Format

On success, `cppgm++` shall write LowIR text to `<outfile>` and exit
`EXIT_SUCCESS`.

The authoritative LowIR definition is `../pa13/lowir.md`. PA20 extends the PA19 lowering
surface only by making more of the C++ source language lower into the already-defined LowIR
family.

Canonical top-level order is part of the LowIR output contract and is defined
in `../pa13/lowir.md`: `declare global`, `declare function`, `global`, then
`function`. The same contract defines required generated-definition ordering,
including source order, demand-emission order, copy-before-move within a
special-member family, and constructor/destructor ABI entrypoint order.

The test harness checks that the generated LowIR is well formed and matches the
checked-in `.ref` files after canonicalizing presentation details that are not
part of the assignment contract. Exact textual LowIR matching is not a PA20
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

PA20 tests live under `tests/`. The suite is split by test role:

- `tests/spec/` contains N3485/spec-anchored constant-evaluation tests. Each
  provided C++ language test in this directory starts with a leading comment of the
  form `// N3485 focus: 7.1.5 [dcl.constexpr] ...` or another exact governing
  clause so a reviewer can find the text in `../doc/n3485.txt`.
- `tests/general/` contains broader constexpr cross-feature and realistic
  constant-evaluation examples that are useful for PA20 but are not one-rule
  spec probes.

The `make test` target runs both directories through the LowIR validator. For
successful tests, the validator checks the reference LowIR and your generated
LowIR for basic structural correctness, then compares the canonicalized LowIR
against the checked-in reference. For rejected tests, the exit status is the
checked result; exact diagnostic text is not checked.

### PA20 Syntax Boundary

PA20 does not ship a new grammar file. It inherits the PA19 source-language
syntax boundary, including the already parsed `constexpr` and `static_assert`
forms. PA20 is a semantic and constant-evaluation assignment: parsing a construct
does not by itself make that construct required unless it is inside the PA20
boundary below.

### Optional Student Test Ideas

When adding your own tests, useful PA20 themes include C++11 `constexpr`
declaration validity, literal type requirements, constant initialization, core
constant-expression rejection, pointer/reference constant evaluation, and
aggregate/object-valued constant evaluation. Broader evaluator stress cases
belong in `tests/general/`.

### Assignment Boundary

PA20 owns full `constexpr` / constant-evaluation semantics over the implemented language
surface inherited from PA19, including:

- full constant-expression evaluation for the implemented expression/type subset
- `constexpr` functions
- `constexpr` constructors and member functions
- `constexpr` variables and constant initialization
- floating-point `constexpr` evaluation over the implemented scalar language surface
- `noexcept` constant expressions over the supported call/expression subset
- object-valued, pointer-valued, and reference-valued constant evaluation where the earlier
  language/object-model milestones already define the underlying semantics
- reuse of the constant evaluator for ordinary language semantics, template arguments, and
  `static_assert`

The intent is no longer a pragmatic subset. By the end of PA20, `constexpr` should be a
complete compiler-owned semantic layer for the supported C++11 language surface, not a
collection of special cases.

More concretely, over the already-implemented language subset, PA20 should cover the full
C++11 `constexpr` forms that later template and library code expect:

- scalar, floating, `nullptr`, and enum constant expressions
- unary, arithmetic, comparison, bitwise, logical, conditional, cast, `sizeof`,
  `alignof`, `sizeof...`, and `noexcept` constant expressions where those operators are
  already part of the supported language surface
- `constexpr` free-function calls, including recursive calls and default arguments
- `constexpr` constructors, including member-initializer lists and base/member
  initialization for literal class types
- `constexpr` member-function calls on constant objects
- constant object values, not just integral scalars:
  - aggregate/class values
  - array values
  - nested aggregate/array values
- member access on constant objects via `.`
- array and string-literal element access via `[]`
- `constexpr` variables whose initializers must be fully evaluated at compile time
- reference-valued constant evaluation and `const T &` / reference parameter passing where
  the implemented object model already defines the underlying semantics
- lookup and reuse of previously computed constant values, including qualified lookup and
  static data members
- function-local static objects over the supported LowIR subset:
  - constant initialization when the initializer is a constant expression
  - dynamic class-object local statics with the required guard/check behavior

PA20 also owns the semantic validation side of C++11 `constexpr`, not just evaluation. In
particular, the compiler should enforce the C++11-facing rules that matter for the
supported language subset, such as:

- `constexpr` variables require a compile-time initializer and a literal type
- `constexpr` function return and parameter types must be literal types
- `constexpr` constructors must produce literal objects through valid base/member
  initialization
- invalid `constexpr` declarations should fail during semantic analysis instead of being
  accepted and only failing later during use

The implementation may support a strict superset of C++11 evaluation rules internally,
such as local variables, assignment, and loops inside constexpr evaluation. That is fine
and often useful for later milestones, but it does not reduce the requirement that the
standard C++11 forms above be covered cleanly and intentionally.

### Out Of Scope

The following are explicitly out of scope for PA20:

- template language features that still remain deferred to PA21
- post-C++11 constant-evaluation features
- hosted/vendor-only compatibility forms that are not part of the standard C++ language

Inputs that rely on those features have undefined behaviour for this milestone.

### Stage Handoff

The intended next stages are PA21 and PA22, which finish the remaining standard template
language using the now-complete constant-evaluation engine:

- PA21: complete the template entity and specialization model
- PA22: complete deduction, substitution, and SFINAE over that model

So PA20 should leave behind:

- a stable constant-evaluation semantic layer
- ordinary lowered declarations that no longer depend on PA19-specific constant-expression
  shortcuts
- a clear boundary where the remaining template-language work can build on real `constexpr`
  support rather than special-casing it

### Design Notes (Non-Normative)

The useful shape for PA20 is one typed constant-evaluation layer shared by
`constexpr`, template arguments, `static_assert`, constant initialization, and
ordinary semantic checks.

Useful intermediate representations include:

- typed constant values for scalars, enums, pointers, references, arrays, and
  class objects
- a distinction between checking whether a declaration is valid `constexpr` and
  evaluating an expression in a constant-evaluation context
- reusable evaluated-value storage for bindings whose constant value is needed
  by lookup, template arguments, and later LowIR lowering
- local-static initialization metadata that records whether LowIR lowering can
  emit a constant initializer or must emit guarded dynamic initialization
