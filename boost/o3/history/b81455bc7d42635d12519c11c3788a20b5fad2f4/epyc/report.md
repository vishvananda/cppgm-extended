# Boost compilation and test execution

Boost 1.92.0, 17 C++11 suites (1,818 declarations). Ratios are reference / cppgm, higher is better.

Compilation counts child CPU of C++ test compilations, including expected rejections. It excludes configuration probes, library dependencies, C, linking and B2. Test execution includes process startup and the GDB integration test; it is correctness-test timing, not an application benchmark. The debugger source has identical volatile breakpoint anchors for all compilers; MPL and TypeTraits test enums use fixed underlying types for their -1 probes. Original test oracles are retained. Compile timings use one clean campaign per compiler; small differences need repeats.

| Suite | Declarations | Compile CPU cppgm | GCC | Clang | Test CPU cppgm | GCC | Clang |
|---|---:|---:|---:|---:|---:|---:|---:|
| [libs/accumulators/test](suites/libs-accumulators-test.md) | 49 | 37.521s | 3.098× | 3.284× | 2.072s | 1.013× | 0.916× |
| [libs/asio/test](suites/libs-asio-test.md) | 295 | 97.205s | 6.533× | 5.411× | 0.198s | 1.169× | 1.078× |
| [libs/container/test](suites/libs-container-test.md) | 89 | 83.671s | 6.866× | 4.205× | 1.537s | 1.166× | 0.937× |
| [libs/function/test](suites/libs-function-test.md) | 49 | 6.097s | 3.286× | 3.034× | 0.056s | 1.216× | 1.048× |
| [libs/fusion/test](suites/libs-fusion-test.md) | 221 | 45.230s | 3.003× | 3.018× | 0.285s | 1.175× | 0.995× |
| [libs/json/test](suites/libs-json-test.md) | 50 | 46.128s | 7.492× | 4.966× | 1.676s | 0.948× | 1.074× |
| [libs/mp11/test](suites/libs-mp11-test.md) | 234 | 30.873s | 5.437× | 3.489× | 0.282s | 1.085× | 1.092× |
| [libs/mpl/test](suites/libs-mpl-test.md) | 99 | 10.822s | 2.473× | 2.784× | 0.012s | 1.088× | 1.159× |
| [libs/optional/test](suites/libs-optional-test.md) | 74 | 6.975s | 2.908× | 2.982× | 0.055s | 1.040× | 1.021× |
| [libs/proto/test](suites/libs-proto-test.md) | 23 | 10.361s | 2.855× | 2.871× | 0.055s | 1.048× | 0.988× |
| [libs/qvm/test](suites/libs-qvm-test.md) | 134 | 26.097s | 2.773× | 3.128× | 0.236s | 0.916× | 0.869× |
| [libs/spirit/repository/test](suites/libs-spirit-repository-test.md) | 8 | 10.987s | 3.773× | 3.830× | 0.012s | 1.034× | 1.003× |
| [libs/type_erasure/test](suites/libs-type_erasure-test.md) | 60 | 24.526s | 5.317× | 4.290× | 0.107s | 0.988× | 0.964× |
| [libs/type_traits/test](suites/libs-type_traits-test.md) | 218 | 24.650s | 3.362× | 5.451× | 0.253s | 1.077× | 1.011× |
| [libs/uuid/test](suites/libs-uuid-test.md) | 80 | 11.791s | 3.844× | 3.398× | 0.514s | 1.030× | 1.007× |
| [libs/variant2/test](suites/libs-variant2-test.md) | 84 | 28.861s | 2.985× | 4.007× | 0.078s | 1.065× | 1.002× |
| [libs/xpressive/test](suites/libs-xpressive-test.md) | 51 | 51.460s | 4.926× | 4.084× | 0.124s | 1.120× | 0.988× |

Performance uses O3 with assertions enabled; the GDB test remains Og and is correctness-only.

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.979× | 0.983×, 0.975× | 1.002× |
| clang | 0.967× | 0.970×, 0.965× | 1.011× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1364 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43493409407).

Exact commands, per-test times, native outcomes and toolchain/host identities are retained with the results.

[Shared comparison](standard/summary.md) · [Version-1 records](standard/records.json) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/43493409407)
