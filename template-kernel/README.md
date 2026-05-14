# Template Kernel Experiment

This directory holds the experimental surface for an isolated
template-semantics tool.

Tool:

- `tmplsolve`

Input extension:

- `.tkq`

Purpose:

- model template selection, default binding, deduction, substitution, and
  observable template hot-path counters without requiring the rest of the
  compiler pipeline

This is still an experimental surface, not a numbered public assignment
directory. The point is to test whether template work should be taught and
validated through:

1. a new standalone assignment, or
2. a new primary oracle reused by the existing template assignments

## Current Contract

Each `.tkq` file contains:

- template declarations
- zero or more queries

Each query produces a deterministic structured answer in a `.ref` file.

The syntax is deliberately line-oriented and smaller than C++ source. Students
should work on template behavior here, not preprocessing, full parsing, LowIR,
or hosted/STL bringup.

## Supported Declarations

- `class_template`
- `variable_template`
- `alias_template`
- `partial_class`
- `partial_variable`
- `explicit_class`
- `explicit_variable`
- `function_template`

Current type grammar:

- named types such as `int` and `char`
- cv-qualified named types such as `const int` and `volatile int`
- pointer types such as `T*`
- reference types such as `T&` and `T&&`
- template-id types such as `Box<T>`
- structural function-shape types such as `fn<int, int>`
- structural array-shape types such as `arr<2, const char>`
- `nondeduced<T>` wrappers for simple non-deduced-context tests

Current template parameter grammar:

- type parameters
- bounded value parameters: `value int N`, `value bool B`
- optional defaults such as `type U = type T` or `value int Gate = value 0`

Current function requirement grammar:

- `when same(T, int)`

## Supported Queries

- `query select_class Name <...>`
- `query select_variable Name <...>`
- `query expand_alias Name <...>`
- `query bind_defaults class|variable|alias Name <...>`
- `query deduce_call Name args (...)`
- `query stats`

## Output Rules

- answers are line-oriented
- each query result starts with `query <n> ...`
- success/failure is explicit via `status ok` or `status error`
- selected specialization/candidate identity is explicit
- resolved bindings are printed deterministically
- defaulted parameters are reported as `default <name> = ...`
- rejected candidates report deterministic drop reasons
- `query stats` exposes cumulative counters for the file

## Example

```txt
template_kernel_v1

class_template Box <type T, type Tag = type T>
partial_class Box same <type U> args <type U, type U>
function_template same pair <type T> params (type T, type nondeduced<T>) return int

query select_class Box <type char>
query deduce_call same args (type int, type int)
query stats
```

## Current Test Bank

The checked-in tests currently cover:

- class partial specialization selection
- function template deduction and drop reasons
- default binding plus explicit class specializations
- default template argument merge across redeclarations
- variable template selection
- alias default binding and expansion
- non-deduced contexts and simple `requires`-style filtering
- partial specialization matching after primary default binding
- function deduction over a type that uses a defaulted class-template argument
- ref-vs-const-ref overload selection
- ambiguous cv-pointer partial ordering
- pointer qualification deduction
- function-reference-shaped parameter binding from `pa34`
- forwarding-reference array-shape binding from `pa33`
- function-shape decomposition into return and parameter types
- bounded integer nontype deduction through array extents
- bool-plus-int-sentinel selection in an enable-if-style pattern
- stats/counter output

The current suite lives under [tests](/private/tmp/cppgm-template-kernel-20260416/template-kernel/tests).
Source reductions are tracked in [REDUCTIONS.md](/private/tmp/cppgm-template-kernel-20260416/template-kernel/REDUCTIONS.md).

## Local Commands

- `make -C dev tmplsolve`
- `make -C template-kernel test`
- `make test-template-kernel`

## What This Surface Does Not Own

- preprocessing
- general C++ parsing
- name lookup outside this model
- body instantiation
- LowIR lowering
- native code generation
- STL compilation as the first oracle

## Next Expansion Areas

The current prototype is enough to support assignment design and reduced
template regressions. The next likely expansions are:

- deeper partial-ordering diagnostics
- deferred/no-eager-instantiation queries
- synthetic scale/performance cases
- a reduction path from hosted/STL failures into `.tkq` cases
