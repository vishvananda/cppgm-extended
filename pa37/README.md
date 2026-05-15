## CPPGM Programming Assignment 37 (Inception)

### Overview

PA37 is the inception assignment. To complete PA37, make `cppgm++` build
`cppgm++` and have that rebuilt compiler match the host-seeded build. In
Makefile terms, the main target is:

```sh
make compare-cppgm++-inception CXX=../dev/cppgm++ CPPGM_HOST_CXX=g++
```

This builds `cppgm++-self`, builds `cppgm++-inception` with that self-built
compiler, and compares the two outputs byte for byte. Passing that comparison
means the compiler can reproduce itself from its own output.

The `test-through-*` preservation ladder is not the final product. It is the
debugging path that helps you reach inception one stage at a time. The earlier
programming assignments build a compiler sequentially by adding one language,
semantic, lowering, runtime, or backend surface at a time; the PA37 ladder uses
the same idea for self-hosting. Each rung runs the already-completed assignment
tests with a self-built checkpoint binary, so missing functionality usually
shows up at the first stage that needs it instead of only as a full
`cppgm++` inception failure.

### What PA37 Tests

PA37 does not add a new language feature, command-line mode, object format, or
runtime ABI. It reuses the compiler implementation and checks that the existing
implementation can compile itself reproducibly.

If the earlier assignment tests cover every language, lowering, runtime,
linking, and optimization feature used by your implementation, PA37 should be a
straightforward build plus a few reproducibility cleanups. In practice, the
self-build usually finds missing coverage. A failure in PA37 is usually evidence
that an earlier compiler surface accepts the assignment tests but still has a
bug in code that the compiler implementation itself happens to use.

### Prerequisites

Complete PA36 before starting PA37. PA37 reuses:

- the PA1-PA9 frontend tools
- the cumulative `cppgm++` compiler
- the native, object, runtime, and hosted compile/link surfaces
- the PA35/PA36 optimization surfaces
- the earlier `pa1` through `pa36` tests for preservation checks

PA37 also needs a host C++ compiler and linker. The host compiler links staged
object files and provides the hosted configuration used by the compiler build.

### Build Variables

Use two compiler variables when running PA37:

- `CXX=../dev/cppgm++` selects the course compiler as the compiler under test.
- `CPPGM_HOST_CXX=<host-cxx>` selects the host C++ compiler used for linking
  checkpoint programs and generating hosted compiler configuration.

For example:

```sh
make compare-cppgm++-inception CXX=../dev/cppgm++ CPPGM_HOST_CXX=g++
```

If `../dev/cppgm++` does not exist yet, the PA37 Makefile first builds it with
`CPPGM_HOST_CXX`.

### Inception Targets

The focused PA37 goal is:

```sh
make cppgm++-inception CXX=../dev/cppgm++ CPPGM_HOST_CXX=g++
make compare-cppgm++-inception CXX=../dev/cppgm++ CPPGM_HOST_CXX=g++
```

Useful broader targets are:

```sh
make inception CXX=../dev/cppgm++ CPPGM_HOST_CXX=g++
make compare-inception CXX=../dev/cppgm++ CPPGM_HOST_CXX=g++
make bitcmp CXX=../dev/cppgm++ CPPGM_HOST_CXX=g++
```

`inception` builds all inception checkpoint binaries. `compare-inception` and
`bitcmp` compare all self-built checkpoint binaries against their inception
versions.

### Intermediate Ladder

To build one checkpoint:

```sh
make pptoken-self CXX=../dev/cppgm++ CPPGM_HOST_CXX=g++
make cppgm++-self CXX=../dev/cppgm++ CPPGM_HOST_CXX=g++
```

To build checkpoints through a point:

```sh
make through-cy86 CXX=../dev/cppgm++ CPPGM_HOST_CXX=g++
make through-cppgm++ CXX=../dev/cppgm++ CPPGM_HOST_CXX=g++
make through-cppeh CXX=../dev/cppgm++ CPPGM_HOST_CXX=g++
```

To run preservation tests through an assignment stage:

```sh
make test-through-pa9 CXX=../dev/cppgm++ CPPGM_HOST_CXX=g++
make test-through-pa36 CXX=../dev/cppgm++ CPPGM_HOST_CXX=g++
```

To test one stage:

```sh
make test-pa1 CXX=../dev/cppgm++ CPPGM_HOST_CXX=g++
make test-pa10 CXX=../dev/cppgm++ CPPGM_HOST_CXX=g++
make test-pa35 CXX=../dev/cppgm++ CPPGM_HOST_CXX=g++
```

The test targets reuse the earlier assignment harnesses. PA37 changes which
binary those harnesses run; it does not change the expected PA1 through PA36
outputs.

### Checkpoint Ownership

The checkpoint used for each assignment stage is:

- `pptoken-self`: PA1
- `posttoken-self`: PA2
- `ctrlexpr-self`: PA3
- `macro-self`: PA4
- `preproc-self`: PA5
- `recog-self`: PA6
- `nsdecl-self`: PA7
- `nsinit-self`: PA8
- `cy86-self`: PA9
- `cppgm++-self`: PA10-PA12, PA14-PA22, and PA26-PA34
- `lowir2cy86-self`: PA13
- `lowir2native-self`: PA23 and PA36
- `cpplink-self`: PA24
- `cppeh-self`: PA25
- `lowiropt-self`: PA35

Checkpoint source sets are fixed by `../dev/frontend_source_sets.mk`. Do not
replace that with a generated source scan; PA37 is checking whether the known
implementation source sets can be rebuilt reproducibly.

### Working Through Failures

Treat PA37 failures as compiler bugs until proven otherwise.

Do not rewrite tests around a failure, and do not add a self-hosting special
case just to make the build move forward. Find the first incorrect behavior and
fix the underlying parser, semantic, lowering, optimizer, backend, runtime, or
reproducibility bug.

A useful workflow is:

1. Find the first failing checkpoint, source file, or preservation test.
2. Reduce the failure to the smallest source that still fails.
3. Identify the earliest assignment surface that owns that behavior.
4. Add the reducer as a focused test under the matching
   `cppgm.tests/course/paN` directory while you work on the fix.
5. Fix the underlying compiler bug and rerun the narrow stage before returning
   to the broader `test-through-*` or inception target.

For example, if `cppgm++-self` fails because a construct in `dev/src/*.cpp` is
miscompiled, reduce that construct and place the focused test in the earliest
`cppgm.tests/course/paN` directory that should have covered it. If
`compare-cppgm++-inception` builds both compilers but the bytes differ, look for
reproducibility issues such as unstable output order, generated configuration
drift, embedded paths, timestamps, or linker determinism.

The best PA37 fixes usually improve an earlier assignment surface and leave a
small focused test behind.
