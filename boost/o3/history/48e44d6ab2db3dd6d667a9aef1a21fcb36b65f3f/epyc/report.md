# Boost compilation and test execution

Boost 1.92.0, 17 C++11 suites (1,818 declarations). Ratios are reference / cppgm, higher is better.

Compilation counts child CPU of C++ test compilations, including expected rejections. It excludes configuration probes, library dependencies, C, linking and B2. Test execution includes process startup and the GDB integration test; it is correctness-test timing, not an application benchmark. The debugger source has identical volatile breakpoint anchors for all compilers; MPL and TypeTraits test enums use fixed underlying types for their -1 probes. Original test oracles are retained. Compile timings use one clean campaign per compiler; small differences need repeats.

| Suite | Declarations | Compile CPU cppgm | GCC | Clang | Test CPU cppgm | GCC | Clang |
|---|---:|---:|---:|---:|---:|---:|---:|
| [libs/accumulators/test](suites/libs-accumulators-test.md) | 49 | 74.860s | 2.918× | 3.716× | 3.349s | 1.048× | 0.922× |
| [libs/asio/test](suites/libs-asio-test.md) | 295 | 199.745s | 5.025× | 5.656× | 0.286s | 1.071× | 1.058× |
| [libs/container/test](suites/libs-container-test.md) | 89 | 137.915s | 6.134× | 5.052× | 2.651s | 0.944× | 0.981× |
| [libs/function/test](suites/libs-function-test.md) | 49 | 12.641s | 2.646× | 3.511× | 0.081s | 1.070× | 1.086× |
| [libs/fusion/test](suites/libs-fusion-test.md) | 221 | 95.847s | 2.397× | 3.322× | 0.396s | 1.057× | 1.049× |
| [libs/json/test](suites/libs-json-test.md) | 50 | 83.968s | 6.773× | 5.692× | 3.529s | 0.837× | 1.077× |
| [libs/mp11/test](suites/libs-mp11-test.md) | 234 | 61.137s | 5.193× | 4.018× | 0.393s | 1.111× | 1.142× |
| [libs/mpl/test](suites/libs-mpl-test.md) | 99 | 23.917s | 2.108× | 3.079× | 0.018s | 1.091× | 1.223× |
| [libs/optional/test](suites/libs-optional-test.md) | 74 | 15.097s | 2.596× | 3.396× | 0.079s | 1.030× | 1.082× |
| [libs/proto/test](suites/libs-proto-test.md) | 23 | 21.748s | 2.595× | 3.235× | 0.080s | 1.024× | 1.074× |
| [libs/qvm/test](suites/libs-qvm-test.md) | 134 | 54.609s | 2.589× | 3.768× | 0.368s | 0.873× | 0.869× |
| [libs/spirit/repository/test](suites/libs-spirit-repository-test.md) | 8 | 21.551s | 3.426× | 3.988× | 0.017s | 0.966× | 0.896× |
| [libs/type_erasure/test](suites/libs-type_erasure-test.md) | 60 | 51.839s | 4.462× | 4.320× | 0.162s | 0.933× | 0.982× |
| [libs/type_traits/test](suites/libs-type_traits-test.md) | 218 | 55.164s | 3.121× | 6.440× | 0.372s | 1.022× | 1.033× |
| [libs/uuid/test](suites/libs-uuid-test.md) | 80 | 24.856s | 3.407× | 3.887× | 0.182s | 1.043× | 1.039× |
| [libs/variant2/test](suites/libs-variant2-test.md) | 84 | 55.061s | 2.955× | 4.859× | 0.109s | 1.072× | 1.012× |
| [libs/xpressive/test](suites/libs-xpressive-test.md) | 51 | 99.762s | 4.518× | 4.374× | 0.193s | 1.137× | 0.984× |

Performance uses O3 with assertions enabled; the GDB test remains Og and is correctness-only.

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.948× | 0.942×, 0.954× | 0.998× |
| clang | 0.945× | 0.946×, 0.944× | 0.995× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761).

Exact commands, per-test times, native outcomes and toolchain/host identities are retained with the results.

[Shared comparison](standard/summary.md) · [Version-1 records](standard/records.json) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761)
