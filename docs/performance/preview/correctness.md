## Correctness and external builds

Results below are a preview snapshot for `783dbfcf0`. Performance rows name their own measured revision; older measurements are labeled separately.

| Suite / configuration | Result | What is checked | Evidence |
|---|---|---|---|
| Required internal suite | 9,832 / 9,832 | Stage, IR, ABI, driver and execution contracts | [CI](https://github.com/vishvananda/cppgm-extended/actions/runs/42476441247) |
| GCC C++11 profile | 3,968 / 3,968 | 2,678 acceptance; 1,290 required diagnostics; 162 link/run cases | [Scope and timings](upstream-cxx11.md) |
| Clang C++11 profile | 815 / 815 | 310 acceptance; 505 required diagnostics; compile-only | [Scope and timings](upstream-cxx11.md) |
| Internal native overlap | 5,614 groups compile; 1,210 matched controls run | cppgm, GCC and Clang, O0 and O3 | [Details and exclusions](internal.md) |
| Public programs | 81 / 81 | Builds and matching output | [Per-program results](public.md) |
| index-cpp | 50 / 50 | Original checksums, all three compilers | [Per-workload results](index-cpp.md) |
| simdjson scalar / SSE | Pass, both backends | twitter and citm canonical DOM output | [Current CI](https://github.com/vishvananda/cppgm-extended/actions/runs/42476441186) |
| Boost | Pinned advisory suites pass | Upstream build, run and rejection actions | [CI](https://github.com/vishvananda/cppgm-extended/actions/runs/42476437427) |
| Self-build and inception | Pass | Complete rebuild; identical self/inception compiler binaries | [CI](https://github.com/vishvananda/cppgm-extended/actions/runs/42476437424) |
| Student export | Pass | Exported compiler, fixtures and reference wrappers | [CI](https://github.com/vishvananda/cppgm-extended/actions/runs/42476441242) |
| Godot | Planned | Configurations and benchmark inputs not yet declared | Not measured |

Upstream counts describe selected C++11 profiles. Exclusions remain visible and are not passes. The internal native overlap is a source/object projection, separate from the complete course harness. The [original GCC audit and source dispositions](../../../reports/cxx11-final-source-review.txt) retain the broader scope limits.

