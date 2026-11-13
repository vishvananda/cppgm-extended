# Compiler build and codegen benchmarks: AMD EPYC

Measured compiler: [`393d1b1d0`](https://github.com/vishvananda/cppgm-extended/commit/393d1b1d09089ffdade676f4e7cbad8759c15d2a) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/43357224913) · [Raw rebuild results](selfhost.json)

Speed is reference CPU time divided by cppgm++ CPU time; higher is better.

CPU: AMD EPYC 9V74 80-Core Processor.

Measured compiler: 393d1b1d09089ffdade676f4e7cbad8759c15d2a.

GCC: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0.

Clang: Ubuntu clang version 21.1.8 (6ubuntu1).

Build jobs: 4.

| Measure | vs GCC ↑ | vs Clang ↑ |
| --- | ---: | ---: |
| Building cppgm++ | 4.05× | 3.90× |
| Self-built compiler speed | 0.97× | 0.99× |

Building cppgm++ compares toolchains compiling the same sources. Self-built compiler speed compares fixed native-built and self-built cppgm++ binaries rebuilding identical inputs in paired ABBA/BAAB blocks. Every rebuild must produce identical objects and compiler binaries.

### Building the compiler

| Compiler doing the rebuild | CPU seconds |
| --- | ---: |
| GCC | 531.110 |
| Clang | 512.190 |
| GCC-built cppgm++ | 131.170 |
| Self-built cppgm++ | 136.160 |

These are the initial bootstrap builds. GCC and Clang are each measured once.

#### Paired self-built compiler measurements

| Block | GCC-built cppgm++ (s) | Self-built cppgm++ (s) | Self-built speed ↑ |
| ---: | ---: | ---: | ---: |
| 1 | 128.065 | 131.001 | 0.98× |
| 2 | 128.712 | 133.684 | 0.96× |

Each block averages its two observations per compiler. The badge uses the median block ratio; calibration and every observation are retained in the raw results.

GCC-built A/A CPU ratio: 0.996. A/A variation limits conclusions about small differences.

#### Paired codegen measurements vs Clang-built compiler

| Block | Clang-built cppgm++ (s) | Self-built cppgm++ (s) | Self-built speed ↑ |
| ---: | ---: | ---: | ---: |
| 1 | 132.621 | 132.883 | 1.00× |
| 2 | 132.185 | 134.778 | 0.98× |

Each block averages its two observations per compiler. The badge uses the median block ratio; calibration and every observation are retained in the raw results.

Clang-built A/A CPU ratio: 0.997. A/A variation limits conclusions about small differences.

[Public program results](report.md) have their own measured revision.
