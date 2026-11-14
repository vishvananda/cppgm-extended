# Compiler build and codegen benchmarks: Intel (GitHub)

Measured compiler: [`1597e573a`](https://github.com/vishvananda/cppgm-extended/commit/1597e573a4882a000810eb15f86d4cb230ce4afd) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/43572026801) · [Raw rebuild results](selfhost.json)

Speed is reference CPU time divided by cppgm++ CPU time; higher is better.

CPU: INTEL(R) XEON(R) PLATINUM 8573C.

Measured compiler: 1597e573a4882a000810eb15f86d4cb230ce4afd.

GCC: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0.

Clang: Ubuntu clang version 21.1.8 (6ubuntu1).

Build jobs: 4.

| Measure | vs GCC ↑ | vs Clang ↑ |
| --- | ---: | ---: |
| Building cppgm++ | 4.06× | 3.59× |
| Self-built compiler speed | 0.99× | 0.99× |

Building cppgm++ compares toolchains compiling the same sources. Self-built compiler speed compares fixed native-built and self-built cppgm++ binaries rebuilding identical inputs in paired ABBA/BAAB blocks. Every rebuild must produce identical objects and compiler binaries.

### Building the compiler

| Compiler doing the rebuild | CPU seconds |
| --- | ---: |
| GCC | 636.800 |
| Clang | 562.680 |
| GCC-built cppgm++ | 156.790 |
| Self-built cppgm++ | 159.410 |

These are the initial bootstrap builds. GCC and Clang are each measured once.

#### Paired self-built compiler measurements

| Block | GCC-built cppgm++ (s) | Self-built cppgm++ (s) | Self-built speed ↑ |
| ---: | ---: | ---: | ---: |
| 1 | 155.501 | 157.768 | 0.99× |
| 2 | 156.105 | 156.897 | 0.99× |

Each block averages its two observations per compiler. The badge uses the median block ratio; calibration and every observation are retained in the raw results.

GCC-built A/A CPU ratio: 1.006. A/A variation limits conclusions about small differences.

#### Paired codegen measurements vs Clang-built compiler

| Block | Clang-built cppgm++ (s) | Self-built cppgm++ (s) | Self-built speed ↑ |
| ---: | ---: | ---: | ---: |
| 1 | 156.370 | 158.747 | 0.99× |
| 2 | 158.318 | 159.557 | 0.99× |

Each block averages its two observations per compiler. The badge uses the median block ratio; calibration and every observation are retained in the raw results.

Clang-built A/A CPU ratio: 0.997. A/A variation limits conclusions about small differences.

[Public program results](report.md) have their own measured revision.
