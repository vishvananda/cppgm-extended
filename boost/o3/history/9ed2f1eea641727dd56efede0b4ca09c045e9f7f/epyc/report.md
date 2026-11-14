# Boost compilation and test execution

Boost 1.92.0, 17 C++11 suites (1,818 declarations). Ratios are reference / cppgm, higher is better.

Compilation counts child CPU of C++ test compilations, including expected rejections. It excludes configuration probes, library dependencies, C, linking and B2. Test execution includes process startup and the GDB integration test; it is correctness-test timing, not an application benchmark. The debugger source has identical volatile breakpoint anchors for all compilers; MPL and TypeTraits test enums use fixed underlying types for their -1 probes. Original test oracles are retained. Compile timings use one clean campaign per compiler; small differences need repeats.

| Suite | Declarations | Compile CPU cppgm | GCC | Clang | Test CPU cppgm | GCC | Clang |
|---|---:|---:|---:|---:|---:|---:|---:|
| [libs/accumulators/test](suites/libs-accumulators-test.md) | 49 | 51.785s | 3.094× | 3.566× | 2.795s | 1.036× | 0.883× |
| [libs/asio/test](suites/libs-asio-test.md) | 295 | 137.732s | 5.621× | 5.479× | 0.244s | 1.096× | 1.044× |
| [libs/container/test](suites/libs-container-test.md) | 89 | 111.239s | 5.678× | 4.324× | 2.153s | 0.898× | 0.933× |
| [libs/function/test](suites/libs-function-test.md) | 49 | 8.593s | 2.861× | 3.363× | 0.071s | 1.044× | 1.001× |
| [libs/fusion/test](suites/libs-fusion-test.md) | 221 | 64.522s | 2.599× | 3.235× | 0.339s | 1.041× | 0.981× |
| [libs/json/test](suites/libs-json-test.md) | 50 | 62.924s | 6.997× | 5.213× | 2.326s | 0.780× | 1.087× |
| [libs/mp11/test](suites/libs-mp11-test.md) | 234 | 42.438s | 5.351× | 3.811× | 0.338s | 1.057× | 1.104× |
| [libs/mpl/test](suites/libs-mpl-test.md) | 99 | 15.303s | 2.402× | 3.052× | 0.015s | 1.082× | 1.165× |
| [libs/optional/test](suites/libs-optional-test.md) | 74 | 9.692s | 2.935× | 3.324× | 0.066s | 1.045× | 1.034× |
| [libs/proto/test](suites/libs-proto-test.md) | 23 | 14.367s | 2.875× | 3.161× | 0.065s | 1.078× | 1.037× |
| [libs/qvm/test](suites/libs-qvm-test.md) | 134 | 35.926s | 2.819× | 3.630× | 0.255s | 1.042× | 1.009× |
| [libs/spirit/repository/test](suites/libs-spirit-repository-test.md) | 8 | 15.019s | 3.762× | 4.122× | 0.014s | 1.036× | 0.989× |
| [libs/type_erasure/test](suites/libs-type_erasure-test.md) | 60 | 34.075s | 5.301× | 4.991× | 0.127s | 0.984× | 0.986× |
| [libs/type_traits/test](suites/libs-type_traits-test.md) | 218 | 34.816s | 3.450× | 6.704× | 0.293s | 1.094× | 1.146× |
| [libs/uuid/test](suites/libs-uuid-test.md) | 80 | 16.350s | 3.741× | 3.821× | 0.146s | 1.091× | 1.055× |
| [libs/variant2/test](suites/libs-variant2-test.md) | 84 | 38.982s | 3.060× | 4.583× | 0.091s | 1.058× | 0.993× |
| [libs/xpressive/test](suites/libs-xpressive-test.md) | 51 | 67.950s | 4.835× | 4.416× | 0.149s | 1.109× | 0.994× |

Performance uses O3 with assertions enabled; the GDB test remains Og and is correctness-only.

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.961× | 0.964×, 0.958× | 1.011× |
| clang | 0.940× | 0.934×, 0.946× | 0.999× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1363 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43601267198).

Exact commands, per-test times, native outcomes and toolchain/host identities are retained with the results.

[Shared comparison](standard/summary.md) · [Version-1 records](standard/records.json) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/43601267198)
