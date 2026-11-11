# Compiler build and codegen benchmarks: Intel Xeon (dedicated)

Measured compiler: [`6c589867b`](https://github.com/vishvananda/cppgm-extended/commit/6c589867bdd5c99bc35b90267a46778f59e1127e) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/42682028781) · [Raw rebuild results](selfhost.json)

Speed is reference CPU time divided by cppgm++ CPU time; higher is better.

CPU: Intel(R) Xeon(R) CPU E5-2696 v4 @ 2.20GHz.

Measured compiler: 6c589867bdd5c99bc35b90267a46778f59e1127e.

GCC: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0.

Clang: Ubuntu clang version 21.1.8 (6ubuntu1).

Build jobs: 16.

| Measure | vs GCC ↑ | vs Clang ↑ |
| --- | ---: | ---: |
| Building cppgm++ | 4.21× | 3.94× |
| Self-built compiler speed | 0.98× | 1.00× |

Building cppgm++ compares toolchains compiling the same sources. Self-built compiler speed compares fixed native-built and self-built cppgm++ binaries rebuilding identical inputs in paired ABBA/BAAB blocks. Every rebuild must produce identical objects and compiler binaries.

### Building the compiler

| Compiler doing the rebuild | CPU seconds |
| --- | ---: |
| GCC | 562.320 |
| Clang | 527.040 |
| GCC-built cppgm++ | 133.650 |
| Self-built cppgm++ | 137.220 |

These are the initial bootstrap builds. GCC and Clang are each measured once.

#### Paired self-built compiler measurements

| Block | GCC-built cppgm++ (s) | Self-built cppgm++ (s) | Self-built speed ↑ |
| ---: | ---: | ---: | ---: |
| 1 | 135.158 | 137.136 | 0.99× |
| 2 | 134.728 | 137.476 | 0.98× |

Each block averages its two observations per compiler. The badge uses the median block ratio; calibration and every observation are retained in the raw results.

GCC-built A/A CPU ratio: 0.997. A/A variation limits conclusions about small differences.

#### Paired codegen measurements vs Clang-built compiler

| Block | Clang-built cppgm++ (s) | Self-built cppgm++ (s) | Self-built speed ↑ |
| ---: | ---: | ---: | ---: |
| 1 | 135.268 | 133.939 | 1.01× |
| 2 | 134.565 | 134.801 | 1.00× |

Each block averages its two observations per compiler. The badge uses the median block ratio; calibration and every observation are retained in the raw results.

Clang-built A/A CPU ratio: 0.995. A/A variation limits conclusions about small differences.

[Public program results](report.md) have their own measured revision.
