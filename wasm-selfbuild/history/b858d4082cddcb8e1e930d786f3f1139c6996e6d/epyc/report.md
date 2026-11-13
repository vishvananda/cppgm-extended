# WASI compiler self-build

cppgm and Clang each build `cppgm++` for WASI. Both WASI compilers then rebuild `cppgm++` from the same 420 sources in one pinned engine. Speedup is Clang time / cppgm time; higher is better.

| Measurement | cppgm | Clang | Speedup |
|---|---:|---:|---:|
| Build the WASI compiler, CPU | 90.9 s | 575.2 s | 6.33× |
| Build the WASI compiler, elapsed | 51.0 s | 294.4 s | 5.78× |
| Rebuild with the WASI compiler, CPU | 142.0 s | 127.3 s | 0.887× |
| Rebuild with the WASI compiler, elapsed | 77.0 s | 69.7 s | 0.895× |

The build is one observation. The rebuild is the median of 3 paired ABBA blocks.

CPU: AMD EPYC 9V45 96-Core Processor. [Raw CI observations](https://github.com/vishvananda/cppgm-extended/actions/runs/43479738104).
