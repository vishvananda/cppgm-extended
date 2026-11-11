# simdjson benchmarks

Compiler: `9ca649bf59cb1e14979655d1669ade353a9ae881` · CPU: Intel(R) Xeon(R) CPU E5-2696 v4 @ 2.20GHz

Unmodified simdjson v5.1.1 (`40a686ab89dc5ee58ec10d0514dd5c99abbc472b`), C++17 `-O3 -g0 -DNDEBUG`. Each backend is compiled separately; the SSE profile enables `-mpopcnt` for all three compilers. Compile times cover `src/simdjson.cpp` to an object; runtime covers repeated DOM parsing of hot input with a reused parser. One GCC-built driver per backend isolates library code generation. All three libraries produce identical canonical JSON for both inputs.

Ratios are reference time / cppgm time: **higher is better**. Medians of paired ABBA/BAAB block ratios; A/A calibration before and after each comparison.

| Backend | Measurement | vs GCC CPU | vs Clang CPU | vs GCC wall | vs Clang wall |
|---|---|---:|---:|---:|---:|
| scalar | Compile | 4.521× | 2.283× | 4.455× | 2.250× |
| scalar | twitter.json | 1.350× | 1.296× | 1.327× | 1.315× |
| scalar | citm_catalog.json | 1.112× | 1.130× | 1.088× | 1.131× |
| sse | Compile | 4.651× | 2.526× | 4.593× | 2.496× |
| sse | twitter.json | 0.966× | 0.949× | 0.965× | 0.950× |
| sse | citm_catalog.json | 0.952× | 0.971× | 0.951× | 0.966× |

Calibration variation above 2%: scalar Compile gcc cpu A/A after: 0.878×; scalar Compile gcc wall A/A after: 0.842×; scalar twitter.json gcc cpu A/A after: 1.030×; scalar twitter.json gcc wall A/A after: 1.029×; scalar twitter.json clang wall A/A before: 1.030×; scalar twitter.json clang wall A/A after: 0.954×; scalar citm_catalog.json gcc wall A/A before: 1.035×; scalar citm_catalog.json clang wall A/A before: 1.022×; sse Compile gcc cpu A/A before: 0.970×; sse Compile gcc cpu A/A after: 0.980×; sse Compile gcc wall A/A before: 0.970×. No observations are dropped.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results and per-block ratios](results.json) · [Provenance and header hashes](metadata.json) · [Compile observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42711159711) · [Runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42711159711)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42711159711)
