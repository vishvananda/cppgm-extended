# index-cpp benchmarks

Compiler checkout: `0aa83232b4c697f9a6f25ef8f318a561973797b4` · CPU: AMD EPYC 9V74 80-Core Processor

Unchanged upstream: `79e0b423094d420da37fb714f20c53fc35eeb2a3`. All 50 checksums pass with cppgm, GCC and Clang. Full runtime inputs, paired ABBA/BAAB blocks with A/A calibration. Speeds are reference time / cppgm time: higher is better. The aggregate gives all 50 workloads equal weight and is separate from the public suite.

| Measurement | vs GCC | vs Clang |
|---|---:|---:|
| Generated code | 1.001× | 0.983× |
| Compile CPU | 6.764× | 5.667× |

| Compiler | Executable bytes | .text bytes | Compile peak RSS (KiB) |
|---|---:|---:|---:|
| cppgm | 823,072 | 385,154 | 229,102 |
| gcc | 540,112 | 280,921 | 486,032 |
| clang | 513,720 | 264,561 | 318,886 |

| Workload | cppgm seconds | GCC seconds | Clang seconds | vs GCC | vs Clang | A/A warning |
|---|---:|---:|---:|---:|---:|---|
| Base64::Decode | 1.867 | 0.950 | 0.990 | 0.509× | 0.530× | — |
| Base64::Encode | 1.415 | 1.192 | 1.000 | 0.842× | 0.705× | — |
| Binarytrees::Arena | 2.065 | 2.194 | 2.174 | 1.059× | 1.052× | — |
| Binarytrees::Obj | 1.998 | 1.940 | 2.012 | 0.965× | 1.011× | gcc |
| Brainfuck::Array | 2.374 | 2.141 | 1.559 | 0.927× | 0.683× | gcc, clang |
| Brainfuck::Recursion | 1.366 | 1.177 | 1.152 | 0.865× | 0.844× | clang |
| CLBG::Fannkuchredux | 0.986 | 0.976 | 0.914 | 0.988× | 0.922× | — |
| CLBG::Mandelbrot | 1.240 | 1.171 | 1.115 | 0.944× | 0.899× | — |
| CLBG::Nbody | 0.887 | 0.891 | 0.895 | 1.005× | 1.010× | — |
| CLBG::Spectralnorm | 0.954 | 0.925 | 1.576 | 0.970× | 1.652× | — |
| CSV::Parse | 0.964 | 0.825 | 0.903 | 0.862× | 0.928× | — |
| Calculator::Ast | 0.958 | 0.747 | 0.784 | 0.782× | 0.816× | — |
| Calculator::Interpreter | 1.121 | 1.071 | 1.148 | 0.969× | 1.021× | gcc |
| Compress::ArithDecode | 0.765 | 0.812 | 1.395 | 1.062× | 1.822× | — |
| Compress::ArithEncode | 0.940 | 0.855 | 0.952 | 0.910× | 1.017× | — |
| Compress::BWTDecode | 0.563 | 0.538 | 0.559 | 0.956× | 0.992× | — |
| Compress::BWTEncode | 0.912 | 0.855 | 0.986 | 0.942× | 1.077× | — |
| Compress::HuffDecode | 0.514 | 0.595 | 0.412 | 1.158× | 0.802× | — |
| Compress::HuffEncode | 0.702 | 0.774 | 0.916 | 1.085× | 1.302× | — |
| Compress::LZWDecode | 0.593 | 0.613 | 0.560 | 1.029× | 0.943× | — |
| Compress::LZWEncode | 1.000 | 0.982 | 0.993 | 0.982× | 0.993× | — |
| Distance::Jaro | 1.068 | 1.050 | 0.977 | 0.984× | 0.914× | — |
| Distance::NGram | 1.107 | 1.073 | 1.051 | 0.968× | 0.948× | — |
| Etc::CacheSimulation | 0.894 | 0.881 | 0.889 | 0.987× | 0.996× | — |
| Etc::GameOfLife | 1.228 | 0.998 | 1.013 | 0.805× | 0.834× | gcc |
| Etc::LogParser | 0.459 | 1.753 | 0.505 | 3.822× | 1.126× | clang |
| Etc::NeuralNet | 1.012 | 0.837 | 0.817 | 0.877× | 0.807× | clang |
| Etc::Sieve | 1.181 | 1.564 | 1.192 | 1.323× | 1.008× | — |
| Etc::TextRaytracer | 0.923 | 0.881 | 1.059 | 0.954× | 1.147× | — |
| Etc::Words | 1.014 | 0.970 | 1.008 | 0.955× | 0.990× | — |
| Graph::AStar | 1.192 | 1.219 | 0.968 | 1.007× | 0.812× | — |
| Graph::BFS | 1.821 | 1.255 | 1.107 | 0.683× | 0.616× | — |
| Graph::DFS | 1.395 | 1.207 | 1.241 | 0.860× | 0.883× | — |
| Hash::CRC32 | 0.257 | 0.706 | 0.802 | 2.747× | 3.121× | — |
| Hash::SHA256 | 1.196 | 1.168 | 1.176 | 0.975× | 0.984× | — |
| Json::Generate | 0.779 | 0.773 | 0.762 | 0.995× | 0.978× | — |
| Json::ParseDom | 0.937 | 0.911 | 0.887 | 0.978× | 0.944× | — |
| Json::ParseMapping | 0.892 | 0.869 | 0.853 | 0.977× | 0.954× | — |
| Matmul::Single | 1.199 | 1.187 | 1.187 | 0.990× | 0.991× | — |
| Matmul::T16 | 2.338 | 2.306 | 2.282 | 0.993× | 0.981× | — |
| Matmul::T4 | 1.167 | 1.168 | 1.163 | 1.000× | 0.997× | — |
| Matmul::T8 | 1.209 | 1.196 | 1.196 | 0.993× | 0.988× | — |
| Maze::AStar | 1.054 | 1.141 | 1.240 | 0.992× | 1.126× | gcc |
| Maze::BFS | 0.887 | 1.062 | 1.012 | 0.998× | 0.963× | gcc |
| Maze::Generator | 1.256 | 1.280 | 1.257 | 1.020× | 0.986× | — |
| Sort::Merge | 0.998 | 0.987 | 0.878 | 0.989× | 0.879× | — |
| Sort::Quick | 1.173 | 1.084 | 1.098 | 0.925× | 0.936× | — |
| Sort::Self | 1.144 | 1.041 | 1.057 | 0.907× | 0.920× | — |
| Template::Parse | 1.001 | 1.043 | 0.972 | 1.002× | 0.977× | gcc |
| Template::Regex | 0.871 | 1.086 | 1.134 | 1.236× | 1.312× | gcc |

A/A warnings flag variation above 2%; no cases are dropped. Small timing differences need confirmation.
Compile gcc A/A CPU ratio: 0.996×.
Compile clang A/A CPU ratio: 0.999×.

Hardware counters: unavailable: perf is not installed on this host.

## Toolchains

- **cppgm**: cppgm++ (host configuration: g++)
- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and header hashes](metadata.json) · [Raw compile observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42570114064) · [Raw runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42570114064)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42570114064)
