# cppgm-extended

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

## Compiler Performance (Higher is Better)

| **vs GCC** | **EPYC** | **Intel Xeon** |
| --- | --- | --- |
| `-O0` compile speed | [<img alt="epyc `-O0` compile speed" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/epyc/o0-compile.svg">](benchmarks/public/README.md) | [<img alt="intel `-O0` compile speed" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/intel/o0-compile.svg">](benchmarks/public/README.md) |
| `-O0` generated code speed | [<img alt="epyc `-O0` generated code speed" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/epyc/o0-generated.svg">](benchmarks/public/README.md) | [<img alt="intel `-O0` generated code speed" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/intel/o0-generated.svg">](benchmarks/public/README.md) |
| `-O3` compile speed | [<img alt="epyc `-O3` compile speed" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/epyc/o3-compile.svg">](benchmarks/public/README.md) | [<img alt="intel `-O3` compile speed" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/intel/o3-compile.svg">](benchmarks/public/README.md) |
| `-O3` generated code speed | [<img alt="epyc `-O3` generated code speed" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/epyc/o3-generated.svg">](benchmarks/public/README.md) | [<img alt="intel `-O3` generated code speed" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/intel/o3-generated.svg">](benchmarks/public/README.md) |
| Building `cppgm++` | [<img alt="epyc Building `cppgm++`" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/epyc/self-compile.svg">](benchmarks/public/README.md) | [<img alt="intel Building `cppgm++`" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/intel/self-compile.svg">](benchmarks/public/README.md) |
| Self-built compiler speed | [<img alt="epyc Self-built compiler speed" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/epyc/self-built.svg">](benchmarks/public/README.md) | [<img alt="intel Self-built compiler speed" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/intel/self-built.svg">](benchmarks/public/README.md) |
| `-O3` timed programs | [<img alt="epyc O3 timed programs" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/epyc/o3-programs.svg">](benchmarks/public/README.md) | [<img alt="intel O3 timed programs" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/intel/o3-programs.svg">](benchmarks/public/README.md) |
| Measured commit | [<img alt="epyc Measured commit" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/epyc/revision.svg">](benchmarks/public/README.md) | [<img alt="intel Measured commit" height="24" src="https://raw.githubusercontent.com/vishvananda/cppgm-extended/benchmark-badges/intel/revision.svg">](benchmarks/public/README.md) |

See [benchmark methodology](benchmarks/public/README.md) for how the numbers are calculated.
Detailed compile timings: [EPYC](https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/epyc/report.md)
and [Intel Xeon](https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/intel/report.md).

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
