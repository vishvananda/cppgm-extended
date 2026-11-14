# Optimization quality

At O1–O3, satisfy the `quality` bounds in each fixture's `.wasm-expect`.
They are upper bounds, not required instruction sequences. The same bounds
apply to direct source compilation, serialized LowIR and mixed LLVM objects.
O0 still runs the correctness checks.
An explicit `levels` list may also apply a module data-size bound at O0;
representing zero-initialized storage efficiently does not require optimization.

The checker measures each named exported function in the independently
validated final module:

| Metric | Meaning |
| --- | --- |
| `locals` | Declared locals, excluding parameters |
| `code_bytes` | Function body bytes, including local declarations |
| `instructions` | Static instructions, including control markers |
| `memory_ops` | Static scalar/SIMD/bulk linear-memory operations |
| `zero_offset_memory_ops` | Static scalar/SIMD loads and stores with zero memory-immediate offset |
| `bulk_memory_ops` | Static bulk linear-memory operations: `memory.init`, `memory.copy` and `memory.fill` |
| `control_depth` | Maximum nested block, loop, if or exception regions |

A sidecar's `quality.module` bounds use `module`, `code` and `data` for total
module bytes and the code/data section payload bytes. Storage omitted from a
data section must retain its full size, alignment, addresses and initial values.
Bounds allow segment merging and padding; they do not prescribe segment layout.

The standalone `lowiropt` path must respect the selected target's library
surface. A fixture's `allowed_function_imports` constrains external dependencies,
not instruction selection. Optimizations must not introduce unavailable helpers.

These measurements exclude callees. A call's `fuel` bounds include all
executed Wasm instructions, including initialization and callees, using the
pinned engine. They bound dynamic work rather than elapsed time. Preserve the
expected result within the fuel budget; moving work into a helper does not
remove it from that budget.

The portable kernels require scalar expressions and register-only recurrences
to avoid linear-memory traffic. Array loops retain their necessary accesses.
Bounds allow different local allocation, stackification, loop unrolling and
vectorization choices. Larger runtime counts also check that control lowering
and repeated work remain bounded. Benchmark timings are reported separately.

The unsigned-vector controls require packed arithmetic and constant-index
extraction to avoid temporary linear-memory storage. They also check high-bit
values, variable shifts, operand evaluation and volatile accesses against a
scalar calculation. The bounds allow different instruction sequences and
local allocation.

Use the selected target when deciding whether a transformation pays for its
extra work. Wasm address arithmetic and expanded operations need their own
costs, and local values survive calls. Keep invariant arithmetic out of repeated
loop work when it is safe; do not assume x86 addressing modes or register
pressure. Native targets must retain their earlier quality requirements.

The machine dump should help explain these results. Show the actual signature,
function, global and tag indices consumed by calls and control regions; frame
objects with size, alignment and offset; typed locals; and each instruction's
effects and available source location. Keep its view consistent with the
function that is validated and encoded.
