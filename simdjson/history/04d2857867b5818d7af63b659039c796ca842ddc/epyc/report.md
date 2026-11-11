# simdjson benchmarks

Compiler: `04d2857867b5818d7af63b659039c796ca842ddc` · CPU: AMD EPYC 7763 64-Core Processor

Unmodified simdjson v5.1.1 (`40a686ab89dc5ee58ec10d0514dd5c99abbc472b`), C++17 `-O3 -g0 -DNDEBUG`. Each backend is compiled separately; the SSE profile enables `-mpopcnt` for all three compilers. Compile times cover `src/simdjson.cpp` to an object; runtime covers repeated DOM parsing of hot input with a reused parser. One GCC-built driver per backend isolates library code generation. All three libraries produce identical canonical JSON for both inputs.

Ratios are reference time / cppgm time: **higher is better**. Medians of paired ABBA/BAAB block ratios; A/A calibration before and after each comparison.

| Backend | Measurement | vs GCC CPU | vs Clang CPU | vs GCC wall | vs Clang wall |
|---|---|---:|---:|---:|---:|
| scalar | Compile | 4.657× | 2.383× | 4.590× | 2.379× |
| scalar | twitter.json | 1.091× | 0.993× | 1.091× | 0.993× |
| scalar | citm_catalog.json | 1.002× | 1.051× | 1.002× | 1.051× |
| sse | Compile | 4.595× | 2.599× | 4.544× | 2.576× |
| sse | twitter.json | 1.022× | 1.012× | 1.022× | 1.012× |
| sse | citm_catalog.json | 1.027× | 1.012× | 1.027× | 1.012× |

Calibration variation above 2%: scalar citm_catalog.json gcc cpu A/A after: 1.035×; scalar citm_catalog.json gcc wall A/A after: 1.036×. No observations are dropped.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results and per-block ratios](results.json) · [Provenance and header hashes](metadata.json) · [Compile observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039922) · [Runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039922)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039922)
