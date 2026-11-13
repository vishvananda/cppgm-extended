# Boost compilation and test execution

Boost 1.92.0, 17 C++11 suites (1,818 declarations). Ratios are reference / cppgm, higher is better.

Compilation counts child CPU of C++ test compilations, including expected rejections. It excludes configuration probes, library dependencies, C, linking and B2. Test execution includes process startup and the GDB integration test; it is correctness-test timing, not an application benchmark. The debugger source has identical volatile breakpoint anchors for all compilers; MPL and TypeTraits test enums use fixed underlying types for their -1 probes. Original test oracles are retained. Compile timings use one clean campaign per compiler; small differences need repeats.

| Suite | Declarations | Compile CPU cppgm | GCC | Clang | Test CPU cppgm | GCC | Clang |
|---|---:|---:|---:|---:|---:|---:|---:|
| [libs/accumulators/test](suites/libs-accumulators-test.md) | 49 | 68.652s | 3.126× | 3.987× | 3.647s | 0.967× | 0.852× |
| [libs/asio/test](suites/libs-asio-test.md) | 295 | 184.755s | 5.374× | 6.060× | 0.287s | 1.037× | 1.041× |
| [libs/container/test](suites/libs-container-test.md) | 89 | 143.208s | 5.710× | 4.759× | 2.677s | 0.932× | 0.973× |
| [libs/function/test](suites/libs-function-test.md) | 49 | 11.197s | 3.001× | 3.948× | 0.080s | 1.030× | 1.056× |
| [libs/fusion/test](suites/libs-fusion-test.md) | 221 | 84.748s | 2.681× | 3.710× | 0.383s | 1.050× | 1.045× |
| [libs/json/test](suites/libs-json-test.md) | 50 | 80.753s | 6.976× | 5.852× | 3.402s | 0.796× | 1.041× |
| [libs/mp11/test](suites/libs-mp11-test.md) | 234 | 56.486s | 5.453× | 4.293× | 0.387s | 1.048× | 1.128× |
| [libs/mpl/test](suites/libs-mpl-test.md) | 99 | 20.551s | 2.427× | 3.441× | 0.018s | 0.997× | 1.108× |
| [libs/optional/test](suites/libs-optional-test.md) | 74 | 13.004s | 2.982× | 3.803× | 0.075s | 1.053× | 1.059× |
| [libs/proto/test](suites/libs-proto-test.md) | 23 | 18.983s | 2.942× | 3.580× | 0.079s | 1.014× | 1.020× |
| [libs/qvm/test](suites/libs-qvm-test.md) | 134 | 47.968s | 2.925× | 4.193× | 0.379s | 0.827× | 0.800× |
| [libs/spirit/repository/test](suites/libs-spirit-repository-test.md) | 8 | 19.533s | 3.698× | 4.321× | 0.015s | 1.081× | 0.981× |
| [libs/type_erasure/test](suites/libs-type_erasure-test.md) | 60 | 45.062s | 5.055× | 4.929× | 0.157s | 0.933× | 0.962× |
| [libs/type_traits/test](suites/libs-type_traits-test.md) | 218 | 48.395s | 3.503× | 7.266× | 0.340s | 1.083× | 1.097× |
| [libs/uuid/test](suites/libs-uuid-test.md) | 80 | 21.942s | 3.805× | 4.355× | 0.177s | 1.034× | 1.042× |
| [libs/variant2/test](suites/libs-variant2-test.md) | 84 | 52.283s | 3.064× | 5.078× | 0.106s | 1.035× | 1.050× |
| [libs/xpressive/test](suites/libs-xpressive-test.md) | 51 | 92.728s | 4.698× | 4.644× | 0.187s | 1.124× | 1.009× |

Performance uses O3 with assertions enabled; the GDB test remains Og and is correctness-only.

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.954× | 0.955×, 0.952× | 0.998× |
| clang | 0.944× | 0.945×, 0.943× | 0.995× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43348824563).

Exact commands, per-test times, native outcomes and toolchain/host identities are retained with the results.

[Shared comparison](standard/summary.md) · [Version-1 records](standard/records.json) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/43348824563)
