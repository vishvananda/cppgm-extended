## CPPGM Programming Assignment 5 (`cppgm++ --emit-ast`)

### Overview

Write the first `cppgm++` source-compiler mode:

```sh
cppgm++ --emit-ast -o <outfile> <srcfile1> [<srcfile2> ...]
```

The program reads one or more C++ source files, runs translation phases 1
through 7 for each file, parses each translation unit using the PA5 syntax
subset, and writes a deterministic text dump of the parsed syntax tree.

PA5 is a syntax assignment. Build a tree-producing parser whose structured
AST later assignments consume directly. The required name-category information
helps choose syntax; type checking, overload resolution, and template
instantiation come later.

### Prerequisites

Complete the PA4 preprocessor. Reuse the PA1–PA4 token pipeline and source
locations. This is the first C++ parser you build in the course. Its structured
AST and deterministic dump are the deliverable.

Read [parsing.md](parsing.md) for the implementation order, grammar notation,
name-category boundary, and ambiguity examples. Parsing template syntax here
does not require implementing template deduction or instantiation.

### Starter Kit

The PA5 starter kit contains:

- `README.md`, this assignment handout
- `Makefile`, which builds `cppgm++` and runs the PA5 tests
- `cppgm++.cpp`, a link to the editable `dev/cppgm++.cpp` entry point
- the grammar for this assignment called `pa5.gram`
- an HTML grammar explorer of `pa5.gram` in the sub-directory `grammar/`
- `scripts/run_all_tests.pl` and `scripts/compare_results.pl`
- `tests/spec/`, clause-anchored syntax tests
- `tests/general/`, broader parser tests
- checked-in `.ref` and `.ref.exit_status` files used as the oracle

Your main editable file is `dev/cppgm++.cpp`. You may add or change other
implementation files under `dev/` as needed. Preserve the supplied test inputs,
harness scripts and grammar. Corrections to reference outputs follow the
[reference policy](../TESTING_AND_REFERENCES.md).

The starter `dev/cppgm++.cpp` is a command-line scaffold for the long-lived
`cppgm++` binary. It establishes the expected mode flags and help path; the AST
parser, AST data model, and AST output behavior are your assignment work.

The checked-in reference files under `tests/` are the grading oracle. The
`cppgm++-ref` wrapper is available for investigation under the root reference
policy; the harness runs your compiler.

### Build And Test Commands

From the `pa5/` directory:

```sh
make
make check TEST='tests/spec/100-*.t'
make check TEST='tests/general/100-*.t'
make check TEST='tests/general/200-*.t'
make check TEST='tests/general/300-*.t'
make test
```

`make` builds `cppgm++`. `make test` runs the local PA5 suite.

### Required Driver Surface

PA5 requires:

- `--emit-ast`
- `-o <outfile>`
- one or more source-file operands

The following modes and driver features are not part of PA5:

- `--emit-types`
- `--emit-semantics`
- `--emit-lowir`
- native compile or link driver behavior such as `-c`, `-E`, `-I`, `-L`, or
  `-l`
- hosted-toolchain query flags such as `--version`, `-v`, `-dumpmachine`, or
  `-print-search-dirs`

Behavior is undefined unless the command line has this form:

```sh
cppgm++ --emit-ast -o <outfile> <srcfile1> [<srcfile2> ...]
```

### Input Contract

The authoritative source syntax is the shared `cppgm++` source grammar, exposed
for this assignment as `pa5.gram`. The grammar defines accepted syntax only;
the PA5 AST-output requirements are defined by the Required Features and Out Of
Scope sections below.

The grammar uses the token vocabulary from preprocessing and the extended BNF
notation explained in [parsing.md](parsing.md). That guide defines the small
name-category table and lexical fallback used before full semantic lookup
exists. Preserve names and template arguments in structured syntax nodes.
Known declarations and parameter kinds take precedence over lexical hints.

The shared grammar intentionally includes syntax whose semantic meaning is added
by later assignments. PA5 is responsible for structured syntax preservation,
not for deciding whether a later semantic or lowering stage would accept the
program.

Behavior is undefined for input that:

- does not match `pa5.gram`
- requires semantic disambiguation beyond the PA5 requirements
- depends on ill-formed-program diagnostics that PA5 is not required to issue

If this README and `pa5.gram` disagree about accepted source syntax, use
`pa5.gram`.

### Output Format

On success, `cppgm++` writes the AST dump to `<outfile>`.

The first line is:

```text
<n> translation units
```

where `<n>` is the number of source files on the command line.

For each translation unit, in command-line order, the output contains:

```text
start translation unit <k>
...
end translation unit
```

where `<k>` is the 1-based translation-unit index.

Between those wrapper lines, write a deterministic line-oriented tree dump
rooted at:

```text
translation-unit
```

The dump must use explicit syntax nodes for supported constructs. The checked-in
`.ref` files define the exact indentation, ordering, spelling, and leaf-token
format expected by the tests. Common node families include:

- declarations such as `empty-declaration`, `simple-declaration`,
  `function-definition`, `class-specifier`, `enum-specifier`,
  `template-declaration`, and `static-assert-declaration`
- declarator and type-id syntax
- statement nodes such as `compound-statement`, `if-statement`,
  `return-statement`, and `expression-statement`
- expression nodes such as `id-expression`, `literal`, `call-expression`,
  `new-expression`, `cast-expression`, and `lambda-expression`
- unresolved syntax nodes that later semantic assignments will classify

PA5 output is a syntax tree, not a semantic dump. For example, base specifiers
and constructor mem-initializers should preserve the unresolved names and
argument syntax, but PA5 does not decide whether a name denotes a base, member,
or delegating constructor target.

Standard output and standard error are ignored by the automated PA5 tests.
They may contain diagnostics or tracing text, but they are not part of the
grading contract.

### Error Handling

If preprocessing, tokenization, or parsing fails, `cppgm++` must exit with
`EXIT_FAILURE`.

The contents of `<outfile>` are unspecified on failure. For failing tests, the
harness compares only the named exit status, not diagnostic text and not the
output file.

### Required Features

PA5 must build structured AST nodes for the source subset that later PA6 and
PA7 passes consume, including:

- translation units in command-line order
- namespace definitions and aliases
- using directives and using declarations
- alias declarations and `typedef`
- simple declarations and function definitions
- class, struct, union, enum, and scoped enum declarations
- class members, access labels, bit-fields, base clauses, and constructor
  initializer syntax
- template declarations with type, non-type, and template-template parameters.
  A parameter's kind belongs to that declaration: reusing a name that an
  earlier declaration gave a different kind must not carry the earlier one
  over, or the later parameter stops disambiguating the same way
- dependent template arguments whose qualified operands participate in
  parenthesized logical expressions, even when the prefix could also begin a
  type-id
- common template-id (including operator and literal-operator template ids),
  explicit-instantiation, and explicit-specialization syntax
- `static_assert`
- declarator-derived syntax, including pointers, references, arrays, function
  parameter clauses, exception specifications, cv/ref qualifiers, default
  arguments, and trailing return types
- structured `type-id` syntax in casts, `new`, `typeid`, `alignof`,
  `sizeof(type)`, and `noexcept`
- compound statements, selection statements, iteration statements, labels,
  `break`, `continue`, `goto`, `return`, `throw`, and `try` / `catch`
- literals, identifiers, parenthesized expressions, calls, subscripts, member
  access, unary and binary operators, conditional expressions, assignments,
  comma expressions, keyword casts, `new` / `delete`, lambdas, `typeid`,
  `alignof`, `sizeof`, `noexcept`, and braced init lists

Unsupported statement or expression forms inside the PA5 boundary should fail
during parsing rather than being preserved as opaque placeholder nodes.

### Out Of Scope

PA5 does not require:

- full semantic name lookup beyond the syntactic categories described above
- type checking
- overload resolution
- conversion ranking
- template argument deduction
- semantic validation or constant evaluation of non-type template parameters
  and arguments
- complete parsing of every C++11 corner case
- semantic resolution of `template-id` versus `<`
- semantic resolution of ambiguous template arguments such as `foo<T*p>`

Those topics belong to later semantic and template assignments.

### Testing And Grading Contract

The PA5 harness discovers every `.t` file under the requested test root.
For each test case `x.t`, it runs:

```sh
cppgm++ --emit-ast -o x.my x.t
```

and records `x.my.exit_status`.

Comparison rules:

- `x.my.exit_status` must match `x.ref.exit_status`.
- If the reference status is `EXIT_FAILURE`, the test passes after the exit
  status comparison.
- If the reference status is `EXIT_SUCCESS`, `x.my` must match `x.ref` exactly.
- Standard output and standard error are not compared.

The local suite is split by role:

- `tests/spec/` contains small tests tied to specific C++11 syntax clauses.
  These files begin with an `N3485 focus` comment.
- `tests/general/` contains broader parser tests, cross-feature syntax
  combinations, and useful intake cases that are not a single-clause oracle.

### Design Notes (Non-Normative)

The simplest successful design keeps these concerns separate:

- preprocessing and token preparation
- recursive-descent or table-driven parsing
- AST node ownership
- deterministic AST printing

Plan for ambiguity. Some C++ syntax cannot be classified fully until later
semantic passes exist. PA5 should still preserve the required structure in the
tree so later assignments can classify it without reparsing source text.

### Handoff

The next assignment builds scopes, bindings, and canonical types on this AST.
Keep declarations, declarators, names, expressions, and source locations
available as structured data. Later semantic information can refine ambiguous
names without replacing the parser or reparsing token spellings from its dump.
Use stable node ownership and keep printing separate from parsing; no particular
arena, class hierarchy, or reference implementation layout is required.

### Multi-file test groups

For a root `name.t`, companions such as `name.t2` are additional primary
source files in the same invocation, in lexical order. They remain separate
translation units; the harness counts the group once. Both batch and ordinary
execution use this file interface. Headers and other support files stay with
the owning case and are not concatenated into one source file.
