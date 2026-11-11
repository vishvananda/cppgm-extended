# simdjson benchmarks

Compiler: `e28ecf1897299c1d70259b05c8315697e3bf589b` · CPU: AMD EPYC 7763 64-Core Processor

Unmodified simdjson v5.1.1 (`40a686ab89dc5ee58ec10d0514dd5c99abbc472b`), C++17 `-O3 -g0 -DNDEBUG`. Each backend is compiled separately; the SSE profile enables `-mpopcnt` for all three compilers. Compile times cover `src/simdjson.cpp` to an object; runtime covers repeated DOM parsing of hot input with a reused parser. One GCC-built driver per backend isolates library code generation. All three libraries produce identical canonical JSON for both inputs.

Ratios are reference time / cppgm time: **higher is better**. Medians of paired ABBA/BAAB block ratios; A/A calibration before and after each comparison.

| Backend | Measurement | vs GCC CPU | vs Clang CPU | vs GCC wall | vs Clang wall |
|---|---|---:|---:|---:|---:|
| scalar | Compile | 4.418× | 2.307× | 4.324× | 2.255× |
| scalar | twitter.json | 0.865× | 0.781× | 0.865× | 0.781× |
| scalar | citm_catalog.json | 0.750× | 0.795× | 0.749× | 0.795× |
| sse | Compile | 4.498× | 2.562× | 4.411× | 2.503× |
| sse | twitter.json | 0.901× | 0.894× | 0.901× | 0.894× |
| sse | citm_catalog.json | 0.899× | 0.887× | 0.899× | 0.887× |

Calibration variation above 2%: scalar twitter.json clang cpu A/A after: 1.053×; scalar twitter.json clang wall A/A after: 1.053×; sse twitter.json gcc cpu A/A after: 1.031×; sse twitter.json gcc wall A/A after: 1.032×. No observations are dropped.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results and per-block ratios](results.json) · [Provenance and header hashes](metadata.json) · [Compile observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42538308648) · [Runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42538308648)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42538308648)
