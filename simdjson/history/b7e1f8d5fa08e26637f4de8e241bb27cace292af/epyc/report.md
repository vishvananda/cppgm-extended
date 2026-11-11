# simdjson benchmarks

Compiler: `b7e1f8d5fa08e26637f4de8e241bb27cace292af` · CPU: AMD EPYC 7763 64-Core Processor

Unmodified simdjson v5.1.1 (`40a686ab89dc5ee58ec10d0514dd5c99abbc472b`), C++17 `-O3 -g0 -DNDEBUG`. Each backend is compiled separately; the SSE profile enables `-mpopcnt` for all three compilers. Compile times cover `src/simdjson.cpp` to an object; runtime covers repeated DOM parsing of hot input with a reused parser. One GCC-built driver per backend isolates library code generation. All three libraries produce identical canonical JSON for both inputs.

Ratios are reference time / cppgm time: **higher is better**. Medians of paired ABBA/BAAB block ratios; A/A calibration before and after each comparison.

| Backend | Measurement | vs GCC CPU | vs Clang CPU | vs GCC wall | vs Clang wall |
|---|---|---:|---:|---:|---:|
| scalar | Compile | 4.530× | 2.346× | 4.415× | 2.298× |
| scalar | twitter.json | 1.047× | 0.963× | 1.047× | 0.963× |
| scalar | citm_catalog.json | 0.979× | 0.998× | 0.979× | 0.998× |
| sse | Compile | 4.634× | 2.574× | 4.574× | 2.517× |
| sse | twitter.json | 0.903× | 0.894× | 0.903× | 0.894× |
| sse | citm_catalog.json | 0.899× | 0.888× | 0.899× | 0.888× |

Calibration variation above 2%: sse citm_catalog.json clang cpu A/A before: 0.947×; sse citm_catalog.json clang wall A/A before: 0.945×. No observations are dropped.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results and per-block ratios](results.json) · [Provenance and header hashes](metadata.json) · [Compile observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42557497655) · [Runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42557497655)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42557497655)
