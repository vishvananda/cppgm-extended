# Boost compilation and test execution

Boost 1.92.0, 17 C++11 suites (1,818 declarations). Ratios are reference / cppgm, higher is better.

Compilation counts child CPU of C++ test compilations, including expected rejections. It excludes configuration probes, library dependencies, C, linking and B2. Test execution includes process startup and the GDB integration test; it is correctness-test timing, not an application benchmark. The debugger source has identical volatile breakpoint anchors for all compilers; MPL and TypeTraits test enums use fixed underlying types for their -1 probes. Original test oracles are retained. Compile timings use one clean campaign per compiler; small differences need repeats.

| Suite | Declarations | Compile CPU cppgm | GCC | Clang | Test CPU cppgm | GCC | Clang |
|---|---:|---:|---:|---:|---:|---:|---:|
| [libs/accumulators/test](suites/libs-accumulators-test.md) | 49 | 49.753s | 3.144× | 2.990× | 2.221s | 1.061× | 0.918× |
| [libs/asio/test](suites/libs-asio-test.md) | 295 | 141.719s | 5.210× | 4.484× | 0.266s | 1.057× | 1.022× |
| [libs/container/test](suites/libs-container-test.md) | 89 | 86.525s | 6.868× | 4.847× | 1.640s | 1.019× | 0.955× |
| [libs/function/test](suites/libs-function-test.md) | 49 | 8.730s | 2.624× | 2.640× | 0.075s | 1.061× | 1.012× |
| [libs/fusion/test](suites/libs-fusion-test.md) | 221 | 63.485s | 2.400× | 2.645× | 0.357s | 1.097× | 1.041× |
| [libs/json/test](suites/libs-json-test.md) | 50 | 55.641s | 8.399× | 5.318× | 2.036s | 0.722× | 0.917× |
| [libs/mp11/test](suites/libs-mp11-test.md) | 234 | 40.294s | 5.104× | 3.146× | 0.353s | 1.106× | 1.103× |
| [libs/mpl/test](suites/libs-mpl-test.md) | 99 | 15.209s | 2.126× | 2.369× | 0.015s | 1.116× | 1.213× |
| [libs/optional/test](suites/libs-optional-test.md) | 74 | 9.839s | 2.591× | 2.513× | 0.070s | 1.017× | 0.975× |
| [libs/proto/test](suites/libs-proto-test.md) | 23 | 14.358s | 2.482× | 2.527× | 0.068s | 0.988× | 1.009× |
| [libs/qvm/test](suites/libs-qvm-test.md) | 134 | 35.380s | 2.455× | 2.820× | 0.280s | 0.944× | 0.926× |
| [libs/spirit/repository/test](suites/libs-spirit-repository-test.md) | 8 | 14.692s | 3.422× | 3.556× | 0.016s | 0.943× | 0.902× |
| [libs/type_erasure/test](suites/libs-type_erasure-test.md) | 60 | 33.994s | 5.308× | 4.139× | 0.130s | 0.935× | 0.975× |
| [libs/type_traits/test](suites/libs-type_traits-test.md) | 218 | 33.545s | 3.056× | 4.787× | 0.329s | 1.012× | 1.042× |
| [libs/uuid/test](suites/libs-uuid-test.md) | 80 | 16.741s | 3.254× | 2.729× | 0.545s | 1.014× | 0.981× |
| [libs/variant2/test](suites/libs-variant2-test.md) | 84 | 38.046s | 2.724× | 3.653× | 0.102s | 1.004× | 0.946× |
| [libs/xpressive/test](suites/libs-xpressive-test.md) | 51 | 68.128s | 4.553× | 3.657× | 0.155s | 1.058× | 0.937× |

Performance uses O3 with assertions enabled; the GDB test remains Og and is correctness-only.

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.956× | 0.948×, 0.965× | 1.018× |
| clang | 0.953× | 0.958×, 0.948× | 0.983× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1361 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903).

Exact commands, per-test times, native outcomes and toolchain/host identities are retained with the results.

[Shared comparison](standard/summary.md) · [Version-1 records](standard/records.json) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903)
