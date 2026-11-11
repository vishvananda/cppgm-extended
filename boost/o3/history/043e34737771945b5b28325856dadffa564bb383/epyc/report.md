# Boost compilation and test execution

Boost 1.92.0, 17 C++11 suites (1,818 declarations). Ratios are reference / cppgm, higher is better.

Compilation counts child CPU of C++ test compilations, including expected rejections. It excludes configuration probes, library dependencies, C, linking and B2. Test execution includes process startup and the GDB integration test; it is correctness-test timing, not an application benchmark. The debugger source has identical volatile breakpoint anchors for all compilers; MPL and TypeTraits test enums use fixed underlying types for their -1 probes. Original test oracles are retained. Compile timings use one clean campaign per compiler; small differences need repeats.

| Suite | Declarations | Compile CPU cppgm | GCC | Clang | Test CPU cppgm | GCC | Clang |
|---|---:|---:|---:|---:|---:|---:|---:|
| [libs/accumulators/test](suites/libs-accumulators-test.md) | 49 | 49.731s | 2.850× | 2.760× | 2.197s | 1.042× | 0.928× |
| [libs/asio/test](suites/libs-asio-test.md) | 295 | 133.293s | 4.874× | 4.603× | 0.272s | 0.977× | 0.954× |
| [libs/container/test](suites/libs-container-test.md) | 89 | 93.816s | 5.561× | 4.496× | 1.607s | 0.973× | 1.007× |
| [libs/function/test](suites/libs-function-test.md) | 49 | 8.107s | 2.431× | 2.753× | 0.079s | 0.901× | 0.969× |
| [libs/fusion/test](suites/libs-fusion-test.md) | 221 | 59.577s | 2.257× | 2.791× | 0.345s | 1.025× | 1.061× |
| [libs/json/test](suites/libs-json-test.md) | 50 | 55.911s | 6.863× | 5.040× | 1.971s | 0.742× | 0.925× |
| [libs/mp11/test](suites/libs-mp11-test.md) | 234 | 39.101s | 4.812× | 3.204× | 0.346s | 1.020× | 1.128× |
| [libs/mpl/test](suites/libs-mpl-test.md) | 99 | 14.892s | 2.086× | 2.394× | 0.016s | 1.023× | 1.120× |
| [libs/optional/test](suites/libs-optional-test.md) | 74 | 9.700s | 2.451× | 2.592× | 0.072s | 0.976× | 1.032× |
| [libs/proto/test](suites/libs-proto-test.md) | 23 | 13.575s | 2.568× | 2.650× | 0.065s | 1.044× | 1.061× |
| [libs/qvm/test](suites/libs-qvm-test.md) | 134 | 33.493s | 2.484× | 2.972× | 0.268s | 0.949× | 0.987× |
| [libs/spirit/repository/test](suites/libs-spirit-repository-test.md) | 8 | 14.067s | 3.563× | 3.610× | 0.019s | 0.813× | 0.800× |
| [libs/type_erasure/test](suites/libs-type_erasure-test.md) | 60 | 33.530s | 4.842× | 3.577× | 0.128s | 0.967× | 0.905× |
| [libs/type_traits/test](suites/libs-type_traits-test.md) | 218 | 31.714s | 3.136× | 4.707× | 0.313s | 1.018× | 1.006× |
| [libs/uuid/test](suites/libs-uuid-test.md) | 80 | 15.102s | 3.398× | 2.891× | 0.535s | 1.023× | 0.999× |
| [libs/variant2/test](suites/libs-variant2-test.md) | 84 | 34.721s | 2.774× | 3.628× | 0.093s | 1.041× | 0.951× |
| [libs/xpressive/test](suites/libs-xpressive-test.md) | 51 | 69.988s | 4.001× | 3.369× | 0.155s | 1.010× | 0.883× |

Performance uses O3 with assertions enabled; the GDB test remains Og and is correctness-only.

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.991× | 0.998×, 0.984× | 0.989× |
| clang | 0.972× | 0.954×, 0.990× | 1.016× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1362 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045).

Exact commands, per-test times, native outcomes and toolchain/host identities are retained with the results.

[Shared comparison](standard/summary.md) · [Version-1 records](standard/records.json) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045)
