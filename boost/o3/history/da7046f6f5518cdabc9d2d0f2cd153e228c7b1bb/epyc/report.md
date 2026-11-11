# Boost compilation and test execution

Boost 1.92.0, 17 C++11 suites (1,818 declarations). Ratios are reference / cppgm, higher is better.

Compilation counts child CPU of C++ test compilations, including expected rejections. It excludes configuration probes, library dependencies, C, linking and B2. Test execution includes process startup and the GDB integration test; it is correctness-test timing, not an application benchmark. The debugger source has identical volatile breakpoint anchors for all compilers; MPL and TypeTraits test enums use fixed underlying types for their -1 probes. Original test oracles are retained. Compile timings use one clean campaign per compiler; small differences need repeats.

| Suite | Declarations | Compile CPU cppgm | GCC | Clang | Test CPU cppgm | GCC | Clang |
|---|---:|---:|---:|---:|---:|---:|---:|
| [libs/accumulators/test](suites/libs-accumulators-test.md) | 49 | 47.659s | 2.459× | 2.578× | 2.171s | 1.016× | 0.868× |
| [libs/asio/test](suites/libs-asio-test.md) | 295 | 119.410s | 4.760× | 4.429× | 0.240s | 0.919× | 0.911× |
| [libs/container/test](suites/libs-container-test.md) | 89 | 82.732s | 5.722× | 4.306× | 1.484s | 1.000× | 0.976× |
| [libs/function/test](suites/libs-function-test.md) | 49 | 6.907s | 2.590× | 2.682× | 0.061s | 1.063× | 0.987× |
| [libs/fusion/test](suites/libs-fusion-test.md) | 221 | 52.192s | 2.317× | 2.689× | 0.293s | 1.045× | 1.028× |
| [libs/json/test](suites/libs-json-test.md) | 50 | 47.989s | 6.970× | 5.031× | 1.785s | 0.795× | 1.058× |
| [libs/mp11/test](suites/libs-mp11-test.md) | 234 | 33.538s | 5.020× | 3.513× | 0.290s | 1.072× | 1.206× |
| [libs/mpl/test](suites/libs-mpl-test.md) | 99 | 12.879s | 2.040× | 2.506× | 0.013s | 1.087× | 1.214× |
| [libs/optional/test](suites/libs-optional-test.md) | 74 | 8.089s | 2.525× | 2.826× | 0.056s | 1.037× | 1.151× |
| [libs/proto/test](suites/libs-proto-test.md) | 23 | 11.857s | 2.522× | 2.740× | 0.057s | 1.060× | 1.089× |
| [libs/qvm/test](suites/libs-qvm-test.md) | 134 | 28.913s | 2.502× | 3.026× | 0.231s | 0.946× | 0.962× |
| [libs/spirit/repository/test](suites/libs-spirit-repository-test.md) | 8 | 11.797s | 3.485× | 3.830× | 0.012s | 1.063× | 1.070× |
| [libs/type_erasure/test](suites/libs-type_erasure-test.md) | 60 | 27.919s | 4.729× | 4.051× | 0.111s | 0.978× | 1.003× |
| [libs/type_traits/test](suites/libs-type_traits-test.md) | 218 | 27.523s | 3.011× | 5.167× | 0.263s | 1.053× | 1.073× |
| [libs/uuid/test](suites/libs-uuid-test.md) | 80 | 13.934s | 3.189× | 3.129× | 0.525s | 1.009× | 1.012× |
| [libs/variant2/test](suites/libs-variant2-test.md) | 84 | 31.443s | 2.743× | 3.956× | 0.080s | 1.044× | 1.075× |
| [libs/xpressive/test](suites/libs-xpressive-test.md) | 51 | 57.641s | 4.419× | 3.934× | 0.132s | 1.057× | 0.996× |

Performance uses O3 with assertions enabled; the GDB test remains Og and is correctness-only.

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.995× | 1.003×, 0.987× | 1.006× |
| clang | 0.980× | 0.972×, 0.988× | 0.966× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1361 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613).

Exact commands, per-test times, native outcomes and toolchain/host identities are retained with the results.

[Shared comparison](standard/summary.md) · [Version-1 records](standard/records.json) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613)
