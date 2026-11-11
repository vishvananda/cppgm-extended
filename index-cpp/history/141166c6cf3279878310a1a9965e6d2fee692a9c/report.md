# index-cpp benchmarks

Compiler checkout: `141166c6cf3279878310a1a9965e6d2fee692a9c` · CPU: AMD EPYC 9V74 80-Core Processor

Unchanged upstream: `79e0b423094d420da37fb714f20c53fc35eeb2a3`. All 50 checksums pass with cppgm, GCC and Clang. Full runtime inputs, paired ABBA/BAAB blocks with A/A calibration. Speeds are reference time / cppgm time: higher is better. The aggregate gives all 50 workloads equal weight and is separate from the public suite.

| Measurement | vs GCC | vs Clang |
|---|---:|---:|
| Generated code | 0.998× | 0.999× |
| Compile CPU | 6.874× | 5.698× |

| Compiler | Executable bytes | .text bytes | Compile peak RSS (KiB) |
|---|---:|---:|---:|
| cppgm | 807,560 | 385,970 | 224,402 |
| gcc | 540,112 | 280,921 | 486,076 |
| clang | 513,720 | 264,561 | 318,282 |

| Workload | cppgm seconds | GCC seconds | Clang seconds | vs GCC | vs Clang | A/A warning |
|---|---:|---:|---:|---:|---:|---|
| Base64::Decode | 1.665 | 0.952 | 1.002 | 0.574× | 0.601× | — |
| Base64::Encode | 1.438 | 1.192 | 1.022 | 0.836× | 0.711× | — |
| Binarytrees::Arena | 2.099 | 2.199 | 2.196 | 1.051× | 1.044× | — |
| Binarytrees::Obj | 2.079 | 1.921 | 2.115 | 0.938× | 1.026× | — |
| Brainfuck::Array | 1.622 | 2.138 | 1.499 | 1.135× | 0.976× | gcc, clang |
| Brainfuck::Recursion | 1.077 | 1.180 | 1.143 | 1.095× | 1.062× | — |
| CLBG::Fannkuchredux | 0.990 | 0.980 | 0.898 | 0.999× | 0.913× | — |
| CLBG::Mandelbrot | 1.240 | 1.170 | 1.114 | 0.944× | 0.898× | — |
| CLBG::Nbody | 0.887 | 0.891 | 0.895 | 1.004× | 1.008× | — |
| CLBG::Spectralnorm | 0.955 | 0.929 | 1.577 | 0.973× | 1.655× | — |
| CSV::Parse | 0.911 | 0.820 | 0.905 | 0.903× | 0.995× | — |
| Calculator::Ast | 0.990 | 0.764 | 0.790 | 0.770× | 0.795× | — |
| Calculator::Interpreter | 1.285 | 1.078 | 1.333 | 0.934× | 1.009× | gcc, clang |
| Compress::ArithDecode | 0.777 | 0.812 | 1.395 | 1.044× | 1.794× | — |
| Compress::ArithEncode | 0.940 | 0.855 | 0.952 | 0.916× | 1.017× | — |
| Compress::BWTDecode | 0.565 | 0.539 | 0.563 | 0.950× | 0.995× | — |
| Compress::BWTEncode | 0.914 | 0.855 | 0.988 | 0.932× | 1.082× | — |
| Compress::HuffDecode | 0.516 | 0.595 | 0.414 | 1.156× | 0.803× | clang |
| Compress::HuffEncode | 0.702 | 0.776 | 0.916 | 1.098× | 1.306× | clang |
| Compress::LZWDecode | 0.593 | 0.613 | 0.563 | 1.034× | 0.951× | clang |
| Compress::LZWEncode | 1.004 | 0.976 | 0.995 | 0.976× | 0.990× | — |
| Distance::Jaro | 1.064 | 1.051 | 0.977 | 0.985× | 0.918× | — |
| Distance::NGram | 1.107 | 1.075 | 1.054 | 0.972× | 0.956× | — |
| Etc::CacheSimulation | 0.897 | 0.880 | 0.891 | 0.982× | 0.991× | gcc |
| Etc::GameOfLife | 1.510 | 1.055 | 1.512 | 0.765× | 0.988× | clang |
| Etc::LogParser | 0.451 | 1.784 | 0.523 | 3.820× | 1.160× | clang |
| Etc::NeuralNet | 1.001 | 0.890 | 0.836 | 0.837× | 0.833× | gcc |
| Etc::Sieve | 1.185 | 1.567 | 1.192 | 1.322× | 1.006× | — |
| Etc::TextRaytracer | 1.149 | 0.880 | 1.061 | 0.767× | 0.923× | — |
| Etc::Words | 1.015 | 0.968 | 1.012 | 0.955× | 0.990× | — |
| Graph::AStar | 1.253 | 1.236 | 1.036 | 0.989× | 0.824× | gcc, clang |
| Graph::BFS | 2.005 | 1.286 | 1.280 | 0.648× | 0.638× | — |
| Graph::DFS | 1.437 | 1.228 | 1.280 | 0.859× | 0.893× | — |
| Hash::CRC32 | 0.257 | 0.706 | 0.802 | 2.747× | 3.121× | — |
| Hash::SHA256 | 1.199 | 1.169 | 1.176 | 0.977× | 0.980× | — |
| Json::Generate | 0.780 | 0.780 | 0.775 | 1.000× | 0.991× | — |
| Json::ParseDom | 0.942 | 0.917 | 0.903 | 0.984× | 0.945× | — |
| Json::ParseMapping | 0.895 | 0.873 | 0.864 | 0.987× | 0.962× | clang |
| Matmul::Single | 1.196 | 1.183 | 1.186 | 0.992× | 0.990× | — |
| Matmul::T16 | 2.356 | 2.312 | 2.333 | 0.985× | 0.974× | — |
| Matmul::T4 | 1.169 | 1.168 | 1.164 | 1.000× | 0.996× | — |
| Matmul::T8 | 1.222 | 1.196 | 1.200 | 0.981× | 0.981× | — |
| Maze::AStar | 1.430 | 1.095 | 1.583 | 0.848× | 1.099× | gcc, clang |
| Maze::BFS | 1.343 | 1.126 | 1.383 | 0.905× | 0.986× | gcc, clang |
| Maze::Generator | 1.434 | 1.269 | 1.454 | 0.953× | 0.992× | gcc |
| Sort::Merge | 0.998 | 0.988 | 0.881 | 0.989× | 0.880× | — |
| Sort::Quick | 1.174 | 1.085 | 1.101 | 0.926× | 0.937× | — |
| Sort::Self | 1.140 | 1.040 | 1.057 | 0.912× | 0.928× | — |
| Template::Parse | 1.472 | 1.200 | 1.731 | 1.058× | 0.982× | gcc, clang |
| Template::Regex | 1.112 | 1.175 | 1.434 | 1.209× | 1.237× | clang |

A/A warnings flag variation above 2%; no cases are dropped. Small timing differences need confirmation.
Compile gcc A/A CPU ratio: 0.999×.
Compile clang A/A CPU ratio: 1.005×.

Hardware counters: unavailable: perf is not installed on this host.

## Toolchains

- **cppgm**: cppgm++ (host configuration: g++)
- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and header hashes](metadata.json) · [Raw compile observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42295082569) · [Raw runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42295082569)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42295082569)
