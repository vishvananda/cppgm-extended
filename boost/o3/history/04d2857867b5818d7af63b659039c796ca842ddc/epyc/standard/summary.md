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
| [Boost compilation and test execution / 17 C++11 suites at O3 with assertions; GDB correctness-only](boost.md) | declared: O3, mixed; recorded: O3, mixed | recorded | benchmarks/boost/README.md |
| Boost compilation and test execution / Historical 17-suite C++11 debug campaigns | declared: O0, mixed; recorded: none | not measured | benchmarks/boost/README.md |
| Boost compilation and test execution / Historical correctness snapshots | declared: mixed; recorded: none | not measured | benchmarks/boost/README.md |
| Public benchmark suite / Declared public programs | declared: O0, O3; recorded: none | not measured | benchmarks/public/README.md |
| Build the compiler / Course build | declared: mixed; recorded: none | not measured | benchmarks/self_compile/README.md |
| cppgm code generation / Self-built versus host-built compiler | declared: mixed; recorded: none | not measured | benchmarks/self_compile/README.md |
| index-cpp / All 50 checksummed workloads | declared: O0, O2, O3; recorded: none | not measured | benchmarks/index_cpp/README.md |
| simdjson / Scalar fallback | declared: O3; recorded: none | not measured | benchmarks/simdjson/README.md |
| simdjson / SSE backend | declared: O3; recorded: none | not measured | benchmarks/simdjson/README.md |
| Godot build and benchmark / Full headless release and five checked GDScript workloads | declared: O3; recorded: none | not measured | benchmarks/godot/README.md |

## Correctness coverage

| Workload / configuration | Oracle | CPU / environment | Opt | Passed / executed / selected | Excluded | Status | Revision |
|---|---|---|---|---|---|---|---|
| [boost/o3](boost.md) | Original B2 compile/link/run and expected-failure actions, including GDB; all three compilers | AMD EPYC 9V45 96-Core Processor / GitHub hosted; Ubuntu 26.04 container | mixed | 1818 / 1818 / 1818 Boost test declarations per compiler | 0 | passed | 04d285786 |

## AMD EPYC 9V45 96-Core Processor — GitHub hosted; Ubuntu 26.04 container

Architecture: x86_64; host profile `boost-b9d57f72a2f9ce76`.

| Workload / configuration / test | Opt | Phase / metric | vs GCC | vs Clang | Validation | Case set | Revision |
|---|---|---|---:|---:|---|---|---|
| [boost/o3/all-suites](boost.md) | O3 | compile / cpu_seconds | 4.571× | 3.858× | clang: 1817/1817 cppgm, 1817/1817 reference; gcc: 1817/1817 cppgm, 1817/1817 reference | 1817 test declarations; set 56256889 | `04d285786` |
| [boost/o3/all-suites](boost.md) | O3 | runtime / cpu_seconds | 0.956× | 0.953× | clang: 1375/1375 cppgm, 1375/1375 reference; gcc: 1375/1375 cppgm, 1375/1375 reference | 1375 native test programs; set f805acfd | `04d285786` |
| [boost/o3/all-suites](boost.md) | mixed | test-execution / cpu_seconds | 0.964× | 0.954× | clang: 1378/1378 cppgm, 1378/1378 reference; gcc: 1378/1378 cppgm, 1378/1378 reference | 1378 native test actions; set 41c66211 | `04d285786` |
