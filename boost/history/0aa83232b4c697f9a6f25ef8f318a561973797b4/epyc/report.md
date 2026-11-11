# Boost compilation and test execution

Boost 1.92.0, the existing 17-suite C++11 debug selection (1,818 declarations). Ratios are reference / cppgm, higher is better.

Compilation counts child CPU of C++ test compilations, including expected rejections. It excludes configuration probes, library dependencies, C, linking and B2. Test execution includes process startup and the GDB integration test; it is correctness-test timing, not an application benchmark. The debugger source has identical volatile breakpoint anchors for all compilers; MPL and TypeTraits test enums use fixed underlying types for their -1 probes. Original test oracles and optimization flags are retained. Each compiler completes one clean campaign; small differences need repeats.

| Suite | Declarations | Compile CPU cppgm | GCC | Clang | Test CPU cppgm | GCC | Clang |
|---|---:|---:|---:|---:|---:|---:|---:|
| [libs/accumulators/test](suites/libs-accumulators-test.md) | 49 | 43.358s | 3.527× | 2.969× | 28.681s | 1.010× | 1.158× |
| [libs/asio/test](suites/libs-asio-test.md) | 295 | 84.316s | 4.326× | 4.367× | 0.259s | 1.162× | 1.151× |
| [libs/container/test](suites/libs-container-test.md) | 89 | 30.914s | 6.698× | 4.523× | 14.910s | 1.019× | 1.072× |
| [libs/function/test](suites/libs-function-test.md) | 49 | 6.996s | 2.357× | 2.680× | 0.066s | 1.085× | 1.078× |
| [libs/fusion/test](suites/libs-fusion-test.md) | 221 | 52.377s | 2.978× | 2.861× | 0.321s | 1.097× | 1.071× |
| [libs/json/test](suites/libs-json-test.md) | 50 | 32.967s | 4.518× | 4.052× | 4.082s | 1.060× | 1.052× |
| [libs/mp11/test](suites/libs-mp11-test.md) | 234 | 31.376s | 3.211× | 3.251× | 0.315s | 1.149× | 1.243× |
| [libs/mpl/test](suites/libs-mpl-test.md) | 99 | 14.064s | 1.980× | 2.471× | 0.015s | 1.040× | 1.216× |
| [libs/optional/test](suites/libs-optional-test.md) | 74 | 8.268s | 2.267× | 2.634× | 0.065s | 1.136× | 1.039× |
| [libs/proto/test](suites/libs-proto-test.md) | 23 | 12.948s | 2.527× | 2.542× | 0.078s | 1.090× | 1.089× |
| [libs/qvm/test](suites/libs-qvm-test.md) | 134 | 31.545s | 2.153× | 2.482× | 0.554s | 1.025× | 1.055× |
| [libs/spirit/repository/test](suites/libs-spirit-repository-test.md) | 8 | 11.657s | 4.952× | 3.589× | 0.015s | 1.052× | 1.013× |
| [libs/type_erasure/test](suites/libs-type_erasure-test.md) | 60 | 26.702s | 3.578× | 3.052× | 0.151s | 1.021× | 1.047× |
| [libs/type_traits/test](suites/libs-type_traits-test.md) | 218 | 32.041s | 3.386× | 4.518× | 0.315s | 1.058× | 1.045× |
| [libs/uuid/test](suites/libs-uuid-test.md) | 80 | 14.035s | 2.603× | 2.775× | 0.568s | 1.026× | 1.005× |
| [libs/variant2/test](suites/libs-variant2-test.md) | 84 | 32.282s | 3.034× | 3.711× | 0.093s | 1.051× | 1.015× |
| [libs/xpressive/test](suites/libs-xpressive-test.md) | 51 | 47.145s | 4.196× | 3.415× | 0.268s | 1.080× | 1.091× |

Exact commands, per-test times, native outcomes and toolchain/host identities are retained with the results.

[Shared comparison](standard/summary.md) · [Version-1 records](standard/records.json) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/42570114108)
