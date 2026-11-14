# Compiler build and codegen benchmarks: Intel (GitHub)

Measured compiler: [`a649ddb21`](https://github.com/vishvananda/cppgm-extended/commit/a649ddb21dd810338ac87d93e7d2e713d0f98ab1) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/43609274618) · [Raw rebuild results](selfhost.json)

Speed is reference CPU time divided by cppgm++ CPU time; higher is better.

CPU: Intel(R) Xeon(R) 6975P-C.

Measured compiler: a649ddb21dd810338ac87d93e7d2e713d0f98ab1.

GCC: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0.

Clang: Ubuntu clang version 21.1.8 (6ubuntu1).

Build jobs: 4.

| Measure | vs GCC ↑ | vs Clang ↑ |
| --- | ---: | ---: |
| Building cppgm++ | 4.20× | 3.73× |
| Self-built compiler speed | 0.98× | 0.99× |

Building cppgm++ compares toolchains compiling the same sources. Self-built compiler speed compares fixed native-built and self-built cppgm++ binaries rebuilding identical inputs in paired ABBA/BAAB blocks. Every rebuild must produce identical objects and compiler binaries.

### Building the compiler

| Compiler doing the rebuild | CPU seconds |
| --- | ---: |
| GCC | 458.050 |
| Clang | 406.320 |
| GCC-built cppgm++ | 108.990 |
| Self-built cppgm++ | 112.000 |

These are the initial bootstrap builds. GCC and Clang are each measured once.

#### Paired self-built compiler measurements

| Block | GCC-built cppgm++ (s) | Self-built cppgm++ (s) | Self-built speed ↑ |
| ---: | ---: | ---: | ---: |
| 1 | 108.037 | 110.297 | 0.98× |
| 2 | 108.057 | 109.658 | 0.99× |

Each block averages its two observations per compiler. The badge uses the median block ratio; calibration and every observation are retained in the raw results.

GCC-built A/A CPU ratio: 0.995. A/A variation limits conclusions about small differences.

#### Paired codegen measurements vs Clang-built compiler

| Block | Clang-built cppgm++ (s) | Self-built cppgm++ (s) | Self-built speed ↑ |
| ---: | ---: | ---: | ---: |
| 1 | 108.586 | 109.840 | 0.99× |
| 2 | 109.920 | 110.199 | 1.00× |

Each block averages its two observations per compiler. The badge uses the median block ratio; calibration and every observation are retained in the raw results.

Clang-built A/A CPU ratio: 1.000. A/A variation limits conclusions about small differences.

[Public program results](report.md) have their own measured revision.
