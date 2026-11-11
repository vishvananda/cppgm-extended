# simdjson benchmarks

Compiler: `48e44d6ab2db3dd6d667a9aef1a21fcb36b65f3f` · CPU: AMD EPYC 9V45 96-Core Processor

Unmodified simdjson v5.1.1 (`40a686ab89dc5ee58ec10d0514dd5c99abbc472b`), C++17 `-O3 -g0 -DNDEBUG`. Each backend is compiled separately; the SSE profile enables `-mpopcnt` for all three compilers. Compile times cover `src/simdjson.cpp` to an object; runtime covers repeated DOM parsing of hot input with a reused parser. One GCC-built driver per backend isolates library code generation. All three libraries produce identical canonical JSON for both inputs.

Ratios are reference time / cppgm time: **higher is better**. Medians of paired ABBA/BAAB block ratios; A/A calibration before and after each comparison.

| Backend | Measurement | vs GCC CPU | vs Clang CPU | vs GCC wall | vs Clang wall |
|---|---|---:|---:|---:|---:|
| scalar | Compile | 4.599× | 2.168× | 4.473× | 2.129× |
| scalar | twitter.json | 1.040× | 1.027× | 1.040× | 1.027× |
| scalar | citm_catalog.json | 1.039× | 1.068× | 1.039× | 1.068× |
| sse | Compile | 4.577× | 2.101× | 4.453× | 2.035× |
| sse | twitter.json | 0.962× | 0.957× | 0.963× | 0.957× |
| sse | citm_catalog.json | 0.971× | 0.995× | 0.971× | 0.995× |

Calibration variation above 2%: scalar Compile clang cpu A/A after: 1.020×; scalar twitter.json gcc cpu A/A after: 0.936×; scalar twitter.json gcc wall A/A after: 0.937×; sse twitter.json gcc cpu A/A after: 1.044×; sse twitter.json gcc wall A/A after: 1.044×; sse citm_catalog.json gcc cpu A/A before: 1.043×; sse citm_catalog.json gcc wall A/A before: 1.043×; sse citm_catalog.json clang cpu A/A before: 0.975×; sse citm_catalog.json clang wall A/A before: 0.975×. No observations are dropped.

## Toolchains

- cppgm: cppgm++ (host configuration: g++)
- gcc: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- clang: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results and per-block ratios](results.json) · [Provenance and header hashes](metadata.json) · [Compile observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204695) · [Runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204695)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204695)
