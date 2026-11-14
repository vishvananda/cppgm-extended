# Compiler build and codegen benchmarks: Intel (GitHub)

Measured compiler: [`01bac6195`](https://github.com/vishvananda/cppgm-extended/commit/01bac61953a982df28933dfde0fa03eb65838ea4) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/43559814532) · [Raw rebuild results](selfhost.json)

Speed is reference CPU time divided by cppgm++ CPU time; higher is better.

CPU: Intel(R) Xeon(R) 6975P-C.

Measured compiler: 01bac61953a982df28933dfde0fa03eb65838ea4.

GCC: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0.

Clang: Ubuntu clang version 21.1.8 (6ubuntu1).

Build jobs: 4.

| Measure | vs GCC ↑ | vs Clang ↑ |
| --- | ---: | ---: |
| Building cppgm++ | 4.17× | 3.68× |
| Self-built compiler speed | 0.98× | 0.99× |

Building cppgm++ compares toolchains compiling the same sources. Self-built compiler speed compares fixed native-built and self-built cppgm++ binaries rebuilding identical inputs in paired ABBA/BAAB blocks. Every rebuild must produce identical objects and compiler binaries.

### Building the compiler

| Compiler doing the rebuild | CPU seconds |
| --- | ---: |
| GCC | 462.850 |
| Clang | 408.010 |
| GCC-built cppgm++ | 110.870 |
| Self-built cppgm++ | 112.650 |

These are the initial bootstrap builds. GCC and Clang are each measured once.

#### Paired self-built compiler measurements

| Block | GCC-built cppgm++ (s) | Self-built cppgm++ (s) | Self-built speed ↑ |
| ---: | ---: | ---: | ---: |
| 1 | 109.186 | 110.978 | 0.98× |
| 2 | 110.415 | 112.405 | 0.98× |

Each block averages its two observations per compiler. The badge uses the median block ratio; calibration and every observation are retained in the raw results.

GCC-built A/A CPU ratio: 1.016. A/A variation limits conclusions about small differences.

#### Paired codegen measurements vs Clang-built compiler

| Block | Clang-built cppgm++ (s) | Self-built cppgm++ (s) | Self-built speed ↑ |
| ---: | ---: | ---: | ---: |
| 1 | 111.085 | 111.713 | 0.99× |
| 2 | 111.893 | 112.666 | 0.99× |

Each block averages its two observations per compiler. The badge uses the median block ratio; calibration and every observation are retained in the raw results.

Clang-built A/A CPU ratio: 1.006. A/A variation limits conclusions about small differences.

[Public program results](report.md) have their own measured revision.
