# cppgm-extended

> **README preview:** proposed layout with real, pinned observations. Review this branch; the main README keeps its existing layout.

[![Tests](https://github.com/vishvananda/cppgm-extended/actions/workflows/tests.yml/badge.svg)](https://github.com/vishvananda/cppgm-extended/actions/workflows/tests.yml)

A C++11 compiler course designed for agents, with 35 cumulative assignments from
preprocessing to native code, optimization and self-hosting. This repository
contains a high-performance reference implementation in `dev/`; `pa1/` through
`pa35/` hold the handouts, tests and wrappers.

For how the reference compiler was built and its performance results, read
[How I Built a Compiler That Is Faster Than GCC and Clang](https://medium.com/@vishvananda/how-i-built-a-compiler-that-is-faster-than-gcc-and-clang-4c8c1d4818fa).

To take the course, start with
[cppgm-assignments](https://github.com/vishvananda/cppgm-assignments), the export
containing scaffolds, handouts, tests and downloadable reference tools.
The export's references are generated from this compiler.

## Correctness and external builds

Results below are a preview snapshot for `783dbfcf0`. Performance rows name their own measured revision; older measurements are labeled separately.

| Suite / configuration | Result | What is checked | Evidence |
|---|---|---|---|
| Required internal suite | 9,832 / 9,832 | Stage, IR, ABI, driver and execution contracts | [CI](https://github.com/vishvananda/cppgm-extended/actions/runs/42476441247) |
| GCC C++11 profile | 3,968 / 3,968 | 2,678 acceptance; 1,290 required diagnostics; 162 link/run cases | [Scope and timings](docs/performance/preview/upstream-cxx11.md) |
| Clang C++11 profile | 815 / 815 | 310 acceptance; 505 required diagnostics; compile-only | [Scope and timings](docs/performance/preview/upstream-cxx11.md) |
| Internal native overlap | 5,614 groups compile; 1,210 matched controls run | cppgm, GCC and Clang, O0 and O3 | [Details and exclusions](docs/performance/preview/internal.md) |
| Public programs | 81 / 81 | Builds and matching output | [Per-program results](docs/performance/preview/public.md) |
| index-cpp | 50 / 50 | Original checksums, all three compilers | [Per-workload results](docs/performance/preview/index-cpp.md) |
| simdjson scalar / SSE | Pass, both backends | twitter and citm canonical DOM output | [Current CI](https://github.com/vishvananda/cppgm-extended/actions/runs/42476441186) |
| Boost | Pinned advisory suites pass | Upstream build, run and rejection actions | [CI](https://github.com/vishvananda/cppgm-extended/actions/runs/42476437427) |
| Self-build and inception | Pass | Complete rebuild; identical self/inception compiler binaries | [CI](https://github.com/vishvananda/cppgm-extended/actions/runs/42476437424) |
| Student export | Pass | Exported compiler, fixtures and reference wrappers | [CI](https://github.com/vishvananda/cppgm-extended/actions/runs/42476441242) |
| Godot | Planned | Configurations and benchmark inputs not yet declared | Not measured |

Upstream counts describe selected C++11 profiles. Exclusions remain visible and are not passes. The internal native overlap is a source/object projection, separate from the complete course harness. The [original GCC audit and source dispositions](reports/cxx11-final-source-review.txt) retain the broader scope limits.

## Performance

**Speedup = reference time / cppgm time; higher is better.** GCC and Clang are separate baselines. CPU time is used unless a row explicitly says elapsed time. Every hardware section is an exact CPU model and execution environment. Small differences need confirmation; the linked reports retain estimator and calibration information.

### AMD EPYC 9V45 96-Core Processor · GitHub hosted

| Workload / configuration | Opt | Compile vs GCC | Compile vs Clang | Runtime vs GCC | Runtime vs Clang | Timed coverage | Revision / details |
|---|---|---:|---:|---:|---:|---|---|
| Public programs | O0 | 2.753× | 2.641× | 1.031× | 0.999× | compile 76/81; run 81/81 | [`783dbfcf0`](docs/performance/preview/public.md) |
| Public programs | O3 | 4.021× | 3.492× | 0.991× | 0.999× | compile 79/81; run 80/81 | [`783dbfcf0`](docs/performance/preview/public.md) |

Public aggregates are equal-weight geometric means. The existing 10 ms eligibility floor explains smaller timed counts; all 81 programs remain in the detail tables.

| Compiler build / execution | Opt | vs GCC | vs Clang | Comparison | Details |
|---|---|---:|---|---|---|
| Build cppgm++ | O3 | 4.237× | Not measured | Initial bootstrap CPU; GCC 279.08 s / cppgm 65.87 s | [783dbfcf0](docs/performance/preview/public.md#building-the-compiler) |
| Generated compiler execution | O3 | Not applicable | Not applicable | Self-built / GCC-built cppgm speed: 0.969×; paired CPU | [Inception details](docs/performance/preview/public.md#paired-self-built-compiler-measurements) |

### AMD EPYC 9V74 80-Core Processor · GitHub hosted

| Workload / configuration | Opt | Compile CPU vs GCC | Compile CPU vs Clang | Runtime elapsed vs GCC | Runtime elapsed vs Clang | Coverage | Revision / details |
|---|---|---:|---:|---:|---:|---|---|
| index-cpp / full inputs | O3 | 6.765× | 5.889× | 0.994× | 0.989× | 50 / 50 | [`c9774d6a9`](docs/performance/preview/index-cpp.md) |

Runtime is the equal-weight geometric mean of all 50 workload loop elapsed speedups. A/A warnings stay visible and no workloads are dropped.

### AMD EPYC 7763 64-Core Processor · GitHub hosted

| Workload / configuration | Opt | Compile vs GCC | Compile vs Clang | Runtime vs GCC | Runtime vs Clang | Coverage | Revision / details |
|---|---|---:|---:|---:|---:|---|---|
| simdjson / scalar | O3 | 4.467× | 2.329× | 0.814× | 0.801× | one library; two inputs | [`7dbe6549e`](docs/performance/preview/simdjson.md) |
| simdjson / sse | O3 | 4.492× | 2.558× | 0.900× | 0.891× | one library; two inputs | [`7dbe6549e`](docs/performance/preview/simdjson.md) |

These are the labeled earlier published snapshots. Runtime aggregates give twitter and citm equal weight; per-input CPU and elapsed results remain in the linked tables. SSE uses the westmere backend (SSE4.2, PCLMUL and POPCNT).

### Intel(R) Xeon(R) Platinum 8370C CPU @ 2.80GHz · GitHub hosted

| Workload / configuration | Opt | Compile vs GCC | Compile vs Clang | Runtime vs GCC | Runtime vs Clang | Coverage | Revision / details |
|---|---|---:|---:|---:|---:|---|---|
| simdjson / scalar | O3 | 4.431× | 2.101× | 0.851× | 0.849× | one library; two inputs | [`adb8a65ad`](docs/performance/preview/simdjson.md) |
| simdjson / sse | O3 | 4.344× | 2.221× | 0.988× | 0.960× | one library; two inputs | [`adb8a65ad`](docs/performance/preview/simdjson.md) |

These are the labeled earlier published snapshots. Runtime aggregates give twitter and citm equal weight; per-input CPU and elapsed results remain in the linked tables. SSE uses the westmere backend (SSE4.2, PCLMUL and POPCNT).

### Intel Xeon E5-2696 v4 · local shared host

| Workload / configuration | Opt | Compile CPU vs GCC | Compile CPU vs Clang | Execution observation | Revision / details |
|---|---|---:|---:|---|---|
| Internal C++ overlap | O0 | 3.981× | 6.302× | 1,210 matched normal-exit controls pass | [`783dbfcf0`](docs/performance/preview/internal.md) |
| Internal C++ overlap | O3 | 4.461× | 6.849× | 1,210 matched normal-exit controls pass | [`783dbfcf0`](docs/performance/preview/internal.md) |

This is one complete observation under load, using four workers and identical resource limits. The 5,614 fixture groups contain 5,724 translation units. Execution totals include process startup and appear in detail rather than as an application-runtime score.

### Adding a workload or configuration

Add a named configuration to the [workload catalog](benchmarks/catalog.json), emit [versioned correctness and comparison records](docs/performance/results-format.md), and regenerate the summary and linked tables. Tests/inputs are children of a configuration; compiling one library is counted once. Godot can add build presets and benchmark scenes using this same contract.

This preview uses pinned snapshots. Existing automatic publishers still need migration to the [shared reporting contract](docs/performance/results-format.md#producer-migration-checks). [Snapshot provenance](docs/performance/preview/sources.json).

## Caveats

- C++11 is the baseline; other dialects have only partial extension support.
- Targets are Linux x86-64 and optional Intel macOS; other architectures are unsupported.
- General inline assembly is unsupported; a few GNU forms are recognized.
- No C++ standard library is bundled; use a host-installed library.
- The compiler may contain bugs.
- Code generation may be poor for unfamiliar program shapes.

## Build and validate

The course target is Linux x86-64. Install a C++11 host compiler, GNU make,
Bash, Perl and Python 3; later checks also use binutils and a debugger.

```sh
make CXX=g++ CPPGM_HOST_CXX=g++
make test-pa8
make test-report
make inception
```

Intel macOS support and its optional checks are described in
[the macOS guide](docs/intel-macos.md).

See [ROADMAP.md](ROADMAP.md) for the assignment sequence,
[AGENTS.md](AGENTS.md) for contributor instructions, and
[TESTING_AND_REFERENCES.md](TESTING_AND_REFERENCES.md) for validation and
reference policy. Personal experiments go in `student.tests/`.

## Origins and license

The original C++ Grandmaster Certification course supplied early assignment
material, tests and starter code. The cppgm-extended authors have substantially
revised, combined, renumbered and extended that material into this independent
35-assignment course. Today's handouts, harnesses and reference implementation
describe this version; current PA numbers do not identify original CPPGM lessons.

Copyright 2026 The cppgm-extended Authors. Project-maintained code and materials
use the Apache License 2.0; inherited material retains its original notices.
See [LICENSE](LICENSE), [NOTICE](NOTICE) and [AUTHORS](AUTHORS).

The project also explores sustained agent-assisted compiler development. Its
background is in the blog series beginning with
[I Spent 2 Billion Tokens Writing a C++ Compiler So You Don't Have To](https://medium.com/@vishvananda/i-spent-2-billion-tokens-writing-a-c-compiler-so-you-dont-have-to-d3e4eec4781e).
