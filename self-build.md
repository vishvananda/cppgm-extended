# Compiler build and codegen benchmarks: AMD EPYC

Measured compiler: [`a0ff7d04a`](https://github.com/vishvananda/cppgm-extended/commit/a0ff7d04a7ed4d67461b0912fca4a4fa7e055d74) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/42974658211) · [Raw rebuild results](selfhost.json)

Speed is reference CPU time divided by cppgm++ CPU time; higher is better.

CPU: AMD EPYC 9V45 96-Core Processor.

Measured compiler: a0ff7d04a7ed4d67461b0912fca4a4fa7e055d74.

GCC: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0.

Clang: Ubuntu clang version 21.1.8 (6ubuntu1).

Build jobs: 4.

| Measure | vs GCC ↑ | vs Clang ↑ |
| --- | ---: | ---: |
| Building cppgm++ | 4.02× | 3.57× |
| Self-built compiler speed | 0.97× | 1.01× |

Building cppgm++ compares toolchains compiling the same sources. Self-built compiler speed compares fixed native-built and self-built cppgm++ binaries rebuilding identical inputs in paired ABBA/BAAB blocks. Every rebuild must produce identical objects and compiler binaries.

### Building the compiler

| Compiler doing the rebuild | CPU seconds |
| --- | ---: |
| GCC | 328.180 |
| Clang | 291.340 |
| GCC-built cppgm++ | 81.640 |
| Self-built cppgm++ | 84.950 |

These are the initial bootstrap builds. GCC and Clang are each measured once.

#### Paired self-built compiler measurements

| Block | GCC-built cppgm++ (s) | Self-built cppgm++ (s) | Self-built speed ↑ |
| ---: | ---: | ---: | ---: |
| 1 | 94.360 | 97.612 | 0.97× |
| 2 | 94.825 | 96.504 | 0.98× |

Each block averages its two observations per compiler. The badge uses the median block ratio; calibration and every observation are retained in the raw results.

GCC-built A/A CPU ratio: 1.027. A/A variation limits conclusions about small differences.

#### Paired codegen measurements vs Clang-built compiler

| Block | Clang-built cppgm++ (s) | Self-built cppgm++ (s) | Self-built speed ↑ |
| ---: | ---: | ---: | ---: |
| 1 | 83.679 | 83.326 | 1.00× |
| 2 | 83.790 | 83.286 | 1.01× |

Each block averages its two observations per compiler. The badge uses the median block ratio; calibration and every observation are retained in the raw results.

Clang-built A/A CPU ratio: 1.023. A/A variation limits conclusions about small differences.

[Public program results](report.md) have their own measured revision.
