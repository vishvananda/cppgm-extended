# Boost compilation and test execution

Boost 1.92.0, 17 C++11 suites (1,818 declarations). Ratios are reference / cppgm, higher is better.

Compilation counts child CPU of C++ test compilations, including expected rejections. It excludes configuration probes, library dependencies, C, linking and B2. Test execution includes process startup and the GDB integration test; it is correctness-test timing, not an application benchmark. The debugger source has identical volatile breakpoint anchors for all compilers; MPL and TypeTraits test enums use fixed underlying types for their -1 probes. Original test oracles are retained. Compile timings use one clean campaign per compiler; small differences need repeats.

| Suite | Declarations | Compile CPU cppgm | GCC | Clang | Test CPU cppgm | GCC | Clang |
|---|---:|---:|---:|---:|---:|---:|---:|
| [libs/accumulators/test](suites/libs-accumulators-test.md) | 49 | 51.206s | 3.083× | 3.630× | 2.837s | 1.024× | 0.869× |
| [libs/asio/test](suites/libs-asio-test.md) | 295 | 137.730s | 5.375× | 5.470× | 0.246s | 1.053× | 1.028× |
| [libs/container/test](suites/libs-container-test.md) | 89 | 109.202s | 5.621× | 4.375× | 2.280s | 0.936× | 0.888× |
| [libs/function/test](suites/libs-function-test.md) | 49 | 8.363s | 2.886× | 3.413× | 0.068s | 1.035× | 1.024× |
| [libs/fusion/test](suites/libs-fusion-test.md) | 221 | 62.753s | 2.634× | 3.344× | 0.330s | 1.065× | 1.014× |
| [libs/json/test](suites/libs-json-test.md) | 50 | 60.991s | 7.142× | 5.379× | 2.390s | 0.814× | 1.057× |
| [libs/mp11/test](suites/libs-mp11-test.md) | 234 | 41.653s | 5.404× | 3.854× | 0.329s | 1.071× | 1.117× |
| [libs/mpl/test](suites/libs-mpl-test.md) | 99 | 15.090s | 2.401× | 3.098× | 0.015s | 1.103× | 1.183× |
| [libs/optional/test](suites/libs-optional-test.md) | 74 | 9.569s | 2.921× | 3.361× | 0.064s | 1.022× | 1.030× |
| [libs/proto/test](suites/libs-proto-test.md) | 23 | 14.127s | 2.866× | 3.228× | 0.066s | 1.037× | 1.024× |
| [libs/qvm/test](suites/libs-qvm-test.md) | 134 | 35.650s | 2.810× | 3.636× | 0.303s | 0.857× | 0.818× |
| [libs/spirit/repository/test](suites/libs-spirit-repository-test.md) | 8 | 14.993s | 3.683× | 4.059× | 0.013s | 1.087× | 1.050× |
| [libs/type_erasure/test](suites/libs-type_erasure-test.md) | 60 | 33.829s | 5.156× | 4.586× | 0.127s | 0.976× | 0.981× |
| [libs/type_traits/test](suites/libs-type_traits-test.md) | 218 | 34.605s | 3.434× | 6.661× | 0.292s | 1.093× | 1.065× |
| [libs/uuid/test](suites/libs-uuid-test.md) | 80 | 16.127s | 3.733× | 3.847× | 0.147s | 1.066× | 1.035× |
| [libs/variant2/test](suites/libs-variant2-test.md) | 84 | 39.043s | 3.024× | 4.611× | 0.090s | 1.040× | 1.038× |
| [libs/xpressive/test](suites/libs-xpressive-test.md) | 51 | 70.115s | 4.671× | 4.245× | 0.151s | 1.120× | 1.000× |

Performance uses O3 with assertions enabled; the GDB test remains Og and is correctness-only.

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.973× | 0.974×, 0.971× | 0.998× |
| clang | 0.951× | 0.952×, 0.950× | 1.011× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1362 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43357224918).

Exact commands, per-test times, native outcomes and toolchain/host identities are retained with the results.

[Shared comparison](standard/summary.md) · [Version-1 records](standard/records.json) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/43357224918)
