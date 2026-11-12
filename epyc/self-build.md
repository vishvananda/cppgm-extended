# Compiler build and codegen benchmarks: AMD EPYC

Measured compiler: [`e2831a513`](https://github.com/vishvananda/cppgm-extended/commit/e2831a513b668c602faa8098e55eb22ee08a670f) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/43133898284) · [Raw rebuild results](selfhost.json)

Speed is reference CPU time divided by cppgm++ CPU time; higher is better.

CPU: AMD EPYC 7763 64-Core Processor.

Measured compiler: e2831a513b668c602faa8098e55eb22ee08a670f.

GCC: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0.

Clang: Ubuntu clang version 21.1.8 (6ubuntu1).

Build jobs: 4.

| Measure | vs GCC ↑ | vs Clang ↑ |
| --- | ---: | ---: |
| Building cppgm++ | 4.11× | 4.40× |
| Self-built compiler speed | 0.98× | 0.98× |

Building cppgm++ compares toolchains compiling the same sources. Self-built compiler speed compares fixed native-built and self-built cppgm++ binaries rebuilding identical inputs in paired ABBA/BAAB blocks. Every rebuild must produce identical objects and compiler binaries.

### Building the compiler

| Compiler doing the rebuild | CPU seconds |
| --- | ---: |
| GCC | 586.870 |
| Clang | 628.840 |
| GCC-built cppgm++ | 142.930 |
| Self-built cppgm++ | 147.770 |

These are the initial bootstrap builds. GCC and Clang are each measured once.

#### Paired self-built compiler measurements

| Block | GCC-built cppgm++ (s) | Self-built cppgm++ (s) | Self-built speed ↑ |
| ---: | ---: | ---: | ---: |
| 1 | 142.859 | 146.721 | 0.97× |
| 2 | 143.297 | 146.614 | 0.98× |

Each block averages its two observations per compiler. The badge uses the median block ratio; calibration and every observation are retained in the raw results.

GCC-built A/A CPU ratio: 1.004. A/A variation limits conclusions about small differences.

#### Paired codegen measurements vs Clang-built compiler

| Block | Clang-built cppgm++ (s) | Self-built cppgm++ (s) | Self-built speed ↑ |
| ---: | ---: | ---: | ---: |
| 1 | 144.367 | 147.228 | 0.98× |
| 2 | 144.744 | 147.022 | 0.98× |

Each block averages its two observations per compiler. The badge uses the median block ratio; calibration and every observation are retained in the raw results.

Clang-built A/A CPU ratio: 0.997. A/A variation limits conclusions about small differences.

[Public program results](report.md) have their own measured revision.
