# simdjson benchmarks

Compiler: `a0ff7d04a7ed4d67461b0912fca4a4fa7e055d74` · CPU: AMD EPYC 7763 64-Core Processor

Unmodified simdjson v5.1.1 (`40a686ab89dc5ee58ec10d0514dd5c99abbc472b`), C++17 `-O3 -g0 -DNDEBUG`. Each backend is compiled separately; the SSE profile enables `-mpopcnt` for all three compilers. Compile times cover `src/simdjson.cpp` to an object; runtime covers repeated DOM parsing of hot input with a reused parser. One GCC-built driver per backend isolates library code generation. All three libraries produce identical canonical JSON for both inputs.

Ratios are reference time / cppgm time: **higher is better**. Medians of paired ABBA/BAAB block ratios; A/A calibration before and after each comparison.

| Backend | Measurement | vs GCC CPU | vs Clang CPU | vs GCC wall | vs Clang wall |
|---|---|---:|---:|---:|---:|
| scalar | Compile | 4.547× | 2.333× | 4.429× | 2.320× |
| scalar | twitter.json | 1.080× | 0.977× | 1.080× | 0.977× |
| scalar | citm_catalog.json | 0.992× | 1.053× | 0.992× | 1.053× |
| sse | Compile | 4.531× | 2.549× | 4.485× | 2.534× |
| sse | twitter.json | 1.020× | 1.015× | 1.020× | 1.015× |
| sse | citm_catalog.json | 1.026× | 1.008× | 1.026× | 1.008× |

Calibration variation above 2%: scalar citm_catalog.json clang cpu A/A before: 0.973×; scalar citm_catalog.json clang wall A/A before: 0.973×. No observations are dropped.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results and per-block ratios](results.json) · [Provenance and header hashes](metadata.json) · [Compile observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42974658030) · [Runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42974658030)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42974658030)
