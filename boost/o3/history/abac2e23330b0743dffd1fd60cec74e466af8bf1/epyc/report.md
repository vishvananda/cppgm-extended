# Boost compilation and test execution

Boost 1.92.0, 17 C++11 suites (1,818 declarations). Ratios are reference / cppgm, higher is better.

Compilation counts child CPU of C++ test compilations, including expected rejections. It excludes configuration probes, library dependencies, C, linking and B2. Test execution includes process startup and the GDB integration test; it is correctness-test timing, not an application benchmark. The debugger source has identical volatile breakpoint anchors for all compilers; MPL and TypeTraits test enums use fixed underlying types for their -1 probes. Original test oracles are retained. Compile timings use one clean campaign per compiler; small differences need repeats.

| Suite | Declarations | Compile CPU cppgm | GCC | Clang | Test CPU cppgm | GCC | Clang |
|---|---:|---:|---:|---:|---:|---:|---:|
| [libs/accumulators/test](suites/libs-accumulators-test.md) | 49 | 78.033s | 2.888× | 3.725× | 3.461s | 1.031× | 0.902× |
| [libs/asio/test](suites/libs-asio-test.md) | 295 | 211.621s | 4.960× | 5.576× | 0.307s | 1.064× | 1.060× |
| [libs/container/test](suites/libs-container-test.md) | 89 | 143.953s | 6.001× | 4.964× | 2.593s | 0.967× | 1.007× |
| [libs/function/test](suites/libs-function-test.md) | 49 | 13.249s | 2.628× | 3.438× | 0.086s | 1.052× | 1.037× |
| [libs/fusion/test](suites/libs-fusion-test.md) | 221 | 100.808s | 2.357× | 3.230× | 0.414s | 1.069× | 1.040× |
| [libs/json/test](suites/libs-json-test.md) | 50 | 88.010s | 6.908× | 5.704× | 3.637s | 0.801× | 1.032× |
| [libs/mp11/test](suites/libs-mp11-test.md) | 234 | 63.937s | 5.016× | 3.918× | 0.420s | 1.061× | 1.122× |
| [libs/mpl/test](suites/libs-mpl-test.md) | 99 | 24.716s | 2.110× | 2.962× | 0.020s | 1.053× | 1.175× |
| [libs/optional/test](suites/libs-optional-test.md) | 74 | 15.495s | 2.614× | 3.291× | 0.081s | 1.057× | 1.077× |
| [libs/proto/test](suites/libs-proto-test.md) | 23 | 22.399s | 2.610× | 3.144× | 0.086s | 0.980× | 1.012× |
| [libs/qvm/test](suites/libs-qvm-test.md) | 134 | 55.728s | 2.626× | 3.736× | 0.368s | 0.898× | 0.873× |
| [libs/spirit/repository/test](suites/libs-spirit-repository-test.md) | 8 | 21.883s | 3.505× | 4.056× | 0.017s | 1.029× | 0.992× |
| [libs/type_erasure/test](suites/libs-type_erasure-test.md) | 60 | 52.849s | 4.735× | 4.476× | 0.161s | 0.969× | 0.957× |
| [libs/type_traits/test](suites/libs-type_traits-test.md) | 218 | 55.510s | 3.185× | 6.493× | 0.371s | 1.047× | 1.083× |
| [libs/uuid/test](suites/libs-uuid-test.md) | 80 | 25.840s | 3.405× | 3.818× | 0.193s | 1.040× | 1.023× |
| [libs/variant2/test](suites/libs-variant2-test.md) | 84 | 56.492s | 2.998× | 4.890× | 0.113s | 1.091× | 1.035× |
| [libs/xpressive/test](suites/libs-xpressive-test.md) | 51 | 104.228s | 4.340× | 4.324× | 0.194s | 1.128× | 0.999× |

Performance uses O3 with assertions enabled; the GDB test remains Og and is correctness-only.

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.963× | 0.964×, 0.961× | 1.001× |
| clang | 0.953× | 0.953×, 0.953× | 0.993× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255).

Exact commands, per-test times, native outcomes and toolchain/host identities are retained with the results.

[Shared comparison](standard/summary.md) · [Version-1 records](standard/records.json) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255)
