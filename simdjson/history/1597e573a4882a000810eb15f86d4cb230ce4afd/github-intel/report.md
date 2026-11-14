# simdjson benchmarks

Compiler: `1597e573a4882a000810eb15f86d4cb230ce4afd` · CPU: INTEL(R) XEON(R) PLATINUM 8573C

Unmodified simdjson v5.1.1 (`40a686ab89dc5ee58ec10d0514dd5c99abbc472b`), C++17 `-O3 -g0 -DNDEBUG`. Each backend is compiled separately; the SSE profile enables `-mpopcnt` for all three compilers. Compile times cover `src/simdjson.cpp` to an object; runtime covers repeated DOM parsing of hot input with a reused parser. One GCC-built driver per backend isolates library code generation. All three libraries produce identical canonical JSON for both inputs.

Ratios are reference time / cppgm time: **higher is better**. Medians of paired ABBA/BAAB block ratios; A/A calibration before and after each comparison.

| Backend | Measurement | vs GCC CPU | vs Clang CPU | vs GCC wall | vs Clang wall |
|---|---|---:|---:|---:|---:|
| scalar | Compile | 3.732× | 1.756× | 3.655× | 1.720× |
| scalar | twitter.json | 1.149× | 1.019× | 1.149× | 1.019× |
| scalar | citm_catalog.json | 1.060× | 1.043× | 1.060× | 1.043× |
| sse | Compile | 3.908× | 1.908× | 3.850× | 1.893× |
| sse | twitter.json | 0.968× | 0.964× | 0.968× | 0.964× |
| sse | citm_catalog.json | 0.966× | 0.995× | 0.966× | 0.995× |

Calibration variation above 2%: sse citm_catalog.json clang cpu A/A after: 1.024×; sse citm_catalog.json clang wall A/A after: 1.024×. No observations are dropped.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results and per-block ratios](results.json) · [Provenance and header hashes](metadata.json) · [Compile observations](compile-observations.jsonl) · [Runtime observations](runtime-observations.jsonl)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43572026938)
