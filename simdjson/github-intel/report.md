# simdjson benchmarks

Compiler: `535ed4f12c6c6470e67367b113376f7751e4e65d` · CPU: Intel(R) Xeon(R) Platinum 8370C CPU @ 2.80GHz

Unmodified simdjson v5.1.1 (`40a686ab89dc5ee58ec10d0514dd5c99abbc472b`), C++17 `-O3 -g0 -DNDEBUG`. Each backend is compiled separately; the SSE profile enables `-mpopcnt` for all three compilers. Compile times cover `src/simdjson.cpp` to an object; runtime covers repeated DOM parsing of hot input with a reused parser. One GCC-built driver per backend isolates library code generation. All three libraries produce identical canonical JSON for both inputs.

Ratios are reference time / cppgm time: **higher is better**. Medians of paired ABBA/BAAB block ratios; A/A calibration before and after each comparison.

| Backend | Measurement | vs GCC CPU | vs Clang CPU | vs GCC wall | vs Clang wall |
|---|---|---:|---:|---:|---:|
| scalar | Compile | 4.474× | 2.153× | 4.342× | 2.114× |
| scalar | twitter.json | 1.003× | 0.970× | 1.003× | 0.970× |
| scalar | citm_catalog.json | 1.034× | 1.067× | 1.034× | 1.067× |
| sse | Compile | 4.456× | 2.257× | 4.371× | 2.241× |
| sse | twitter.json | 0.988× | 0.955× | 0.988× | 0.956× |
| sse | citm_catalog.json | 0.973× | 0.960× | 0.973× | 0.960× |

Calibration variation above 2%: scalar citm_catalog.json gcc cpu A/A after: 0.976×; scalar citm_catalog.json gcc wall A/A after: 0.976×; sse twitter.json clang cpu A/A after: 0.970×; sse twitter.json clang wall A/A after: 0.970×. No observations are dropped.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results and per-block ratios](results.json) · [Provenance and header hashes](metadata.json) · [Compile observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136114) · [Runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136114)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136114)
