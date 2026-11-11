# simdjson benchmarks

Compiler: `7dbe6549e963e023f8fa4deca3f4e77211771e15` · CPU: AMD EPYC 7763 64-Core Processor

Unmodified simdjson v5.1.1 (`40a686ab89dc5ee58ec10d0514dd5c99abbc472b`), C++17 `-O3 -g0 -DNDEBUG`. Each backend is compiled separately; the SSE profile enables `-mpopcnt` for all three compilers. Compile times cover `src/simdjson.cpp` to an object; runtime covers repeated DOM parsing of hot input with a reused parser. One GCC-built driver per backend isolates library code generation. All three libraries produce identical canonical JSON for both inputs.

Ratios are reference time / cppgm time: **higher is better**. Medians of paired ABBA/BAAB block ratios; A/A calibration before and after each comparison.

| Backend | Measurement | vs GCC CPU | vs Clang CPU | vs GCC wall | vs Clang wall |
|---|---|---:|---:|---:|---:|
| scalar | Compile | 4.467× | 2.329× | 4.379× | 2.309× |
| scalar | twitter.json | 0.875× | 0.801× | 0.875× | 0.800× |
| scalar | citm_catalog.json | 0.757× | 0.801× | 0.757× | 0.801× |
| sse | Compile | 4.492× | 2.558× | 4.456× | 2.533× |
| sse | twitter.json | 0.902× | 0.896× | 0.902× | 0.896× |
| sse | citm_catalog.json | 0.898× | 0.887× | 0.898× | 0.887× |

Calibration variation above 2%: scalar citm_catalog.json gcc cpu A/A after: 1.041×; scalar citm_catalog.json gcc wall A/A after: 1.041×. No observations are dropped.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results and per-block ratios](results.json) · [Provenance and header hashes](metadata.json) · [Compile observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42416405199) · [Runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42416405199)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42416405199)
