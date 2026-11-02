# cppgm-extended

[![Tests](https://github.com/vishvananda/cppgm-extended/actions/workflows/tests.yml/badge.svg)](https://github.com/vishvananda/cppgm-extended/actions/workflows/tests.yml)

A C++11 compiler course in 35 cumulative assignments, from preprocessing to
native code, optimization and self-hosting. This repository contains the
complete compiler solution in `dev/`; `pa1/` through `pa35/` hold the handouts,
tests and wrappers.

Students should start with
[cppgm-assignments](https://github.com/vishvananda/cppgm-assignments), the export
containing scaffolds, handouts, tests and downloadable reference tools.
The export's references are generated from this compiler.

## Compiler Performance (Higher is Better)

| **vs GCC** | **EPYC (GitHub)** | **Intel Xeon (manual)** |
| --- | --- | --- |
| `-O0` compile speed | [![epyc `-O0` compile speed](https://img.shields.io/endpoint?url=https%3A%2F%2Fraw.githubusercontent.com%2Fvishvananda%2Fcppgm-extended%2Fbenchmark-badges%2Fepyc%2Fo0-compile.json&label=)](benchmarks/public/README.md) | [![intel `-O0` compile speed](https://img.shields.io/endpoint?url=https%3A%2F%2Fraw.githubusercontent.com%2Fvishvananda%2Fcppgm-extended%2Fbenchmark-badges%2Fintel%2Fo0-compile.json&label=)](benchmarks/public/README.md) |
| `-O0` generated code speed | [![epyc `-O0` generated code speed](https://img.shields.io/endpoint?url=https%3A%2F%2Fraw.githubusercontent.com%2Fvishvananda%2Fcppgm-extended%2Fbenchmark-badges%2Fepyc%2Fo0-generated.json&label=)](benchmarks/public/README.md) | [![intel `-O0` generated code speed](https://img.shields.io/endpoint?url=https%3A%2F%2Fraw.githubusercontent.com%2Fvishvananda%2Fcppgm-extended%2Fbenchmark-badges%2Fintel%2Fo0-generated.json&label=)](benchmarks/public/README.md) |
| `-O3` compile speed | [![epyc `-O3` compile speed](https://img.shields.io/endpoint?url=https%3A%2F%2Fraw.githubusercontent.com%2Fvishvananda%2Fcppgm-extended%2Fbenchmark-badges%2Fepyc%2Fo3-compile.json&label=)](benchmarks/public/README.md) | [![intel `-O3` compile speed](https://img.shields.io/endpoint?url=https%3A%2F%2Fraw.githubusercontent.com%2Fvishvananda%2Fcppgm-extended%2Fbenchmark-badges%2Fintel%2Fo3-compile.json&label=)](benchmarks/public/README.md) |
| `-O3` generated code speed | [![epyc `-O3` generated code speed](https://img.shields.io/endpoint?url=https%3A%2F%2Fraw.githubusercontent.com%2Fvishvananda%2Fcppgm-extended%2Fbenchmark-badges%2Fepyc%2Fo3-generated.json&label=)](benchmarks/public/README.md) | [![intel `-O3` generated code speed](https://img.shields.io/endpoint?url=https%3A%2F%2Fraw.githubusercontent.com%2Fvishvananda%2Fcppgm-extended%2Fbenchmark-badges%2Fintel%2Fo3-generated.json&label=)](benchmarks/public/README.md) |
| Building `cppgm++` | [![epyc Building `cppgm++`](https://img.shields.io/endpoint?url=https%3A%2F%2Fraw.githubusercontent.com%2Fvishvananda%2Fcppgm-extended%2Fbenchmark-badges%2Fepyc%2Fself-compile.json&label=)](benchmarks/public/README.md) | [![intel Building `cppgm++`](https://img.shields.io/endpoint?url=https%3A%2F%2Fraw.githubusercontent.com%2Fvishvananda%2Fcppgm-extended%2Fbenchmark-badges%2Fintel%2Fself-compile.json&label=)](benchmarks/public/README.md) |
| Self-built compiler speed | [![epyc Self-built compiler speed](https://img.shields.io/endpoint?url=https%3A%2F%2Fraw.githubusercontent.com%2Fvishvananda%2Fcppgm-extended%2Fbenchmark-badges%2Fepyc%2Fself-built.json&label=)](benchmarks/public/README.md) | [![intel Self-built compiler speed](https://img.shields.io/endpoint?url=https%3A%2F%2Fraw.githubusercontent.com%2Fvishvananda%2Fcppgm-extended%2Fbenchmark-badges%2Fintel%2Fself-built.json&label=)](benchmarks/public/README.md) |
| Measured commit | [![epyc Measured commit](https://img.shields.io/endpoint?url=https%3A%2F%2Fraw.githubusercontent.com%2Fvishvananda%2Fcppgm-extended%2Fbenchmark-badges%2Fepyc%2Frevision.json&label=)](benchmarks/public/README.md) | [![intel Measured commit](https://img.shields.io/endpoint?url=https%3A%2F%2Fraw.githubusercontent.com%2Fvishvananda%2Fcppgm-extended%2Fbenchmark-badges%2Fintel%2Frevision.json&label=)](benchmarks/public/README.md) |

The Intel run is optional and manually triggered. Each column retains its last
measurement; the commit row identifies the compiler it measured. CPU models,
toolchain versions and workflow links are recorded with the
[benchmark results](benchmarks/public/README.md).

## Build and validate

The course target is Linux x86-64. Install a C++11 host compiler, GNU make,
Bash, Perl and Python 3; later checks also use binutils and a debugger.

```sh
make CXX=g++ CPPGM_HOST_CXX=g++
make test-pa8
make test-report
make inception
```

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
