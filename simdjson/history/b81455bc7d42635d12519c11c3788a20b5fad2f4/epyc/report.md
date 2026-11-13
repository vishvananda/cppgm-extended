# simdjson benchmarks

Compiler: `b81455bc7d42635d12519c11c3788a20b5fad2f4` · CPU: AMD EPYC 7763 64-Core Processor

Unmodified simdjson v5.1.1 (`40a686ab89dc5ee58ec10d0514dd5c99abbc472b`), C++17 `-O3 -g0 -DNDEBUG`. Each backend is compiled separately; the SSE profile enables `-mpopcnt` for all three compilers. Compile times cover `src/simdjson.cpp` to an object; runtime covers repeated DOM parsing of hot input with a reused parser. One GCC-built driver per backend isolates library code generation. All three libraries produce identical canonical JSON for both inputs.

Ratios are reference time / cppgm time: **higher is better**. Medians of paired ABBA/BAAB block ratios; A/A calibration before and after each comparison.

| Backend | Measurement | vs GCC CPU | vs Clang CPU | vs GCC wall | vs Clang wall |
|---|---|---:|---:|---:|---:|
| scalar | Compile | 4.183× | 2.176× | 4.142× | 2.178× |
| scalar | twitter.json | 1.012× | 0.919× | 1.012× | 0.919× |
| scalar | citm_catalog.json | 0.948× | 0.997× | 0.948× | 0.997× |
| sse | Compile | 4.375× | 2.479× | 4.297× | 2.424× |
| sse | twitter.json | 1.033× | 1.022× | 1.033× | 1.022× |
| sse | citm_catalog.json | 1.013× | 1.001× | 1.013× | 1.001× |

Calibration variation above 2%: none. No observations are dropped.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results and per-block ratios](results.json) · [Provenance and header hashes](metadata.json) · [Compile observations](compile-observations.jsonl) · [Runtime observations](runtime-observations.jsonl)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43493409387)
