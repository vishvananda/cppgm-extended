# Boost compilation and test execution

Boost 1.92.0, 17 C++11 suites (1,818 declarations). Ratios are reference / cppgm, higher is better.

Compilation counts child CPU of C++ test compilations, including expected rejections. It excludes configuration probes, library dependencies, C, linking and B2. Test execution includes process startup and the GDB integration test; it is correctness-test timing, not an application benchmark. The debugger source has identical volatile breakpoint anchors for all compilers; MPL and TypeTraits test enums use fixed underlying types for their -1 probes. Original test oracles are retained. Compile timings use one clean campaign per compiler; small differences need repeats.

| Suite | Declarations | Compile CPU cppgm | GCC | Clang | Test CPU cppgm | GCC | Clang |
|---|---:|---:|---:|---:|---:|---:|---:|
| [libs/accumulators/test](suites/libs-accumulators-test.md) | 49 | 41.952s | 3.265× | 3.508× | 2.411s | 1.033× | 0.887× |
| [libs/asio/test](suites/libs-asio-test.md) | 295 | 112.524s | 6.016× | 5.583× | 0.168s | 1.052× | 1.024× |
| [libs/container/test](suites/libs-container-test.md) | 89 | 106.114s | 5.375× | 3.980× | 1.929s | 1.017× | 1.040× |
| [libs/function/test](suites/libs-function-test.md) | 49 | 6.765s | 3.082× | 3.375× | 0.047s | 1.054× | 1.056× |
| [libs/fusion/test](suites/libs-fusion-test.md) | 221 | 51.248s | 2.760× | 3.264× | 0.242s | 1.017× | 1.014× |
| [libs/json/test](suites/libs-json-test.md) | 50 | 55.566s | 7.211× | 5.108× | 2.039s | 0.849× | 1.003× |
| [libs/mp11/test](suites/libs-mp11-test.md) | 234 | 34.095s | 5.950× | 3.825× | 0.223s | 1.127× | 1.197× |
| [libs/mpl/test](suites/libs-mpl-test.md) | 99 | 11.744s | 2.607× | 3.116× | 0.010s | 1.180× | 1.270× |
| [libs/optional/test](suites/libs-optional-test.md) | 74 | 7.686s | 3.107× | 3.302× | 0.043s | 1.106× | 1.192× |
| [libs/proto/test](suites/libs-proto-test.md) | 23 | 11.546s | 3.036× | 3.224× | 0.045s | 0.983× | 1.047× |
| [libs/qvm/test](suites/libs-qvm-test.md) | 134 | 27.772s | 3.075× | 3.680× | 0.180s | 1.044× | 1.137× |
| [libs/spirit/repository/test](suites/libs-spirit-repository-test.md) | 8 | 12.352s | 4.035× | 4.337× | 0.010s | 1.018× | 1.141× |
| [libs/type_erasure/test](suites/libs-type_erasure-test.md) | 60 | 27.554s | 5.668× | 4.961× | 0.077s | 1.012× | 1.039× |
| [libs/type_traits/test](suites/libs-type_traits-test.md) | 218 | 26.527s | 3.645× | 6.429× | 0.201s | 1.126× | 1.243× |
| [libs/uuid/test](suites/libs-uuid-test.md) | 80 | 13.380s | 3.957× | 3.686× | 0.124s | 1.060× | 1.075× |
| [libs/variant2/test](suites/libs-variant2-test.md) | 84 | 31.672s | 3.207× | 4.538× | 0.063s | 1.048× | 1.087× |
| [libs/xpressive/test](suites/libs-xpressive-test.md) | 51 | 57.833s | 5.152× | 4.447× | 0.107s | 1.069× | 1.013× |

Performance uses O3 with assertions enabled; the GDB test remains Og and is correctness-only.

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.982× | 0.981×, 0.983× | 1.000× |
| clang | 0.992× | 0.991×, 0.993× | 0.998× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1364 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43559814672).

Exact commands, per-test times, native outcomes and toolchain/host identities are retained with the results.

[Shared comparison](standard/summary.md) · [Version-1 records](standard/records.json) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/43559814672)
