# simdjson benchmarks

Compiler: `6c589867bdd5c99bc35b90267a46778f59e1127e` · CPU: Intel(R) Xeon(R) CPU E5-2696 v4 @ 2.20GHz

Unmodified simdjson v5.1.1 (`40a686ab89dc5ee58ec10d0514dd5c99abbc472b`), C++17 `-O3 -g0 -DNDEBUG`. Each backend is compiled separately; the SSE profile enables `-mpopcnt` for all three compilers. Compile times cover `src/simdjson.cpp` to an object; runtime covers repeated DOM parsing of hot input with a reused parser. One GCC-built driver per backend isolates library code generation. All three libraries produce identical canonical JSON for both inputs.

Ratios are reference time / cppgm time: **higher is better**. Medians of paired ABBA/BAAB block ratios; A/A calibration before and after each comparison.

| Backend | Measurement | vs GCC CPU | vs Clang CPU | vs GCC wall | vs Clang wall |
|---|---|---:|---:|---:|---:|
| scalar | Compile | 4.551× | 2.245× | 4.639× | 2.315× |
| scalar | twitter.json | 1.075× | 1.045× | 1.081× | 1.043× |
| scalar | citm_catalog.json | 0.623× | 0.653× | 0.623× | 0.653× |
| sse | Compile | 4.616× | 2.535× | 5.036× | 2.487× |
| sse | twitter.json | 0.964× | 0.965× | 0.945× | 0.938× |
| sse | citm_catalog.json | 0.940× | 0.970× | 0.882× | 0.933× |

Calibration variation above 2%: scalar Compile gcc cpu A/A before: 1.020×; scalar Compile gcc cpu A/A after: 0.960×; scalar Compile gcc wall A/A after: 0.938×; scalar Compile clang cpu A/A after: 0.963×; scalar Compile clang wall A/A after: 0.966×; scalar citm_catalog.json gcc cpu A/A before: 0.962×; scalar citm_catalog.json gcc wall A/A before: 0.963×; scalar citm_catalog.json clang cpu A/A after: 1.024×; scalar citm_catalog.json clang wall A/A after: 1.024×; sse Compile gcc cpu A/A after: 1.088×; sse Compile gcc wall A/A before: 0.934×; sse Compile gcc wall A/A after: 1.064×; sse Compile clang cpu A/A before: 1.042×; sse Compile clang wall A/A before: 1.026×; sse twitter.json gcc cpu A/A after: 0.980×; sse twitter.json gcc wall A/A before: 1.080×; sse twitter.json clang wall A/A before: 0.972×; sse citm_catalog.json gcc wall A/A before: 1.191×; sse citm_catalog.json clang cpu A/A after: 1.028×; sse citm_catalog.json clang wall A/A before: 0.960×; sse citm_catalog.json clang wall A/A after: 1.058×. No observations are dropped.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results and per-block ratios](results.json) · [Provenance and header hashes](metadata.json) · [Compile observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42694500988) · [Runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42694500988)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42694500988)
