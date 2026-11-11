# Compiler build and codegen benchmarks: AMD EPYC

Measured compiler: [`1ddd7507e`](https://github.com/vishvananda/cppgm-extended/commit/1ddd7507ee861649d43d1e72dcde7bb17a85a691) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/42988175107) · [Raw rebuild results](selfhost.json)

Speed is reference CPU time divided by cppgm++ CPU time; higher is better.

CPU: AMD EPYC 9V74 80-Core Processor.

Measured compiler: 1ddd7507ee861649d43d1e72dcde7bb17a85a691.

GCC: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0.

Clang: Ubuntu clang version 21.1.8 (6ubuntu1).

Build jobs: 4.

| Measure | vs GCC ↑ | vs Clang ↑ |
| --- | ---: | ---: |
| Building cppgm++ | 3.99× | 3.94× |
| Self-built compiler speed | 0.97× | 1.00× |

Building cppgm++ compares toolchains compiling the same sources. Self-built compiler speed compares fixed native-built and self-built cppgm++ binaries rebuilding identical inputs in paired ABBA/BAAB blocks. Every rebuild must produce identical objects and compiler binaries.

### Building the compiler

| Compiler doing the rebuild | CPU seconds |
| --- | ---: |
| GCC | 424.110 |
| Clang | 419.150 |
| GCC-built cppgm++ | 106.300 |
| Self-built cppgm++ | 111.220 |

These are the initial bootstrap builds. GCC and Clang are each measured once.

#### Paired self-built compiler measurements

| Block | GCC-built cppgm++ (s) | Self-built cppgm++ (s) | Self-built speed ↑ |
| ---: | ---: | ---: | ---: |
| 1 | 106.338 | 109.666 | 0.97× |
| 2 | 106.517 | 110.260 | 0.97× |

Each block averages its two observations per compiler. The badge uses the median block ratio; calibration and every observation are retained in the raw results.

GCC-built A/A CPU ratio: 1.000. A/A variation limits conclusions about small differences.

#### Paired codegen measurements vs Clang-built compiler

| Block | Clang-built cppgm++ (s) | Self-built cppgm++ (s) | Self-built speed ↑ |
| ---: | ---: | ---: | ---: |
| 1 | 110.347 | 109.598 | 1.01× |
| 2 | 109.858 | 109.711 | 1.00× |

Each block averages its two observations per compiler. The badge uses the median block ratio; calibration and every observation are retained in the raw results.

Clang-built A/A CPU ratio: 0.999. A/A variation limits conclusions about small differences.

[Public program results](report.md) have their own measured revision.
