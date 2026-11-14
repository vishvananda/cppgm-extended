# Compiler build and codegen benchmarks: AMD EPYC

Measured compiler: [`9ed2f1eea`](https://github.com/vishvananda/cppgm-extended/commit/9ed2f1eea641727dd56efede0b4ca09c045e9f7f) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/43601267037) · [Raw rebuild results](selfhost.json)

Speed is reference CPU time divided by cppgm++ CPU time; higher is better.

CPU: AMD EPYC 7763 64-Core Processor.

Measured compiler: 9ed2f1eea641727dd56efede0b4ca09c045e9f7f.

GCC: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0.

Clang: Ubuntu clang version 21.1.8 (6ubuntu1).

Build jobs: 4.

| Measure | vs GCC ↑ | vs Clang ↑ |
| --- | ---: | ---: |
| Building cppgm++ | 4.02× | 4.42× |
| Self-built compiler speed | 0.99× | 0.99× |

Building cppgm++ compares toolchains compiling the same sources. Self-built compiler speed compares fixed native-built and self-built cppgm++ binaries rebuilding identical inputs in paired ABBA/BAAB blocks. Every rebuild must produce identical objects and compiler binaries.

### Building the compiler

| Compiler doing the rebuild | CPU seconds |
| --- | ---: |
| GCC | 710.280 |
| Clang | 781.060 |
| GCC-built cppgm++ | 176.560 |
| Self-built cppgm++ | 183.710 |

These are the initial bootstrap builds. GCC and Clang are each measured once.

#### Paired self-built compiler measurements

| Block | GCC-built cppgm++ (s) | Self-built cppgm++ (s) | Self-built speed ↑ |
| ---: | ---: | ---: | ---: |
| 1 | 178.843 | 180.753 | 0.99× |
| 2 | 177.463 | 179.632 | 0.99× |

Each block averages its two observations per compiler. The badge uses the median block ratio; calibration and every observation are retained in the raw results.

GCC-built A/A CPU ratio: 1.001. A/A variation limits conclusions about small differences.

#### Paired codegen measurements vs Clang-built compiler

| Block | Clang-built cppgm++ (s) | Self-built cppgm++ (s) | Self-built speed ↑ |
| ---: | ---: | ---: | ---: |
| 1 | 179.952 | 181.590 | 0.99× |
| 2 | 180.266 | 182.011 | 0.99× |

Each block averages its two observations per compiler. The badge uses the median block ratio; calibration and every observation are retained in the raw results.

Clang-built A/A CPU ratio: 1.005. A/A variation limits conclusions about small differences.

[Public program results](report.md) have their own measured revision.
