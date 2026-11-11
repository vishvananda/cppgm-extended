# simdjson benchmarks

Compiler: `8287f480c7b5bdb2074c8941689aa36f9c233bd7` · CPU: AMD EPYC 9V74 80-Core Processor

Unmodified simdjson v5.1.1 (`40a686ab89dc5ee58ec10d0514dd5c99abbc472b`), C++17 `-O3 -g0 -DNDEBUG`. Each backend is compiled separately; the SSE profile enables `-mpopcnt` for all three compilers. Compile times cover `src/simdjson.cpp` to an object; runtime covers repeated DOM parsing of hot input with a reused parser. One GCC-built driver per backend isolates library code generation. All three libraries produce identical canonical JSON for both inputs.

Ratios are reference time / cppgm time: **higher is better**. Medians of paired ABBA/BAAB block ratios; A/A calibration before and after each comparison.

| Backend | Measurement | vs GCC CPU | vs Clang CPU | vs GCC wall | vs Clang wall |
|---|---|---:|---:|---:|---:|
| scalar | Compile | 4.529× | 2.133× | 4.479× | 2.128× |
| scalar | twitter.json | 1.041× | 1.044× | 1.041× | 1.044× |
| scalar | citm_catalog.json | 0.998× | 1.115× | 0.998× | 1.115× |
| sse | Compile | 4.502× | 2.264× | 4.349× | 2.218× |
| sse | twitter.json | 0.989× | 0.976× | 0.989× | 0.976× |
| sse | citm_catalog.json | 1.022× | 1.015× | 1.022× | 1.015× |

Calibration variation above 2%: none. No observations are dropped.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results and per-block ratios](results.json) · [Provenance and header hashes](metadata.json) · [Compile observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798170) · [Runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798170)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798170)
