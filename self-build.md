# Compiler build and codegen benchmarks: AMD EPYC

Measured compiler: [`a00938892`](https://github.com/vishvananda/cppgm-extended/commit/a0093889216091a98b4ab4561769544b02092c58) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/43509588990) · [Raw rebuild results](selfhost.json)

Speed is reference CPU time divided by cppgm++ CPU time; higher is better.

CPU: AMD EPYC 9V45 96-Core Processor.

Measured compiler: a0093889216091a98b4ab4561769544b02092c58.

GCC: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0.

Clang: Ubuntu clang version 21.1.8 (6ubuntu1).

Build jobs: 4.

| Measure | vs GCC ↑ | vs Clang ↑ |
| --- | ---: | ---: |
| Building cppgm++ | 3.95× | 3.68× |
| Self-built compiler speed | 0.97× | 0.97× |

Building cppgm++ compares toolchains compiling the same sources. Self-built compiler speed compares fixed native-built and self-built cppgm++ binaries rebuilding identical inputs in paired ABBA/BAAB blocks. Every rebuild must produce identical objects and compiler binaries.

### Building the compiler

| Compiler doing the rebuild | CPU seconds |
| --- | ---: |
| GCC | 409.800 |
| Clang | 381.730 |
| GCC-built cppgm++ | 103.860 |
| Self-built cppgm++ | 107.500 |

These are the initial bootstrap builds. GCC and Clang are each measured once.

#### Paired self-built compiler measurements

| Block | GCC-built cppgm++ (s) | Self-built cppgm++ (s) | Self-built speed ↑ |
| ---: | ---: | ---: | ---: |
| 1 | 106.990 | 110.424 | 0.97× |
| 2 | 108.111 | 110.590 | 0.98× |

Each block averages its two observations per compiler. The badge uses the median block ratio; calibration and every observation are retained in the raw results.

GCC-built A/A CPU ratio: 0.987. A/A variation limits conclusions about small differences.

#### Paired codegen measurements vs Clang-built compiler

| Block | Clang-built cppgm++ (s) | Self-built cppgm++ (s) | Self-built speed ↑ |
| ---: | ---: | ---: | ---: |
| 1 | 110.321 | 112.568 | 0.98× |
| 2 | 110.292 | 114.696 | 0.96× |

Each block averages its two observations per compiler. The badge uses the median block ratio; calibration and every observation are retained in the raw results.

Clang-built A/A CPU ratio: 1.016. A/A variation limits conclusions about small differences.

[Public program results](report.md) have their own measured revision.
