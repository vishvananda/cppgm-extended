# simdjson benchmarks

Compiler: `da7046f6f5518cdabc9d2d0f2cd153e228c7b1bb` · CPU: AMD EPYC 7763 64-Core Processor

Unmodified simdjson v5.1.1 (`40a686ab89dc5ee58ec10d0514dd5c99abbc472b`), C++17 `-O3 -g0 -DNDEBUG`. Each backend is compiled separately; the SSE profile enables `-mpopcnt` for all three compilers. Compile times cover `src/simdjson.cpp` to an object; runtime covers repeated DOM parsing of hot input with a reused parser. One GCC-built driver per backend isolates library code generation. All three libraries produce identical canonical JSON for both inputs.

Ratios are reference time / cppgm time: **higher is better**. Medians of paired ABBA/BAAB block ratios; A/A calibration before and after each comparison.

| Backend | Measurement | vs GCC CPU | vs Clang CPU | vs GCC wall | vs Clang wall |
|---|---|---:|---:|---:|---:|
| scalar | Compile | 4.579× | 2.400× | 4.431× | 2.393× |
| scalar | twitter.json | 0.837× | 0.764× | 0.836× | 0.764× |
| scalar | citm_catalog.json | 0.607× | 0.644× | 0.607× | 0.644× |
| sse | Compile | 4.577× | 2.596× | 4.523× | 2.554× |
| sse | twitter.json | 0.905× | 0.909× | 0.905× | 0.909× |
| sse | citm_catalog.json | 0.905× | 0.894× | 0.905× | 0.894× |

Calibration variation above 2%: scalar twitter.json clang cpu A/A before: 1.029×; scalar twitter.json clang wall A/A before: 1.028×; sse Compile clang cpu A/A before: 1.023×; sse Compile clang wall A/A before: 1.023×. No observations are dropped.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results and per-block ratios](results.json) · [Provenance and header hashes](metadata.json) · [Compile observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893696) · [Runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893696)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893696)
