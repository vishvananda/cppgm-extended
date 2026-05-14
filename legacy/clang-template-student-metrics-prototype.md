# Clang Student Metrics Prototype

This is an experimental Clang plugin prototype for the template-kernel buildout.

It exists to answer one narrow question:

- can a stock Clang frontend plugin emit the core **student-facing** template
  observations from real C++ source without modifying the compiler?

Current answer:

- yes for selected template uses and final bindings
- no for loser/drop reasons and internal fallback metrics

## What The Plugin Emits

The plugin name is `student-template-metrics`.

It emits one JSON document per translation unit with:

- `class_use` events for class template specializations spelled in the main file
- `alias_use` events for alias template uses spelled in the main file
- `variable_use` events for variable template references spelled in the main
  file
- `function_call` events for resolved function-template calls in the main file

Each event carries a student-oriented surface:

- use-site location
- template name
- selected declaration kind when Clang exposes it directly
  - `primary`
  - `partial`
  - `explicit`
- selected declaration location
- final template arguments
- per-parameter bindings with a source tag:
  - `explicit`
  - `deduced`
  - `defaulted`

That is enough to prototype the core “what did the compiler select and how were
the template parameters filled?” side of the assignment surface.

The current demo shows an important nuance:

- class and variable template defaults are visible cleanly
- alias expansion is visible cleanly, but alias default bindings are not fully
  reconstructed from stock AST alone
- function-template calls expose the selected callee and final arguments, but
  stock AST does not give a perfectly clean deduced-vs-defaulted split for
  every non-explicit function template argument

## What It Does Not Emit

A pure plugin over stock AST/Sema hooks does **not** give us a clean
student-facing stream for:

- dropped overload/template candidates
- deduction failure reasons for the losers
- ambiguity detail beyond the compile error itself
- internal reparsing/fallback counters like `text_fallbacks`

Those require deeper Sema instrumentation or a patched frontend, not just a
loadable plugin.

## Build

The plugin build is intentionally optional and does not participate in the
normal repo build.

From `dev/`:

```bash
make clang-template-student-metrics-plugin
```

That looks for Homebrew LLVM under:

- `/usr/local/opt/llvm`
- `/opt/homebrew/opt/llvm`

On macOS it builds a loadable `.dylib`. On Linux it builds a `.so`.

## Demo

The sample source lives at
[template-kernel/clang-plugin/student-template-metrics-demo.cpp](/private/tmp/cppgm-template-kernel-20260416/template-kernel/clang-plugin/student-template-metrics-demo.cpp).

Run the sample from `dev/`:

```bash
make clang-template-student-metrics-demo
```

Or directly:

```bash
/usr/local/opt/llvm/bin/clang++ -std=c++17 -fsyntax-only \
  -Xclang -load -Xclang ./clang-template-student-metrics-plugin.dylib \
  -Xclang -add-plugin -Xclang student-template-metrics \
  ../template-kernel/clang-plugin/student-template-metrics-demo.cpp
```

## Why This Matters

This plugin is a useful baseline for the template-kernel work because it tells
us which parts of the public/student template contract can be recovered from a
real hosted compiler without inventing custom IR first.

Today that looks like:

- **possible from stock Clang plugin**
  - resolved class/variable template selection
  - alias expansion result
  - resolved function-template call target
  - final parameter bindings
  - whether each binding was explicit, deduced, or defaulted

- **not available as a clean stock plugin surface**
  - loser candidates
  - drop reasons
  - internal fallback/reparse metrics

That split is a good fit with the earlier template-kernel distinction:

- student-facing semantic contract
- maintainer-only optimization/debug contract
