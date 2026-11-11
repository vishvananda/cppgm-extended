# Boost compilation and test execution

Boost 1.92.0, the existing 17-suite C++11 debug selection (1,818 declarations). Ratios are reference / cppgm, higher is better.

Compilation counts child CPU of C++ test compilations, including expected rejections. It excludes configuration probes, library dependencies, C, linking and B2. Test execution includes process startup and the GDB integration test; it is correctness-test timing, not an application benchmark. The debugger source has identical volatile breakpoint anchors for all compilers; MPL and TypeTraits test enums use fixed underlying types for their -1 probes. Original test oracles and optimization flags are retained. Each compiler completes one clean campaign; small differences need repeats.

| Suite | Declarations | Compile CPU cppgm | GCC | Clang | Test CPU cppgm | GCC | Clang |
|---|---:|---:|---:|---:|---:|---:|---:|
| [libs/accumulators/test](suites/libs-accumulators-test.md) | 49 | 38.290s | 3.492× | 2.885× | 26.684s | 1.047× | 1.179× |
| [libs/asio/test](suites/libs-asio-test.md) | 295 | 78.104s | 4.046× | 4.091× | 0.233s | 1.101× | 1.059× |
| [libs/container/test](suites/libs-container-test.md) | 89 | 29.019s | 6.519× | 4.169× | 14.880s | 0.997× | 1.001× |
| [libs/function/test](suites/libs-function-test.md) | 49 | 6.573s | 2.371× | 2.498× | 0.061s | 1.118× | 1.003× |
| [libs/fusion/test](suites/libs-fusion-test.md) | 221 | 48.758s | 3.101× | 2.691× | 0.293s | 1.137× | 0.998× |
| [libs/json/test](suites/libs-json-test.md) | 50 | 30.233s | 4.599× | 3.717× | 3.884s | 1.095× | 1.022× |
| [libs/mp11/test](suites/libs-mp11-test.md) | 234 | 28.951s | 3.223× | 3.085× | 0.287s | 1.114× | 1.110× |
| [libs/mpl/test](suites/libs-mpl-test.md) | 99 | 13.292s | 2.017× | 2.227× | 0.013s | 1.120× | 1.089× |
| [libs/optional/test](suites/libs-optional-test.md) | 74 | 7.305s | 2.347× | 2.653× | 0.058s | 1.083× | 0.977× |
| [libs/proto/test](suites/libs-proto-test.md) | 23 | 11.449s | 2.489× | 2.494× | 0.071s | 1.096× | 1.010× |
| [libs/qvm/test](suites/libs-qvm-test.md) | 134 | 26.734s | 2.338× | 2.546× | 0.498s | 1.100× | 1.041× |
| [libs/spirit/repository/test](suites/libs-spirit-repository-test.md) | 8 | 10.153s | 4.950× | 3.424× | 0.014s | 0.968× | 0.972× |
| [libs/type_erasure/test](suites/libs-type_erasure-test.md) | 60 | 23.387s | 3.462× | 3.015× | 0.134s | 1.040× | 1.044× |
| [libs/type_traits/test](suites/libs-type_traits-test.md) | 218 | 28.110s | 3.163× | 4.590× | 0.259s | 1.064× | 1.035× |
| [libs/uuid/test](suites/libs-uuid-test.md) | 80 | 12.017s | 2.522× | 2.869× | 0.553s | 0.995× | 0.992× |
| [libs/variant2/test](suites/libs-variant2-test.md) | 84 | 28.539s | 2.917× | 3.726× | 0.078s | 1.087× | 1.017× |
| [libs/xpressive/test](suites/libs-xpressive-test.md) | 51 | 41.126s | 4.212× | 3.409× | 0.234s | 1.132× | 1.122× |

Exact commands, per-test times, native outcomes and toolchain/host identities are retained with the results.

[Shared comparison](standard/summary.md) · [Version-1 records](standard/records.json) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/42568872153)
