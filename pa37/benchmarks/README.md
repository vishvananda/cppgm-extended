# Wasm32 measurements

The benchmark compares this compiler with the pinned Clang Wasm target using
the same Wasmtime/Cranelift engine. It checks hashing, filtered sums, matrix
products and word transforms against independent Perl arithmetic. Scalar and
explicit SIMD128 source forms use identical inputs; automatic vectorization
is permitted in both.

From the assignment directory, install the benchmark engine once, then run:

```sh
perl ../scripts/install_wasm_tools.pl --prefix ../obj/wasm-tools --benchmark-engine
make benchmark
```

The default is O3. For another level, use
`make benchmark WASM_BENCH_ARGS="--level 0"`. Each run creates a fresh directory
under `obj/wasm-benchmarks/` and prints its path. Read its `details.md` and
`results.json`; `raw/` retains requests and observations.

Ratios are reference / cppgm, so higher is better. Object compilation and static
linking report CPU and elapsed time separately, including tool process startup.
Module size and encoded code/data payloads are bytes; zero-initialized linear
memory is separate from encoded data. Instantiation/start includes the module's
start function, excluding VM compilation and Store construction/destruction.
Steady execution uses warmed exports after input preparation. VM compilation
events remain in the raw logs.

A/A calibration and paired ABBA blocks use frozen inputs/modules and validate
all checksums. Read calibration cautions before interpreting small differences.
Host, flags, engine configuration and compiler/module hashes are recorded.
These timings are advisory; the assignment's correctness and quality bounds
remain required. GCC is not a baseline for this Wasm target.
