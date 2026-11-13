# Compiler build and codegen benchmarks: AMD EPYC

Measured compiler: [`dc9084330`](https://github.com/vishvananda/cppgm-extended/commit/dc908433031b1e72d07206262cb240cdedc9d13b) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/43395345084) · [Raw rebuild results](selfhost.json)

Speed is reference CPU time divided by cppgm++ CPU time; higher is better.

CPU: AMD EPYC 9V74 80-Core Processor.

Measured compiler: dc908433031b1e72d07206262cb240cdedc9d13b.

GCC: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0.

Clang: Ubuntu clang version 21.1.8 (6ubuntu1).

Build jobs: 4.

| Measure | vs GCC ↑ | vs Clang ↑ |
| --- | ---: | ---: |
| Building cppgm++ | 4.14× | 3.91× |
| Self-built compiler speed | 0.98× | 0.99× |

Building cppgm++ compares toolchains compiling the same sources. Self-built compiler speed compares fixed native-built and self-built cppgm++ binaries rebuilding identical inputs in paired ABBA/BAAB blocks. Every rebuild must produce identical objects and compiler binaries.

### Building the compiler

| Compiler doing the rebuild | CPU seconds |
| --- | ---: |
| GCC | 539.330 |
| Clang | 508.760 |
| GCC-built cppgm++ | 130.280 |
| Self-built cppgm++ | 134.930 |

These are the initial bootstrap builds. GCC and Clang are each measured once.

#### Paired self-built compiler measurements

| Block | GCC-built cppgm++ (s) | Self-built cppgm++ (s) | Self-built speed ↑ |
| ---: | ---: | ---: | ---: |
| 1 | 125.968 | 129.148 | 0.98× |
| 2 | 126.353 | 129.349 | 0.98× |

Each block averages its two observations per compiler. The badge uses the median block ratio; calibration and every observation are retained in the raw results.

GCC-built A/A CPU ratio: 0.996. A/A variation limits conclusions about small differences.

#### Paired codegen measurements vs Clang-built compiler

| Block | Clang-built cppgm++ (s) | Self-built cppgm++ (s) | Self-built speed ↑ |
| ---: | ---: | ---: | ---: |
| 1 | 128.892 | 128.311 | 1.00× |
| 2 | 130.884 | 132.892 | 0.98× |

Each block averages its two observations per compiler. The badge uses the median block ratio; calibration and every observation are retained in the raw results.

Clang-built A/A CPU ratio: 1.008. A/A variation limits conclusions about small differences.

[Public program results](report.md) have their own measured revision.
