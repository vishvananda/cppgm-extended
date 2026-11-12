# simdjson benchmarks

Compiler: `e2831a513b668c602faa8098e55eb22ee08a670f` · CPU: AMD EPYC 9V74 80-Core Processor

Unmodified simdjson v5.1.1 (`40a686ab89dc5ee58ec10d0514dd5c99abbc472b`), C++17 `-O3 -g0 -DNDEBUG`. Each backend is compiled separately; the SSE profile enables `-mpopcnt` for all three compilers. Compile times cover `src/simdjson.cpp` to an object; runtime covers repeated DOM parsing of hot input with a reused parser. One GCC-built driver per backend isolates library code generation. All three libraries produce identical canonical JSON for both inputs.

Ratios are reference time / cppgm time: **higher is better**. Medians of paired ABBA/BAAB block ratios; A/A calibration before and after each comparison.

| Backend | Measurement | vs GCC CPU | vs Clang CPU | vs GCC wall | vs Clang wall |
|---|---|---:|---:|---:|---:|
| scalar | Compile | 4.292× | 1.983× | 4.131× | 1.948× |
| scalar | twitter.json | 1.032× | 1.024× | 1.032× | 1.024× |
| scalar | citm_catalog.json | 0.880× | 0.979× | 0.880× | 0.979× |
| sse | Compile | 4.380× | 2.166× | 4.301× | 2.149× |
| sse | twitter.json | 1.010× | 1.002× | 1.010× | 1.002× |
| sse | citm_catalog.json | 1.014× | 1.009× | 1.014× | 1.009× |

Calibration variation above 2%: scalar Compile gcc wall A/A before: 0.976×. No observations are dropped.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results and per-block ratios](results.json) · [Provenance and header hashes](metadata.json) · [Compile observations](compile-observations.jsonl) · [Runtime observations](runtime-observations.jsonl)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43133898350)
