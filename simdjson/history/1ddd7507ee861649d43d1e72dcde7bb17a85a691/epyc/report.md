# simdjson benchmarks

Compiler: `1ddd7507ee861649d43d1e72dcde7bb17a85a691` · CPU: AMD EPYC 7763 64-Core Processor

Unmodified simdjson v5.1.1 (`40a686ab89dc5ee58ec10d0514dd5c99abbc472b`), C++17 `-O3 -g0 -DNDEBUG`. Each backend is compiled separately; the SSE profile enables `-mpopcnt` for all three compilers. Compile times cover `src/simdjson.cpp` to an object; runtime covers repeated DOM parsing of hot input with a reused parser. One GCC-built driver per backend isolates library code generation. All three libraries produce identical canonical JSON for both inputs.

Ratios are reference time / cppgm time: **higher is better**. Medians of paired ABBA/BAAB block ratios; A/A calibration before and after each comparison.

| Backend | Measurement | vs GCC CPU | vs Clang CPU | vs GCC wall | vs Clang wall |
|---|---|---:|---:|---:|---:|
| scalar | Compile | 4.593× | 2.379× | 4.556× | 2.332× |
| scalar | twitter.json | 1.078× | 0.995× | 1.078× | 0.995× |
| scalar | citm_catalog.json | 1.003× | 1.051× | 1.003× | 1.051× |
| sse | Compile | 4.529× | 2.578× | 4.456× | 2.554× |
| sse | twitter.json | 1.016× | 1.016× | 1.016× | 1.016× |
| sse | citm_catalog.json | 1.031× | 1.014× | 1.031× | 1.014× |

Calibration variation above 2%: scalar citm_catalog.json gcc cpu A/A before: 1.024×; scalar citm_catalog.json gcc wall A/A before: 1.023×. No observations are dropped.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results and per-block ratios](results.json) · [Provenance and header hashes](metadata.json) · [Compile observations](compile-observations.jsonl) · [Runtime observations](runtime-observations.jsonl)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42988175309)
