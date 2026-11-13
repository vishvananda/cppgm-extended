# Optional typed LowIR library

PA27 can optionally expose the host-object backend as a library. PA8 supplies
shared records in `dev/src/lowir_records.h`; `dev/src/lowir_interface.h`
supplies the `lowir_model::BodyConstruction` contract and versioned C facade. Use the same records in your compiler and library. Keep storage,
validation, optimization and emission in your existing compiler modules.

This lane is separate from `make test`, strict reports and assignment completion.
It needs an x86-64 Linux or macOS host, a C compiler and the same C++ host
compiler/standard library used to build the tools. It needs no Crystal or myc.

```sh
make -C pa27 test-lowir-library     # O0 construction and host objects
make -C pa33 test-lowir-library     # adds O1, O2 and O3
make test-lowir-library            # both optional lanes
```

Implement `dev/lowir_library.cpp`, replacing its failing starter functions, and
add reusable implementation modules to
`FRONTEND_LIBRARY_SOURCE_IDS_lowir_library` in `dev/frontend_source_sets.mk`.
`make lowir-library` builds static and shared libraries from independent PIC
objects. The default nine tools remain the default build. Pass `CXX`,
`CPPGM_HOST_CXX` and `CPPGM_STDLIB_FLAGS` consistently for both builds. C clients
compile with `CC`; use the C++ host driver to link the library.

## Construction and ownership

1. Call `cppgm_lowir_create` with `CPPGM_LOWIR_ABI_VERSION` and target 0 (Linux
   x86-64 ELF) or 1 (macOS x86-64 Mach-O). Reject unsupported versions/targets.
2. Intern symbols and add signatures, declarations, definitions and globals.
   IDs are dense and stable. Symbol strings, signature parameter arrays and
   global initializer arrays are borrowed for the call and copied as needed.
   A function declaration may become a definition with the same signature ID.
   Apply the same conventional entry/init/fini roles as the text reader.
3. Reserve instruction, extra operand, literal, slot and block-layout capacity.
   The returned buffers belong to the module. Fill their initialized prefixes
   directly using the scaffold records. Preserve existing contents on growth;
   grow geometrically when counts are unknown. Reacquire all pointers after a
   reserve on that function; commit, compilation and destruction also invalidate
   its writable pointers. Never retain pointers to caller-owned instruction arrays.
4. Parameters occupy the first temporary IDs. Assign dense IDs to definitions.
   Each instruction's construction-time `location` is its insertion block ID;
   `next` is ignored. `layout` is a permutation of block IDs. Foreign slots have
   zero name, plain kind, zero reserved byte and zero presentation index.
5. Commit initialized counts and total temporary count exactly once. Bind
   definitions and block links in place and clear construction locations. Commit
   does not copy instruction/operand/literal pools or remap IDs. Validate all
   external enum codes, types, ranges and the ordinary LowIR structural rules.
   Reject open, duplicate or malformed bodies before optimization/emission.
6. Optionally serialize a diagnostic view before compiling. Compile once,
   optionally with PIC, using the same validator, optimizer and host-object
   emitter as the CLI. At PA27 support level 0; the PA33 extension adds 1–3.
   Return module-owned object bytes valid until destruction. No text roundtrip,
   temporary files, subprocess or host compiler invocation belongs in compile.

C++ uses the vocabulary's enums and `Type`; C uses fixed-width numeric fields
from the same record declarations. The supported ABI is little endian, with
8-byte `Type` and `Value`, 16-byte `Literal` and 64-byte `Instruction`. Type words
encode kind in bits 0–7, alignment log2 in 8–15 and object size in 16–63. Values
store kind and floating kind as separate bytes, a zero reserved halfword and an
index. See the headers and PA8 format reference for opcode properties and flags.

Failures return zero (or `CPPGM_NO_ID` for IDs), retain a diagnostic and poison
the module. Failed creation returns null; destruction accepts null. No C++
exception may cross C. Calls on one module must be serialized. Independent
modules own independent construction state; shared backend scratch state must
be synchronized during compilation if your implementation retains any.

## Checks

The supplied C frontend builds a loop with phis, a global, a host function call
and an entry function. Both static and shared clients run. Checks cover buffer
growth, invalid records/ranges, uncommitted and duplicate commits, consumption,
concurrent module isolation, host linking and execution. The C++ client checks
that the scaffold types are the same types used by its construction interface.
PA33 checks the optimized results across many inputs. CLI serialization and
direct construction must preserve behavior; object bytes need not be identical
across valid student implementations. No timing threshold is graded.

This extension covers native C-linkage scalar functions, globals and plain
slots. Wasm objects, inline assembly, debug information and the complete C++
aggregate ABI are outside this optional facade. Their ordinary compiler lanes
continue to exercise the full LowIR model.
