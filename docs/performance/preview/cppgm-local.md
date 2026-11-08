# cppgm++ compile time and codegen · local Intel

CPU: **Intel(R) Xeon(R) CPU E5-2696 v4 @ 2.20GHz**, local shared host, 8 build workers. Measured source revision: [`17a21ad27`](https://github.com/vishvananda/cppgm-extended/commit/17a21ad27ace6eda20b948e57c2e78ab3d568374).

Native toolchains: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0; Ubuntu clang version 21.1.8 (6ubuntu1). Both native builds preserve the same g++ target/header configuration; their generated configuration hashes match.

| Measurement | vs GCC | vs Clang |
|---|---:|---:|
| Compiler build CPU | 4.034× | 3.723× |
| Codegen: generated compiler execution CPU | 0.982× | 1.003× |

Higher is better. Native compiler builds are single fresh CPU observations. Codegen uses two paired ABBA/BAAB blocks per baseline, following warmups and A/A measurements. All 28 rebuilds pass object and compiler-binary equality checks within their frozen input comparison; all 484 source-file hashes remain unchanged. Small differences should be read alongside the A/A measurements below.

### Building the compiler

| Compiler doing the rebuild | CPU seconds |
| --- | ---: |
| GCC | 477.440 |
| Clang | 440.640 |
| GCC-built cppgm++ | 118.360 |
| Self-built cppgm++ | 121.050 |

These are the initial bootstrap builds. The GCC rebuild is measured once.

#### Paired self-built compiler measurements

| Block | GCC-built cppgm++ (s) | Self-built cppgm++ (s) | Self-built speed ↑ |
| ---: | ---: | ---: | ---: |
| 1 | 118.634 | 120.475 | 0.98× |
| 2 | 118.901 | 121.355 | 0.98× |

Each block averages its two observations per compiler. The badge uses the median block ratio; calibration and every observation are retained in the raw results.

GCC-built A/A CPU ratio: 1.001. A/A variation limits conclusions about small differences.

#### Paired codegen measurements vs Clang-built compiler

| Block | Clang-built cppgm++ (s) | Self-built cppgm++ (s) | Self-built speed ↑ |
| ---: | ---: | ---: | ---: |
| 1 | 118.798 | 119.025 | 1.00× |
| 2 | 118.856 | 117.914 | 1.01× |

Each block averages its two observations per compiler. The badge uses the median block ratio; calibration and every observation are retained in the raw results.

Clang-built A/A CPU ratio: 0.998. A/A variation limits conclusions about small differences.

Raw results, every observation, command, compiler/source hash and output receipt are retained at:

```text
/mnt/vizier/cppgm-cxx11-upstream/cppgm-clang-compare-v1
```
