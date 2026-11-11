# simdjson benchmarks

Compiler: `783dbfcf01360d923ff590d2048a0c1764761cc3` · CPU: AMD EPYC 7763 64-Core Processor

Unmodified simdjson v5.1.1 (`40a686ab89dc5ee58ec10d0514dd5c99abbc472b`), C++17 `-O3 -g0 -DNDEBUG`. Each backend is compiled separately; the SSE profile enables `-mpopcnt` for all three compilers. Compile times cover `src/simdjson.cpp` to an object; runtime covers repeated DOM parsing of hot input with a reused parser. One GCC-built driver per backend isolates library code generation. All three libraries produce identical canonical JSON for both inputs.

Ratios are reference time / cppgm time: **higher is better**. Medians of paired ABBA/BAAB block ratios; A/A calibration before and after each comparison.

| Backend | Measurement | vs GCC CPU | vs Clang CPU | vs GCC wall | vs Clang wall |
|---|---|---:|---:|---:|---:|
| scalar | Compile | 4.577× | 2.363× | 4.526× | 2.299× |
| scalar | twitter.json | 0.876× | 0.789× | 0.876× | 0.789× |
| scalar | citm_catalog.json | 0.757× | 0.794× | 0.757× | 0.794× |
| sse | Compile | 4.684× | 2.697× | 4.645× | 2.664× |
| sse | twitter.json | 0.903× | 0.897× | 0.903× | 0.897× |
| sse | citm_catalog.json | 0.898× | 0.888× | 0.898× | 0.888× |

Calibration variation above 2%: scalar Compile gcc cpu A/A before: 1.028×; scalar Compile gcc wall A/A before: 1.028×; sse Compile gcc cpu A/A after: 0.924×; sse Compile gcc wall A/A after: 0.928×; sse Compile clang cpu A/A before: 1.115×; sse Compile clang wall A/A before: 1.113×. No observations are dropped.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results and per-block ratios](results.json) · [Provenance and header hashes](metadata.json) · [Compile observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42493504541) · [Runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42493504541)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42493504541)
