# simdjson benchmarks

Compiler: `0aa83232b4c697f9a6f25ef8f318a561973797b4` · CPU: AMD EPYC 7763 64-Core Processor

Unmodified simdjson v5.1.1 (`40a686ab89dc5ee58ec10d0514dd5c99abbc472b`), C++17 `-O3 -g0 -DNDEBUG`. Each backend is compiled separately; the SSE profile enables `-mpopcnt` for all three compilers. Compile times cover `src/simdjson.cpp` to an object; runtime covers repeated DOM parsing of hot input with a reused parser. One GCC-built driver per backend isolates library code generation. All three libraries produce identical canonical JSON for both inputs.

Ratios are reference time / cppgm time: **higher is better**. Medians of paired ABBA/BAAB block ratios; A/A calibration before and after each comparison.

| Backend | Measurement | vs GCC CPU | vs Clang CPU | vs GCC wall | vs Clang wall |
|---|---|---:|---:|---:|---:|
| scalar | Compile | 4.887× | 2.456× | 4.740× | 2.383× |
| scalar | twitter.json | 0.929× | 0.858× | 0.929× | 0.858× |
| scalar | citm_catalog.json | 0.744× | 0.790× | 0.744× | 0.790× |
| sse | Compile | 4.803× | 2.563× | 4.695× | 2.531× |
| sse | twitter.json | 0.897× | 0.892× | 0.897× | 0.893× |
| sse | citm_catalog.json | 0.901× | 0.895× | 0.901× | 0.895× |

Calibration variation above 2%: none. No observations are dropped.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results and per-block ratios](results.json) · [Provenance and header hashes](metadata.json) · [Compile observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42570114062) · [Runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42570114062)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42570114062)
