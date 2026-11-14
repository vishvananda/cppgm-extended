# Boost compilation and test execution

Boost 1.92.0, 17 C++11 suites (1,818 declarations). Ratios are reference / cppgm, higher is better.

Compilation counts child CPU of C++ test compilations, including expected rejections. It excludes configuration probes, library dependencies, C, linking and B2. Test execution includes process startup and the GDB integration test; it is correctness-test timing, not an application benchmark. The debugger source has identical volatile breakpoint anchors for all compilers; MPL and TypeTraits test enums use fixed underlying types for their -1 probes. Original test oracles are retained. Compile timings use one clean campaign per compiler; small differences need repeats.

| Suite | Declarations | Compile CPU cppgm | GCC | Clang | Test CPU cppgm | GCC | Clang |
|---|---:|---:|---:|---:|---:|---:|---:|
| [libs/accumulators/test](suites/libs-accumulators-test.md) | 49 | 43.855s | 2.888× | 2.800× | 2.183s | 0.995× | 0.858× |
| [libs/asio/test](suites/libs-asio-test.md) | 295 | 121.136s | 4.905× | 4.247× | 0.248s | 0.986× | 0.841× |
| [libs/container/test](suites/libs-container-test.md) | 89 | 96.941s | 5.032× | 3.646× | 1.818s | 0.832× | 0.828× |
| [libs/function/test](suites/libs-function-test.md) | 49 | 7.132s | 2.599× | 2.609× | 0.069s | 0.929× | 0.848× |
| [libs/fusion/test](suites/libs-fusion-test.md) | 221 | 53.564s | 2.337× | 2.567× | 0.330s | 0.958× | 0.869× |
| [libs/json/test](suites/libs-json-test.md) | 50 | 52.902s | 6.559× | 4.382× | 1.901s | 0.773× | 0.950× |
| [libs/mp11/test](suites/libs-mp11-test.md) | 234 | 32.090s | 5.280× | 3.404× | 0.295s | 1.030× | 1.090× |
| [libs/mpl/test](suites/libs-mpl-test.md) | 99 | 11.330s | 2.334× | 2.698× | 0.013s | 1.045× | 1.134× |
| [libs/optional/test](suites/libs-optional-test.md) | 74 | 7.413s | 2.766× | 2.823× | 0.059s | 0.977× | 0.981× |
| [libs/proto/test](suites/libs-proto-test.md) | 23 | 10.916s | 2.733× | 2.751× | 0.062s | 0.950× | 0.923× |
| [libs/qvm/test](suites/libs-qvm-test.md) | 134 | 27.666s | 2.650× | 3.065× | 0.218s | 1.008× | 0.974× |
| [libs/spirit/repository/test](suites/libs-spirit-repository-test.md) | 8 | 11.434s | 3.638× | 3.677× | 0.012s | 1.011× | 1.207× |
| [libs/type_erasure/test](suites/libs-type_erasure-test.md) | 60 | 26.004s | 5.088× | 4.088× | 0.111s | 0.972× | 0.948× |
| [libs/type_traits/test](suites/libs-type_traits-test.md) | 218 | 25.380s | 3.294× | 5.333× | 0.262s | 1.036× | 0.992× |
| [libs/uuid/test](suites/libs-uuid-test.md) | 80 | 13.088s | 3.445× | 3.093× | 0.530s | 0.992× | 0.977× |
| [libs/variant2/test](suites/libs-variant2-test.md) | 84 | 29.969s | 2.891× | 3.934× | 0.084s | 1.001× | 0.935× |
| [libs/xpressive/test](suites/libs-xpressive-test.md) | 51 | 53.604s | 4.718× | 4.021× | 0.129s | 1.070× | 0.974× |

Performance uses O3 with assertions enabled; the GDB test remains Og and is correctness-only.

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.974× | 0.971×, 0.977× | 0.995× |
| clang | 0.963× | 0.961×, 0.965× | 0.992× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1364 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43572026855).

Exact commands, per-test times, native outcomes and toolchain/host identities are retained with the results.

[Shared comparison](standard/summary.md) · [Version-1 records](standard/records.json) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/43572026855)
