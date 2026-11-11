# Boost compilation and test execution

Boost 1.92.0, 17 C++11 suites (1,818 declarations). Ratios are reference / cppgm, higher is better.

Compilation counts child CPU of C++ test compilations, including expected rejections. It excludes configuration probes, library dependencies, C, linking and B2. Test execution includes process startup and the GDB integration test; it is correctness-test timing, not an application benchmark. The debugger source has identical volatile breakpoint anchors for all compilers; MPL and TypeTraits test enums use fixed underlying types for their -1 probes. Original test oracles are retained. Compile timings use one clean campaign per compiler; small differences need repeats.

| Suite | Declarations | Compile CPU cppgm | GCC | Clang | Test CPU cppgm | GCC | Clang |
|---|---:|---:|---:|---:|---:|---:|---:|
| [libs/accumulators/test](suites/libs-accumulators-test.md) | 49 | 68.825s | 3.063× | 3.259× | 3.215s | 1.035× | 0.890× |
| [libs/asio/test](suites/libs-asio-test.md) | 295 | 185.280s | 5.328× | 5.061× | 0.235s | 1.051× | 1.019× |
| [libs/container/test](suites/libs-container-test.md) | 89 | 140.725s | 5.822× | 4.294× | 2.255s | 0.962× | 1.029× |
| [libs/function/test](suites/libs-function-test.md) | 49 | 11.313s | 2.795× | 3.074× | 0.066s | 1.049× | 1.032× |
| [libs/fusion/test](suites/libs-fusion-test.md) | 221 | 87.144s | 2.455× | 2.903× | 0.332s | 1.013× | 0.983× |
| [libs/json/test](suites/libs-json-test.md) | 50 | 81.377s | 6.948× | 4.991× | 3.253s | 0.833× | 1.009× |
| [libs/mp11/test](suites/libs-mp11-test.md) | 234 | 56.201s | 5.299× | 3.491× | 0.332s | 1.034× | 1.089× |
| [libs/mpl/test](suites/libs-mpl-test.md) | 99 | 21.800s | 2.137× | 2.566× | 0.015s | 0.986× | 1.087× |
| [libs/optional/test](suites/libs-optional-test.md) | 74 | 13.749s | 2.651× | 2.807× | 0.066s | 0.986× | 1.002× |
| [libs/proto/test](suites/libs-proto-test.md) | 23 | 19.615s | 2.689× | 2.787× | 0.063s | 1.011× | 0.978× |
| [libs/qvm/test](suites/libs-qvm-test.md) | 134 | 48.617s | 2.735× | 3.226× | 0.312s | 0.869× | 0.852× |
| [libs/spirit/repository/test](suites/libs-spirit-repository-test.md) | 8 | 19.779s | 3.534× | 3.736× | 0.013s | 0.988× | 0.997× |
| [libs/type_erasure/test](suites/libs-type_erasure-test.md) | 60 | 47.716s | 4.658× | 3.973× | 0.117s | 0.911× | 0.895× |
| [libs/type_traits/test](suites/libs-type_traits-test.md) | 218 | 47.813s | 3.238× | 5.726× | 0.299s | 1.016× | 1.027× |
| [libs/uuid/test](suites/libs-uuid-test.md) | 80 | 22.851s | 3.499× | 3.301× | 0.165s | 1.016× | 0.978× |
| [libs/variant2/test](suites/libs-variant2-test.md) | 84 | 49.509s | 3.059× | 4.404× | 0.092s | 0.994× | 0.952× |
| [libs/xpressive/test](suites/libs-xpressive-test.md) | 51 | 95.726s | 4.471× | 3.802× | 0.160s | 1.064× | 0.926× |

Performance uses O3 with assertions enabled; the GDB test remains Og and is correctness-only.

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.952× | 0.955×, 0.949× | 1.004× |
| clang | 0.969× | 0.970×, 0.968× | 0.999× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325).

Exact commands, per-test times, native outcomes and toolchain/host identities are retained with the results.

[Shared comparison](standard/summary.md) · [Version-1 records](standard/records.json) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325)
