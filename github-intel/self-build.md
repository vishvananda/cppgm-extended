# Compiler build and codegen benchmarks: Intel (GitHub)

Measured compiler: [`8287f480c`](https://github.com/vishvananda/cppgm-extended/commit/8287f480c7b5bdb2074c8941689aa36f9c233bd7) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798155) · [Raw rebuild results](selfhost.json)

Speed is reference CPU time divided by cppgm++ CPU time; higher is better.

CPU: Intel(R) Xeon(R) Platinum 8370C CPU @ 2.80GHz.

Measured compiler: 8287f480c7b5bdb2074c8941689aa36f9c233bd7.

GCC: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0.

Clang: Ubuntu clang version 21.1.8 (6ubuntu1).

Build jobs: 4.

| Measure | vs GCC ↑ | vs Clang ↑ |
| --- | ---: | ---: |
| Building cppgm++ | 4.25× | 3.93× |
| Self-built compiler speed | 0.98× | 1.01× |

Building cppgm++ compares toolchains compiling the same sources. Self-built compiler speed compares fixed native-built and self-built cppgm++ binaries rebuilding identical inputs in paired ABBA/BAAB blocks. Every rebuild must produce identical objects and compiler binaries.

### Building the compiler

| Compiler doing the rebuild | CPU seconds |
| --- | ---: |
| GCC | 553.440 |
| Clang | 511.150 |
| GCC-built cppgm++ | 130.110 |
| Self-built cppgm++ | 133.930 |

These are the initial bootstrap builds. GCC and Clang are each measured once.

#### Paired self-built compiler measurements

| Block | GCC-built cppgm++ (s) | Self-built cppgm++ (s) | Self-built speed ↑ |
| ---: | ---: | ---: | ---: |
| 1 | 129.694 | 132.746 | 0.98× |
| 2 | 129.831 | 132.408 | 0.98× |

Each block averages its two observations per compiler. The badge uses the median block ratio; calibration and every observation are retained in the raw results.

GCC-built A/A CPU ratio: 1.004. A/A variation limits conclusions about small differences.

#### Paired codegen measurements vs Clang-built compiler

| Block | Clang-built cppgm++ (s) | Self-built cppgm++ (s) | Self-built speed ↑ |
| ---: | ---: | ---: | ---: |
| 1 | 134.066 | 132.528 | 1.01× |
| 2 | 134.260 | 132.307 | 1.01× |

Each block averages its two observations per compiler. The badge uses the median block ratio; calibration and every observation are retained in the raw results.

Clang-built A/A CPU ratio: 1.000. A/A variation limits conclusions about small differences.

[Public program results](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798155) have their own measured revision.
