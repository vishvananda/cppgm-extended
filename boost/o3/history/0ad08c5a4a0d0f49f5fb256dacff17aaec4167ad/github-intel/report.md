# Boost compilation and test execution

Boost 1.92.0, 17 C++11 suites (1,818 declarations). Ratios are reference / cppgm, higher is better.

Compilation counts child CPU of C++ test compilations, including expected rejections. It excludes configuration probes, library dependencies, C, linking and B2. Test execution includes process startup and the GDB integration test; it is correctness-test timing, not an application benchmark. The debugger source has identical volatile breakpoint anchors for all compilers; MPL and TypeTraits test enums use fixed underlying types for their -1 probes. Original test oracles are retained. Compile timings use one clean campaign per compiler; small differences need repeats.

| Suite | Declarations | Compile CPU cppgm | GCC | Clang | Test CPU cppgm | GCC | Clang |
|---|---:|---:|---:|---:|---:|---:|---:|
| [libs/accumulators/test](suites/libs-accumulators-test.md) | 49 | 65.748s | 3.008× | 3.142× | 3.244s | 1.024× | 0.889× |
| [libs/asio/test](suites/libs-asio-test.md) | 295 | 184.363s | 5.105× | 4.668× | 0.251s | 1.037× | 1.006× |
| [libs/container/test](suites/libs-container-test.md) | 89 | 144.423s | 5.496× | 3.990× | 2.390s | 0.955× | 1.027× |
| [libs/function/test](suites/libs-function-test.md) | 49 | 10.985s | 2.688× | 2.826× | 0.070s | 1.007× | 0.986× |
| [libs/fusion/test](suites/libs-fusion-test.md) | 221 | 82.312s | 2.462× | 2.822× | 0.339s | 1.045× | 1.018× |
| [libs/json/test](suites/libs-json-test.md) | 50 | 78.089s | 7.165× | 5.086× | 3.158s | 0.756× | 1.001× |
| [libs/mp11/test](suites/libs-mp11-test.md) | 234 | 55.362s | 5.150× | 3.296× | 0.355s | 1.001× | 1.072× |
| [libs/mpl/test](suites/libs-mpl-test.md) | 99 | 20.396s | 2.153× | 2.525× | 0.016s | 1.063× | 1.163× |
| [libs/optional/test](suites/libs-optional-test.md) | 74 | 13.226s | 2.606× | 2.672× | 0.070s | 0.968× | 0.994× |
| [libs/proto/test](suites/libs-proto-test.md) | 23 | 18.743s | 2.664× | 2.747× | 0.066s | 0.982× | 0.963× |
| [libs/qvm/test](suites/libs-qvm-test.md) | 134 | 46.328s | 2.655× | 3.069× | 0.315s | 0.859× | 0.865× |
| [libs/spirit/repository/test](suites/libs-spirit-repository-test.md) | 8 | 19.256s | 3.558× | 3.620× | 0.014s | 1.030× | 0.960× |
| [libs/type_erasure/test](suites/libs-type_erasure-test.md) | 60 | 45.572s | 4.859× | 4.073× | 0.118s | 0.944× | 0.927× |
| [libs/type_traits/test](suites/libs-type_traits-test.md) | 218 | 44.389s | 3.207× | 5.403× | 0.314s | 1.034× | 1.035× |
| [libs/uuid/test](suites/libs-uuid-test.md) | 80 | 21.911s | 3.401× | 3.142× | 0.167s | 0.997× | 1.015× |
| [libs/variant2/test](suites/libs-variant2-test.md) | 84 | 46.617s | 2.975× | 4.170× | 0.094s | 0.991× | 1.001× |
| [libs/xpressive/test](suites/libs-xpressive-test.md) | 51 | 94.335s | 4.367× | 3.693× | 0.159s | 1.007× | 0.933× |

Performance uses O3 with assertions enabled; the GDB test remains Og and is correctness-only.

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.965× | 0.960×, 0.969× | 1.007× |
| clang | 0.984× | 0.981×, 0.987× | 0.996× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980).

Exact commands, per-test times, native outcomes and toolchain/host identities are retained with the results.

[Shared comparison](standard/summary.md) · [Version-1 records](standard/records.json) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980)
