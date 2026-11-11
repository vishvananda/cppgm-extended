# Boost compilation and test execution

Boost 1.92.0, 17 C++11 suites (1,818 declarations). Ratios are reference / cppgm, higher is better.

Compilation counts child CPU of C++ test compilations, including expected rejections. It excludes configuration probes, library dependencies, C, linking and B2. Test execution includes process startup and the GDB integration test; it is correctness-test timing, not an application benchmark. The debugger source has identical volatile breakpoint anchors for all compilers; MPL and TypeTraits test enums use fixed underlying types for their -1 probes. Original test oracles are retained. Compile timings use one clean campaign per compiler; small differences need repeats.

| Suite | Declarations | Compile CPU cppgm | GCC | Clang | Test CPU cppgm | GCC | Clang |
|---|---:|---:|---:|---:|---:|---:|---:|
| [libs/accumulators/test](suites/libs-accumulators-test.md) | 49 | 57.048s | 2.796× | 3.302× | 2.748s | 1.077× | 0.897× |
| [libs/asio/test](suites/libs-asio-test.md) | 295 | 151.632s | 4.973× | 5.067× | 0.255s | 1.058× | 1.051× |
| [libs/container/test](suites/libs-container-test.md) | 89 | 100.879s | 6.181× | 4.792× | 2.347s | 0.825× | 0.868× |
| [libs/function/test](suites/libs-function-test.md) | 49 | 9.479s | 2.576× | 3.077× | 0.072s | 1.039× | 1.029× |
| [libs/fusion/test](suites/libs-fusion-test.md) | 221 | 72.099s | 2.316× | 2.944× | 0.348s | 1.053× | 1.021× |
| [libs/json/test](suites/libs-json-test.md) | 50 | 62.036s | 7.187× | 5.402× | 2.630s | 0.777× | 0.923× |
| [libs/mp11/test](suites/libs-mp11-test.md) | 234 | 45.974s | 4.943× | 3.538× | 0.344s | 1.069× | 1.122× |
| [libs/mpl/test](suites/libs-mpl-test.md) | 99 | 17.801s | 2.055× | 2.645× | 0.016s | 1.054× | 1.150× |
| [libs/optional/test](suites/libs-optional-test.md) | 74 | 11.076s | 2.550× | 2.953× | 0.067s | 1.028× | 1.038× |
| [libs/proto/test](suites/libs-proto-test.md) | 23 | 16.239s | 2.515× | 2.825× | 0.069s | 1.013× | 0.971× |
| [libs/qvm/test](suites/libs-qvm-test.md) | 134 | 40.012s | 2.531× | 3.287× | 0.292s | 0.912× | 0.884× |
| [libs/spirit/repository/test](suites/libs-spirit-repository-test.md) | 8 | 15.980s | 3.476× | 3.871× | 0.014s | 1.045× | 1.040× |
| [libs/type_erasure/test](suites/libs-type_erasure-test.md) | 60 | 38.748s | 4.653× | 4.098× | 0.132s | 0.973× | 0.943× |
| [libs/type_traits/test](suites/libs-type_traits-test.md) | 218 | 38.910s | 3.087× | 5.970× | 0.311s | 1.059× | 1.064× |
| [libs/uuid/test](suites/libs-uuid-test.md) | 80 | 18.416s | 3.310× | 3.423× | 0.152s | 1.072× | 1.018× |
| [libs/variant2/test](suites/libs-variant2-test.md) | 84 | 41.237s | 2.895× | 4.409× | 0.094s | 1.053× | 1.030× |
| [libs/xpressive/test](suites/libs-xpressive-test.md) | 51 | 75.355s | 4.400× | 4.003× | 0.153s | 1.116× | 0.987× |

Performance uses O3 with assertions enabled; the GDB test remains Og and is correctness-only.

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.966× | 0.971×, 0.961× | 1.006× |
| clang | 0.941× | 0.944×, 0.939× | 0.997× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1362 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104).

Exact commands, per-test times, native outcomes and toolchain/host identities are retained with the results.

[Shared comparison](standard/summary.md) · [Version-1 records](standard/records.json) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104)
