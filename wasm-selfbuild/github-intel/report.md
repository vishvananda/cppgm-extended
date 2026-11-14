# WASI compiler self-build

cppgm and Clang each build `cppgm++` for WASI. Both WASI compilers then rebuild `cppgm++` from the same 423 sources in one pinned engine. Speedup is Clang time / cppgm time; higher is better.

| Measurement | cppgm | Clang | Speedup |
|---|---:|---:|---:|
| Build the WASI compiler, CPU | 138.4 s | 796.0 s | 5.75× |
| Build the WASI compiler, elapsed | 75.3 s | 405.1 s | 5.38× |
| Rebuild with the WASI compiler, CPU | 208.0 s | 211.3 s | 1.017× |
| Rebuild with the WASI compiler, elapsed | 110.8 s | 112.1 s | 1.012× |

The build is one observation. The rebuild is the median of 3 paired ABBA blocks.

CPU: INTEL(R) XEON(R) PLATINUM 8573C. [Raw CI observations](https://github.com/vishvananda/cppgm-extended/actions/runs/43601267019).
