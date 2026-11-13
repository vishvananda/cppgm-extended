# simdjson benchmarks

Compiler: `72d294c916f0f3f43e50e812f7efa91937eb9b66` · CPU: AMD EPYC 9V74 80-Core Processor

Unmodified simdjson v5.1.1 (`40a686ab89dc5ee58ec10d0514dd5c99abbc472b`), C++17 `-O3 -g0 -DNDEBUG`. Each backend is compiled separately; the SSE profile enables `-mpopcnt` for all three compilers. Compile times cover `src/simdjson.cpp` to an object; runtime covers repeated DOM parsing of hot input with a reused parser. One GCC-built driver per backend isolates library code generation. All three libraries produce identical canonical JSON for both inputs.

Ratios are reference time / cppgm time: **higher is better**. Medians of paired ABBA/BAAB block ratios; A/A calibration before and after each comparison.

| Backend | Measurement | vs GCC CPU | vs Clang CPU | vs GCC wall | vs Clang wall |
|---|---|---:|---:|---:|---:|
| scalar | Compile | 4.253× | 2.042× | 4.110× | 2.010× |
| scalar | twitter.json | 0.969× | 0.969× | 0.969× | 0.969× |
| scalar | citm_catalog.json | 0.949× | 1.056× | 0.949× | 1.056× |
| sse | Compile | 4.407× | 2.202× | 4.307× | 2.150× |
| sse | twitter.json | 1.007× | 0.998× | 1.007× | 0.998× |
| sse | citm_catalog.json | 0.999× | 1.002× | 0.999× | 1.002× |

Calibration variation above 2%: sse Compile gcc wall A/A after: 1.037×. No observations are dropped.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results and per-block ratios](results.json) · [Provenance and header hashes](metadata.json) · [Compile observations](compile-observations.jsonl) · [Runtime observations](runtime-observations.jsonl)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43348824559)
