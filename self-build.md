# Compiler build and codegen benchmarks: AMD EPYC

Measured compiler: [`72d294c91`](https://github.com/vishvananda/cppgm-extended/commit/72d294c916f0f3f43e50e812f7efa91937eb9b66) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/43348824651) · [Raw rebuild results](selfhost.json)

Speed is reference CPU time divided by cppgm++ CPU time; higher is better.

CPU: AMD EPYC 9V45 96-Core Processor.

Measured compiler: 72d294c916f0f3f43e50e812f7efa91937eb9b66.

GCC: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0.

Clang: Ubuntu clang version 21.1.8 (6ubuntu1).

Build jobs: 4.

| Measure | vs GCC ↑ | vs Clang ↑ |
| --- | ---: | ---: |
| Building cppgm++ | 4.19× | 3.87× |
| Self-built compiler speed | 0.98× | 0.97× |

Building cppgm++ compares toolchains compiling the same sources. Self-built compiler speed compares fixed native-built and self-built cppgm++ binaries rebuilding identical inputs in paired ABBA/BAAB blocks. Every rebuild must produce identical objects and compiler binaries.

### Building the compiler

| Compiler doing the rebuild | CPU seconds |
| --- | ---: |
| GCC | 341.250 |
| Clang | 314.940 |
| GCC-built cppgm++ | 81.450 |
| Self-built cppgm++ | 88.020 |

These are the initial bootstrap builds. GCC and Clang are each measured once.

#### Paired self-built compiler measurements

| Block | GCC-built cppgm++ (s) | Self-built cppgm++ (s) | Self-built speed ↑ |
| ---: | ---: | ---: | ---: |
| 1 | 81.342 | 82.748 | 0.98× |
| 2 | 80.456 | 82.407 | 0.98× |

Each block averages its two observations per compiler. The badge uses the median block ratio; calibration and every observation are retained in the raw results.

GCC-built A/A CPU ratio: 0.996. A/A variation limits conclusions about small differences.

#### Paired codegen measurements vs Clang-built compiler

| Block | Clang-built cppgm++ (s) | Self-built cppgm++ (s) | Self-built speed ↑ |
| ---: | ---: | ---: | ---: |
| 1 | 80.191 | 81.955 | 0.98× |
| 2 | 81.169 | 83.572 | 0.97× |

Each block averages its two observations per compiler. The badge uses the median block ratio; calibration and every observation are retained in the raw results.

Clang-built A/A CPU ratio: 1.005. A/A variation limits conclusions about small differences.

[Public program results](report.md) have their own measured revision.
