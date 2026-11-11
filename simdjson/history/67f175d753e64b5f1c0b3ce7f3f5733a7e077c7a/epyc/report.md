# simdjson benchmarks

Compiler: `67f175d753e64b5f1c0b3ce7f3f5733a7e077c7a` · CPU: AMD EPYC 9V45 96-Core Processor

Unmodified simdjson v5.1.1 (`40a686ab89dc5ee58ec10d0514dd5c99abbc472b`), C++17 `-O3 -g0 -DNDEBUG`. Each backend is compiled separately; the SSE profile enables `-mpopcnt` for all three compilers. Compile times cover `src/simdjson.cpp` to an object; runtime covers repeated DOM parsing of hot input with a reused parser. One GCC-built driver per backend isolates library code generation. All three libraries produce identical canonical JSON for both inputs.

Ratios are reference time / cppgm time: **higher is better**. Medians of paired ABBA/BAAB block ratios; A/A calibration before and after each comparison.

| Backend | Measurement | vs GCC CPU | vs Clang CPU | vs GCC wall | vs Clang wall |
|---|---|---:|---:|---:|---:|
| scalar | Compile | 4.380× | 1.987× | 4.288× | 1.922× |
| scalar | twitter.json | 0.862× | 0.853× | 0.862× | 0.853× |
| scalar | citm_catalog.json | 0.927× | 0.923× | 0.927× | 0.923× |
| sse | Compile | 4.367× | 2.037× | 4.295× | 2.012× |
| sse | twitter.json | 0.935× | 0.946× | 0.935× | 0.946× |
| sse | citm_catalog.json | 0.960× | 1.000× | 0.960× | 1.000× |

Calibration variation above 2%: scalar Compile gcc wall A/A before: 0.979×; sse twitter.json gcc cpu A/A before: 1.054×; sse twitter.json gcc wall A/A before: 1.054×. No observations are dropped.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results and per-block ratios](results.json) · [Provenance and header hashes](metadata.json) · [Compile observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42506478754) · [Runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42506478754)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42506478754)
