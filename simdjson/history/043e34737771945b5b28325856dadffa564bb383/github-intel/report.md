# simdjson benchmarks

Compiler: `043e34737771945b5b28325856dadffa564bb383` · CPU: Intel(R) Xeon(R) Platinum 8370C CPU @ 2.80GHz

Unmodified simdjson v5.1.1 (`40a686ab89dc5ee58ec10d0514dd5c99abbc472b`), C++17 `-O3 -g0 -DNDEBUG`. Each backend is compiled separately; the SSE profile enables `-mpopcnt` for all three compilers. Compile times cover `src/simdjson.cpp` to an object; runtime covers repeated DOM parsing of hot input with a reused parser. One GCC-built driver per backend isolates library code generation. All three libraries produce identical canonical JSON for both inputs.

Ratios are reference time / cppgm time: **higher is better**. Medians of paired ABBA/BAAB block ratios; A/A calibration before and after each comparison.

| Backend | Measurement | vs GCC CPU | vs Clang CPU | vs GCC wall | vs Clang wall |
|---|---|---:|---:|---:|---:|
| scalar | Compile | 4.552× | 2.201× | 4.423× | 2.165× |
| scalar | twitter.json | 0.853× | 0.822× | 0.853× | 0.822× |
| scalar | citm_catalog.json | 0.645× | 0.670× | 0.645× | 0.670× |
| sse | Compile | 4.540× | 2.305× | 4.500× | 2.262× |
| sse | twitter.json | 0.988× | 0.953× | 0.988× | 0.953× |
| sse | citm_catalog.json | 0.978× | 0.966× | 0.978× | 0.966× |

Calibration variation above 2%: none. No observations are dropped.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results and per-block ratios](results.json) · [Provenance and header hashes](metadata.json) · [Compile observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42700385905) · [Runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42700385905)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42700385905)
