# simdjson benchmarks

Compiler: `01bac61953a982df28933dfde0fa03eb65838ea4` · CPU: AMD EPYC 7763 64-Core Processor

Unmodified simdjson v5.1.1 (`40a686ab89dc5ee58ec10d0514dd5c99abbc472b`), C++17 `-O3 -g0 -DNDEBUG`. Each backend is compiled separately; the SSE profile enables `-mpopcnt` for all three compilers. Compile times cover `src/simdjson.cpp` to an object; runtime covers repeated DOM parsing of hot input with a reused parser. One GCC-built driver per backend isolates library code generation. All three libraries produce identical canonical JSON for both inputs.

Ratios are reference time / cppgm time: **higher is better**. Medians of paired ABBA/BAAB block ratios; A/A calibration before and after each comparison.

| Backend | Measurement | vs GCC CPU | vs Clang CPU | vs GCC wall | vs Clang wall |
|---|---|---:|---:|---:|---:|
| scalar | Compile | 4.199× | 2.198× | 4.124× | 2.140× |
| scalar | twitter.json | 1.020× | 0.924× | 1.020× | 0.924× |
| scalar | citm_catalog.json | 0.811× | 0.852× | 0.811× | 0.852× |
| sse | Compile | 4.393× | 2.479× | 4.327× | 2.459× |
| sse | twitter.json | 1.031× | 1.024× | 1.031× | 1.024× |
| sse | citm_catalog.json | 1.009× | 0.991× | 1.009× | 0.991× |

Calibration variation above 2%: none. No observations are dropped.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results and per-block ratios](results.json) · [Provenance and header hashes](metadata.json) · [Compile observations](compile-observations.jsonl) · [Runtime observations](runtime-observations.jsonl)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43559814649)
