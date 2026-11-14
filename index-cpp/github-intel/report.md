# index-cpp benchmarks

Compiler checkout: `411e9fbf346b7707d45b8b4e8cbfa3fea6e2fafd` · CPU: Intel(R) Xeon(R) 6975P-C

Unchanged upstream: `79e0b423094d420da37fb714f20c53fc35eeb2a3`. All 50 checksums pass with cppgm, GCC and Clang. Full runtime inputs, paired ABBA/BAAB blocks with A/A calibration. Speeds are reference time / cppgm time: higher is better. The aggregate gives all 50 workloads equal weight and is separate from the public suite.

| Measurement | vs GCC | vs Clang |
|---|---:|---:|
| Generated code | 1.049× | 1.052× |
| Compile CPU | 6.217× | 5.029× |

| Compiler | Executable bytes | .text bytes | Compile peak RSS (KiB) |
|---|---:|---:|---:|
| cppgm | 710,384 | 336,434 | 228,342 |
| gcc | 540,112 | 280,921 | 486,050 |
| clang | 513,720 | 264,561 | 319,406 |

| Workload | cppgm seconds | GCC seconds | Clang seconds | vs GCC | vs Clang | A/A warning |
|---|---:|---:|---:|---:|---:|---|
| Base64::Decode | 0.850 | 0.730 | 0.849 | 0.868× | 0.973× | clang |
| Base64::Encode | 0.966 | 0.994 | 0.794 | 1.036× | 0.820× | — |
| Binarytrees::Arena | 1.991 | 2.054 | 2.237 | 1.106× | 1.076× | clang |
| Binarytrees::Obj | 0.986 | 0.832 | 1.230 | 0.895× | 1.204× | gcc, clang |
| Brainfuck::Array | 1.623 | 1.655 | 1.712 | 1.021× | 1.049× | — |
| Brainfuck::Recursion | 1.247 | 1.200 | 1.423 | 0.997× | 1.151× | gcc |
| CLBG::Fannkuchredux | 0.901 | 0.974 | 0.833 | 1.079× | 0.926× | — |
| CLBG::Mandelbrot | 1.060 | 0.980 | 0.972 | 0.924× | 0.916× | — |
| CLBG::Nbody | 0.660 | 0.624 | 0.637 | 0.947× | 0.963× | — |
| CLBG::Spectralnorm | 0.977 | 0.940 | 1.320 | 0.962× | 1.350× | — |
| CSV::Parse | 0.911 | 0.798 | 0.927 | 0.875× | 1.018× | — |
| Calculator::Ast | 0.867 | 0.782 | 0.848 | 0.937× | 0.944× | clang |
| Calculator::Interpreter | 1.362 | 1.122 | 1.572 | 0.958× | 1.062× | gcc, clang |
| Compress::ArithDecode | 0.861 | 1.014 | 1.493 | 1.181× | 1.732× | — |
| Compress::ArithEncode | 1.030 | 0.998 | 1.076 | 0.971× | 1.045× | — |
| Compress::BWTDecode | 1.304 | 1.305 | 1.306 | 1.002× | 1.000× | — |
| Compress::BWTEncode | 0.750 | 0.778 | 0.926 | 1.037× | 1.224× | — |
| Compress::HuffDecode | 0.798 | 0.730 | 0.389 | 0.917× | 0.483× | — |
| Compress::HuffEncode | 0.810 | 0.933 | 0.831 | 1.147× | 1.023× | — |
| Compress::LZWDecode | 0.571 | 0.573 | 0.554 | 1.006× | 0.968× | — |
| Compress::LZWEncode | 0.959 | 0.948 | 0.956 | 0.991× | 0.995× | — |
| Distance::Jaro | 1.169 | 1.190 | 1.207 | 1.020× | 1.030× | — |
| Distance::NGram | 1.171 | 1.190 | 1.128 | 1.021× | 0.964× | — |
| Etc::CacheSimulation | 0.812 | 0.784 | 0.806 | 0.967× | 0.991× | — |
| Etc::GameOfLife | 1.136 | 1.162 | 1.119 | 1.019× | 0.985× | — |
| Etc::LogParser | 0.471 | 1.218 | 0.440 | 2.628× | 0.934× | — |
| Etc::NeuralNet | 0.781 | 0.726 | 0.710 | 0.930× | 0.906× | — |
| Etc::Sieve | 2.438 | 3.154 | 2.422 | 1.287× | 0.991× | — |
| Etc::TextRaytracer | 0.917 | 0.635 | 0.828 | 0.693× | 0.902× | — |
| Etc::Words | 0.919 | 0.899 | 0.915 | 0.978× | 0.991× | — |
| Graph::AStar | 1.180 | 1.202 | 1.084 | 1.015× | 0.915× | — |
| Graph::BFS | 0.970 | 0.754 | 0.954 | 0.913× | 0.976× | gcc, clang |
| Graph::DFS | 1.107 | 0.970 | 1.036 | 0.887× | 0.929× | — |
| Hash::CRC32 | 0.245 | 0.718 | 1.006 | 2.933× | 4.108× | — |
| Hash::SHA256 | 0.335 | 0.915 | 0.783 | 2.743× | 2.332× | — |
| Json::Generate | 0.860 | 0.863 | 0.855 | 1.008× | 0.992× | — |
| Json::ParseDom | 1.021 | 0.984 | 0.973 | 0.969× | 0.950× | — |
| Json::ParseMapping | 0.972 | 0.935 | 0.927 | 0.969× | 0.951× | — |
| Matmul::Single | 0.756 | 0.758 | 0.764 | 0.999× | 1.005× | — |
| Matmul::T16 | 1.647 | 1.639 | 1.632 | 0.998× | 0.985× | — |
| Matmul::T4 | 0.780 | 0.776 | 0.780 | 1.003× | 0.997× | — |
| Matmul::T8 | 0.836 | 0.837 | 0.835 | 1.003× | 0.999× | clang |
| Maze::AStar | 1.363 | 0.855 | 1.653 | 0.905× | 1.072× | gcc, clang |
| Maze::BFS | 0.752 | 0.716 | 0.980 | 0.977× | 1.253× | gcc, clang |
| Maze::Generator | 1.463 | 1.344 | 1.855 | 1.027× | 1.230× | gcc, clang |
| Sort::Merge | 0.972 | 1.055 | 0.853 | 1.089× | 0.874× | — |
| Sort::Quick | 1.104 | 1.147 | 1.128 | 1.039× | 1.017× | — |
| Sort::Self | 1.155 | 1.085 | 1.107 | 0.939× | 0.955× | — |
| Template::Parse | 1.143 | 1.070 | 1.228 | 0.966× | 0.997× | gcc, clang |
| Template::Regex | 0.923 | 0.958 | 1.144 | 1.046× | 1.200× | gcc |

A/A warnings flag variation above 2%; no cases are dropped. Small timing differences need confirmation.
Compile gcc A/A CPU ratio: 1.004×.
Compile clang A/A CPU ratio: 0.991×.

Hardware counters: unavailable: perf is not installed on this host.

## Toolchains

- **cppgm**: cppgm++ (host configuration: g++)
- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and header hashes](metadata.json) · [Raw compile observations](compile-observations.jsonl) · [Raw runtime observations](runtime-observations.jsonl)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43558051743)
