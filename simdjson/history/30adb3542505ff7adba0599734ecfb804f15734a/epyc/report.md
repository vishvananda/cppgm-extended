# simdjson benchmarks

Compiler: `30adb3542505ff7adba0599734ecfb804f15734a` · CPU: AMD EPYC 9V45 96-Core Processor

Unmodified simdjson v5.1.1 (`40a686ab89dc5ee58ec10d0514dd5c99abbc472b`), C++17 `-O3 -g0 -DNDEBUG`. Each backend is compiled separately; the SSE profile enables `-mpopcnt` for all three compilers. Compile times cover `src/simdjson.cpp` to an object; runtime covers repeated DOM parsing of hot input with a reused parser. One GCC-built driver per backend isolates library code generation. All three libraries produce identical canonical JSON for both inputs.

Ratios are reference time / cppgm time: **higher is better**. Medians of paired ABBA/BAAB block ratios; A/A calibration before and after each comparison.

| Backend | Measurement | vs GCC CPU | vs Clang CPU | vs GCC wall | vs Clang wall |
|---|---|---:|---:|---:|---:|
| scalar | Compile | 4.722× | 2.207× | 4.560× | 2.158× |
| scalar | twitter.json | 0.837× | 0.813× | 0.837× | 0.813× |
| scalar | citm_catalog.json | 0.885× | 0.888× | 0.885× | 0.888× |
| sse | Compile | 4.620× | 2.151× | 4.455× | 2.153× |
| sse | twitter.json | 0.948× | 0.949× | 0.948× | 0.949× |
| sse | citm_catalog.json | 0.960× | 1.007× | 0.960× | 1.007× |

Calibration variation above 2%: scalar Compile gcc cpu A/A after: 0.971×; scalar Compile gcc wall A/A after: 0.964×; scalar Compile clang cpu A/A before: 0.958×; scalar Compile clang wall A/A before: 0.974×; scalar twitter.json gcc cpu A/A after: 1.055×; scalar twitter.json gcc wall A/A after: 1.055×; scalar citm_catalog.json clang cpu A/A after: 0.979×; scalar citm_catalog.json clang wall A/A after: 0.979×. No observations are dropped.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results and per-block ratios](results.json) · [Provenance and header hashes](metadata.json) · [Compile observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42573958837) · [Runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42573958837)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42573958837)
