# simdjson benchmarks

Compiler: `a649ddb21dd810338ac87d93e7d2e713d0f98ab1` · CPU: AMD EPYC 7763 64-Core Processor

Unmodified simdjson v5.1.1 (`40a686ab89dc5ee58ec10d0514dd5c99abbc472b`), C++17 `-O3 -g0 -DNDEBUG`. Each backend is compiled separately; the SSE profile enables `-mpopcnt` for all three compilers. Compile times cover `src/simdjson.cpp` to an object; runtime covers repeated DOM parsing of hot input with a reused parser. One GCC-built driver per backend isolates library code generation. All three libraries produce identical canonical JSON for both inputs.

Ratios are reference time / cppgm time: **higher is better**. Medians of paired ABBA/BAAB block ratios; A/A calibration before and after each comparison.

| Backend | Measurement | vs GCC CPU | vs Clang CPU | vs GCC wall | vs Clang wall |
|---|---|---:|---:|---:|---:|
| scalar | Compile | 4.167× | 2.192× | 4.086× | 2.184× |
| scalar | twitter.json | 1.032× | 0.939× | 1.032× | 0.939× |
| scalar | citm_catalog.json | 0.987× | 1.045× | 0.987× | 1.045× |
| sse | Compile | 4.315× | 2.459× | 4.253× | 2.434× |
| sse | twitter.json | 1.024× | 1.014× | 1.024× | 1.014× |
| sse | citm_catalog.json | 0.992× | 0.984× | 0.992× | 0.984× |

Calibration variation above 2%: scalar Compile clang wall A/A after: 1.023×; scalar citm_catalog.json gcc cpu A/A before: 1.028×; scalar citm_catalog.json gcc wall A/A before: 1.028×. No observations are dropped.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results and per-block ratios](results.json) · [Provenance and header hashes](metadata.json) · [Compile observations](compile-observations.jsonl) · [Runtime observations](runtime-observations.jsonl)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43609274684)
