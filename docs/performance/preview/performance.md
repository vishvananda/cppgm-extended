## Performance

**Speedup = reference time / cppgm time; higher is better.** GCC and Clang are separate baselines. CPU time is used unless a row explicitly says elapsed time. Every hardware section is an exact CPU model and execution environment. Small differences need confirmation; the linked reports retain estimator and calibration information.

### AMD EPYC 9V45 96-Core Processor · GitHub hosted

| Workload / configuration | Opt | Compile vs GCC | Compile vs Clang | Runtime vs GCC | Runtime vs Clang | Timed coverage | Revision / details |
|---|---|---:|---:|---:|---:|---|---|
| Public programs | O0 | 2.753× | 2.641× | 1.031× | 0.999× | compile 76/81; run 81/81 | [`783dbfcf0`](public.md) |
| Public programs | O3 | 4.021× | 3.492× | 0.991× | 0.999× | compile 79/81; run 80/81 | [`783dbfcf0`](public.md) |

Public aggregates are equal-weight geometric means. The existing 10 ms eligibility floor explains smaller timed counts; all 81 programs remain in the detail tables.

| Compiler build / execution | Opt | vs GCC | vs Clang | Comparison | Details |
|---|---|---:|---|---|---|
| Build cppgm++ | O3 | 4.237× | Not measured | Initial bootstrap CPU; GCC 279.08 s / cppgm 65.87 s | [783dbfcf0](public.md#building-the-compiler) |
| cppgm++ codegen | O3 | 0.969× | Not measured | Self-built versus GCC-built cppgm; paired CPU | [Inception details](public.md#paired-self-built-compiler-measurements) |

### AMD EPYC 9V74 80-Core Processor · GitHub hosted

| Workload / configuration | Opt | Compile CPU vs GCC | Compile CPU vs Clang | Runtime elapsed vs GCC | Runtime elapsed vs Clang | Coverage | Revision / details |
|---|---|---:|---:|---:|---:|---|---|
| index-cpp / full inputs | O3 | 6.765× | 5.889× | 0.994× | 0.989× | 50 / 50 | [`c9774d6a9`](index-cpp.md) |

Runtime is the equal-weight geometric mean of all 50 workload loop elapsed speedups. A/A warnings stay visible and no workloads are dropped.

### AMD EPYC 7763 64-Core Processor · GitHub hosted

| Workload / configuration | Opt | Compile vs GCC | Compile vs Clang | Runtime vs GCC | Runtime vs Clang | Coverage | Revision / details |
|---|---|---:|---:|---:|---:|---|---|
| simdjson / scalar | O3 | 4.467× | 2.329× | 0.814× | 0.801× | one library; two inputs | [`7dbe6549e`](simdjson.md) |
| simdjson / sse | O3 | 4.492× | 2.558× | 0.900× | 0.891× | one library; two inputs | [`7dbe6549e`](simdjson.md) |

These are the labeled earlier published snapshots. Runtime aggregates give twitter and citm equal weight; per-input CPU and elapsed results remain in the linked tables. SSE uses the westmere backend (SSE4.2, PCLMUL and POPCNT).

### Intel(R) Xeon(R) Platinum 8370C CPU @ 2.80GHz · GitHub hosted

| Workload / configuration | Opt | Compile vs GCC | Compile vs Clang | Runtime vs GCC | Runtime vs Clang | Coverage | Revision / details |
|---|---|---:|---:|---:|---:|---|---|
| simdjson / scalar | O3 | 4.431× | 2.101× | 0.851× | 0.849× | one library; two inputs | [`adb8a65ad`](simdjson.md) |
| simdjson / sse | O3 | 4.344× | 2.221× | 0.988× | 0.960× | one library; two inputs | [`adb8a65ad`](simdjson.md) |

These are the labeled earlier published snapshots. Runtime aggregates give twitter and citm equal weight; per-input CPU and elapsed results remain in the linked tables. SSE uses the westmere backend (SSE4.2, PCLMUL and POPCNT).

### Intel Xeon E5-2696 v4 · local shared host

| Workload / configuration | Opt | Compile CPU vs GCC | Compile CPU vs Clang | Execution observation | Revision / details |
|---|---|---:|---:|---|---|
| Internal C++ overlap | O0 | 3.981× | 6.302× | 1,210 matched normal-exit controls pass | [`783dbfcf0`](internal.md) |
| Internal C++ overlap | O3 | 4.461× | 6.849× | 1,210 matched normal-exit controls pass | [`783dbfcf0`](internal.md) |

This is one complete observation under load, using four workers and identical resource limits. The 5,614 fixture groups contain 5,724 translation units. Execution totals include process startup and appear in detail rather than as an application-runtime score.

### Adding a workload or configuration

Add a named configuration to the [workload catalog](../../../benchmarks/catalog.json), emit [versioned correctness and comparison records](../results-format.md), and regenerate the summary and linked tables. Tests/inputs are children of a configuration; compiling one library is counted once. Godot can add build presets and benchmark scenes using this same contract.

This preview uses pinned snapshots. Existing automatic publishers still need migration to the [shared reporting contract](../results-format.md#producer-migration-checks). [Snapshot provenance](sources.json).


## Additional Intel snapshots

- [Dedicated Xeon E5-2696 v4: public programs and cppgm self-build/codegen](public-intel.md).
- [Hosted Xeon Platinum 8370C: index-cpp](index-cpp-hosted-intel.md).

Processor badges retain independent measured revisions and environments. Local shared-host measurements retain their environment in each badge tooltip and linked report.

## cppgm comparisons against both native compilers

[Local Intel: compiler build time and paired codegen vs GCC and Clang](cppgm-local.md).
