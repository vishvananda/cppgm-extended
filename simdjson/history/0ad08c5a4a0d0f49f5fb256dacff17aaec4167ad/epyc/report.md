# simdjson benchmarks

Compiler: `0ad08c5a4a0d0f49f5fb256dacff17aaec4167ad` · CPU: AMD EPYC 7763 64-Core Processor

Unmodified simdjson v5.1.1 (`40a686ab89dc5ee58ec10d0514dd5c99abbc472b`), C++17 `-O3 -g0 -DNDEBUG`. Each backend is compiled separately; the SSE profile enables `-mpopcnt` for all three compilers. Compile times cover `src/simdjson.cpp` to an object; runtime covers repeated DOM parsing of hot input with a reused parser. One GCC-built driver per backend isolates library code generation. All three libraries produce identical canonical JSON for both inputs.

Ratios are reference time / cppgm time: **higher is better**. Medians of paired ABBA/BAAB block ratios; A/A calibration before and after each comparison.

| Backend | Measurement | vs GCC CPU | vs Clang CPU | vs GCC wall | vs Clang wall |
|---|---|---:|---:|---:|---:|
| scalar | Compile | 4.850× | 2.453× | 4.736× | 2.417× |
| scalar | twitter.json | 1.074× | 0.978× | 1.074× | 0.977× |
| scalar | citm_catalog.json | 0.992× | 1.048× | 0.992× | 1.048× |
| sse | Compile | 4.786× | 2.640× | 4.670× | 2.587× |
| sse | twitter.json | 1.028× | 1.021× | 1.028× | 1.021× |
| sse | citm_catalog.json | 1.030× | 1.015× | 1.029× | 1.015× |

Calibration variation above 2%: none. No observations are dropped.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results and per-block ratios](results.json) · [Provenance and header hashes](metadata.json) · [Compile observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42770606675) · [Runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42770606675)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42770606675)
