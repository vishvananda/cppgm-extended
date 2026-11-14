# index-cpp benchmarks

Compiler checkout: `1597e573a4882a000810eb15f86d4cb230ce4afd` · CPU: Intel(R) Xeon(R) 6975P-C

Unchanged upstream: `79e0b423094d420da37fb714f20c53fc35eeb2a3`. All 50 checksums pass with cppgm, GCC and Clang. Full runtime inputs, paired ABBA/BAAB blocks with A/A calibration. Speeds are reference time / cppgm time: higher is better. The aggregate gives all 50 workloads equal weight and is separate from the public suite.

| Measurement | vs GCC | vs Clang |
|---|---:|---:|
| Generated code | 1.066× | 1.046× |
| Compile CPU | 6.199× | 5.032× |

| Compiler | Executable bytes | .text bytes | Compile peak RSS (KiB) |
|---|---:|---:|---:|
| cppgm | 714,480 | 340,898 | 228,490 |
| gcc | 540,112 | 280,921 | 485,998 |
| clang | 513,720 | 264,561 | 318,946 |

| Workload | cppgm seconds | GCC seconds | Clang seconds | vs GCC | vs Clang | A/A warning |
|---|---:|---:|---:|---:|---:|---|
| Base64::Decode | 0.869 | 0.778 | 0.847 | 0.891× | 0.958× | — |
| Base64::Encode | 0.982 | 1.027 | 0.800 | 1.045× | 0.815× | — |
| Binarytrees::Arena | 2.057 | 2.254 | 2.215 | 1.079× | 1.080× | gcc |
| Binarytrees::Obj | 1.026 | 0.940 | 1.239 | 0.919× | 1.199× | gcc, clang |
| Brainfuck::Array | 1.683 | 1.665 | 1.721 | 0.992× | 1.022× | — |
| Brainfuck::Recursion | 1.135 | 1.208 | 1.433 | 1.066× | 1.261× | — |
| CLBG::Fannkuchredux | 0.915 | 0.972 | 0.839 | 1.063× | 0.919× | — |
| CLBG::Mandelbrot | 1.092 | 0.988 | 0.979 | 0.902× | 0.895× | — |
| CLBG::Nbody | 0.664 | 0.627 | 0.640 | 0.949× | 0.963× | — |
| CLBG::Spectralnorm | 0.990 | 0.944 | 1.330 | 0.955× | 1.341× | — |
| CSV::Parse | 0.849 | 0.798 | 0.935 | 0.941× | 1.099× | — |
| Calculator::Ast | 0.923 | 0.857 | 0.856 | 0.919× | 0.922× | — |
| Calculator::Interpreter | 1.484 | 1.381 | 1.444 | 0.931× | 0.959× | gcc, clang |
| Compress::ArithDecode | 0.881 | 1.022 | 1.502 | 1.155× | 1.703× | — |
| Compress::ArithEncode | 1.016 | 1.005 | 1.083 | 0.986× | 1.067× | — |
| Compress::BWTDecode | 1.321 | 1.319 | 1.321 | 1.001× | 1.000× | — |
| Compress::BWTEncode | 0.759 | 0.790 | 0.932 | 1.044× | 1.225× | — |
| Compress::HuffDecode | 0.832 | 0.732 | 0.393 | 0.887× | 0.473× | — |
| Compress::HuffEncode | 0.863 | 0.936 | 0.833 | 1.090× | 0.969× | — |
| Compress::LZWDecode | 0.581 | 0.582 | 0.561 | 1.002× | 0.961× | — |
| Compress::LZWEncode | 0.965 | 0.954 | 0.960 | 0.989× | 0.994× | — |
| Distance::Jaro | 1.167 | 1.198 | 1.212 | 1.026× | 1.041× | — |
| Distance::NGram | 1.176 | 1.199 | 1.135 | 1.020× | 0.969× | — |
| Etc::CacheSimulation | 0.816 | 0.790 | 0.808 | 0.969× | 0.990× | — |
| Etc::GameOfLife | 1.228 | 1.230 | 1.159 | 1.011× | 0.981× | gcc, clang |
| Etc::LogParser | 0.418 | 1.225 | 0.444 | 2.884× | 1.063× | — |
| Etc::NeuralNet | 0.788 | 0.733 | 0.716 | 0.929× | 0.908× | — |
| Etc::Sieve | 2.506 | 3.220 | 2.454 | 1.288× | 0.990× | — |
| Etc::TextRaytracer | 0.923 | 0.638 | 0.833 | 0.690× | 0.905× | — |
| Etc::Words | 0.939 | 0.911 | 0.925 | 0.969× | 0.986× | — |
| Graph::AStar | 1.188 | 1.261 | 1.082 | 1.056× | 0.908× | — |
| Graph::BFS | 0.974 | 0.974 | 0.961 | 0.995× | 0.983× | — |
| Graph::DFS | 1.074 | 1.036 | 1.035 | 0.962× | 0.965× | — |
| Hash::CRC32 | 0.246 | 0.722 | 1.012 | 2.923× | 4.122× | — |
| Hash::SHA256 | 0.356 | 0.922 | 0.786 | 2.591× | 2.208× | — |
| Json::Generate | 0.863 | 0.865 | 0.861 | 1.003× | 0.995× | — |
| Json::ParseDom | 1.023 | 0.998 | 0.986 | 0.970× | 0.956× | — |
| Json::ParseMapping | 0.973 | 0.950 | 0.940 | 0.974× | 0.961× | — |
| Matmul::Single | 0.773 | 0.778 | 0.774 | 1.001× | 0.996× | — |
| Matmul::T16 | 1.884 | 1.905 | 1.889 | 1.012× | 1.012× | gcc |
| Matmul::T4 | 0.800 | 0.802 | 0.798 | 1.001× | 0.999× | — |
| Matmul::T8 | 0.902 | 0.893 | 0.899 | 0.991× | 0.985× | clang |
| Maze::AStar | 1.490 | 1.636 | 1.464 | 1.057× | 0.962× | gcc, clang |
| Maze::BFS | 0.782 | 1.013 | 0.915 | 1.204× | 1.058× | gcc |
| Maze::Generator | 1.485 | 1.972 | 1.815 | 1.313× | 1.187× | gcc, clang |
| Sort::Merge | 0.982 | 1.066 | 0.854 | 1.086× | 0.870× | — |
| Sort::Quick | 1.124 | 1.158 | 1.133 | 1.029× | 1.008× | — |
| Sort::Self | 1.159 | 1.096 | 1.109 | 0.948× | 0.957× | — |
| Template::Parse | 1.228 | 1.229 | 1.221 | 0.982× | 1.016× | gcc |
| Template::Regex | 1.004 | 1.019 | 1.155 | 1.028× | 1.158× | gcc |

A/A warnings flag variation above 2%; no cases are dropped. Small timing differences need confirmation.
Compile gcc A/A CPU ratio: 1.001×.
Compile clang A/A CPU ratio: 1.011×.

Hardware counters: unavailable: perf is not installed on this host.

## Toolchains

- **cppgm**: cppgm++ (host configuration: g++)
- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and header hashes](metadata.json) · [Raw compile observations](compile-observations.jsonl) · [Raw runtime observations](runtime-observations.jsonl)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43572027128)
