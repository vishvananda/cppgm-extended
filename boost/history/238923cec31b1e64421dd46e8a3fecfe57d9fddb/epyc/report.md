# Boost compilation and test execution

Boost 1.92.0, the existing 17-suite C++11 debug selection (1,818 declarations). Ratios are reference / cppgm, higher is better.

Compilation counts child CPU of C++ test compilations, including expected rejections. It excludes configuration probes, library dependencies, C, linking and B2. Test execution includes process startup and the GDB integration test; it is correctness-test timing, not an application benchmark. The debugger source has identical volatile breakpoint anchors for all compilers; MPL and TypeTraits test enums use fixed underlying types for their -1 probes. Original test oracles and optimization flags are retained. Each compiler completes one clean campaign; small differences need repeats.

| Suite | Declarations | Compile CPU cppgm | GCC | Clang | Test CPU cppgm | GCC | Clang |
|---|---:|---:|---:|---:|---:|---:|---:|
| [libs/accumulators/test](suites/libs-accumulators-test.md) | 49 | 44.652s | 3.522× | 2.709× | 29.243s | 0.996× | 1.145× |
| [libs/asio/test](suites/libs-asio-test.md) | 295 | 96.012s | 3.944× | 3.681× | 0.290s | 1.084× | 0.943× |
| [libs/container/test](suites/libs-container-test.md) | 89 | 33.244s | 6.517× | 3.981× | 14.976s | 1.033× | 1.056× |
| [libs/function/test](suites/libs-function-test.md) | 49 | 7.759s | 2.105× | 2.243× | 0.070s | 1.027× | 0.931× |
| [libs/fusion/test](suites/libs-fusion-test.md) | 221 | 57.882s | 2.761× | 2.474× | 0.358s | 1.014× | 0.929× |
| [libs/json/test](suites/libs-json-test.md) | 50 | 36.124s | 4.418× | 3.518× | 4.078s | 1.039× | 1.043× |
| [libs/mp11/test](suites/libs-mp11-test.md) | 234 | 34.761s | 2.996× | 2.749× | 0.357s | 1.048× | 0.992× |
| [libs/mpl/test](suites/libs-mpl-test.md) | 99 | 14.857s | 1.948× | 2.184× | 0.016s | 1.071× | 1.062× |
| [libs/optional/test](suites/libs-optional-test.md) | 74 | 8.765s | 2.183× | 2.429× | 0.070s | 1.049× | 0.934× |
| [libs/proto/test](suites/libs-proto-test.md) | 23 | 13.839s | 2.337× | 2.279× | 0.083s | 1.062× | 0.965× |
| [libs/qvm/test](suites/libs-qvm-test.md) | 134 | 32.080s | 2.122× | 2.334× | 0.576s | 1.007× | 0.983× |
| [libs/spirit/repository/test](suites/libs-spirit-repository-test.md) | 8 | 11.816s | 4.935× | 3.333× | 0.015s | 1.047× | 0.969× |
| [libs/type_erasure/test](suites/libs-type_erasure-test.md) | 60 | 27.090s | 3.659× | 2.922× | 0.149s | 1.063× | 1.034× |
| [libs/type_traits/test](suites/libs-type_traits-test.md) | 218 | 32.223s | 3.435× | 4.369× | 0.322s | 1.063× | 0.978× |
| [libs/uuid/test](suites/libs-uuid-test.md) | 80 | 13.974s | 2.506× | 2.682× | 0.564s | 1.026× | 1.007× |
| [libs/variant2/test](suites/libs-variant2-test.md) | 84 | 33.754s | 2.842× | 3.457× | 0.098s | 1.006× | 0.917× |
| [libs/xpressive/test](suites/libs-xpressive-test.md) | 51 | 48.222s | 3.911× | 3.244× | 0.263s | 1.061× | 1.086× |

Exact commands, per-test times, native outcomes and toolchain/host identities are retained with the results.

[Shared comparison](standard/summary.md) · [Version-1 records](standard/records.json) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/42562778965)
