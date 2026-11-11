# Boost compilation and test execution

Boost 1.92.0, the existing 17-suite C++11 debug selection (1,818 declarations). Ratios are reference / cppgm, higher is better.

Compilation counts child CPU of C++ test compilations, including expected rejections. It excludes configuration probes, library dependencies, C, linking and B2. Test execution includes process startup and the GDB integration test; it is correctness-test timing, not an application benchmark. The debugger source has identical volatile breakpoint anchors for all compilers; MPL and TypeTraits test enums use fixed underlying types for their -1 probes. Original test oracles and optimization flags are retained. Each compiler completes one clean campaign; small differences need repeats.

| Suite | Declarations | Compile CPU cppgm | GCC | Clang | Test CPU cppgm | GCC | Clang |
|---|---:|---:|---:|---:|---:|---:|---:|
| [libs/accumulators/test](suites/libs-accumulators-test.md) | 49 | 52.401s | 3.414× | 3.257× | 39.818s | 1.125× | 1.215× |
| [libs/asio/test](suites/libs-asio-test.md) | 295 | 108.769s | 3.950× | 4.720× | 0.287s | 1.094× | 1.073× |
| [libs/container/test](suites/libs-container-test.md) | 89 | 37.540s | 6.409× | 4.807× | 18.945s | 1.126× | 1.169× |
| [libs/function/test](suites/libs-function-test.md) | 49 | 8.810s | 2.222× | 2.944× | 0.068s | 1.085× | 1.023× |
| [libs/fusion/test](suites/libs-fusion-test.md) | 221 | 67.666s | 2.774× | 2.999× | 0.338s | 1.057× | 1.016× |
| [libs/json/test](suites/libs-json-test.md) | 50 | 41.173s | 4.246× | 4.274× | 5.717s | 1.107× | 1.126× |
| [libs/mp11/test](suites/libs-mp11-test.md) | 234 | 39.453s | 3.068× | 3.461× | 0.336s | 1.055× | 1.119× |
| [libs/mpl/test](suites/libs-mpl-test.md) | 99 | 17.543s | 1.943× | 2.629× | 0.016s | 1.011× | 1.077× |
| [libs/optional/test](suites/libs-optional-test.md) | 74 | 10.022s | 2.268× | 3.024× | 0.067s | 1.022× | 0.985× |
| [libs/proto/test](suites/libs-proto-test.md) | 23 | 15.782s | 2.396× | 2.753× | 0.084s | 1.102× | 1.087× |
| [libs/qvm/test](suites/libs-qvm-test.md) | 134 | 36.628s | 2.255× | 3.041× | 0.695s | 1.013× | 1.035× |
| [libs/spirit/repository/test](suites/libs-spirit-repository-test.md) | 8 | 14.168s | 4.602× | 3.689× | 0.015s | 1.075× | 1.030× |
| [libs/type_erasure/test](suites/libs-type_erasure-test.md) | 60 | 31.866s | 3.325× | 3.379× | 0.163s | 1.054× | 1.074× |
| [libs/type_traits/test](suites/libs-type_traits-test.md) | 218 | 39.375s | 3.226× | 5.649× | 0.305s | 1.059× | 1.046× |
| [libs/uuid/test](suites/libs-uuid-test.md) | 80 | 16.553s | 2.508× | 3.294× | 0.203s | 1.033× | 1.035× |
| [libs/variant2/test](suites/libs-variant2-test.md) | 84 | 38.747s | 2.976× | 4.325× | 0.091s | 1.066× | 1.024× |
| [libs/xpressive/test](suites/libs-xpressive-test.md) | 51 | 55.962s | 4.041× | 3.727× | 0.325s | 1.134× | 1.129× |

Exact commands, per-test times, native outcomes and toolchain/host identities are retained with the results.

[Shared comparison](standard/summary.md) · [Version-1 records](standard/records.json) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/42557497688)
