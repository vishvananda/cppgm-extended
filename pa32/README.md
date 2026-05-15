## CPPGM Programming Assignment 32 (`cppgm++ -c`)

### Overview

Write one C++ application called `cppgm++`.

PA32 is the host C++ ABI/runtime interoperability assignment. It builds on the
ordinary host-linkable object contract from PA31 and makes the behavior of the
host-linked program part of the assignment contract.

The main PA32 question is: once host link succeeds, does the resulting program
behave correctly under the ordinary host C++ ABI/runtime?

The tested ABI/runtime surface includes:

- virtual dispatch, vtable ownership, and imported/exported vtables
- RTTI object ownership and `dynamic_cast` / `typeid`
- covariant return adjustment
- host exception handling in the exercised throw/catch/rethrow/cleanup subset
- host-compatible unwind and relocation facts where the tests inspect objects

### Prerequisites

Complete PA31 before starting this assignment.

You will want to reuse:

- the full C++ language pipeline through PA31
- the PA31 host-compatible `cppgm++ -c` path
- the PA23/PA24 native object emission path
- the PA25 exception/runtime lowering concepts

The tests assume a POSIX-like shell environment with `make`, `bash`,
`perl`, and a working host C/C++ toolchain. The harness selects host tools from:

- `CPPGM_HOST_CXX` or `CXX` for the host C++ compiler/link driver
- `CPPGM_HOST_CC` or `CC` for host C helper objects

If those are not set, the harness searches for common compilers such as
`clang++`, `g++`, `c++`, `clang`, `gcc`, and `cc`. Archive and inspection tests
also require `ar`, `nm`, and `readelf`. The checked-in tests assume the normal
x86_64 Linux host C++ ABI.

Some PA32 object-inspection tests use course-provided helper tools from earlier
assignments, such as `cpplink` and `cppeh`, to compare the host exception object
surface. Those helpers support the harness; the PA32 assignment binary is
still `cppgm++`.

### Starter Kit

The starter kit provides:

- `dev/cppgm++.cpp`, populated from the `cppgm++` scaffold for the cumulative
  PA10+ compiler driver
- the shared `dev/` sources needed by the scaffold
- `pa32/cppgm++.cpp`, a link to `../dev/cppgm++.cpp`
- `pa32/Makefile`
- `pa32/scripts/`, the host-ABI test harness
- `pa32/tests/general/`, the PA32 tests and checked-in reference files

Student code changes should go in `dev/`, especially `dev/cppgm++.cpp` and the
shared implementation files it calls. Do not edit generated `.my` files. Test
inputs and references are part of the handout unless your instructor asks you
to add or update tests.

There is no separate PA32 reference binary in the starter kit. The checked-in
`.ref.*` files are the oracle.

### Command-Line Contract

PA32 does not introduce new command-line flags. It reuses the PA31 compile-mode
surface:

```sh
cppgm++ -c -o <objfile> <srcfile>
cppgm++ -c --target <target> -o <objfile> <srcfile>
cppgm++ -c -I <dir> -o <objfile> <srcfile>
cppgm++ -c -I<dir> -o <objfile> <srcfile>
cppgm++ -c --target <target> -I <dir> -o <objfile> <srcfile>
cppgm++ -c --target <target> -I<dir> -o <objfile> <srcfile>
```

`<target>` may be `linux` or the corresponding x86_64 Linux host triple form
accepted by your implementation. PA32 only requires compile mode. The normal
PA32 final link is performed outside `cppgm++` by the host C++ compiler driver.

### Output Format

`cppgm++ -c` shall continue to write one host-linker-compatible relocatable
object file to `<objfile>`.

The PA32 tests do not compare object bytes directly. They observe:

- `cppgm++ -c` exit status
- host final-link exit status
- final program exit status
- final program standard output
- optional object-inspection output for ABI, unwind, relocation, RTTI, vtable,
  thunk, and symbol-ownership checks

### Error Handling

If preprocessing, parsing, semantic analysis, lowering, object emission, or
output writing fails, `cppgm++` shall exit with failure.

For negative tests, exact diagnostics are not the grading contract. The harness
compares exit status first. If the reference compile/link path fails, stdout and
stderr are diagnostic side effects rather than required output.

### Testing

Run the PA32 suite with:

```sh
make test
```

To run one test through the shared check target:

```sh
make check TEST=tests/general/100-host-eh-same-tu-throw-catch.t
```

The local tests live in `tests/general/`. They cover host C++ ABI/runtime
behavior, host-linked exception handling, RTTI, vtables, thunks, and object
inspection around those host-runtime surfaces. They are not direct N3485 clause
tests.

For each test anchor `x.t`, companion C++ sources are named:

```text
x.t.1
x.t.2
...
```

Optional sidecars control or check the host flow:

- `x.compile.flags`: extra flags passed to `cppgm++ -c`
- `x.link.flags`: extra flags passed to the host link driver
- `x.lib.*`: host-built C or C++ helper sources
- `x.inspect.cmd`, `x.inspect.expect`, or `x.inspect.plan`: object-inspection
  checks that use the host symbol and object tools

The checked-in PA32 tests cover:

- same-TU and cross-TU host exception throw/catch behavior
- cleanup, rethrow, noexcept termination, and foreign catch-all behavior in the
  exercised subset
- virtual dispatch, imported/exported vtable ownership, and polymorphic header
  duplication
- RTTI-driven `dynamic_cast` and `typeid`
- covariant return adjustment
- host ABI mangling for dependent/template/lambda/standard-library-adjacent
  names needed by this milestone
- object facts such as unwind sections, relocation classes, weak/undefined
  symbols, and vtable/RTTI ownership when a test includes an inspect sidecar

### Assignment Boundary

PA32 owns practical host-linked C++ ABI/runtime behavior.

To complete PA32, preserve this behavior within the supported subset:

- virtual dispatch and imported/exported vtable ownership
- RTTI-driven `dynamic_cast` / `typeid`
- covariant return adjustment
- ordinary host-linked throw/catch/rethrow behavior
- cleanup and unwind behavior visible to the host runtime
- foreign catch-all interaction in the tested subset

If host link succeeds but the host C++ ABI/runtime behavior is wrong, the issue
belongs in PA32.

### Out Of Scope

The following are out of scope for PA32:

- private course-only exception/runtime ABI details that are not visible through
  the host-linked program or object checks
- hosted standard-library header/source compatibility, which belongs in PA33
- hosted header-emitted link/runtime behavior, which belongs in PA34
- bootstrap or self-host builds

### Design Notes (Non-Normative)

PA32 is not just a runtime-output assignment. Runtime behavior is the primary
oracle, but some tests inspect object facts because host C++ ABI correctness is
often decided before the program starts: symbol names, weak ownership, unwind
sections, RTTI/vtable objects, and relocation classes must match the host
toolchain's expectations closely enough for ordinary linking and unwinding.

### Stage Handoff

The next stage is PA33, which shifts from host ABI/runtime ownership to hosted
source/header compatibility.
