# Boost compilation and test execution

Boost 1.92.0, 17 C++11 suites (1,818 declarations). Ratios are reference / cppgm, higher is better.

Compilation counts child CPU of C++ test compilations, including expected rejections. It excludes configuration probes, library dependencies, C, linking and B2. Test execution includes process startup and the GDB integration test; it is correctness-test timing, not an application benchmark. The debugger source has identical volatile breakpoint anchors for all compilers; MPL and TypeTraits test enums use fixed underlying types for their -1 probes. Original test oracles are retained. Compile timings use one clean campaign per compiler; small differences need repeats.

| Suite | Declarations | Compile CPU cppgm | GCC | Clang | Test CPU cppgm | GCC | Clang |
|---|---:|---:|---:|---:|---:|---:|---:|
| [libs/accumulators/test](suites/libs-accumulators-test.md) | 49 | 43.559s | 2.849× | 3.297× | 2.340s | 0.956× | 0.868× |
| [libs/asio/test](suites/libs-asio-test.md) | 295 | 117.884s | 5.347× | 5.174× | 0.240s | 1.059× | 1.082× |
| [libs/container/test](suites/libs-container-test.md) | 89 | 93.156s | 5.588× | 4.337× | 1.782s | 0.904× | 0.904× |
| [libs/function/test](suites/libs-function-test.md) | 49 | 6.960s | 2.909× | 3.101× | 0.067s | 1.063× | 1.047× |
| [libs/fusion/test](suites/libs-fusion-test.md) | 221 | 52.247s | 2.591× | 3.093× | 0.327s | 1.054× | 1.077× |
| [libs/json/test](suites/libs-json-test.md) | 50 | 53.275s | 6.853× | 5.191× | 1.887s | 0.811× | 0.962× |
| [libs/mp11/test](suites/libs-mp11-test.md) | 234 | 36.099s | 4.876× | 3.404× | 0.335s | 0.958× | 1.094× |
| [libs/mpl/test](suites/libs-mpl-test.md) | 99 | 12.661s | 2.207× | 2.680× | 0.015s | 0.929× | 1.091× |
| [libs/optional/test](suites/libs-optional-test.md) | 74 | 8.141s | 2.589× | 2.920× | 0.064s | 0.938× | 1.083× |
| [libs/proto/test](suites/libs-proto-test.md) | 23 | 12.004s | 2.610× | 2.881× | 0.063s | 0.975× | 1.036× |
| [libs/qvm/test](suites/libs-qvm-test.md) | 134 | 30.038s | 2.560× | 3.111× | 0.240s | 0.954× | 0.995× |
| [libs/spirit/repository/test](suites/libs-spirit-repository-test.md) | 8 | 12.869s | 3.506× | 3.763× | 0.013s | 1.041× | 1.033× |
| [libs/type_erasure/test](suites/libs-type_erasure-test.md) | 60 | 28.511s | 5.268× | 4.275× | 0.121s | 0.966× | 0.941× |
| [libs/type_traits/test](suites/libs-type_traits-test.md) | 218 | 27.710s | 3.309× | 5.369× | 0.290s | 1.043× | 1.055× |
| [libs/uuid/test](suites/libs-uuid-test.md) | 80 | 12.957s | 3.835× | 3.523× | 0.527s | 1.029× | 1.022× |
| [libs/variant2/test](suites/libs-variant2-test.md) | 84 | 31.096s | 3.116× | 4.263× | 0.083s | 1.116× | 1.078× |
| [libs/xpressive/test](suites/libs-xpressive-test.md) | 51 | 54.088s | 5.320× | 4.414× | 0.131s | 1.178× | 1.050× |

Performance uses O3 with assertions enabled; the GDB test remains Og and is correctness-only.

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.980×† | 0.945×, 1.015× | 1.016× |
| clang | 0.967× | 0.972×, 0.962× | 0.998× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43558051632).

Exact commands, per-test times, native outcomes and toolchain/host identities are retained with the results.

[Shared comparison](standard/summary.md) · [Version-1 records](standard/records.json) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/43558051632)
