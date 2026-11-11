# Boost compilation and test execution

Boost 1.92.0, 17 C++11 suites (1,818 declarations). Ratios are reference / cppgm, higher is better.

Compilation counts child CPU of C++ test compilations, including expected rejections. It excludes configuration probes, library dependencies, C, linking and B2. Test execution includes process startup and the GDB integration test; it is correctness-test timing, not an application benchmark. The debugger source has identical volatile breakpoint anchors for all compilers; MPL and TypeTraits test enums use fixed underlying types for their -1 probes. Original test oracles are retained. Compile timings use one clean campaign per compiler; small differences need repeats.

| Suite | Declarations | Compile CPU cppgm | GCC | Clang | Test CPU cppgm | GCC | Clang |
|---|---:|---:|---:|---:|---:|---:|---:|
| [libs/accumulators/test](suites/libs-accumulators-test.md) | 49 | 68.694s | 3.148× | 3.569× | 3.010s | 1.131× | 0.977× |
| [libs/asio/test](suites/libs-asio-test.md) | 295 | 214.939s | 4.767× | 4.920× | 0.384s | 1.085× | 0.998× |
| [libs/container/test](suites/libs-container-test.md) | 89 | 156.340s | 5.789× | 3.906× | 2.545s | 0.950× | 0.988× |
| [libs/function/test](suites/libs-function-test.md) | 49 | 12.986s | 2.565× | 2.932× | 0.074s | 1.199× | 1.170× |
| [libs/fusion/test](suites/libs-fusion-test.md) | 221 | 86.284s | 2.611× | 3.099× | 0.365s | 1.102× | 1.094× |
| [libs/json/test](suites/libs-json-test.md) | 50 | 115.201s | 6.037× | 3.915× | 4.376s | 0.628× | 0.713× |
| [libs/mp11/test](suites/libs-mp11-test.md) | 234 | 56.343s | 5.273× | 3.833× | 0.388s | 1.083× | 1.092× |
| [libs/mpl/test](suites/libs-mpl-test.md) | 99 | 22.251s | 2.230× | 2.755× | 0.019s | 1.000× | 1.667× |
| [libs/optional/test](suites/libs-optional-test.md) | 74 | 13.294s | 2.827× | 3.228× | 0.074s | 0.968× | 0.997× |
| [libs/proto/test](suites/libs-proto-test.md) | 23 | 19.537s | 2.746× | 3.000× | 0.076s | 0.851× | 0.895× |
| [libs/qvm/test](suites/libs-qvm-test.md) | 134 | 47.036s | 2.810× | 3.633× | 0.314s | 0.959× | 0.963× |
| [libs/spirit/repository/test](suites/libs-spirit-repository-test.md) | 8 | 21.275s | 3.646× | 4.097× | 0.023s | 0.864× | 0.806× |
| [libs/type_erasure/test](suites/libs-type_erasure-test.md) | 60 | 50.688s | 5.928× | 4.426× | 0.140s | 1.050× | 1.013× |
| [libs/type_traits/test](suites/libs-type_traits-test.md) | 218 | 45.413s | 3.705× | 6.480× | 0.325s | 1.109× | 1.142× |
| [libs/uuid/test](suites/libs-uuid-test.md) | 80 | 22.287s | 4.148× | 3.621× | 0.207s | 1.137× | 1.064× |
| [libs/variant2/test](suites/libs-variant2-test.md) | 84 | 50.311s | 3.152× | 4.700× | 0.103s | 1.078× | 1.086× |
| [libs/xpressive/test](suites/libs-xpressive-test.md) | 51 | 100.174s | 4.355× | 3.779× | 0.172s | 1.230× | 1.050× |

Performance uses O3 with assertions enabled; the GDB test remains Og and is correctness-only.

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.963× | 0.954×, 0.971× | 0.987× |
| clang | 0.937×† | 0.875×, 0.998× | 1.000× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1353 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202).

Exact commands, per-test times, native outcomes and toolchain/host identities are retained with the results.

[Shared comparison](standard/summary.md) · [Version-1 records](standard/records.json) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202)
