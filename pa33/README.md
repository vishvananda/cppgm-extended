## CPPGM Programming Assignment 33 (`cppgm++ -E` / `cppgm++ -c`)

### Overview

PA33 is the hosted source/header compatibility assignment. Its job is to make
`cppgm++` preprocess and compile the hosted standard-library and vendor
extension environment needed by later bootstrap-style builds.

This milestone is intentionally distinct from the previous host-toolchain
assignments:

- PA31 is about ordinary host-linkable object files.
- PA32 is about host C++ ABI/runtime correctness after host link.
- PA33 is about preprocessing, parsing, semantic analysis, and lowering
  compatibility for hosted source/header inputs.

To complete PA33, implement these goals:

- hosted preprocessor compatibility
- GNU/Clang parser concessions used by the selected hosted headers
- GNU builtin type and literal forms used by the selected hosted environment
- builtin traits, transforms, and intrinsics used by that hosted environment
- enough hosted-header/source compile compatibility to make later bootstrap
  work realistic

### Prerequisites

Complete PA32 before starting this assignment.

You will want to reuse:

- the full earlier language, template, and lowering stack
- the PA31/PA32 `cppgm++ -c` host-object path
- the PA32 host-ABI-compatible output path
- the earlier preprocessor pipeline, now with hosted-driver controls

The tests assume a Linux shell environment with `make`, `bash`, `perl`, and a
working host C++ compiler with hosted C++ headers installed.
You may override the compiler with `CXX=...`. `CPPGM_HOST_CXX` selects the host
compiler used for builtin macro/include probing. If it is not set, it defaults
to `CXX`.

The hosted tests rely on the host compiler's target, predefined macros, standard
library include paths, and standard-library selection flags. When you use a
non-default standard library, pass the same choice through `CPPGM_STDLIB_FLAGS`
so the course compiler and host compiler agree.

### Starter Kit

The starter kit provides:

- `dev/cppgm++.cpp`, populated from the `cppgm++` scaffold for the cumulative
  PA10+ compiler driver
- the shared `dev/` sources needed by the scaffold
- `pa33/cppgm++.cpp`, a link to `../dev/cppgm++.cpp`
- `pa33/Makefile`
- `pa33/scripts/`, the hosted preprocessor/compile test harness
- `pa33/tests/preproc/`, hosted preprocessor tests and references
- `pa33/tests/compile/`, hosted compile-only tests and references

Student code changes should go in `dev/`, especially `dev/cppgm++.cpp` and the
shared implementation files it calls. Do not edit generated `.my` files. Test
inputs and references are part of the handout unless your instructor asks you
to add or update tests.

There is no separate PA33 reference binary in the starter kit. The checked-in
`.ref.*` files are the oracle.

### Driver Surface

Previously required:

- the PA30 compile/link surface: `-c`, default link mode, `-I`, `-L`, `-l`, and
  `--target`
- the PA31/PA32 host-compatible behavior for the relevant compile-mode subset

New or newly required in PA33:

- hosted preprocess mode: `-E`
- hosted preprocessor-control flags:
  - `-D <macro>` and `-D<macro>`
  - `-U <macro>` and `-U<macro>`
  - `-include <file>`
  - `-isystem <dir>` and `-isystem<dir>`
- direct driver query forms:
  - `--version`
  - `-v`
  - `-dumpmachine`
  - `-dumpversion`
  - `-print-search-dirs`
- compatibility handling for common build-system flags that should either be
  honored or harmlessly accepted when they do not affect the tested output

### Command-Line Contract

PA33 continues extending the same `cppgm++` frontend used in PA30-PA32.

Required preprocess forms:

```sh
cppgm++ -E -o <outfile> <srcfile>
cppgm++ -E <srcfile1> <srcfile2> ...
cppgm++ -E -D <macro> -U <macro> -include <file> -isystem <dir> <srcfile>
```

Required compile forms:

```sh
cppgm++ -c -o <objfile> <srcfile>
cppgm++ -c <srcfile1> <srcfile2> ...
cppgm++ -c -D <macro> -U <macro> -include <file> -isystem <dir> -o <objfile> <srcfile>
```

Query flags are required only as direct driver queries. For example:

```sh
cppgm++ --version
cppgm++ -dumpmachine
cppgm++ -print-search-dirs
```

### Output Format

`cppgm++ -E` shall write the same structured posttoken/preprocessor stream
format used by the PA5 `preproc` frontend. When `-o <outfile>` is present, that
stream is written to `<outfile>`.

`cppgm++ -c` shall continue to write host-linker-compatible relocatable object
files as in PA31/PA32.

The new PA33 contract is not a new object format. It is the ability to
preprocess and compile hosted source/header inputs successfully through the
`cppgm++` path.

### Error Handling

If preprocessing, parsing, semantic analysis, lowering, object emission, or
output writing fails, `cppgm++` shall exit with failure.

For compile-only tests, exact diagnostics are not the grading contract. The
harness compares exit status and any checked output sidecars. If the reference
run fails, stdout and stderr are diagnostic side effects rather than required
output.

### Testing

Run the PA33 suite with:

```sh
make test
```

To run one test through the shared check target:

```sh
make check TEST=tests/preproc/300-has-include.t
make check TEST=tests/compile/500-builtin-transforms-and-traits.t
```

PA33 has two test directories:

- `tests/preproc/`: hosted preprocessor compatibility. The oracle is the
  PA5-style structured preprocessor stream plus exit status.
- `tests/compile/`: hosted compile-only compatibility. The oracle is successful
  object emission, compile exit status, and any stdout/reference sidecars used
  by the harness.

The checked-in tests are hosted/vendor compatibility cases, standard-library
sentinels, reducers, and bootstrap-facing compile smokes rather than direct
N3485 clause tests.

Optional sidecars include:

- `x.env`: environment variables for one test, such as additional standard
  include paths
- `x.compile.flags`: extra flags passed to `cppgm++ -c`

The default preprocessor references are intentionally host-agnostic. The test harness checks that checked-in PA33 preprocessor refs do not accidentally
pin local host macro values such as platform-specific integer or floating-point
limits.

### Assignment Boundary

PA33 owns hosted compatibility needed before bootstrap, including:

- predefined macro import, `_Pragma`, `__has_*`, `#include_next`, `#warning`,
  ignored unknown pragmas, and hosted hex-float preprocessing forms such as
  `0x1p+4`
- GNU/Clang parser concessions commonly exercised by the selected hosted
  headers, including dependent nested-angle disambiguation, nested qualified
  template-ids used as outer template arguments, builtin-trait identifiers
  referenced as ordinary names, GNU `__decltype`, parenthesized
  throw-expressions emitted by hosted helper macros, and GNU builtin float type
  specifiers such as `__float128` / `_Float128`
- builtin traits, transforms, intrinsics, and builtin families used during
  hosted compile acceptance
- semantic and lowering compatibility for hosted source patterns used by those
  headers and by the bootstrap source base, including post-declarator parameter
  attributes, explicit specializations of primary-template member functions,
  and non-standard hex-float compile acceptance on ordinary floating types

Standard-language bugs discovered here should still be fixed in their true
earlier owner stage when appropriate. PA33 owns the hosted compatibility
pressure, not a second copy of every earlier language rule.

### Out Of Scope

The following are out of scope for PA33:

- host object/link/runtime contracts already owned by PA31 and PA32
- hosted header-emitted link/runtime behavior, which belongs in PA34
- full build-system emulation beyond the documented query and compatibility
  flags
- recursive hosted-header coverage reporting
- bootstrap or self-host builds

### Design Notes (Non-Normative)

Hosted compatibility is easiest to approach as a sequence of small compatibility
surfaces: preprocessor probes, parser concessions, builtin traits/types, and
then semantic/lowering cases. Keep fixes tied to the source pattern being
exercised. Avoid making broad source-text special cases when an earlier
semantic or template representation can carry the information directly.

### Stage Handoff

The next stage is PA34, which keeps the same hosted source/header environment
but raises the contract from "it compiles" to "its emitted code also links and
runs."
