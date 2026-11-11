# Wasm32 target contract

This contract supplements the [assignment](README.md) and the pinned
[Basic C ABI](../doc/wasm/BasicCABI.md). Apply it before preprocessing, type
checking and layout. The two targets share a data layout and C/C++ ABI;
`wasm32-wasip1` additionally supplies the WASI Preview 1 command environment.

## Values and calls

The target is little-endian. `char` is signed, `wchar_t` is a signed 32-bit
integer, and `size_t` is `unsigned long`.

| Type | Size and natural alignment, bytes | Wasm call representation |
| --- | --- | --- |
| `bool`, character types | 1 | `i32` |
| `short` | 2 | `i32` |
| `int`, `long`, pointers | 4 | `i32` |
| `long long` | 8 | `i64` |
| `float` | 4 | `f32` |
| `double` | 8 | `f64` |
| `__int128`, binary128 `long double` | 16 | Two `i64` arguments; indirect result |

Sign-extend signed narrow integer arguments/results and zero-extend unsigned
ones. Keep the public ABI stable at every optimization level. Available
multi-value support does not change this ABI's indirect result convention.

Ignore empty aggregates at a call boundary. A naturally aligned aggregate
containing exactly one scalar recursively can use that scalar's convention;
other aggregates are indirect. Respect additional alignment and the target's
C++ rules for nontrivial copies. An indirect result pointer precedes the other
arguments. Indirect by-value arguments belong to the caller and need a copy
when the callee could modify them. Use LowIR's typed layout and shape facts;
byte size alone cannot distinguish these cases.

Apply default argument promotions to variadic arguments. Pack those arguments
in source order into an appropriately aligned, caller-owned buffer and pass
its address as the final Wasm parameter. The callee may modify that buffer.
A `va_list` advances through this target layout, independently of native
register-save areas.

Pointers to data address linear memory. Function pointers identify table
entries; index zero is null. Table entries, function indices and type indices
are separate index spaces. Preserve exact indirect-call signatures.

## C++ objects and lifetime

Use the Wasm adaptation of the Itanium C++ ABI, including its symbol names.
Pointers, vtable entries and `ptrdiff_t` fields are four bytes. A pointer to a
member function has two four-byte fields. Its adjustment field stores twice
the `this` adjustment, with its low bit distinguishing a virtual entry;
the first field holds a function-table index or a vtable byte offset.
Do not reuse the native convention that puts the virtual discriminator in
the function field.

Preserve virtual bases, thunks, RTTI identities, nontrivial copies, local static
initialization and destruction. C++ exceptions use the modern Wasm exception
instructions and the runtime's C++ exception tag. A Wasm trap is not a C++
exception. Support matching catches, cleanup and rethrow across translation
units and across the reference-compiler boundary.

## Objects and linking

Emit core Wasm objects using linking metadata version 2 and the pinned
[linking conventions](../doc/wasm/Linking.md). Preserve symbol binding,
COMDAT groups, segment alignment, constructor priorities, tags and feature
requirements. Relocation fields must retain enough space and the specified
signedness. Check both the referenced symbol kind and the encoded field.

Accept ordinary GNU and BSD static archives. Extract members needed to
satisfy live references, including references introduced by extracted members.
An unused member must not introduce duplicate definitions, unresolved-symbol
errors or constructor execution. Resolve weak and strong definitions according
to the linking conventions and reject incompatible function signatures.

An ordinary undefined C/C++ declaration is not permission to create a host
import. Declared imports specify their module, field name and exact signature.
Exports use their requested object names. Reject unresolved live references,
incompatible target/features and malformed objects with a diagnostic.

For freestanding arithmetic that needs supplied helpers, pass the pinned
compiler-rt and C runtime archives as link inputs. Both tools accept those
archives alongside their source or LowIR inputs.

## Initialization and execution

For a freestanding module, initialize static storage before any exported call.
Run dynamic initializers once per module instance and preserve state across
repeated exported calls. The core start function does not call `main`.

For a WASI command, use Preview 1 startup and I/O imports, initialize before
calling the user entry point, run required finalization and preserve the exit
status. The supplied startup calls `__main_void` for the zero-argument form
and `__main_argc_argv` for the argument-taking form. Supply the entry adapters
needed by the pinned runtime, including its compatibility `main` signature.
Source programs still spell both forms `main`; freestanding exports do not
acquire WASI startup behavior.

The test dependencies pin WASI SDK 34.0, Wasmtime and `wasm-tools` in
[`wasm_tools.json`](../scripts/wasm_tools.json). The SDK supplies reference
Clang/LLD and runtime libraries. Its
[exception documentation](https://github.com/WebAssembly/wasi-sdk/blob/wasi-sdk-34/CppExceptions.md)
explains the reference flags for modern exceptions. These tools may build and
check reference objects and supplied libraries; the submitted compiler still
performs its own instruction selection, encoding and static linking.
