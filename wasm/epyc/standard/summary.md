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
| [wasm/scalar](wasm.md) | Independent Perl arithmetic, seeded edge cases and cross-compiler checksums | AMD EPYC 9V74 80-Core Processor / GitHub hosted; Ubuntu 26.04 container | O0 | 32 / 32 / 32 independent oracle cases | 0 | passed | 393d1b1d0 |
| [wasm/simd128](wasm.md) | Independent Perl arithmetic, seeded edge cases and cross-compiler checksums | AMD EPYC 9V74 80-Core Processor / GitHub hosted; Ubuntu 26.04 container | O0 | 32 / 32 / 32 independent oracle cases | 0 | passed | 393d1b1d0 |
| [wasm/scalar](wasm.md) | Independent Perl arithmetic, seeded edge cases and cross-compiler checksums | AMD EPYC 9V74 80-Core Processor / GitHub hosted; Ubuntu 26.04 container | O3 | 32 / 32 / 32 independent oracle cases | 0 | passed | 393d1b1d0 |
| [wasm/simd128](wasm.md) | Independent Perl arithmetic, seeded edge cases and cross-compiler checksums | AMD EPYC 9V74 80-Core Processor / GitHub hosted; Ubuntu 26.04 container | O3 | 32 / 32 / 32 independent oracle cases | 0 | passed | 393d1b1d0 |

## AMD EPYC 9V74 80-Core Processor — GitHub hosted; Ubuntu 26.04 container

Architecture: x86_64; host profile `wasm-5c098553cfcfe614`.

| Workload / configuration / test | Opt | Phase / metric | vs GCC | vs Clang | Validation | Case set | Revision |
|---|---|---|---:|---:|---|---|---|
| [wasm/scalar/checked-kernels](wasm.md) | O0 | runtime / elapsed_seconds | not-applicable | 0.981× | clang: 32/32 cppgm, 32/32 reference; gcc: not-applicable | 32 independent oracle cases; set 831e4780 | `393d1b1d0` |
| [wasm/scalar/checked-kernels](wasm.md) | O3 | runtime / elapsed_seconds | not-applicable | 0.380× | clang: 32/32 cppgm, 32/32 reference; gcc: not-applicable | 32 independent oracle cases; set 831e4780 | `393d1b1d0` |
| [wasm/scalar/instantiation-and-start](wasm.md) | O0 | startup / elapsed_seconds | not-applicable | 1.030× | clang: 32/32 cppgm, 32/32 reference; gcc: not-applicable | 32 independent oracle cases; set 831e4780 | `393d1b1d0` |
| [wasm/scalar/instantiation-and-start](wasm.md) | O3 | startup / elapsed_seconds | not-applicable | 0.944× | clang: 32/32 cppgm, 32/32 reference; gcc: not-applicable | 32 independent oracle cases; set 831e4780 | `393d1b1d0` |
| [wasm/scalar/module-size](wasm.md) | O0 | link / bytes | not-applicable | 0.634× | clang: 32/32 cppgm, 32/32 reference; gcc: not-applicable | 32 independent oracle cases; set 831e4780 | `393d1b1d0` |
| [wasm/scalar/module-size](wasm.md) | O3 | link / bytes | not-applicable | 0.981× | clang: 32/32 cppgm, 32/32 reference; gcc: not-applicable | 32 independent oracle cases; set 831e4780 | `393d1b1d0` |
| [wasm/scalar/one-translation-unit](wasm.md) | O0 | compile / cpu_seconds | not-applicable | 7.561× | clang: 32/32 cppgm, 32/32 reference; gcc: not-applicable | 32 independent oracle cases; set 831e4780 | `393d1b1d0` |
| [wasm/scalar/one-translation-unit](wasm.md) | O0 | compile / elapsed_seconds | not-applicable | 6.712× | clang: 32/32 cppgm, 32/32 reference; gcc: not-applicable | 32 independent oracle cases; set 831e4780 | `393d1b1d0` |
| [wasm/scalar/one-translation-unit](wasm.md) | O0 | link / cpu_seconds | not-applicable | 4.899× | clang: 32/32 cppgm, 32/32 reference; gcc: not-applicable | 32 independent oracle cases; set 831e4780 | `393d1b1d0` |
| [wasm/scalar/one-translation-unit](wasm.md) | O0 | link / elapsed_seconds | not-applicable | 2.977× | clang: 32/32 cppgm, 32/32 reference; gcc: not-applicable | 32 independent oracle cases; set 831e4780 | `393d1b1d0` |
| [wasm/scalar/one-translation-unit](wasm.md) | O3 | compile / cpu_seconds | not-applicable | 12.342× | clang: 32/32 cppgm, 32/32 reference; gcc: not-applicable | 32 independent oracle cases; set 831e4780 | `393d1b1d0` |
| [wasm/scalar/one-translation-unit](wasm.md) | O3 | compile / elapsed_seconds | not-applicable | 11.668× | clang: 32/32 cppgm, 32/32 reference; gcc: not-applicable | 32 independent oracle cases; set 831e4780 | `393d1b1d0` |
| [wasm/scalar/one-translation-unit](wasm.md) | O3 | link / cpu_seconds | not-applicable | 4.464× | clang: 32/32 cppgm, 32/32 reference; gcc: not-applicable | 32 independent oracle cases; set 831e4780 | `393d1b1d0` |
| [wasm/scalar/one-translation-unit](wasm.md) | O3 | link / elapsed_seconds | not-applicable | 4.024× | clang: 32/32 cppgm, 32/32 reference; gcc: not-applicable | 32 independent oracle cases; set 831e4780 | `393d1b1d0` |
| [wasm/simd128/checked-kernels](wasm.md) | O0 | runtime / elapsed_seconds | not-applicable | 0.960× | clang: 32/32 cppgm, 32/32 reference; gcc: not-applicable | 32 independent oracle cases; set 831e4780 | `393d1b1d0` |
| [wasm/simd128/checked-kernels](wasm.md) | O3 | runtime / elapsed_seconds | not-applicable | 0.419× | clang: 32/32 cppgm, 32/32 reference; gcc: not-applicable | 32 independent oracle cases; set 831e4780 | `393d1b1d0` |
| [wasm/simd128/instantiation-and-start](wasm.md) | O0 | startup / elapsed_seconds | not-applicable | 0.708× | clang: 32/32 cppgm, 32/32 reference; gcc: not-applicable | 32 independent oracle cases; set 831e4780 | `393d1b1d0` |
| [wasm/simd128/instantiation-and-start](wasm.md) | O3 | startup / elapsed_seconds | not-applicable | 0.961× | clang: 32/32 cppgm, 32/32 reference; gcc: not-applicable | 32 independent oracle cases; set 831e4780 | `393d1b1d0` |
| [wasm/simd128/module-size](wasm.md) | O0 | link / bytes | not-applicable | 0.485× | clang: 32/32 cppgm, 32/32 reference; gcc: not-applicable | 32 independent oracle cases; set 831e4780 | `393d1b1d0` |
| [wasm/simd128/module-size](wasm.md) | O3 | link / bytes | not-applicable | 0.737× | clang: 32/32 cppgm, 32/32 reference; gcc: not-applicable | 32 independent oracle cases; set 831e4780 | `393d1b1d0` |
| [wasm/simd128/one-translation-unit](wasm.md) | O0 | compile / cpu_seconds | not-applicable | 7.561× | clang: 32/32 cppgm, 32/32 reference; gcc: not-applicable | 32 independent oracle cases; set 831e4780 | `393d1b1d0` |
| [wasm/simd128/one-translation-unit](wasm.md) | O0 | compile / elapsed_seconds | not-applicable | 13.594× | clang: 32/32 cppgm, 32/32 reference; gcc: not-applicable | 32 independent oracle cases; set 831e4780 | `393d1b1d0` |
| [wasm/simd128/one-translation-unit](wasm.md) | O0 | link / cpu_seconds | not-applicable | 4.323× | clang: 32/32 cppgm, 32/32 reference; gcc: not-applicable | 32 independent oracle cases; set 831e4780 | `393d1b1d0` |
| [wasm/simd128/one-translation-unit](wasm.md) | O0 | link / elapsed_seconds | not-applicable | 0.437× | clang: 32/32 cppgm, 32/32 reference; gcc: not-applicable | 32 independent oracle cases; set 831e4780 | `393d1b1d0` |
| [wasm/simd128/one-translation-unit](wasm.md) | O3 | compile / cpu_seconds | not-applicable | 11.403× | clang: 32/32 cppgm, 32/32 reference; gcc: not-applicable | 32 independent oracle cases; set 831e4780 | `393d1b1d0` |
| [wasm/simd128/one-translation-unit](wasm.md) | O3 | compile / elapsed_seconds | not-applicable | 10.748× | clang: 32/32 cppgm, 32/32 reference; gcc: not-applicable | 32 independent oracle cases; set 831e4780 | `393d1b1d0` |
| [wasm/simd128/one-translation-unit](wasm.md) | O3 | link / cpu_seconds | not-applicable | 4.519× | clang: 32/32 cppgm, 32/32 reference; gcc: not-applicable | 32 independent oracle cases; set 831e4780 | `393d1b1d0` |
| [wasm/simd128/one-translation-unit](wasm.md) | O3 | link / elapsed_seconds | not-applicable | 4.062× | clang: 32/32 cppgm, 32/32 reference; gcc: not-applicable | 32 independent oracle cases; set 831e4780 | `393d1b1d0` |
