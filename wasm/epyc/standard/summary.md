# Correctness and performance

Speedup = reference time / cppgm time; higher is better. CPU and elapsed metrics are separate.
Each row keeps GCC and Clang separate for one phase and metric. Missing values are not passes or zeros.

Availability describes the supplied records; measurements from other reports may not have been imported.

| Workload / configuration | Optimization levels | Availability | Details |
|---|---|---|---|
| Internal compiler tests / Required assignment contracts | declared: mixed; recorded: none | not measured | TESTING_AND_REFERENCES.md |
| Internal compiler tests / Qualified C++ object overlap | declared: O0, O3; recorded: none | not measured | TESTING_AND_REFERENCES.md |
| GCC C++11 language tests / Source-reviewed language profile | declared: mixed; recorded: none | not measured | reports/cxx11-completion.txt |
| Clang C++11 language tests / Source-reviewed language profile | declared: mixed; recorded: none | not measured | reports/cxx11-completion.txt |
| Boost compilation and test execution / 17 C++11 suites at O3 with assertions; GDB correctness-only | declared: O3, mixed; recorded: none | not measured | benchmarks/boost/README.md |
| Boost compilation and test execution / Historical 17-suite C++11 debug campaigns | declared: O0, mixed; recorded: none | not measured | benchmarks/boost/README.md |
| Boost compilation and test execution / Historical correctness snapshots | declared: mixed; recorded: none | not measured | benchmarks/boost/README.md |
| Public benchmark suite / Declared public programs | declared: O0, O3; recorded: none | not measured | benchmarks/public/README.md |
| Build the compiler / Course build | declared: mixed; recorded: none | not measured | benchmarks/self_compile/README.md |
| cppgm code generation / Self-built versus host-built compiler | declared: mixed; recorded: none | not measured | benchmarks/self_compile/README.md |
| index-cpp / All 50 checksummed workloads | declared: O0, O2, O3; recorded: none | not measured | benchmarks/index_cpp/README.md |
| simdjson / Scalar fallback | declared: O3; recorded: none | not measured | benchmarks/simdjson/README.md |
| simdjson / SSE backend | declared: O3; recorded: none | not measured | benchmarks/simdjson/README.md |
| Godot build and benchmark / Full headless release and five checked GDScript workloads | declared: O3; recorded: none | not measured | benchmarks/godot/README.md |
| [Wasm32 portable kernels / Scalar source](wasm.md) | declared: O0, O1, O2, O3; recorded: O0, O3 | recorded | pa37/benchmarks/README.md |
| [Wasm32 portable kernels / Explicit SIMD128 source](wasm.md) | declared: O0, O1, O2, O3; recorded: O0, O3 | recorded | pa37/benchmarks/README.md |

## Correctness coverage

| Workload / configuration | Oracle | CPU / environment | Opt | Passed / executed / selected | Excluded | Status | Revision |
|---|---|---|---|---|---|---|---|
| [wasm/scalar](wasm.md) | Independent Perl arithmetic, seeded edge cases and cross-compiler checksums | AMD EPYC 7763 64-Core Processor / GitHub hosted; Ubuntu 26.04 container | O0 | 64 / 64 / 64 independent oracle cases | 0 | passed | b81455bc7 |
| [wasm/simd128](wasm.md) | Independent Perl arithmetic, seeded edge cases and cross-compiler checksums | AMD EPYC 7763 64-Core Processor / GitHub hosted; Ubuntu 26.04 container | O0 | 64 / 64 / 64 independent oracle cases | 0 | passed | b81455bc7 |
| [wasm/scalar](wasm.md) | Independent Perl arithmetic, seeded edge cases and cross-compiler checksums | AMD EPYC 7763 64-Core Processor / GitHub hosted; Ubuntu 26.04 container | O3 | 64 / 64 / 64 independent oracle cases | 0 | passed | b81455bc7 |
| [wasm/simd128](wasm.md) | Independent Perl arithmetic, seeded edge cases and cross-compiler checksums | AMD EPYC 7763 64-Core Processor / GitHub hosted; Ubuntu 26.04 container | O3 | 64 / 64 / 64 independent oracle cases | 0 | passed | b81455bc7 |

## AMD EPYC 7763 64-Core Processor — GitHub hosted; Ubuntu 26.04 container

Architecture: x86_64; host profile `wasm-d8d889911e4534c1`.

| Workload / configuration / test | Opt | Phase / metric | vs GCC | vs Clang | Validation | Case set | Revision |
|---|---|---|---:|---:|---|---|---|
| [wasm/scalar/checked-workloads](wasm.md) | O0 | runtime / cpu_seconds | not-applicable | 0.954× | clang: 64/64 cppgm, 64/64 reference; gcc: not-applicable | 64 independent oracle cases; set e6ee71a3 | `b81455bc7` |
| [wasm/scalar/checked-workloads](wasm.md) | O0 | runtime / elapsed_seconds | not-applicable | 0.953× | clang: 64/64 cppgm, 64/64 reference; gcc: not-applicable | 64 independent oracle cases; set e6ee71a3 | `b81455bc7` |
| [wasm/scalar/checked-workloads](wasm.md) | O3 | runtime / cpu_seconds | not-applicable | 0.861× | clang: 64/64 cppgm, 64/64 reference; gcc: not-applicable | 64 independent oracle cases; set e6ee71a3 | `b81455bc7` |
| [wasm/scalar/checked-workloads](wasm.md) | O3 | runtime / elapsed_seconds | not-applicable | 0.861× | clang: 64/64 cppgm, 64/64 reference; gcc: not-applicable | 64 independent oracle cases; set e6ee71a3 | `b81455bc7` |
| [wasm/scalar/instantiation-and-start](wasm.md) | O0 | startup / elapsed_seconds | not-applicable | 0.938× | clang: 64/64 cppgm, 64/64 reference; gcc: not-applicable | 64 independent oracle cases; set e6ee71a3 | `b81455bc7` |
| [wasm/scalar/instantiation-and-start](wasm.md) | O3 | startup / elapsed_seconds | not-applicable | 1.087× | clang: 64/64 cppgm, 64/64 reference; gcc: not-applicable | 64 independent oracle cases; set e6ee71a3 | `b81455bc7` |
| [wasm/scalar/module-size](wasm.md) | O0 | link / bytes | not-applicable | 0.549× | clang: 64/64 cppgm, 64/64 reference; gcc: not-applicable | 64 independent oracle cases; set e6ee71a3 | `b81455bc7` |
| [wasm/scalar/module-size](wasm.md) | O3 | link / bytes | not-applicable | 0.572× | clang: 64/64 cppgm, 64/64 reference; gcc: not-applicable | 64 independent oracle cases; set e6ee71a3 | `b81455bc7` |
| [wasm/scalar/one-translation-unit](wasm.md) | O0 | compile / cpu_seconds | not-applicable | 3.474× | clang: 64/64 cppgm, 64/64 reference; gcc: not-applicable | 64 independent oracle cases; set e6ee71a3 | `b81455bc7` |
| [wasm/scalar/one-translation-unit](wasm.md) | O0 | compile / elapsed_seconds | not-applicable | 3.410× | clang: 64/64 cppgm, 64/64 reference; gcc: not-applicable | 64 independent oracle cases; set e6ee71a3 | `b81455bc7` |
| [wasm/scalar/one-translation-unit](wasm.md) | O0 | link / cpu_seconds | not-applicable | 1.454× | clang: 64/64 cppgm, 64/64 reference; gcc: not-applicable | 64 independent oracle cases; set e6ee71a3 | `b81455bc7` |
| [wasm/scalar/one-translation-unit](wasm.md) | O0 | link / elapsed_seconds | not-applicable | 1.429× | clang: 64/64 cppgm, 64/64 reference; gcc: not-applicable | 64 independent oracle cases; set e6ee71a3 | `b81455bc7` |
| [wasm/scalar/one-translation-unit](wasm.md) | O3 | compile / cpu_seconds | not-applicable | 10.496× | clang: 64/64 cppgm, 64/64 reference; gcc: not-applicable | 64 independent oracle cases; set e6ee71a3 | `b81455bc7` |
| [wasm/scalar/one-translation-unit](wasm.md) | O3 | compile / elapsed_seconds | not-applicable | 10.323× | clang: 64/64 cppgm, 64/64 reference; gcc: not-applicable | 64 independent oracle cases; set e6ee71a3 | `b81455bc7` |
| [wasm/scalar/one-translation-unit](wasm.md) | O3 | link / cpu_seconds | not-applicable | 1.539× | clang: 64/64 cppgm, 64/64 reference; gcc: not-applicable | 64 independent oracle cases; set e6ee71a3 | `b81455bc7` |
| [wasm/scalar/one-translation-unit](wasm.md) | O3 | link / elapsed_seconds | not-applicable | 1.521× | clang: 64/64 cppgm, 64/64 reference; gcc: not-applicable | 64 independent oracle cases; set e6ee71a3 | `b81455bc7` |
| [wasm/simd128/checked-workloads](wasm.md) | O0 | runtime / cpu_seconds | not-applicable | 0.953× | clang: 64/64 cppgm, 64/64 reference; gcc: not-applicable | 64 independent oracle cases; set e6ee71a3 | `b81455bc7` |
| [wasm/simd128/checked-workloads](wasm.md) | O0 | runtime / elapsed_seconds | not-applicable | 0.953× | clang: 64/64 cppgm, 64/64 reference; gcc: not-applicable | 64 independent oracle cases; set e6ee71a3 | `b81455bc7` |
| [wasm/simd128/checked-workloads](wasm.md) | O3 | runtime / cpu_seconds | not-applicable | 0.797× | clang: 64/64 cppgm, 64/64 reference; gcc: not-applicable | 64 independent oracle cases; set e6ee71a3 | `b81455bc7` |
| [wasm/simd128/checked-workloads](wasm.md) | O3 | runtime / elapsed_seconds | not-applicable | 0.796× | clang: 64/64 cppgm, 64/64 reference; gcc: not-applicable | 64 independent oracle cases; set e6ee71a3 | `b81455bc7` |
| [wasm/simd128/instantiation-and-start](wasm.md) | O0 | startup / elapsed_seconds | not-applicable | 0.990× | clang: 64/64 cppgm, 64/64 reference; gcc: not-applicable | 64 independent oracle cases; set e6ee71a3 | `b81455bc7` |
| [wasm/simd128/instantiation-and-start](wasm.md) | O3 | startup / elapsed_seconds | not-applicable | 0.962× | clang: 64/64 cppgm, 64/64 reference; gcc: not-applicable | 64 independent oracle cases; set e6ee71a3 | `b81455bc7` |
| [wasm/simd128/module-size](wasm.md) | O0 | link / bytes | not-applicable | 0.518× | clang: 64/64 cppgm, 64/64 reference; gcc: not-applicable | 64 independent oracle cases; set e6ee71a3 | `b81455bc7` |
| [wasm/simd128/module-size](wasm.md) | O3 | link / bytes | not-applicable | 0.534× | clang: 64/64 cppgm, 64/64 reference; gcc: not-applicable | 64 independent oracle cases; set e6ee71a3 | `b81455bc7` |
| [wasm/simd128/one-translation-unit](wasm.md) | O0 | compile / cpu_seconds | not-applicable | 3.570× | clang: 64/64 cppgm, 64/64 reference; gcc: not-applicable | 64 independent oracle cases; set e6ee71a3 | `b81455bc7` |
| [wasm/simd128/one-translation-unit](wasm.md) | O0 | compile / elapsed_seconds | not-applicable | 3.526× | clang: 64/64 cppgm, 64/64 reference; gcc: not-applicable | 64 independent oracle cases; set e6ee71a3 | `b81455bc7` |
| [wasm/simd128/one-translation-unit](wasm.md) | O0 | link / cpu_seconds | not-applicable | 1.428× | clang: 64/64 cppgm, 64/64 reference; gcc: not-applicable | 64 independent oracle cases; set e6ee71a3 | `b81455bc7` |
| [wasm/simd128/one-translation-unit](wasm.md) | O0 | link / elapsed_seconds | not-applicable | 1.417× | clang: 64/64 cppgm, 64/64 reference; gcc: not-applicable | 64 independent oracle cases; set e6ee71a3 | `b81455bc7` |
| [wasm/simd128/one-translation-unit](wasm.md) | O3 | compile / cpu_seconds | not-applicable | 10.187× | clang: 64/64 cppgm, 64/64 reference; gcc: not-applicable | 64 independent oracle cases; set e6ee71a3 | `b81455bc7` |
| [wasm/simd128/one-translation-unit](wasm.md) | O3 | compile / elapsed_seconds | not-applicable | 10.079× | clang: 64/64 cppgm, 64/64 reference; gcc: not-applicable | 64 independent oracle cases; set e6ee71a3 | `b81455bc7` |
| [wasm/simd128/one-translation-unit](wasm.md) | O3 | link / cpu_seconds | not-applicable | 1.554× | clang: 64/64 cppgm, 64/64 reference; gcc: not-applicable | 64 independent oracle cases; set e6ee71a3 | `b81455bc7` |
| [wasm/simd128/one-translation-unit](wasm.md) | O3 | link / elapsed_seconds | not-applicable | 1.528× | clang: 64/64 cppgm, 64/64 reference; gcc: not-applicable | 64 independent oracle cases; set e6ee71a3 | `b81455bc7` |
