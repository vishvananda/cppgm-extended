# WASI compiler self-build

cppgm and Clang each build `cppgm++` for WASI. Both WASI compilers then rebuild `cppgm++` from the same 420 sources in one pinned engine. Speedup is Clang time / cppgm time; higher is better.

| Measurement | cppgm | Clang | Speedup |
|---|---:|---:|---:|
| Build the WASI compiler, CPU | 160.8 s | 1156.1 s | 7.19× |
| Build the WASI compiler, elapsed | 86.6 s | 586.2 s | 6.77× |
| Rebuild with the WASI compiler, CPU | 312.0 s | 278.8 s | 0.888× |
| Rebuild with the WASI compiler, elapsed | 162.7 s | 146.4 s | 0.894× |

The build is one observation. The rebuild is the median of 3 paired ABBA blocks.

CPU: AMD EPYC 7763 64-Core Processor. [Raw CI observations](https://github.com/vishvananda/cppgm-extended/actions/runs/43509589094).
