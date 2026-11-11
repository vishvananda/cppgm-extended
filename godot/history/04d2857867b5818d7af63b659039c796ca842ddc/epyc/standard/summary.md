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
| [Godot build and benchmark / Full headless release and five checked GDScript workloads](godot.md) | declared: O3; recorded: O3 | recorded | benchmarks/godot/README.md |

## Correctness coverage

| Workload / configuration | Oracle | CPU / environment | Opt | Passed / executed / selected | Excluded | Status | Revision |
|---|---|---|---|---|---|---|---|
| [godot/headless-release](godot.md) | Independent Python checksums for cppgm, GCC and Clang; full builds succeed | AMD EPYC 9V45 96-Core Processor / GitHub hosted; Ubuntu 26.04 container | O3 | 5 / 5 / 5 checked workloads per compiler | 0 | passed | 04d285786 |

## AMD EPYC 9V45 96-Core Processor — GitHub hosted; Ubuntu 26.04 container

Architecture: x86_64; host profile `godot-b9d57f72a2f9ce76`.

| Workload / configuration / test | Opt | Phase / metric | vs GCC | vs Clang | Validation | Case set | Revision |
|---|---|---|---:|---:|---|---|---|
| [godot/headless-release/checked-workloads](godot.md) | O3 | runtime / cpu_seconds | 0.879× | 0.838× | clang: 5/5 cppgm, 5/5 reference; gcc: 5/5 cppgm, 5/5 reference | 5 checked workloads; set 98039760 | `04d285786` |
| [godot/headless-release/checked-workloads](godot.md) | O3 | runtime / elapsed_seconds | 0.862× | 0.824× | clang: 5/5 cppgm, 5/5 reference; gcc: 5/5 cppgm, 5/5 reference | 5 checked workloads; set 98039760 | `04d285786` |
| [godot/headless-release/full-build](godot.md) | O3 | build / cpu_seconds | 3.912× | 3.199× | clang: 1/1 cppgm, 1/1 reference; gcc: 1/1 cppgm, 1/1 reference | 1 full builds; set 68c69d14 | `04d285786` |
| [godot/headless-release/full-build](godot.md) | O3 | build / elapsed_seconds | 3.828× | 3.170× | clang: 1/1 cppgm, 1/1 reference; gcc: 1/1 cppgm, 1/1 reference | 1 full builds; set 68c69d14 | `04d285786` |
