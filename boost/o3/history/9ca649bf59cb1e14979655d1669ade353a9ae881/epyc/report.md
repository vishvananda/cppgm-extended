# Boost compilation and test execution

Boost 1.92.0, 17 C++11 suites (1,818 declarations). Ratios are reference / cppgm, higher is better.

Compilation counts child CPU of C++ test compilations, including expected rejections. It excludes configuration probes, library dependencies, C, linking and B2. Test execution includes process startup and the GDB integration test; it is correctness-test timing, not an application benchmark. The debugger source has identical volatile breakpoint anchors for all compilers; MPL and TypeTraits test enums use fixed underlying types for their -1 probes. Original test oracles are retained. Compile timings use one clean campaign per compiler; small differences need repeats.

| Suite | Declarations | Compile CPU cppgm | GCC | Clang | Test CPU cppgm | GCC | Clang |
|---|---:|---:|---:|---:|---:|---:|---:|
| [libs/accumulators/test](suites/libs-accumulators-test.md) | 49 | 57.117s | 2.815× | 3.266× | 2.731s | 1.062× | 0.908× |
| [libs/asio/test](suites/libs-asio-test.md) | 295 | 151.819s | 5.075× | 5.018× | 0.257s | 1.087× | 1.009× |
| [libs/container/test](suites/libs-container-test.md) | 89 | 107.269s | 5.782× | 4.471× | 2.015s | 0.969× | 0.994× |
| [libs/function/test](suites/libs-function-test.md) | 49 | 9.551s | 2.530× | 3.028× | 0.071s | 1.053× | 1.039× |
| [libs/fusion/test](suites/libs-fusion-test.md) | 221 | 72.483s | 2.317× | 2.894× | 0.348s | 1.054× | 0.995× |
| [libs/json/test](suites/libs-json-test.md) | 50 | 64.562s | 6.990× | 5.065× | 2.445s | 0.827× | 0.987× |
| [libs/mp11/test](suites/libs-mp11-test.md) | 234 | 46.284s | 4.952× | 3.494× | 0.344s | 1.100× | 1.130× |
| [libs/mpl/test](suites/libs-mpl-test.md) | 99 | 17.793s | 2.052× | 2.628× | 0.016s | 0.990× | 1.068× |
| [libs/optional/test](suites/libs-optional-test.md) | 74 | 11.145s | 2.534× | 2.876× | 0.068s | 1.027× | 0.993× |
| [libs/proto/test](suites/libs-proto-test.md) | 23 | 16.101s | 2.542× | 2.801× | 0.066s | 1.053× | 0.997× |
| [libs/qvm/test](suites/libs-qvm-test.md) | 134 | 39.891s | 2.510× | 3.251× | 0.284s | 0.913× | 0.878× |
| [libs/spirit/repository/test](suites/libs-spirit-repository-test.md) | 8 | 16.170s | 3.393× | 3.749× | 0.014s | 1.029× | 0.963× |
| [libs/type_erasure/test](suites/libs-type_erasure-test.md) | 60 | 38.185s | 4.786× | 4.072× | 0.131s | 0.957× | 0.963× |
| [libs/type_traits/test](suites/libs-type_traits-test.md) | 218 | 38.941s | 3.128× | 5.903× | 0.308s | 1.079× | 1.033× |
| [libs/uuid/test](suites/libs-uuid-test.md) | 80 | 18.477s | 3.482× | 3.366× | 0.151s | 1.109× | 1.008× |
| [libs/variant2/test](suites/libs-variant2-test.md) | 84 | 41.069s | 3.027× | 4.379× | 0.092s | 1.130× | 0.993× |
| [libs/xpressive/test](suites/libs-xpressive-test.md) | 51 | 76.423s | 4.488× | 3.884× | 0.155s | 1.130× | 0.964× |

Performance uses O3 with assertions enabled; the GDB test remains Og and is correctness-only.

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.977× | 0.970×, 0.985× | 0.994× |
| clang | 0.965× | 0.962×, 0.969× | 1.000× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1361 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712).

Exact commands, per-test times, native outcomes and toolchain/host identities are retained with the results.

[Shared comparison](standard/summary.md) · [Version-1 records](standard/records.json) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712)
