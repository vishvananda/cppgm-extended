# simdjson benchmarks

Compiler: `439e2c62e15e7a4d4bf71f07dda6d8e247ed6c9d` · CPU: INTEL(R) XEON(R) PLATINUM 8573C

Unmodified simdjson v5.1.1 (`40a686ab89dc5ee58ec10d0514dd5c99abbc472b`), C++17 `-O3 -g0 -DNDEBUG`. Each backend is compiled separately; the SSE profile enables `-mpopcnt` for all three compilers. Compile times cover `src/simdjson.cpp` to an object; runtime covers repeated DOM parsing of hot input with a reused parser. One GCC-built driver per backend isolates library code generation. All three libraries produce identical canonical JSON for both inputs.

Ratios are reference time / cppgm time: **higher is better**. Medians of paired ABBA/BAAB block ratios; A/A calibration before and after each comparison.

| Backend | Measurement | vs GCC CPU | vs Clang CPU | vs GCC wall | vs Clang wall |
|---|---|---:|---:|---:|---:|
| scalar | Compile | 4.132× | 1.955× | 4.075× | 1.956× |
| scalar | twitter.json | 0.920× | 0.808× | 0.920× | 0.808× |
| scalar | citm_catalog.json | 0.726× | 0.713× | 0.726× | 0.713× |
| sse | Compile | 4.173× | 2.035× | 4.166× | 2.036× |
| sse | twitter.json | 0.947× | 0.951× | 0.947× | 0.951× |
| sse | citm_catalog.json | 0.948× | 0.975× | 0.948× | 0.975× |

Calibration variation above 2%: none. No observations are dropped.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results and per-block ratios](results.json) · [Provenance and header hashes](metadata.json) · [Compile observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42568872247) · [Runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42568872247)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42568872247)
