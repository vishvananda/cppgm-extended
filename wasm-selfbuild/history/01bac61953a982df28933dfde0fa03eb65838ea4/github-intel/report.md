# WASI compiler self-build

cppgm and Clang each build `cppgm++` for WASI. Both WASI compilers then rebuild `cppgm++` from the same 420 sources in one pinned engine. Speedup is Clang time / cppgm time; higher is better.

| Measurement | cppgm | Clang | Speedup |
|---|---:|---:|---:|
| Build the WASI compiler, CPU | 71.3 s | 381.9 s | 5.35× |
| Build the WASI compiler, elapsed | 41.3 s | 197.4 s | 4.78× |
| Rebuild with the WASI compiler, CPU | 123.9 s | 111.4 s | 0.899× |
| Rebuild with the WASI compiler, elapsed | 68.1 s | 62.0 s | 0.909× |

The build is one observation. The rebuild is the median of 3 paired ABBA blocks.

CPU: Intel(R) Xeon(R) 6975P-C. [Raw CI observations](https://github.com/vishvananda/cppgm-extended/actions/runs/43559814664).
