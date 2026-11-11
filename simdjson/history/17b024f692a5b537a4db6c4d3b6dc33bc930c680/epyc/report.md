# simdjson benchmarks

Compiler: `17b024f692a5b537a4db6c4d3b6dc33bc930c680` · CPU: AMD EPYC 7763 64-Core Processor

Unmodified simdjson v5.1.1 (`40a686ab89dc5ee58ec10d0514dd5c99abbc472b`), C++17 `-O3 -g0 -DNDEBUG`. Each backend is compiled separately; the SSE profile enables `-mpopcnt` for all three compilers. Compile times cover `src/simdjson.cpp` to an object; runtime covers repeated DOM parsing of hot input with a reused parser. One GCC-built driver per backend isolates library code generation. All three libraries produce identical canonical JSON for both inputs.

Ratios are reference time / cppgm time: **higher is better**. Medians of paired ABBA/BAAB block ratios; A/A calibration before and after each comparison.

| Backend | Measurement | vs GCC CPU | vs Clang CPU | vs GCC wall | vs Clang wall |
|---|---|---:|---:|---:|---:|
| scalar | Compile | 4.453× | 2.309× | 4.325× | 2.273× |
| scalar | twitter.json | 0.879× | 0.793× | 0.879× | 0.793× |
| scalar | citm_catalog.json | 0.757× | 0.796× | 0.757× | 0.796× |
| sse | Compile | 4.519× | 2.525× | 4.411× | 2.487× |
| sse | twitter.json | 0.897× | 0.891× | 0.897× | 0.891× |
| sse | citm_catalog.json | 0.900× | 0.887× | 0.900× | 0.886× |

Calibration variation above 2%: none. No observations are dropped.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results and per-block ratios](results.json) · [Provenance and header hashes](metadata.json) · [Compile observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42516011731) · [Runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42516011731)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42516011731)
