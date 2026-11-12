# Optimization quality

At O1–O3, satisfy the `quality` bounds in each fixture's `.wasm-expect`.
They are upper bounds, not required instruction sequences. The same bounds
apply to direct source compilation, serialized LowIR and mixed LLVM objects.
O0 still runs the correctness checks.

The checker measures each named exported function in the independently
validated final module:

| Metric | Meaning |
| --- | --- |
| `locals` | Declared locals, excluding parameters |
| `code_bytes` | Function body bytes, including local declarations |
| `instructions` | Static instructions, including control markers |
| `memory_ops` | Static scalar/SIMD/bulk linear-memory operations |
| `control_depth` | Maximum nested block, loop, if or exception regions |

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

The machine dump should help explain these results. Show the actual signature,
function, global and tag indices consumed by calls and control regions; frame
objects with size, alignment and offset; typed locals; and each instruction's
effects and available source location. Keep its view consistent with the
function that is validated and encoded.
