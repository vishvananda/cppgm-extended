# simdjson benchmarks

Compiler: `9ca649bf59cb1e14979655d1669ade353a9ae881` · CPU: AMD EPYC 7763 64-Core Processor

Unmodified simdjson v5.1.1 (`40a686ab89dc5ee58ec10d0514dd5c99abbc472b`), C++17 `-O3 -g0 -DNDEBUG`. Each backend is compiled separately; the SSE profile enables `-mpopcnt` for all three compilers. Compile times cover `src/simdjson.cpp` to an object; runtime covers repeated DOM parsing of hot input with a reused parser. One GCC-built driver per backend isolates library code generation. All three libraries produce identical canonical JSON for both inputs.

Ratios are reference time / cppgm time: **higher is better**. Medians of paired ABBA/BAAB block ratios; A/A calibration before and after each comparison.

| Backend | Measurement | vs GCC CPU | vs Clang CPU | vs GCC wall | vs Clang wall |
|---|---|---:|---:|---:|---:|
| scalar | Compile | 4.558× | 2.383× | 4.503× | 2.344× |
| scalar | twitter.json | 1.085× | 0.975× | 1.085× | 0.975× |
| scalar | citm_catalog.json | 0.980× | 1.048× | 0.980× | 1.048× |
| sse | Compile | 4.601× | 2.579× | 4.485× | 2.557× |
| sse | twitter.json | 0.903× | 0.894× | 0.903× | 0.894× |
| sse | citm_catalog.json | 0.902× | 0.890× | 0.902× | 0.890× |

Calibration variation above 2%: scalar twitter.json clang cpu A/A before: 0.958×; scalar twitter.json clang wall A/A before: 0.958×; scalar citm_catalog.json gcc cpu A/A after: 0.979×; scalar citm_catalog.json gcc wall A/A after: 0.979×; sse Compile gcc cpu A/A after: 1.039×; sse Compile gcc wall A/A after: 1.040×; sse Compile clang cpu A/A before: 1.028×; sse Compile clang wall A/A before: 1.035×. No observations are dropped.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results and per-block ratios](results.json) · [Provenance and header hashes](metadata.json) · [Compile observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011576) · [Runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011576)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011576)
