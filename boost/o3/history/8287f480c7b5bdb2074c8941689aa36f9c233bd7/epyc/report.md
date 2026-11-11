# Boost compilation and test execution

Boost 1.92.0, 17 C++11 suites (1,818 declarations). Ratios are reference / cppgm, higher is better.

Compilation counts child CPU of C++ test compilations, including expected rejections. It excludes configuration probes, library dependencies, C, linking and B2. Test execution includes process startup and the GDB integration test; it is correctness-test timing, not an application benchmark. The debugger source has identical volatile breakpoint anchors for all compilers; MPL and TypeTraits test enums use fixed underlying types for their -1 probes. Original test oracles are retained. Compile timings use one clean campaign per compiler; small differences need repeats.

| Suite | Declarations | Compile CPU cppgm | GCC | Clang | Test CPU cppgm | GCC | Clang |
|---|---:|---:|---:|---:|---:|---:|---:|
| [libs/accumulators/test](suites/libs-accumulators-test.md) | 49 | 55.823s | 2.868× | 3.303× | 2.721s | 1.062× | 0.905× |
| [libs/asio/test](suites/libs-asio-test.md) | 295 | 148.469s | 4.975× | 5.046× | 0.247s | 1.058× | 1.005× |
| [libs/container/test](suites/libs-container-test.md) | 89 | 106.606s | 5.745× | 4.455× | 2.032s | 0.950× | 0.988× |
| [libs/function/test](suites/libs-function-test.md) | 49 | 9.309s | 2.613× | 3.060× | 0.068s | 1.063× | 1.031× |
| [libs/fusion/test](suites/libs-fusion-test.md) | 221 | 71.065s | 2.328× | 2.936× | 0.333s | 1.053× | 0.995× |
| [libs/json/test](suites/libs-json-test.md) | 50 | 63.741s | 6.822× | 5.097× | 2.541s | 0.800× | 0.942× |
| [libs/mp11/test](suites/libs-mp11-test.md) | 234 | 46.018s | 4.881× | 3.481× | 0.347s | 1.019× | 1.065× |
| [libs/mpl/test](suites/libs-mpl-test.md) | 99 | 17.892s | 2.022× | 2.604× | 0.018s | 0.917× | 0.969× |
| [libs/optional/test](suites/libs-optional-test.md) | 74 | 11.184s | 2.492× | 2.875× | 0.068s | 0.983× | 0.993× |
| [libs/proto/test](suites/libs-proto-test.md) | 23 | 16.014s | 2.529× | 2.821× | 0.066s | 1.039× | 1.032× |
| [libs/qvm/test](suites/libs-qvm-test.md) | 134 | 39.534s | 2.540× | 3.267× | 0.289s | 0.905× | 0.856× |
| [libs/spirit/repository/test](suites/libs-spirit-repository-test.md) | 8 | 16.153s | 3.375× | 3.651× | 0.014s | 0.996× | 0.903× |
| [libs/type_erasure/test](suites/libs-type_erasure-test.md) | 60 | 37.756s | 4.610× | 4.072× | 0.129s | 0.970× | 0.966× |
| [libs/type_traits/test](suites/libs-type_traits-test.md) | 218 | 38.233s | 3.099× | 6.052× | 0.297s | 1.073× | 1.061× |
| [libs/uuid/test](suites/libs-uuid-test.md) | 80 | 18.267s | 3.307× | 3.458× | 0.150s | 1.039× | 1.029× |
| [libs/variant2/test](suites/libs-variant2-test.md) | 84 | 41.174s | 2.859× | 4.451× | 0.093s | 1.026× | 1.022× |
| [libs/xpressive/test](suites/libs-xpressive-test.md) | 51 | 75.935s | 4.301× | 3.954× | 0.151s | 1.108× | 1.011× |

Performance uses O3 with assertions enabled; the GDB test remains Og and is correctness-only.

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.982× | 0.976×, 0.988× | 1.000× |
| clang | 0.970× | 0.951×, 0.989× | 0.997× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1363 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837).

Exact commands, per-test times, native outcomes and toolchain/host identities are retained with the results.

[Shared comparison](standard/summary.md) · [Version-1 records](standard/records.json) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837)
