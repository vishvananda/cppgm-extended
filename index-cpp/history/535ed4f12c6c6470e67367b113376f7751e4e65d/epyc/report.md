# index-cpp benchmarks

Compiler checkout: `535ed4f12c6c6470e67367b113376f7751e4e65d` · CPU: AMD EPYC 9V74 80-Core Processor

Unchanged upstream: `79e0b423094d420da37fb714f20c53fc35eeb2a3`. All 50 checksums pass with cppgm, GCC and Clang. Full runtime inputs, paired ABBA/BAAB blocks with A/A calibration. Speeds are reference time / cppgm time: higher is better. The aggregate gives all 50 workloads equal weight and is separate from the public suite.

| Measurement | vs GCC | vs Clang |
|---|---:|---:|
| Generated code | 1.005× | 0.993× |
| Compile CPU | 6.678× | 5.517× |

| Compiler | Executable bytes | .text bytes | Compile peak RSS (KiB) |
|---|---:|---:|---:|
| cppgm | 786,080 | 382,674 | 228,214 |
| gcc | 540,112 | 280,921 | 485,976 |
| clang | 513,720 | 264,561 | 319,316 |

| Workload | cppgm seconds | GCC seconds | Clang seconds | vs GCC | vs Clang | A/A warning |
|---|---:|---:|---:|---:|---:|---|
| Base64::Decode | 1.870 | 0.946 | 0.990 | 0.506× | 0.528× | — |
| Base64::Encode | 1.347 | 1.191 | 0.998 | 0.884× | 0.742× | — |
| Binarytrees::Arena | 2.103 | 2.173 | 2.157 | 1.026× | 1.029× | — |
| Binarytrees::Obj | 1.978 | 1.904 | 2.022 | 0.966× | 0.978× | — |
| Brainfuck::Array | 1.689 | 2.139 | 1.529 | 1.249× | 0.903× | clang |
| Brainfuck::Recursion | 1.132 | 1.177 | 1.152 | 1.040× | 1.016× | gcc |
| CLBG::Fannkuchredux | 1.016 | 0.977 | 0.909 | 0.963× | 0.896× | — |
| CLBG::Mandelbrot | 1.240 | 1.171 | 1.115 | 0.945× | 0.899× | — |
| CLBG::Nbody | 0.886 | 0.891 | 0.895 | 1.006× | 1.011× | — |
| CLBG::Spectralnorm | 0.954 | 0.925 | 1.579 | 0.970× | 1.654× | — |
| CSV::Parse | 0.897 | 0.821 | 0.910 | 0.917× | 1.012× | gcc |
| Calculator::Ast | 0.962 | 0.746 | 0.790 | 0.774× | 0.821× | — |
| Calculator::Interpreter | 1.097 | 1.020 | 1.143 | 0.956× | 1.014× | — |
| Compress::ArithDecode | 0.787 | 0.813 | 1.395 | 1.034× | 1.776× | — |
| Compress::ArithEncode | 0.913 | 0.854 | 0.953 | 0.937× | 1.039× | — |
| Compress::BWTDecode | 0.562 | 0.536 | 0.560 | 0.955× | 0.997× | — |
| Compress::BWTEncode | 0.901 | 0.853 | 0.992 | 0.945× | 1.091× | — |
| Compress::HuffDecode | 0.556 | 0.595 | 0.412 | 1.070× | 0.740× | — |
| Compress::HuffEncode | 0.735 | 0.774 | 0.916 | 1.040× | 1.245× | — |
| Compress::LZWDecode | 0.595 | 0.607 | 0.565 | 1.022× | 0.943× | — |
| Compress::LZWEncode | 1.002 | 0.980 | 0.994 | 0.982× | 0.986× | — |
| Distance::Jaro | 1.046 | 1.051 | 0.977 | 1.004× | 0.935× | — |
| Distance::NGram | 1.118 | 1.072 | 1.050 | 0.960× | 0.940× | — |
| Etc::CacheSimulation | 0.891 | 0.880 | 0.886 | 0.990× | 0.993× | gcc |
| Etc::GameOfLife | 1.225 | 0.949 | 1.063 | 0.785× | 0.848× | clang |
| Etc::LogParser | 0.465 | 1.781 | 0.520 | 3.770× | 1.118× | gcc, clang |
| Etc::NeuralNet | 1.012 | 0.833 | 0.820 | 0.822× | 0.809× | gcc, clang |
| Etc::Sieve | 1.181 | 1.564 | 1.194 | 1.324× | 1.011× | — |
| Etc::TextRaytracer | 0.923 | 0.881 | 1.060 | 0.955× | 1.148× | — |
| Etc::Words | 1.008 | 0.970 | 1.002 | 0.964× | 0.998× | — |
| Graph::AStar | 1.189 | 1.183 | 1.000 | 1.003× | 0.823× | clang |
| Graph::BFS | 1.728 | 1.194 | 1.151 | 0.700× | 0.667× | — |
| Graph::DFS | 1.381 | 1.188 | 1.238 | 0.864× | 0.893× | gcc |
| Hash::CRC32 | 0.257 | 0.706 | 0.802 | 2.747× | 3.121× | — |
| Hash::SHA256 | 1.198 | 1.169 | 1.179 | 0.977× | 0.983× | — |
| Json::Generate | 0.771 | 0.774 | 0.774 | 1.012× | 1.000× | — |
| Json::ParseDom | 0.934 | 0.912 | 0.893 | 0.979× | 0.955× | — |
| Json::ParseMapping | 0.889 | 0.873 | 0.851 | 0.985× | 0.954× | — |
| Matmul::Single | 1.196 | 1.183 | 1.185 | 0.989× | 0.990× | — |
| Matmul::T16 | 2.337 | 2.306 | 2.293 | 0.984× | 0.972× | — |
| Matmul::T4 | 1.169 | 1.167 | 1.163 | 0.999× | 0.994× | — |
| Matmul::T8 | 1.217 | 1.204 | 1.200 | 0.988× | 0.985× | — |
| Maze::AStar | 1.070 | 0.993 | 1.329 | 0.926× | 1.059× | gcc |
| Maze::BFS | 0.919 | 0.891 | 1.075 | 0.982× | 0.966× | — |
| Maze::Generator | 1.284 | 1.240 | 1.282 | 0.973× | 0.992× | clang |
| Sort::Merge | 1.061 | 0.986 | 0.878 | 0.932× | 0.828× | — |
| Sort::Quick | 1.143 | 1.085 | 1.099 | 0.950× | 0.963× | — |
| Sort::Self | 1.120 | 1.040 | 1.059 | 0.929× | 0.945× | — |
| Template::Parse | 0.980 | 0.925 | 1.046 | 0.973× | 0.984× | gcc |
| Template::Regex | 0.827 | 1.042 | 1.165 | 1.265× | 1.349× | — |

A/A warnings flag variation above 2%; no cases are dropped. Small timing differences need confirmation.
Compile gcc A/A CPU ratio: 1.002×.
Compile clang A/A CPU ratio: 0.994×.

Hardware counters: unavailable: perf is not installed on this host.

## Toolchains

- **cppgm**: cppgm++ (host configuration: g++)
- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and header hashes](metadata.json) · [Raw compile observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136294) · [Raw runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136294)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136294)
