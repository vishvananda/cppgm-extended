# Boost compilation and test execution

Boost 1.92.0, 17 C++11 suites (1,818 declarations). Ratios are reference / cppgm, higher is better.

Compilation counts child CPU of C++ test compilations, including expected rejections. It excludes configuration probes, library dependencies, C, linking and B2. Test execution includes process startup and the GDB integration test; it is correctness-test timing, not an application benchmark. The debugger source has identical volatile breakpoint anchors for all compilers; MPL and TypeTraits test enums use fixed underlying types for their -1 probes. Original test oracles are retained. Compile timings use one clean campaign per compiler; small differences need repeats.

| Suite | Declarations | Compile CPU cppgm | GCC | Clang | Test CPU cppgm | GCC | Clang |
|---|---:|---:|---:|---:|---:|---:|---:|
| [libs/accumulators/test](suites/libs-accumulators-test.md) | 49 | 68.885s | 3.126× | 4.021× | 3.565s | 0.990× | 0.865× |
| [libs/asio/test](suites/libs-asio-test.md) | 295 | 184.916s | 5.349× | 6.122× | 0.277s | 1.092× | 1.101× |
| [libs/container/test](suites/libs-container-test.md) | 89 | 144.987s | 5.691× | 4.677× | 2.695s | 0.921× | 0.957× |
| [libs/function/test](suites/libs-function-test.md) | 49 | 11.383s | 2.965× | 3.870× | 0.080s | 1.057× | 1.031× |
| [libs/fusion/test](suites/libs-fusion-test.md) | 221 | 85.620s | 2.671× | 3.722× | 0.385s | 1.075× | 1.054× |
| [libs/json/test](suites/libs-json-test.md) | 50 | 81.934s | 6.869× | 5.810× | 3.582s | 0.732× | 1.069× |
| [libs/mp11/test](suites/libs-mp11-test.md) | 234 | 56.979s | 5.455× | 4.345× | 0.383s | 1.085× | 1.191× |
| [libs/mpl/test](suites/libs-mpl-test.md) | 99 | 20.686s | 2.443× | 3.456× | 0.019s | 1.048× | 1.062× |
| [libs/optional/test](suites/libs-optional-test.md) | 74 | 13.134s | 2.983× | 3.778× | 0.076s | 1.059× | 1.032× |
| [libs/proto/test](suites/libs-proto-test.md) | 23 | 18.993s | 2.979× | 3.635× | 0.080s | 1.060× | 1.059× |
| [libs/qvm/test](suites/libs-qvm-test.md) | 134 | 48.342s | 2.953× | 4.298× | 0.308s | 1.035× | 1.061× |
| [libs/spirit/repository/test](suites/libs-spirit-repository-test.md) | 8 | 19.784s | 3.690× | 4.614× | 0.016s | 1.017× | 1.062× |
| [libs/type_erasure/test](suites/libs-type_erasure-test.md) | 60 | 44.843s | 5.464× | 5.184× | 0.156s | 0.960× | 1.034× |
| [libs/type_traits/test](suites/libs-type_traits-test.md) | 218 | 48.896s | 3.594× | 7.259× | 0.340s | 1.134× | 1.104× |
| [libs/uuid/test](suites/libs-uuid-test.md) | 80 | 22.368s | 3.739× | 4.383× | 0.177s | 1.054× | 1.071× |
| [libs/variant2/test](suites/libs-variant2-test.md) | 84 | 52.719s | 3.057× | 5.182× | 0.106s | 1.048× | 1.085× |
| [libs/xpressive/test](suites/libs-xpressive-test.md) | 51 | 90.413s | 4.962× | 4.742× | 0.185s | 1.164× | 0.997× |

Performance uses O3 with assertions enabled; the GDB test remains Og and is correctness-only.

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.957× | 0.954×, 0.961× | 1.004× |
| clang | 0.951× | 0.952×, 0.949× | 0.995× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43609274616).

Exact commands, per-test times, native outcomes and toolchain/host identities are retained with the results.

[Shared comparison](standard/summary.md) · [Version-1 records](standard/records.json) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/43609274616)
