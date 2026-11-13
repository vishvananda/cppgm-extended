# simdjson benchmarks

Compiler: `dc908433031b1e72d07206262cb240cdedc9d13b` · CPU: INTEL(R) XEON(R) PLATINUM 8573C

Unmodified simdjson v5.1.1 (`40a686ab89dc5ee58ec10d0514dd5c99abbc472b`), C++17 `-O3 -g0 -DNDEBUG`. Each backend is compiled separately; the SSE profile enables `-mpopcnt` for all three compilers. Compile times cover `src/simdjson.cpp` to an object; runtime covers repeated DOM parsing of hot input with a reused parser. One GCC-built driver per backend isolates library code generation. All three libraries produce identical canonical JSON for both inputs.

Ratios are reference time / cppgm time: **higher is better**. Medians of paired ABBA/BAAB block ratios; A/A calibration before and after each comparison.

| Backend | Measurement | vs GCC CPU | vs Clang CPU | vs GCC wall | vs Clang wall |
|---|---|---:|---:|---:|---:|
| scalar | Compile | 3.684× | 1.729× | 3.588× | 1.713× |
| scalar | twitter.json | 0.990× | 0.875× | 0.990× | 0.875× |
| scalar | citm_catalog.json | 0.928× | 0.909× | 0.928× | 0.909× |
| sse | Compile | 3.887× | 1.899× | 3.812× | 1.868× |
| sse | twitter.json | 0.924× | 0.917× | 0.924× | 0.917× |
| sse | citm_catalog.json | 0.923× | 0.952× | 0.923× | 0.952× |

Calibration variation above 2%: scalar twitter.json gcc cpu A/A after: 1.021×; scalar twitter.json gcc wall A/A after: 1.021×. No observations are dropped.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results and per-block ratios](results.json) · [Provenance and header hashes](metadata.json) · [Compile observations](compile-observations.jsonl) · [Runtime observations](runtime-observations.jsonl)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43395345290)
