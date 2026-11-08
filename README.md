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

**O3 speedups; higher is better.** Colors fade from green through yellow and orange to red as speed falls.

Each cell stacks **EPYC / local or dedicated Intel / hosted Intel** badges. Labels identify the CPU; gray `n/m` means not measured. Each badge links to its own revision and details.

| Workload | Compile vs GCC | Compile vs Clang | Runtime vs GCC | Runtime vs Clang |
|---|---|---|---|---|
| Public programs | [![public O3 compile vs gcc; AMD EPYC 9V45 96-Core Processor; GitHub hosted; revision 783dbfcf01360d923ff590d2048a0c1764761cc3](docs/performance/preview/badges/matrix-public-compile-gcc-epyc.svg)](docs/performance/preview/public.md)<br>[![public O3 compile vs gcc; Intel(R) Xeon(R) CPU E5-2696 v4 @ 2.20GHz; Dedicated Intel; revision 08bba949b4a0c3f2a728bf52c6e7089a38b44797](docs/performance/preview/badges/matrix-public-compile-gcc-intel.svg)](docs/performance/preview/public-intel.md)<br>[![public O3 compile vs gcc: hosted-intel not measured](docs/performance/preview/badges/matrix-public-compile-gcc-hosted-intel.svg)](docs/performance/preview/performance.md) | [![public O3 compile vs clang; AMD EPYC 9V45 96-Core Processor; GitHub hosted; revision 783dbfcf01360d923ff590d2048a0c1764761cc3](docs/performance/preview/badges/matrix-public-compile-clang-epyc.svg)](docs/performance/preview/public.md)<br>[![public O3 compile vs clang; Intel(R) Xeon(R) CPU E5-2696 v4 @ 2.20GHz; Dedicated Intel; revision 08bba949b4a0c3f2a728bf52c6e7089a38b44797](docs/performance/preview/badges/matrix-public-compile-clang-intel.svg)](docs/performance/preview/public-intel.md)<br>[![public O3 compile vs clang: hosted-intel not measured](docs/performance/preview/badges/matrix-public-compile-clang-hosted-intel.svg)](docs/performance/preview/performance.md) | [![public O3 runtime vs gcc; AMD EPYC 9V45 96-Core Processor; GitHub hosted; revision 783dbfcf01360d923ff590d2048a0c1764761cc3](docs/performance/preview/badges/matrix-public-runtime-gcc-epyc.svg)](docs/performance/preview/public.md)<br>[![public O3 runtime vs gcc; Intel(R) Xeon(R) CPU E5-2696 v4 @ 2.20GHz; Dedicated Intel; revision 08bba949b4a0c3f2a728bf52c6e7089a38b44797](docs/performance/preview/badges/matrix-public-runtime-gcc-intel.svg)](docs/performance/preview/public-intel.md)<br>[![public O3 runtime vs gcc: hosted-intel not measured](docs/performance/preview/badges/matrix-public-runtime-gcc-hosted-intel.svg)](docs/performance/preview/performance.md) | [![public O3 runtime vs clang; AMD EPYC 9V45 96-Core Processor; GitHub hosted; revision 783dbfcf01360d923ff590d2048a0c1764761cc3](docs/performance/preview/badges/matrix-public-runtime-clang-epyc.svg)](docs/performance/preview/public.md)<br>[![public O3 runtime vs clang; Intel(R) Xeon(R) CPU E5-2696 v4 @ 2.20GHz; Dedicated Intel; revision 08bba949b4a0c3f2a728bf52c6e7089a38b44797](docs/performance/preview/badges/matrix-public-runtime-clang-intel.svg)](docs/performance/preview/public-intel.md)<br>[![public O3 runtime vs clang: hosted-intel not measured](docs/performance/preview/badges/matrix-public-runtime-clang-hosted-intel.svg)](docs/performance/preview/performance.md) |
| index-cpp | [![index O3 compile vs gcc; AMD EPYC 9V74 80-Core Processor; GitHub hosted; revision c9774d6a969d0e28c251e35f4ad9a005c6cafcc6](docs/performance/preview/badges/matrix-index-compile-gcc-epyc.svg)](docs/performance/preview/index-cpp.md)<br>[![index O3 compile vs gcc: intel not measured](docs/performance/preview/badges/matrix-index-compile-gcc-intel.svg)](docs/performance/preview/performance.md)<br>[![index O3 compile vs gcc; Intel(R) Xeon(R) Platinum 8370C CPU @ 2.80GHz; GitHub hosted; revision 7dbe6549e963e023f8fa4deca3f4e77211771e15](docs/performance/preview/badges/matrix-index-compile-gcc-hosted-intel.svg)](docs/performance/preview/index-cpp-hosted-intel.md) | [![index O3 compile vs clang; AMD EPYC 9V74 80-Core Processor; GitHub hosted; revision c9774d6a969d0e28c251e35f4ad9a005c6cafcc6](docs/performance/preview/badges/matrix-index-compile-clang-epyc.svg)](docs/performance/preview/index-cpp.md)<br>[![index O3 compile vs clang: intel not measured](docs/performance/preview/badges/matrix-index-compile-clang-intel.svg)](docs/performance/preview/performance.md)<br>[![index O3 compile vs clang; Intel(R) Xeon(R) Platinum 8370C CPU @ 2.80GHz; GitHub hosted; revision 7dbe6549e963e023f8fa4deca3f4e77211771e15](docs/performance/preview/badges/matrix-index-compile-clang-hosted-intel.svg)](docs/performance/preview/index-cpp-hosted-intel.md) | [![index O3 runtime vs gcc; AMD EPYC 9V74 80-Core Processor; GitHub hosted; revision c9774d6a969d0e28c251e35f4ad9a005c6cafcc6](docs/performance/preview/badges/matrix-index-runtime-gcc-epyc.svg)](docs/performance/preview/index-cpp.md)<br>[![index O3 runtime vs gcc: intel not measured](docs/performance/preview/badges/matrix-index-runtime-gcc-intel.svg)](docs/performance/preview/performance.md)<br>[![index O3 runtime vs gcc; Intel(R) Xeon(R) Platinum 8370C CPU @ 2.80GHz; GitHub hosted; revision 7dbe6549e963e023f8fa4deca3f4e77211771e15](docs/performance/preview/badges/matrix-index-runtime-gcc-hosted-intel.svg)](docs/performance/preview/index-cpp-hosted-intel.md) | [![index O3 runtime vs clang; AMD EPYC 9V74 80-Core Processor; GitHub hosted; revision c9774d6a969d0e28c251e35f4ad9a005c6cafcc6](docs/performance/preview/badges/matrix-index-runtime-clang-epyc.svg)](docs/performance/preview/index-cpp.md)<br>[![index O3 runtime vs clang: intel not measured](docs/performance/preview/badges/matrix-index-runtime-clang-intel.svg)](docs/performance/preview/performance.md)<br>[![index O3 runtime vs clang; Intel(R) Xeon(R) Platinum 8370C CPU @ 2.80GHz; GitHub hosted; revision 7dbe6549e963e023f8fa4deca3f4e77211771e15](docs/performance/preview/badges/matrix-index-runtime-clang-hosted-intel.svg)](docs/performance/preview/index-cpp-hosted-intel.md) |
| simdjson scalar | [![scalar O3 compile vs gcc; AMD EPYC 7763 64-Core Processor; GitHub hosted; revision 7dbe6549e963e023f8fa4deca3f4e77211771e15](docs/performance/preview/badges/matrix-scalar-compile-gcc-epyc.svg)](docs/performance/preview/simdjson.md)<br>[![scalar O3 compile vs gcc: intel not measured](docs/performance/preview/badges/matrix-scalar-compile-gcc-intel.svg)](docs/performance/preview/performance.md)<br>[![scalar O3 compile vs gcc; Intel(R) Xeon(R) Platinum 8370C CPU @ 2.80GHz; GitHub hosted; revision adb8a65ad5137658bf87d446e9fd90e8b1dac65e](docs/performance/preview/badges/matrix-scalar-compile-gcc-hosted-intel.svg)](docs/performance/preview/simdjson.md) | [![scalar O3 compile vs clang; AMD EPYC 7763 64-Core Processor; GitHub hosted; revision 7dbe6549e963e023f8fa4deca3f4e77211771e15](docs/performance/preview/badges/matrix-scalar-compile-clang-epyc.svg)](docs/performance/preview/simdjson.md)<br>[![scalar O3 compile vs clang: intel not measured](docs/performance/preview/badges/matrix-scalar-compile-clang-intel.svg)](docs/performance/preview/performance.md)<br>[![scalar O3 compile vs clang; Intel(R) Xeon(R) Platinum 8370C CPU @ 2.80GHz; GitHub hosted; revision adb8a65ad5137658bf87d446e9fd90e8b1dac65e](docs/performance/preview/badges/matrix-scalar-compile-clang-hosted-intel.svg)](docs/performance/preview/simdjson.md) | [![scalar O3 runtime vs gcc; AMD EPYC 7763 64-Core Processor; GitHub hosted; revision 7dbe6549e963e023f8fa4deca3f4e77211771e15](docs/performance/preview/badges/matrix-scalar-runtime-gcc-epyc.svg)](docs/performance/preview/simdjson.md)<br>[![scalar O3 runtime vs gcc: intel not measured](docs/performance/preview/badges/matrix-scalar-runtime-gcc-intel.svg)](docs/performance/preview/performance.md)<br>[![scalar O3 runtime vs gcc; Intel(R) Xeon(R) Platinum 8370C CPU @ 2.80GHz; GitHub hosted; revision adb8a65ad5137658bf87d446e9fd90e8b1dac65e](docs/performance/preview/badges/matrix-scalar-runtime-gcc-hosted-intel.svg)](docs/performance/preview/simdjson.md) | [![scalar O3 runtime vs clang; AMD EPYC 7763 64-Core Processor; GitHub hosted; revision 7dbe6549e963e023f8fa4deca3f4e77211771e15](docs/performance/preview/badges/matrix-scalar-runtime-clang-epyc.svg)](docs/performance/preview/simdjson.md)<br>[![scalar O3 runtime vs clang: intel not measured](docs/performance/preview/badges/matrix-scalar-runtime-clang-intel.svg)](docs/performance/preview/performance.md)<br>[![scalar O3 runtime vs clang; Intel(R) Xeon(R) Platinum 8370C CPU @ 2.80GHz; GitHub hosted; revision adb8a65ad5137658bf87d446e9fd90e8b1dac65e](docs/performance/preview/badges/matrix-scalar-runtime-clang-hosted-intel.svg)](docs/performance/preview/simdjson.md) |
| simdjson SSE | [![sse O3 compile vs gcc; AMD EPYC 7763 64-Core Processor; GitHub hosted; revision 7dbe6549e963e023f8fa4deca3f4e77211771e15](docs/performance/preview/badges/matrix-sse-compile-gcc-epyc.svg)](docs/performance/preview/simdjson.md)<br>[![sse O3 compile vs gcc: intel not measured](docs/performance/preview/badges/matrix-sse-compile-gcc-intel.svg)](docs/performance/preview/performance.md)<br>[![sse O3 compile vs gcc; Intel(R) Xeon(R) Platinum 8370C CPU @ 2.80GHz; GitHub hosted; revision adb8a65ad5137658bf87d446e9fd90e8b1dac65e](docs/performance/preview/badges/matrix-sse-compile-gcc-hosted-intel.svg)](docs/performance/preview/simdjson.md) | [![sse O3 compile vs clang; AMD EPYC 7763 64-Core Processor; GitHub hosted; revision 7dbe6549e963e023f8fa4deca3f4e77211771e15](docs/performance/preview/badges/matrix-sse-compile-clang-epyc.svg)](docs/performance/preview/simdjson.md)<br>[![sse O3 compile vs clang: intel not measured](docs/performance/preview/badges/matrix-sse-compile-clang-intel.svg)](docs/performance/preview/performance.md)<br>[![sse O3 compile vs clang; Intel(R) Xeon(R) Platinum 8370C CPU @ 2.80GHz; GitHub hosted; revision adb8a65ad5137658bf87d446e9fd90e8b1dac65e](docs/performance/preview/badges/matrix-sse-compile-clang-hosted-intel.svg)](docs/performance/preview/simdjson.md) | [![sse O3 runtime vs gcc; AMD EPYC 7763 64-Core Processor; GitHub hosted; revision 7dbe6549e963e023f8fa4deca3f4e77211771e15](docs/performance/preview/badges/matrix-sse-runtime-gcc-epyc.svg)](docs/performance/preview/simdjson.md)<br>[![sse O3 runtime vs gcc: intel not measured](docs/performance/preview/badges/matrix-sse-runtime-gcc-intel.svg)](docs/performance/preview/performance.md)<br>[![sse O3 runtime vs gcc; Intel(R) Xeon(R) Platinum 8370C CPU @ 2.80GHz; GitHub hosted; revision adb8a65ad5137658bf87d446e9fd90e8b1dac65e](docs/performance/preview/badges/matrix-sse-runtime-gcc-hosted-intel.svg)](docs/performance/preview/simdjson.md) | [![sse O3 runtime vs clang; AMD EPYC 7763 64-Core Processor; GitHub hosted; revision 7dbe6549e963e023f8fa4deca3f4e77211771e15](docs/performance/preview/badges/matrix-sse-runtime-clang-epyc.svg)](docs/performance/preview/simdjson.md)<br>[![sse O3 runtime vs clang: intel not measured](docs/performance/preview/badges/matrix-sse-runtime-clang-intel.svg)](docs/performance/preview/performance.md)<br>[![sse O3 runtime vs clang; Intel(R) Xeon(R) Platinum 8370C CPU @ 2.80GHz; GitHub hosted; revision adb8a65ad5137658bf87d446e9fd90e8b1dac65e](docs/performance/preview/badges/matrix-sse-runtime-clang-hosted-intel.svg)](docs/performance/preview/simdjson.md) |
| Internal C++ subset† | [![native O3 compile vs gcc: epyc not measured](docs/performance/preview/badges/matrix-native-compile-gcc-epyc.svg)](docs/performance/preview/performance.md)<br>[![native O3 compile vs gcc; Intel Xeon E5-2696 v4; Local shared host; revision 783dbfcf01360d923ff590d2048a0c1764761cc3](docs/performance/preview/badges/matrix-native-compile-gcc-intel.svg)](docs/performance/preview/internal.md)<br>[![native O3 compile vs gcc: hosted-intel not measured](docs/performance/preview/badges/matrix-native-compile-gcc-hosted-intel.svg)](docs/performance/preview/performance.md) | [![native O3 compile vs clang: epyc not measured](docs/performance/preview/badges/matrix-native-compile-clang-epyc.svg)](docs/performance/preview/performance.md)<br>[![native O3 compile vs clang; Intel Xeon E5-2696 v4; Local shared host; revision 783dbfcf01360d923ff590d2048a0c1764761cc3](docs/performance/preview/badges/matrix-native-compile-clang-intel.svg)](docs/performance/preview/internal.md)<br>[![native O3 compile vs clang: hosted-intel not measured](docs/performance/preview/badges/matrix-native-compile-clang-hosted-intel.svg)](docs/performance/preview/performance.md) | n/a | n/a |
| cppgm++ | [![cppgm O3 compile vs gcc; AMD EPYC 9V45 96-Core Processor; GitHub hosted; revision 783dbfcf01360d923ff590d2048a0c1764761cc3](docs/performance/preview/badges/matrix-cppgm-compile-gcc-epyc.svg)](docs/performance/preview/public.md#building-the-compiler)<br>[![cppgm O3 compile vs gcc; Intel(R) Xeon(R) CPU E5-2696 v4 @ 2.20GHz; Dedicated Intel; revision 08bba949b4a0c3f2a728bf52c6e7089a38b44797](docs/performance/preview/badges/matrix-cppgm-compile-gcc-intel.svg)](docs/performance/preview/public-intel.md#building-the-compiler)<br>[![cppgm O3 compile vs gcc: hosted-intel not measured](docs/performance/preview/badges/matrix-cppgm-compile-gcc-hosted-intel.svg)](docs/performance/preview/performance.md) | Not measured | [![cppgm O3 runtime vs gcc; AMD EPYC 9V45 96-Core Processor; GitHub hosted; revision 783dbfcf01360d923ff590d2048a0c1764761cc3; cppgm codegen: self-built versus GCC-built compiler](docs/performance/preview/badges/matrix-cppgm-runtime-gcc-epyc.svg)](docs/performance/preview/public.md#paired-self-built-compiler-measurements)<br>[![cppgm O3 runtime vs gcc; Intel(R) Xeon(R) CPU E5-2696 v4 @ 2.20GHz; Dedicated Intel; revision 08bba949b4a0c3f2a728bf52c6e7089a38b44797; cppgm codegen: self-built versus GCC-built compiler](docs/performance/preview/badges/matrix-cppgm-runtime-gcc-intel.svg)](docs/performance/preview/public-intel.md#paired-self-built-compiler-measurements)<br>[![cppgm O3 runtime vs gcc: hosted-intel not measured](docs/performance/preview/badges/matrix-cppgm-runtime-gcc-hosted-intel.svg)](docs/performance/preview/performance.md) | Not measured |

cppgm++ runtime measures **codegen performance**: self-built versus GCC-built compiler.

[O0, CPU models, revisions and per-test results](docs/performance/preview/performance.md). CPU time, except index-cpp runtime uses elapsed time. † Single-run compilation on a shared host.

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
