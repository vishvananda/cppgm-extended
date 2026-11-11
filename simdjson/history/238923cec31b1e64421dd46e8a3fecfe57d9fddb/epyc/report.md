# simdjson benchmarks

Compiler: `238923cec31b1e64421dd46e8a3fecfe57d9fddb` · CPU: AMD EPYC 7763 64-Core Processor

Unmodified simdjson v5.1.1 (`40a686ab89dc5ee58ec10d0514dd5c99abbc472b`), C++17 `-O3 -g0 -DNDEBUG`. Each backend is compiled separately; the SSE profile enables `-mpopcnt` for all three compilers. Compile times cover `src/simdjson.cpp` to an object; runtime covers repeated DOM parsing of hot input with a reused parser. One GCC-built driver per backend isolates library code generation. All three libraries produce identical canonical JSON for both inputs.

Ratios are reference time / cppgm time: **higher is better**. Medians of paired ABBA/BAAB block ratios; A/A calibration before and after each comparison.

| Backend | Measurement | vs GCC CPU | vs Clang CPU | vs GCC wall | vs Clang wall |
|---|---|---:|---:|---:|---:|
| scalar | Compile | 4.515× | 2.354× | 4.447× | 2.320× |
| scalar | twitter.json | 0.921× | 0.840× | 0.921× | 0.840× |
| scalar | citm_catalog.json | 0.757× | 0.789× | 0.757× | 0.789× |
| sse | Compile | 4.560× | 2.563× | 4.493× | 2.549× |
| sse | twitter.json | 0.902× | 0.893× | 0.902× | 0.893× |
| sse | citm_catalog.json | 0.909× | 0.897× | 0.909× | 0.897× |

Calibration variation above 2%: sse twitter.json clang cpu A/A before: 0.972×; sse twitter.json clang wall A/A before: 0.970×. No observations are dropped.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results and per-block ratios](results.json) · [Provenance and header hashes](metadata.json) · [Compile observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42562779000) · [Runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42562779000)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42562779000)
