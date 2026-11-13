# WASI compiler self-build

cppgm and Clang each build `cppgm++` for WASI. Both WASI compilers then rebuild `cppgm++` from the same 420 sources in one pinned engine. Speedup is Clang time / cppgm time; higher is better.

| Measurement | cppgm | Clang | Speedup |
|---|---:|---:|---:|
| Build the WASI compiler, CPU | 149.1 s | 998.5 s | 6.70× |
| Build the WASI compiler, elapsed | 80.8 s | 506.6 s | 6.27× |
| Rebuild with the WASI compiler, CPU | 270.8 s | 241.2 s | 0.892× |
| Rebuild with the WASI compiler, elapsed | 141.9 s | 127.2 s | 0.897× |

The build is one observation. The rebuild is the median of 3 paired ABBA blocks.

CPU: AMD EPYC 9V74 80-Core Processor. [Raw CI observations](https://github.com/vishvananda/cppgm-extended/actions/runs/43493409145).
