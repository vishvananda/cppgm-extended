# WASI compiler self-build

cppgm and Clang each build `cppgm++` for WASI. Both WASI compilers then rebuild `cppgm++` from the same 423 sources in one pinned engine. Speedup is Clang time / cppgm time; higher is better.

| Measurement | cppgm | Clang | Speedup |
|---|---:|---:|---:|
| Build the WASI compiler, CPU | 120.2 s | 695.3 s | 5.79× |
| Build the WASI compiler, elapsed | 65.8 s | 354.5 s | 5.39× |
| Rebuild with the WASI compiler, CPU | 180.3 s | 181.7 s | 1.008× |
| Rebuild with the WASI compiler, elapsed | 96.4 s | 97.1 s | 1.008× |

The build is one observation. The rebuild is the median of 3 paired ABBA blocks.

CPU: INTEL(R) XEON(R) PLATINUM 8573C. [Raw CI observations](https://github.com/vishvananda/cppgm-extended/actions/runs/43609274581).
