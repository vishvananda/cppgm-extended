> README preview snapshot: [index-cpp CI](https://github.com/vishvananda/cppgm-extended/actions/runs/42476441208).

# index-cpp benchmarks

Compiler checkout: `c9774d6a969d0e28c251e35f4ad9a005c6cafcc6` · CPU: AMD EPYC 9V74 80-Core Processor

Unchanged upstream: `79e0b423094d420da37fb714f20c53fc35eeb2a3`. All 50 checksums pass with cppgm, GCC and Clang. Full runtime inputs, paired ABBA/BAAB blocks with A/A calibration. Speeds are reference time / cppgm time: higher is better. The aggregate gives all 50 workloads equal weight and is separate from the public suite.

| Measurement | vs GCC | vs Clang |
|---|---:|---:|
| Generated code | 0.994× | 0.989× |
| Compile CPU | 6.765× | 5.889× |

| Compiler | Executable bytes | .text bytes | Compile peak RSS (KiB) |
|---|---:|---:|---:|
| cppgm | 822,840 | 385,058 | 226,534 |
| gcc | 540,112 | 280,921 | 486,104 |
| clang | 513,720 | 264,561 | 319,060 |

| Workload | cppgm seconds | GCC seconds | Clang seconds | vs GCC | vs Clang | A/A warning |
|---|---:|---:|---:|---:|---:|---|
| Base64::Decode | 1.886 | 0.962 | 1.002 | 0.510× | 0.530× | — |
| Base64::Encode | 1.446 | 1.222 | 1.019 | 0.842× | 0.704× | clang |
| Binarytrees::Arena | 2.101 | 2.221 | 2.200 | 1.052× | 1.053× | — |
| Binarytrees::Obj | 2.186 | 2.120 | 2.127 | 0.939× | 0.998× | gcc |
| Brainfuck::Array | 2.370 | 2.141 | 1.498 | 0.864× | 0.791× | gcc, clang |
| Brainfuck::Recursion | 1.386 | 1.177 | 1.148 | 0.863× | 0.829× | clang |
| CLBG::Fannkuchredux | 0.979 | 0.972 | 0.909 | 0.995× | 0.922× | clang |
| CLBG::Mandelbrot | 1.240 | 1.171 | 1.115 | 0.944× | 0.900× | — |
| CLBG::Nbody | 0.887 | 0.891 | 0.895 | 1.005× | 1.010× | — |
| CLBG::Spectralnorm | 0.954 | 0.926 | 1.579 | 0.970× | 1.656× | — |
| CSV::Parse | 0.970 | 0.833 | 0.918 | 0.864× | 0.946× | — |
| Calculator::Ast | 0.976 | 0.756 | 0.793 | 0.777× | 0.814× | — |
| Calculator::Interpreter | 1.377 | 1.362 | 1.407 | 0.952× | 1.028× | — |
| Compress::ArithDecode | 0.764 | 0.816 | 1.396 | 1.069× | 1.826× | — |
| Compress::ArithEncode | 0.939 | 0.853 | 0.952 | 0.909× | 1.014× | — |
| Compress::BWTDecode | 0.571 | 0.550 | 0.565 | 0.956× | 0.996× | clang |
| Compress::BWTEncode | 0.919 | 0.859 | 0.994 | 0.932× | 1.085× | — |
| Compress::HuffDecode | 0.517 | 0.603 | 0.414 | 1.157× | 0.803× | — |
| Compress::HuffEncode | 0.702 | 0.772 | 0.921 | 1.086× | 1.313× | — |
| Compress::LZWDecode | 0.597 | 0.623 | 0.569 | 1.042× | 0.955× | — |
| Compress::LZWEncode | 0.998 | 0.981 | 1.000 | 0.980× | 0.977× | — |
| Distance::Jaro | 1.069 | 1.051 | 0.977 | 0.986× | 0.915× | — |
| Distance::NGram | 1.101 | 1.080 | 1.054 | 0.976× | 0.959× | — |
| Etc::CacheSimulation | 0.897 | 0.879 | 0.889 | 0.985× | 0.990× | — |
| Etc::GameOfLife | 1.720 | 1.596 | 1.523 | 0.915× | 0.912× | gcc |
| Etc::LogParser | 0.459 | 1.719 | 0.520 | 3.752× | 1.132× | clang |
| Etc::NeuralNet | 1.030 | 0.832 | 0.882 | 0.775× | 0.861× | gcc, clang |
| Etc::Sieve | 1.185 | 1.571 | 1.197 | 1.325× | 1.010× | — |
| Etc::TextRaytracer | 1.149 | 0.882 | 1.061 | 0.768× | 0.923× | — |
| Etc::Words | 1.023 | 0.976 | 1.012 | 0.945× | 0.984× | — |
| Graph::AStar | 1.264 | 1.266 | 1.046 | 1.002× | 0.822× | — |
| Graph::BFS | 2.026 | 1.401 | 1.301 | 0.693× | 0.650× | clang |
| Graph::DFS | 1.457 | 1.261 | 1.287 | 0.863× | 0.888× | — |
| Hash::CRC32 | 0.257 | 0.706 | 0.802 | 2.747× | 3.121× | — |
| Hash::SHA256 | 1.196 | 1.171 | 1.179 | 0.980× | 0.988× | — |
| Json::Generate | 0.764 | 0.776 | 0.767 | 1.017× | 1.006× | — |
| Json::ParseDom | 0.973 | 0.936 | 0.905 | 0.949× | 0.945× | gcc |
| Json::ParseMapping | 0.931 | 0.895 | 0.867 | 0.953× | 0.940× | — |
| Matmul::Single | 1.197 | 1.184 | 1.188 | 0.986× | 0.994× | — |
| Matmul::T16 | 2.389 | 2.379 | 2.340 | 0.973× | 0.986× | — |
| Matmul::T4 | 1.171 | 1.171 | 1.164 | 0.998× | 0.996× | — |
| Matmul::T8 | 1.227 | 1.208 | 1.203 | 0.984× | 0.982× | — |
| Maze::AStar | 1.474 | 1.442 | 1.620 | 0.960× | 1.099× | clang |
| Maze::BFS | 1.398 | 1.383 | 1.413 | 0.984× | 1.042× | clang |
| Maze::Generator | 1.472 | 1.453 | 1.438 | 0.996× | 0.989× | gcc |
| Sort::Merge | 0.962 | 0.992 | 0.881 | 1.032× | 0.912× | — |
| Sort::Quick | 1.167 | 1.087 | 1.101 | 0.932× | 0.943× | — |
| Sort::Self | 1.149 | 1.042 | 1.060 | 0.907× | 0.923× | — |
| Template::Parse | 1.870 | 2.032 | 1.748 | 1.096× | 1.039× | gcc, clang |
| Template::Regex | 1.279 | 1.575 | 1.531 | 1.206× | 1.230× | clang |

A/A warnings flag variation above 2%; no cases are dropped. Small timing differences need confirmation.
Compile gcc A/A CPU ratio: 0.985×.
Compile clang A/A CPU ratio: 0.987×.

Hardware counters: unavailable: perf is not installed on this host.

## Toolchains

- **cppgm**: cppgm++ (host configuration: g++)
- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](https://github.com/vishvananda/cppgm-extended/actions/runs/42476441208#artifacts) · [Provenance and header hashes](https://github.com/vishvananda/cppgm-extended/actions/runs/42476441208#artifacts) · [Raw compile observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42476441208#artifacts) · [Raw runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42476441208#artifacts)

Raw JSON and observation links open the retained CI artifact (14-day GitHub retention); the complete run is also retained locally.
