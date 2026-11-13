# Boost compilation and test execution

Boost 1.92.0, 17 C++11 suites (1,818 declarations). Ratios are reference / cppgm, higher is better.

Compilation counts child CPU of C++ test compilations, including expected rejections. It excludes configuration probes, library dependencies, C, linking and B2. Test execution includes process startup and the GDB integration test; it is correctness-test timing, not an application benchmark. The debugger source has identical volatile breakpoint anchors for all compilers; MPL and TypeTraits test enums use fixed underlying types for their -1 probes. Original test oracles are retained. Compile timings use one clean campaign per compiler; small differences need repeats.

| Suite | Declarations | Compile CPU cppgm | GCC | Clang | Test CPU cppgm | GCC | Clang |
|---|---:|---:|---:|---:|---:|---:|---:|
| [libs/accumulators/test](suites/libs-accumulators-test.md) | 49 | 50.524s | 3.220× | 3.382× | 2.837s | 0.996× | 0.864× |
| [libs/asio/test](suites/libs-asio-test.md) | 295 | 140.998s | 5.483× | 5.071× | 0.203s | 1.006× | 0.934× |
| [libs/container/test](suites/libs-container-test.md) | 89 | 125.169s | 5.316× | 3.822× | 2.094s | 0.904× | 0.964× |
| [libs/function/test](suites/libs-function-test.md) | 49 | 8.207s | 3.024× | 3.213× | 0.055s | 1.042× | 1.008× |
| [libs/fusion/test](suites/libs-fusion-test.md) | 221 | 60.715s | 2.798× | 3.174× | 0.267s | 1.064× | 0.973× |
| [libs/json/test](suites/libs-json-test.md) | 50 | 65.591s | 6.920× | 4.886× | 2.576s | 0.840× | 1.033× |
| [libs/mp11/test](suites/libs-mp11-test.md) | 234 | 42.539s | 5.595× | 3.536× | 0.270s | 1.032× | 1.069× |
| [libs/mpl/test](suites/libs-mpl-test.md) | 99 | 14.463s | 2.541× | 2.913× | 0.012s | 1.046× | 1.049× |
| [libs/optional/test](suites/libs-optional-test.md) | 74 | 9.550s | 2.997× | 2.996× | 0.053s | 1.035× | 0.932× |
| [libs/proto/test](suites/libs-proto-test.md) | 23 | 13.811s | 3.038× | 3.030× | 0.051s | 1.046× | 0.931× |
| [libs/qvm/test](suites/libs-qvm-test.md) | 134 | 34.831s | 2.962× | 3.302× | 0.259s | 0.848× | 0.787× |
| [libs/spirit/repository/test](suites/libs-spirit-repository-test.md) | 8 | 14.946s | 3.820× | 3.776× | 0.011s | 1.006× | 0.894× |
| [libs/type_erasure/test](suites/libs-type_erasure-test.md) | 60 | 34.205s | 5.104× | 4.258× | 0.097s | 0.907× | 0.868× |
| [libs/type_traits/test](suites/libs-type_traits-test.md) | 218 | 33.439s | 3.461× | 5.944× | 0.243s | 1.021× | 0.990× |
| [libs/uuid/test](suites/libs-uuid-test.md) | 80 | 16.237s | 3.835× | 3.480× | 0.140s | 0.979× | 0.929× |
| [libs/variant2/test](suites/libs-variant2-test.md) | 84 | 37.723s | 3.115× | 4.286× | 0.074s | 0.983× | 0.957× |
| [libs/xpressive/test](suites/libs-xpressive-test.md) | 51 | 72.254s | 4.807× | 3.972× | 0.128s | 1.031× | 0.909× |

Performance uses O3 with assertions enabled; the GDB test remains Og and is correctness-only.

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.935× | 0.935×, 0.936× | 0.996× |
| clang | 0.960× | 0.957×, 0.964× | 0.991× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43395345325).

Exact commands, per-test times, native outcomes and toolchain/host identities are retained with the results.

[Shared comparison](standard/summary.md) · [Version-1 records](standard/records.json) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/43395345325)
