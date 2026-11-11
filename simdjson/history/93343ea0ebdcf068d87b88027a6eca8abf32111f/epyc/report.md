# simdjson benchmarks

Compiler: `93343ea0ebdcf068d87b88027a6eca8abf32111f` · CPU: AMD EPYC 7763 64-Core Processor

Unmodified simdjson v5.1.1 (`40a686ab89dc5ee58ec10d0514dd5c99abbc472b`), C++17 `-O3 -g0 -DNDEBUG`. Each backend is compiled separately; the SSE profile enables `-mpopcnt` for all three compilers. Compile times cover `src/simdjson.cpp` to an object; runtime covers repeated DOM parsing of hot input with a reused parser. One GCC-built driver per backend isolates library code generation. All three libraries produce identical canonical JSON for both inputs.

Ratios are reference time / cppgm time: **higher is better**. Medians of paired ABBA/BAAB block ratios; A/A calibration before and after each comparison.

| Backend | Measurement | vs GCC CPU | vs Clang CPU | vs GCC wall | vs Clang wall |
|---|---|---:|---:|---:|---:|
| scalar | Compile | 4.604× | 2.390× | 4.475× | 2.357× |
| scalar | twitter.json | 0.840× | 0.762× | 0.840× | 0.762× |
| scalar | citm_catalog.json | 0.609× | 0.646× | 0.610× | 0.646× |
| sse | Compile | 4.617× | 2.588× | 4.524× | 2.555× |
| sse | twitter.json | 0.906× | 0.896× | 0.906× | 0.896× |
| sse | citm_catalog.json | 0.905× | 0.888× | 0.905× | 0.888× |

Calibration variation above 2%: none. No observations are dropped.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results and per-block ratios](results.json) · [Provenance and header hashes](metadata.json) · [Compile observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42682903991) · [Runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42682903991)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42682903991)
