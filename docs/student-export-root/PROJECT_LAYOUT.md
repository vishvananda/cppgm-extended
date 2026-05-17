# Project Layout

`cppgm` is organized as one cumulative compiler implementation plus PA-specific
assignment harnesses.

## Repository Layout

- `Makefile`: root build, test, report, strict, reference, and inception targets
- `dev/`: compiler entry points
- `dev/src/`: shared compiler implementation files and support headers
- `dev/frontend_source_sets.mk`: per-tool lists of `dev/src/*.cpp` files to
  link into each compiler binary
- `pa1/` through `pa37/`: assignment handouts, Makefiles, tests, scripts, and
  reference fixtures
- `cppgm.tests/`: shared course tests used by assignment harnesses
- `doc/`: public reference material, including `doc/n3485.txt`
- `obj/`: generated build artifacts
- `reference-binaries/`: reference-binary manifest; the large binary payloads
  download automatically when `*-ref` wrappers need them

Most `paN/` directories are thin wrappers around the shared implementation.
They define the milestone contract and test surface; production compiler code
should stay in `dev/` and `dev/src/`.

When you add a new implementation source file under `dev/src/`, also add its
basename to each tool that needs it in `dev/frontend_source_sets.mk`. Use the
path without `.cpp`; for example, `dev/src/parser/foo.cpp` is listed as
`parser/foo`.

## Assignment Arc

- PA1-PA5: preprocessing
- PA6-PA9: grammar recognition, namespace semantics, and CY86 output
- PA10-PA12: AST, types, lookup, conversions, calls, and overload resolution
- PA13-PA22: LowIR, C++ lowering, object model, templates, and constexpr
- PA23-PA25: native backend, linking, and exception/runtime support
- PA26-PA34: C++11 language closure, host ABI, and hosted compatibility
- PA35-PA36: LowIR and machine-backend optimization
- PA37: inception, rebuilding `cppgm++` with `cppgm++`

## Assignment Map

| PA | Tool or target | Main focus |
| --- | --- | --- |
| PA1 | `pptoken` | preprocessing tokens and translation phases 1-3 |
| PA2 | `posttoken` | post-token conversion and literals |
| PA3 | `ctrlexpr` | controlling-expression evaluation |
| PA4 | `macro` | macro definition and expansion |
| PA5 | `preproc` | full preprocessing translation units |
| PA6 | `recog` | C++ grammar recognition |
| PA7 | `nsdecl` | namespace declarations and semantic output |
| PA8 | `nsinit` | namespace initialization and mock program images |
| PA9 | `cy86` | executable output through the CY86 model |
| PA10 | `cppgm++ --emit-ast` | AST construction |
| PA11 | `cppgm++ --emit-types` | types, scopes, and lookup |
| PA12 | `cppgm++ --emit-semantics` | conversions, initialization, and overload resolution |
| PA13 | `lowir2cy86` | LowIR parsing and execution scaffold |
| PA14 | `cppgm++ --emit-lowir` | procedural C++ lowering to LowIR |
| PA15 | `cppgm++ --emit-lowir` | basic classes and object layout |
| PA16 | `cppgm++ --emit-lowir` | value semantics and assignment |
| PA17 | `cppgm++ --emit-lowir` | virtual dispatch |
| PA18 | `cppgm++ --emit-lowir` | basic templates |
| PA19 | `cppgm++ --emit-lowir` | specialization and compile-time evaluation |
| PA20 | `cppgm++ --emit-lowir` | constant evaluation |
| PA21 | `cppgm++ --emit-lowir` | template entities and specialization model |
| PA22 | `cppgm++ --emit-lowir` | deduction, substitution, and SFINAE completion |
| PA23 | `lowir2native` | native backend from LowIR |
| PA24 | `cpplink` | separate compilation and linking |
| PA25 | `cppeh` | exception/runtime support |
| PA26 | `cppgm++ --emit-lowir` | core language closure |
| PA27 | `cppgm++ --emit-lowir` | advanced language closure |
| PA28 | `cppgm++ --emit-lowir` | non-virtual multi-base object model |
| PA29 | `cppgm++ --emit-lowir` | virtual/RTTI object-model completion |
| PA30 | `cppgm++` | compile/link driver integration |
| PA31 | `cppgm++ -c` | host-linkable object interoperability |
| PA32 | `cppgm++ -c` | host C++ ABI and runtime behavior |
| PA33 | `cppgm++ -E`, `cppgm++ -c` | hosted header/source compatibility |
| PA34 | `cppgm++ -c` | hosted header-emitted link/runtime compatibility |
| PA35 | `lowiropt` | LowIR optimization |
| PA36 | `lowir2native -O1/-O2` | machine/backend optimization |
| PA37 | inception targets | rebuild `cppgm++` with `cppgm++` |

## PA37

PA37 is different from the earlier one-binary assignments. Its goal is
inception: build `cppgm++` with `cppgm++` and match the host build. The
`test-through` ladder gives intermediate checkpoints, but the final target is a
matching self-built compiler.
