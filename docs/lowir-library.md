# Embedding the LowIR backend

The optional `libcppgm_lowir` library lets another frontend construct typed
LowIR and obtain a host-linkable x86-64 object in the same process. The library
uses the compiler's existing model, validator, optimizer and object emitter.
It contains no source frontend, LowIR text reader or command-line driver.

Build both library forms and run the C consumer checks:

```sh
make -C dev lowir-library
make test-lowir-library
```

This produces `dev/libcppgm_lowir.a` and `dev/libcppgm_lowir.so` (or `.dylib`
on macOS). Its objects use a separate `obj/lowir-library` directory and PIC;
normal tool and self-host objects keep their existing flags. Choose a host
compiler and standard library consistently, passing `CXX`, `CPPGM_HOST_CXX`
and `CPPGM_STDLIB_FLAGS` as with a normal build.

The public C header is [`dev/src/lowir_interface.h`](../dev/src/lowir_interface.h).
A complete C producer is in
[`pa27/tests/lowir-library/client.c`](../pa27/tests/lowir-library/client.c).
Compile C sources with a C compiler; link with the C++ host driver. For example:

```sh
cc -std=c11 -Idev/src -c producer.c -o producer.o
g++ producer.o dev/libcppgm_lowir.a -pthread -o producer
# Or use -Ldev -lcppgm_lowir and configure the shared library's loader path.
```

## Producer sequence

1. Create a module with `CPPGM_LOWIR_ABI_VERSION` and its target: Linux ELF (0)
   or macOS Mach-O (1). Both targets use the x86-64 object backend.
2. Intern top-level symbols once; IDs support forward references. Add function
   signatures from numeric type records. Define local functions directly and
   declare unresolved externals before compilation. A declaration can also be
   upgraded to a definition with the same signature ID. Functions use C language
   linkage. Conventional entry/init/fini roles are applied as by the text reader.
3. Supply structured global initializers. For each function, reserve writable
   instruction, operand, literal, slot and layout buffers with
   `cppgm_lowir_body_reserve`. Write records directly into those library-owned
   pools. Allocate dense temporary IDs only when emitting definitions; parameters
   come first. An instruction's construction-time `location` carries its block
   ID. Supply the block-layout permutation and commit the initialized counts with
   `cppgm_lowir_body_commit`. Commit binds definition metadata and block links
   in place; it neither copies records nor remaps IDs.
4. Compile at O0–O3. The library validates construction once, runs the ordinary
   optimizer, selects and encodes functions, and releases their bodies.
5. Read the module-owned object byte buffer. Write or link those bytes using
   the embedding application's existing object/link workflow, then destroy the
   module. There are no subprocesses or temporary files inside the library.

The authoritative records and batched construction contract live in the shipped
LowIR scaffold, `dev/src/lowir_records.h` and `dev/src/lowir_interface.h`. The compiler builder, optimizer,
backends and foreign API use these same types. C uses the fixed-width projection
of the same declarations; its enum constants are generated from `lowir_model.h`.
No duplicate private/wire layouts, per-instruction adapter or offset matching is
needed. ABI version 2 replaces the prototype copying body API. C/C++ producers
write typed records directly; foreign FFI producers use corresponding stores.
Pointers expire on reserve, commit, compilation or destruction; IDs survive growth.

Provide known capacity hints to reserve once. Otherwise pools grow geometrically,
which may relocate their allocations. There is no second retained instruction
or operand representation and no instruction interning, parsing, flattening or
ID-remapping pass. Commit performs one metadata-binding pass, followed by the
shared structural validator before compilation.

Symbol strings, signature parameters and global initializer arrays are copied
once. Object encoding and the caller's eventual file write remain necessary work.

`cppgm_lowir_write` is an optional diagnostic view through the existing writer.
It uses the same program as compilation and must run before compilation consumes
it. The course lane checks behavior. Maintainers can additionally run
`python3 scripts/test_lowir_library.py --exact-objects` to check this solution
against its CLI byte for byte for both ELF and Mach-O.

Each module owns independent state. Serialize calls on one module. Separate
modules can construct and validate concurrently; compilation calls serialize
inside the library because the existing optimizer and register allocator reuse
process-wide scratch buffers. The lock is taken once per object. Concurrent
optimization/emission will need explicit per-compilation scratch ownership. Failures return a status or `CPPGM_NO_ID`, preserve a
module-owned diagnostic, and poison that module. Inspect the error and destroy
it. Exceptions from the compiler are contained at the foreign boundary.
A failed creation returns null. The object buffer lives until module destruction;
compilation is allowed once.

This first ABI covers the native GM producer's needs. It does not yet expose
Wasm object generation, assembly templates, debug information, or the complete
C++ aggregate ABI metadata. PA27 owns the optional O0 library lane and PA33 adds O1–O3;
[the student contract](../pa27/lowir-library.md) describes the shared interface.
These checks remain outside default assignment reports. The myc integration and
its timing comparisons are separate maintainer work, with no course dependency
on Crystal or myc.
