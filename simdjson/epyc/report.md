# simdjson benchmarks

Compiler: `393d1b1d09089ffdade676f4e7cbad8759c15d2a` · CPU: AMD EPYC 7763 64-Core Processor

Unmodified simdjson v5.1.1 (`40a686ab89dc5ee58ec10d0514dd5c99abbc472b`), C++17 `-O3 -g0 -DNDEBUG`. Each backend is compiled separately; the SSE profile enables `-mpopcnt` for all three compilers. Compile times cover `src/simdjson.cpp` to an object; runtime covers repeated DOM parsing of hot input with a reused parser. One GCC-built driver per backend isolates library code generation. All three libraries produce identical canonical JSON for both inputs.

Ratios are reference time / cppgm time: **higher is better**. Medians of paired ABBA/BAAB block ratios; A/A calibration before and after each comparison.

| Backend | Measurement | vs GCC CPU | vs Clang CPU | vs GCC wall | vs Clang wall |
|---|---|---:|---:|---:|---:|
| scalar | Compile | 4.359× | 2.208× | 4.305× | 2.180× |
| scalar | twitter.json | 1.008× | 0.916× | 1.008× | 0.916× |
| scalar | citm_catalog.json | 0.942× | 0.991× | 0.942× | 0.991× |
| sse | Compile | 4.392× | 2.448× | 4.337× | 2.420× |
| sse | twitter.json | 1.028× | 1.020× | 1.028× | 1.020× |
| sse | citm_catalog.json | 1.010× | 1.001× | 1.010× | 1.001× |

Calibration variation above 2%: sse Compile clang cpu A/A before: 1.023×; sse Compile clang cpu A/A after: 0.961×; sse Compile clang wall A/A before: 1.023×; sse Compile clang wall A/A after: 0.961×. No observations are dropped.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results and per-block ratios](results.json) · [Provenance and header hashes](metadata.json) · [Compile observations](compile-observations.jsonl) · [Runtime observations](runtime-observations.jsonl)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43357224983)
