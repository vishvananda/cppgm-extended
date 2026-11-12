# Boost compilation and test execution

Boost 1.92.0, 17 C++11 suites (1,818 declarations). Ratios are reference / cppgm, higher is better.

Compilation counts child CPU of C++ test compilations, including expected rejections. It excludes configuration probes, library dependencies, C, linking and B2. Test execution includes process startup and the GDB integration test; it is correctness-test timing, not an application benchmark. The debugger source has identical volatile breakpoint anchors for all compilers; MPL and TypeTraits test enums use fixed underlying types for their -1 probes. Original test oracles are retained. Compile timings use one clean campaign per compiler; small differences need repeats.

| Suite | Declarations | Compile CPU cppgm | GCC | Clang | Test CPU cppgm | GCC | Clang |
|---|---:|---:|---:|---:|---:|---:|---:|
| [libs/accumulators/test](suites/libs-accumulators-test.md) | 49 | 71.924s | 3.043× | 3.820× | 3.513s | 1.004× | 0.879× |
| [libs/asio/test](suites/libs-asio-test.md) | 295 | 191.778s | 5.251× | 5.834× | 0.298s | 1.090× | 1.035× |
| [libs/container/test](suites/libs-container-test.md) | 89 | 145.123s | 5.791× | 4.696× | 3.269s | 0.784× | 0.797× |
| [libs/function/test](suites/libs-function-test.md) | 49 | 11.823s | 2.912× | 3.743× | 0.085s | 1.078× | 1.031× |
| [libs/fusion/test](suites/libs-fusion-test.md) | 221 | 88.717s | 2.638× | 3.552× | 0.411s | 1.070× | 0.995× |
| [libs/json/test](suites/libs-json-test.md) | 50 | 83.198s | 6.993× | 5.672× | 3.623s | 0.792× | 1.056× |
| [libs/mp11/test](suites/libs-mp11-test.md) | 234 | 58.887s | 5.296× | 4.120× | 0.410s | 1.076× | 1.097× |
| [libs/mpl/test](suites/libs-mpl-test.md) | 99 | 21.518s | 2.367× | 3.342× | 0.019s | 1.064× | 1.161× |
| [libs/optional/test](suites/libs-optional-test.md) | 74 | 13.584s | 2.908× | 3.735× | 0.077s | 1.080× | 1.096× |
| [libs/proto/test](suites/libs-proto-test.md) | 23 | 19.669s | 2.886× | 3.531× | 0.081s | 1.044× | 1.085× |
| [libs/qvm/test](suites/libs-qvm-test.md) | 134 | 50.258s | 2.813× | 4.074× | 0.396s | 0.810× | 0.816× |
| [libs/spirit/repository/test](suites/libs-spirit-repository-test.md) | 8 | 20.499s | 3.621× | 4.408× | 0.017s | 1.001× | 1.048× |
| [libs/type_erasure/test](suites/libs-type_erasure-test.md) | 60 | 46.966s | 4.876× | 4.964× | 0.161s | 0.985× | 0.998× |
| [libs/type_traits/test](suites/libs-type_traits-test.md) | 218 | 50.835s | 3.365× | 7.043× | 0.375s | 1.010× | 1.089× |
| [libs/uuid/test](suites/libs-uuid-test.md) | 80 | 23.030s | 3.645× | 4.269× | 0.190s | 1.022× | 1.031× |
| [libs/variant2/test](suites/libs-variant2-test.md) | 84 | 54.679s | 2.969× | 5.009× | 0.116s | 0.991× | 1.047× |
| [libs/xpressive/test](suites/libs-xpressive-test.md) | 51 | 95.643s | 4.595× | 4.769× | 0.192s | 1.099× | 1.038× |

Performance uses O3 with assertions enabled; the GDB test remains Og and is correctness-only.

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.962× | 0.960×, 0.963× | 0.986× |
| clang | 0.955× | 0.954×, 0.956× | 0.997× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43133898266).

Exact commands, per-test times, native outcomes and toolchain/host identities are retained with the results.

[Shared comparison](standard/summary.md) · [Version-1 records](standard/records.json) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/43133898266)
