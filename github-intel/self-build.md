# Compiler build and codegen benchmarks: Intel (GitHub)

Measured compiler: [`b81455bc7`](https://github.com/vishvananda/cppgm-extended/commit/b81455bc7d42635d12519c11c3788a20b5fad2f4) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/43493408998) · [Raw rebuild results](selfhost.json)

Speed is reference CPU time divided by cppgm++ CPU time; higher is better.

CPU: Intel(R) Xeon(R) 6975P-C.

Measured compiler: b81455bc7d42635d12519c11c3788a20b5fad2f4.

GCC: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0.

Clang: Ubuntu clang version 21.1.8 (6ubuntu1).

Build jobs: 4.

| Measure | vs GCC ↑ | vs Clang ↑ |
| --- | ---: | ---: |
| Building cppgm++ | 4.19× | 3.70× |
| Self-built compiler speed | 0.98× | 0.99× |

Building cppgm++ compares toolchains compiling the same sources. Self-built compiler speed compares fixed native-built and self-built cppgm++ binaries rebuilding identical inputs in paired ABBA/BAAB blocks. Every rebuild must produce identical objects and compiler binaries.

### Building the compiler

| Compiler doing the rebuild | CPU seconds |
| --- | ---: |
| GCC | 450.300 |
| Clang | 397.810 |
| GCC-built cppgm++ | 107.430 |
| Self-built cppgm++ | 110.010 |

These are the initial bootstrap builds. GCC and Clang are each measured once.

#### Paired self-built compiler measurements

| Block | GCC-built cppgm++ (s) | Self-built cppgm++ (s) | Self-built speed ↑ |
| ---: | ---: | ---: | ---: |
| 1 | 104.686 | 107.014 | 0.98× |
| 2 | 105.439 | 106.741 | 0.99× |

Each block averages its two observations per compiler. The badge uses the median block ratio; calibration and every observation are retained in the raw results.

GCC-built A/A CPU ratio: 1.003. A/A variation limits conclusions about small differences.

#### Paired codegen measurements vs Clang-built compiler

| Block | Clang-built cppgm++ (s) | Self-built cppgm++ (s) | Self-built speed ↑ |
| ---: | ---: | ---: | ---: |
| 1 | 107.482 | 107.841 | 1.00× |
| 2 | 106.047 | 108.061 | 0.98× |

Each block averages its two observations per compiler. The badge uses the median block ratio; calibration and every observation are retained in the raw results.

Clang-built A/A CPU ratio: 0.992. A/A variation limits conclusions about small differences.

[Public program results](report.md) have their own measured revision.
