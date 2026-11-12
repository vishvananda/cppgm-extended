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

Apply default argument promotions to variadic arguments, then classify their
typed values. Ignore empty records; coerce singleton records like their scalar
member. Copy other records to caller-owned storage and pack a pointer to that
copy. Wide scalar values retain their direct bits. Pack the resulting arguments
in source order into a caller-owned buffer, with slots of at least four bytes
and their required alignment, then pass its address as the final Wasm parameter.
A `va_list` advances through this layout, independently of native register-save
areas. Preserve LowIR's `argument_type`, `argument_shape` and object `va_arg`
`shape` facts so optimization cannot lose a type or infer a calling convention
from object size alone.

Pointers to data address linear memory. Function pointers identify table
entries; index zero is null. Table entries, function indices and type indices
are separate index spaces. Preserve exact indirect-call signatures.

## C++ objects and lifetime

Use the Wasm adaptation of the Itanium C++ ABI, including its symbol names.
Pointers, vtable entries and `ptrdiff_t` fields are four bytes. A pointer to a
member function has two four-byte fields. Its adjustment field stores twice
the `this` adjustment, with its low bit distinguishing a virtual entry;
the first field holds a function-table index or a vtable byte offset.
Pass this pair as an indirect record with a caller-owned copy; return it
through a hidden pointer. Keep that record ABI when operating on its packed
bits. The native function-field discriminator is a different convention.

Constructors and non-deleting destructors physically return their incoming
`this` pointer. Preserve LowIR's `this_return=yes` boundary fact while their
logical result remains void. Deleting destructors retain a void result.
An adjusting destructor thunk returns its target's adjusted `this` value;
represent that adapter with a pointer result in LowIR.
Local static guards occupy four bytes and use the supplied guard API.

Preserve virtual bases, thunks, RTTI identities, nontrivial copies, local static
initialization and destruction. C++ exceptions use the modern Wasm exception
instructions and the runtime's C++ exception tag. A Wasm trap is not a C++
exception. Support matching catches, cleanup and rethrow across translation
units and across the reference-compiler boundary.
Honor exception specifications and lifetime boundaries, including termination
when initialization of a handler parameter throws or a destructor throws during
unwinding. These rules also apply after LowIR serialization.

LowIR's default opaque exception protocol uses the reserved coalesced tag
`__cppgm_opaque_exception : (i64, i64) -> ()`, separate from the C++ tag.
Its parameters carry the low and high halves of the payload. Narrow scalars
occupy their low storage bits; floating values preserve their IEEE bits, and
32-bit data/function pointers retain their respective address spaces. Catch-all,
cleanup and resume must work without C++ runtime libraries. C++
source and serialized source LowIR select `exception_abi=cxx`. Reject typed
C++ catches in an opaque unit and opaque `throw` in a C++ unit.

## Objects and linking

Emit core Wasm objects using linking metadata version 2 and the pinned
[linking conventions](../doc/wasm/Linking.md). Preserve symbol binding,
COMDAT groups, segment alignment, constructor priorities, tags and feature
requirements. Relocation fields must retain enough space and the specified
signedness. Check both the referenced symbol kind and the encoded field.

Preserve multi-result signatures in objects and typed block, loop and if
parameters/results. Branches transfer the target's complete tuple: loop
parameters on a backedge and block results on an exit. A block type may be
an inline value type or a signed 33-bit type index. Validate its relocated
signature and reject missing, incompatible or overflowing fields. These
internal signatures do not change the public C ABI described above.

Preserve exception-tag signatures and their relocated identities. Tags carry
parameters and no results. Modern handlers transfer the complete payload and,
for reference catches, an `exnref` to the target label. Validate catch depths,
payload types and tag relocations before emission. Preserve exception
references through rethrow and restore the linear-memory stack on exceptional
edges; the Wasm value stack and linear-memory stack are distinct.

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

Select target headers/libraries with `--sysroot` and builtin headers/compiler-rt
with `-resource-dir`. C++ links select the supplied C++, ABI and unwind
libraries automatically; `-fno-exceptions` selects their no-EH configuration.
Serialized C++ LowIR selects the EH runtime by default. Preserve explicit
`-nostartfiles`, `-nodefaultlibs` and `-nostdlib` choices. Do not reload an
explicitly supplied runtime file when adding default libraries.
Never use native host headers to describe wasm32. Respect
core export names in input objects, including names different from their
linking symbols. Resolve archive members before discarding unreachable code:
a reference from a discarded function can select a member whose strong
definition replaces a live weak fallback. Unresolved references that remain
unreachable need not prevent a final link.

The test dependencies pin WASI SDK 34.0, Wasmtime and `wasm-tools` in
[`wasm_tools.json`](../scripts/wasm_tools.json). The SDK supplies reference
Clang/LLD and runtime libraries. Its
[exception documentation](https://github.com/WebAssembly/wasi-sdk/blob/wasi-sdk-34/CppExceptions.md)
explains the reference flags for modern exceptions. These tools may build and
check reference objects and supplied libraries; the submitted compiler still
performs its own instruction selection, encoding and static linking.
