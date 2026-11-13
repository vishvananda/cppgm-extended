# Boost compilation and test execution

Boost 1.92.0, 17 C++11 suites (1,818 declarations). Ratios are reference / cppgm, higher is better.

Compilation counts child CPU of C++ test compilations, including expected rejections. It excludes configuration probes, library dependencies, C, linking and B2. Test execution includes process startup and the GDB integration test; it is correctness-test timing, not an application benchmark. The debugger source has identical volatile breakpoint anchors for all compilers; MPL and TypeTraits test enums use fixed underlying types for their -1 probes. Original test oracles are retained. Compile timings use one clean campaign per compiler; small differences need repeats.

| Suite | Declarations | Compile CPU cppgm | GCC | Clang | Test CPU cppgm | GCC | Clang |
|---|---:|---:|---:|---:|---:|---:|---:|
| [libs/accumulators/test](suites/libs-accumulators-test.md) | 49 | 53.919s | 2.990× | 3.568× | 2.789s | 1.070× | 0.888× |
| [libs/asio/test](suites/libs-asio-test.md) | 295 | 145.389s | 5.196× | 5.268× | 0.255s | 1.035× | 1.026× |
| [libs/container/test](suites/libs-container-test.md) | 89 | 114.419s | 5.687× | 4.240× | 2.071s | 0.947× | 0.977× |
| [libs/function/test](suites/libs-function-test.md) | 49 | 8.769s | 2.849× | 3.320× | 0.073s | 1.034× | 1.019× |
| [libs/fusion/test](suites/libs-fusion-test.md) | 221 | 65.509s | 2.623× | 3.263× | 0.348s | 1.048× | 1.012× |
| [libs/json/test](suites/libs-json-test.md) | 50 | 63.644s | 7.210× | 5.464× | 2.426s | 0.778× | 0.939× |
| [libs/mp11/test](suites/libs-mp11-test.md) | 234 | 42.747s | 5.323× | 3.861× | 0.341s | 1.056× | 1.154× |
| [libs/mpl/test](suites/libs-mpl-test.md) | 99 | 15.618s | 2.361× | 3.082× | 0.015s | 1.073× | 1.204× |
| [libs/optional/test](suites/libs-optional-test.md) | 74 | 9.791s | 2.885× | 3.434× | 0.066s | 1.016× | 1.056× |
| [libs/proto/test](suites/libs-proto-test.md) | 23 | 14.580s | 2.885× | 3.188× | 0.067s | 1.046× | 1.042× |
| [libs/qvm/test](suites/libs-qvm-test.md) | 134 | 37.813s | 2.737× | 3.523× | 0.309s | 0.877× | 0.843× |
| [libs/spirit/repository/test](suites/libs-spirit-repository-test.md) | 8 | 15.293s | 3.695× | 4.187× | 0.014s | 1.175× | 0.989× |
| [libs/type_erasure/test](suites/libs-type_erasure-test.md) | 60 | 35.103s | 5.181× | 4.627× | 0.131s | 0.973× | 0.960× |
| [libs/type_traits/test](suites/libs-type_traits-test.md) | 218 | 35.328s | 3.446× | 6.595× | 0.302s | 1.082× | 1.092× |
| [libs/uuid/test](suites/libs-uuid-test.md) | 80 | 16.267s | 3.877× | 3.908× | 0.146s | 1.119× | 1.058× |
| [libs/variant2/test](suites/libs-variant2-test.md) | 84 | 39.287s | 3.123× | 4.639× | 0.091s | 1.087× | 1.008× |
| [libs/xpressive/test](suites/libs-xpressive-test.md) | 51 | 68.438s | 4.943× | 4.382× | 0.150s | 1.133× | 0.997× |

Performance uses O3 with assertions enabled; the GDB test remains Og and is correctness-only.

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.971× | 0.970×, 0.972× | 1.002× |
| clang | 0.954× | 0.960×, 0.947× | 1.002× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1363 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43479737952).

Exact commands, per-test times, native outcomes and toolchain/host identities are retained with the results.

[Shared comparison](standard/summary.md) · [Version-1 records](standard/records.json) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/43479737952)
