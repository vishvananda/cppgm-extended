# Boost compilation and test execution

Boost 1.92.0, the existing 17-suite C++11 debug selection (1,818 declarations). Ratios are reference / cppgm, higher is better.

Compilation counts child CPU of C++ test compilations, including expected rejections. It excludes configuration probes, library dependencies, C, linking and B2. Test execution includes process startup and the GDB integration test; it is correctness-test timing, not an application benchmark. The debugger source has identical volatile breakpoint anchors for all compilers; MPL and TypeTraits test enums use fixed underlying types for their -1 probes. Original test oracles and optimization flags are retained. Each compiler completes one clean campaign; small differences need repeats.

| Suite | Declarations | Compile CPU cppgm | GCC | Clang | Test CPU cppgm | GCC | Clang |
|---|---:|---:|---:|---:|---:|---:|---:|
| [libs/accumulators/test](suites/libs-accumulators-test.md) | 49 | 54.732s | 3.273× | 3.192× | 39.710s | 1.138× | 1.222× |
| [libs/asio/test](suites/libs-asio-test.md) | 295 | 112.894s | 3.846× | 4.796× | 0.305s | 1.028× | 1.090× |
| [libs/container/test](suites/libs-container-test.md) | 89 | 38.783s | 6.389× | 4.905× | 18.644s | 1.137× | 1.172× |
| [libs/function/test](suites/libs-function-test.md) | 49 | 9.247s | 2.132× | 2.864× | 0.072s | 0.991× | 1.039× |
| [libs/fusion/test](suites/libs-fusion-test.md) | 221 | 70.659s | 2.688× | 2.965× | 0.358s | 1.013× | 1.024× |
| [libs/json/test](suites/libs-json-test.md) | 50 | 43.462s | 4.138× | 4.108× | 5.720s | 1.097× | 1.122× |
| [libs/mp11/test](suites/libs-mp11-test.md) | 234 | 41.053s | 2.980× | 3.458× | 0.355s | 1.029× | 1.145× |
| [libs/mpl/test](suites/libs-mpl-test.md) | 99 | 18.624s | 1.841× | 2.512× | 0.017s | 0.966× | 1.099× |
| [libs/optional/test](suites/libs-optional-test.md) | 74 | 10.709s | 2.129× | 2.887× | 0.071s | 0.985× | 0.939× |
| [libs/proto/test](suites/libs-proto-test.md) | 23 | 16.620s | 2.269× | 2.653× | 0.089s | 1.035× | 1.012× |
| [libs/qvm/test](suites/libs-qvm-test.md) | 134 | 37.357s | 2.241× | 3.020× | 0.707s | 1.034× | 1.041× |
| [libs/spirit/repository/test](suites/libs-spirit-repository-test.md) | 8 | 14.814s | 4.397× | 3.739× | 0.016s | 0.997× | 1.088× |
| [libs/type_erasure/test](suites/libs-type_erasure-test.md) | 60 | 33.145s | 3.216× | 3.348× | 0.165s | 1.043× | 1.084× |
| [libs/type_traits/test](suites/libs-type_traits-test.md) | 218 | 40.229s | 3.191× | 5.561× | 0.323s | 1.021× | 1.015× |
| [libs/uuid/test](suites/libs-uuid-test.md) | 80 | 17.271s | 2.429× | 3.168× | 0.205s | 1.037× | 1.039× |
| [libs/variant2/test](suites/libs-variant2-test.md) | 84 | 40.222s | 2.880× | 4.335× | 0.098s | 1.011× | 1.028× |
| [libs/xpressive/test](suites/libs-xpressive-test.md) | 51 | 56.522s | 4.040× | 3.854× | 0.325s | 1.136× | 1.196× |

Exact commands, per-test times, native outcomes and toolchain/host identities are retained with the results.

[Shared comparison](standard/summary.md) · [Version-1 records](standard/records.json) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/42570601152)
