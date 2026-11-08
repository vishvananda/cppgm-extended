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

## Correctness

| Suite | Result / details |
|---|---|
| Internal assignment tests | [![Internal assignment tests: 9,832 / 9,832](docs/performance/preview/badges/tests-internal.svg)](https://github.com/vishvananda/cppgm-extended/actions/runs/42476441247) |
| GCC C++11 profile | [![GCC C++11 profile: 3,968 / 3,968](docs/performance/preview/badges/tests-gcc.svg)](docs/performance/preview/upstream-cxx11.md) |
| Clang C++11 profile | [![Clang C++11 profile: 815 / 815](docs/performance/preview/badges/tests-clang.svg)](docs/performance/preview/upstream-cxx11.md) |
| Internal C++ subset · all three compilers | [![Internal C++ subset · all three compilers: 5,614 compile; 1,210 run](docs/performance/preview/badges/tests-native.svg)](docs/performance/preview/internal.md) |
| Public programs | [![Public programs: 81 / 81](docs/performance/preview/badges/tests-public.svg)](docs/performance/preview/public.md) |
| index-cpp | [![index-cpp: 50 / 50](docs/performance/preview/badges/tests-index.svg)](docs/performance/preview/index-cpp.md) |
| simdjson · scalar + SSE | [![simdjson · scalar + SSE: passing](docs/performance/preview/badges/tests-simd.svg)](https://github.com/vishvananda/cppgm-extended/actions/runs/42476441186) |
| Boost advisory | [![Boost advisory: passing](docs/performance/preview/badges/tests-boost.svg)](https://github.com/vishvananda/cppgm-extended/actions/runs/42476437427) |
| Self-build + inception | [![Self-build + inception: passing](docs/performance/preview/badges/tests-self.svg)](https://github.com/vishvananda/cppgm-extended/actions/runs/42476437424) |
| Student export | [![Student export: passing](docs/performance/preview/badges/tests-export.svg)](https://github.com/vishvananda/cppgm-extended/actions/runs/42476441242) |
| Godot | [![Godot: planned](docs/performance/preview/badges/tests-godot.svg)](docs/performance/preview/correctness.md) |

[Coverage, exclusions and test scope](docs/performance/preview/correctness.md).

## Performance

**O3 speedups; higher is better.** Each badge links to its measured revision and details.
CPU time, except index-cpp runtime uses elapsed time.

| Workload · host | Compile vs GCC | Compile vs Clang | Runtime vs GCC | Runtime vs Clang |
|---|---:|---:|---:|---:|
| Public programs · EPYC 9V45 | [![Public programs · EPYC 9V45 O3 compile-gcc: 4.02×](docs/performance/preview/badges/public-compile-gcc.svg)](docs/performance/preview/public.md) | [![Public programs · EPYC 9V45 O3 compile-clang: 3.49×](docs/performance/preview/badges/public-compile-clang.svg)](docs/performance/preview/public.md) | [![Public programs · EPYC 9V45 O3 runtime-gcc: 0.99×](docs/performance/preview/badges/public-runtime-gcc.svg)](docs/performance/preview/public.md) | [![Public programs · EPYC 9V45 O3 runtime-clang: 1.00×](docs/performance/preview/badges/public-runtime-clang.svg)](docs/performance/preview/public.md) |
| index-cpp · EPYC 9V74 | [![index-cpp · EPYC 9V74 O3 compile-gcc: 6.76×](docs/performance/preview/badges/index-compile-gcc.svg)](docs/performance/preview/index-cpp.md) | [![index-cpp · EPYC 9V74 O3 compile-clang: 5.89×](docs/performance/preview/badges/index-compile-clang.svg)](docs/performance/preview/index-cpp.md) | [![index-cpp · EPYC 9V74 O3 runtime-gcc: 0.99×](docs/performance/preview/badges/index-runtime-gcc.svg)](docs/performance/preview/index-cpp.md) | [![index-cpp · EPYC 9V74 O3 runtime-clang: 0.99×](docs/performance/preview/badges/index-runtime-clang.svg)](docs/performance/preview/index-cpp.md) |
| simdjson scalar · EPYC 7763 | [![simdjson scalar · EPYC 7763 O3 compile-gcc: 4.47×](docs/performance/preview/badges/scalar-compile-gcc.svg)](docs/performance/preview/simdjson.md) | [![simdjson scalar · EPYC 7763 O3 compile-clang: 2.33×](docs/performance/preview/badges/scalar-compile-clang.svg)](docs/performance/preview/simdjson.md) | [![simdjson scalar · EPYC 7763 O3 runtime-gcc: 0.81×](docs/performance/preview/badges/scalar-runtime-gcc.svg)](docs/performance/preview/simdjson.md) | [![simdjson scalar · EPYC 7763 O3 runtime-clang: 0.80×](docs/performance/preview/badges/scalar-runtime-clang.svg)](docs/performance/preview/simdjson.md) |
| simdjson SSE · EPYC 7763 | [![simdjson SSE · EPYC 7763 O3 compile-gcc: 4.49×](docs/performance/preview/badges/sse-compile-gcc.svg)](docs/performance/preview/simdjson.md) | [![simdjson SSE · EPYC 7763 O3 compile-clang: 2.56×](docs/performance/preview/badges/sse-compile-clang.svg)](docs/performance/preview/simdjson.md) | [![simdjson SSE · EPYC 7763 O3 runtime-gcc: 0.90×](docs/performance/preview/badges/sse-runtime-gcc.svg)](docs/performance/preview/simdjson.md) | [![simdjson SSE · EPYC 7763 O3 runtime-clang: 0.89×](docs/performance/preview/badges/sse-runtime-clang.svg)](docs/performance/preview/simdjson.md) |
| Internal C++ subset† · Xeon E5-2696 v4 | [![Internal C++ subset† · Xeon E5-2696 v4 O3 compile-gcc: 4.46×](docs/performance/preview/badges/native-compile-gcc.svg)](docs/performance/preview/internal.md) | [![Internal C++ subset† · Xeon E5-2696 v4 O3 compile-clang: 6.85×](docs/performance/preview/badges/native-compile-clang.svg)](docs/performance/preview/internal.md) | n/a | n/a |
| Build cppgm++ · EPYC 9V45 | [![Build cppgm++ · EPYC 9V45 O3 compile-gcc: 4.24×](docs/performance/preview/badges/build-compile-gcc.svg)](docs/performance/preview/public.md#building-the-compiler) | Not measured | n/a | n/a |

Self-built compiler speed · EPYC 9V45: [![Self-built versus GCC-built cppgm: 0.97×](docs/performance/preview/badges/inception.svg)](docs/performance/preview/public.md#paired-self-built-compiler-measurements) vs GCC-built cppgm.

[O0, Intel, per-test results and methodology](docs/performance/preview/performance.md). † Internal compilation is a single-run observation on a shared host.

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
