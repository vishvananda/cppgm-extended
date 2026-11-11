# simdjson benchmarks

Compiler: `6a6a15f446cf4249b207924c70da749b98b10d1a` · CPU: AMD EPYC 7763 64-Core Processor

Unmodified simdjson v5.1.1 (`40a686ab89dc5ee58ec10d0514dd5c99abbc472b`), C++17 `-O3 -g0 -DNDEBUG`. Each backend is compiled separately; the SSE profile enables `-mpopcnt` for all three compilers. Compile times cover `src/simdjson.cpp` to an object; runtime covers repeated DOM parsing of hot input with a reused parser. One GCC-built driver per backend isolates library code generation. All three libraries produce identical canonical JSON for both inputs.

Ratios are reference time / cppgm time: **higher is better**. Medians of paired ABBA/BAAB block ratios; A/A calibration before and after each comparison.

| Backend | Measurement | vs GCC CPU | vs Clang CPU | vs GCC wall | vs Clang wall |
|---|---|---:|---:|---:|---:|
| scalar | Compile | 4.480× | 2.335× | 4.378× | 2.291× |
| scalar | twitter.json | 0.875× | 0.800× | 0.875× | 0.800× |
| scalar | citm_catalog.json | 0.751× | 0.792× | 0.751× | 0.792× |
| sse | Compile | 4.543× | 2.548× | 4.471× | 2.503× |
| sse | twitter.json | 0.905× | 0.898× | 0.905× | 0.898× |
| sse | citm_catalog.json | 0.901× | 0.890× | 0.901× | 0.890× |

Calibration variation above 2%: none. No observations are dropped.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results and per-block ratios](results.json) · [Provenance and header hashes](metadata.json) · [Compile observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42517561959) · [Runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42517561959)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42517561959)
