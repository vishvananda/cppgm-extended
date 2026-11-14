# WASI compiler self-build

cppgm and Clang each build `cppgm++` for WASI. Both WASI compilers then rebuild `cppgm++` from the same 423 sources in one pinned engine. Speedup is Clang time / cppgm time; higher is better.

| Measurement | cppgm | Clang | Speedup |
|---|---:|---:|---:|
| Build the WASI compiler, CPU | 138.6 s | 791.6 s | 5.71× |
| Build the WASI compiler, elapsed | 75.3 s | 403.0 s | 5.35× |
| Rebuild with the WASI compiler, CPU | 204.0 s | 206.0 s | 1.010× |
| Rebuild with the WASI compiler, elapsed | 108.6 s | 109.6 s | 1.009× |

The build is one observation. The rebuild is the median of 3 paired ABBA blocks.

CPU: INTEL(R) XEON(R) PLATINUM 8573C. [Raw CI observations](https://github.com/vishvananda/cppgm-extended/actions/runs/43572026880).
