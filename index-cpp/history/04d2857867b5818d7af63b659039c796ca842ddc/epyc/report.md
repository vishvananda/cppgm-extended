# index-cpp benchmarks

Compiler checkout: `04d2857867b5818d7af63b659039c796ca842ddc` · CPU: AMD EPYC 9V74 80-Core Processor

Unchanged upstream: `79e0b423094d420da37fb714f20c53fc35eeb2a3`. All 50 checksums pass with cppgm, GCC and Clang. Full runtime inputs, paired ABBA/BAAB blocks with A/A calibration. Speeds are reference time / cppgm time: higher is better. The aggregate gives all 50 workloads equal weight and is separate from the public suite.

| Measurement | vs GCC | vs Clang |
|---|---:|---:|
| Generated code | 0.999× | 0.987× |
| Compile CPU | 6.747× | 5.482× |

| Compiler | Executable bytes | .text bytes | Compile peak RSS (KiB) |
|---|---:|---:|---:|
| cppgm | 779,328 | 371,586 | 228,188 |
| gcc | 540,112 | 280,921 | 485,972 |
| clang | 513,720 | 264,561 | 318,942 |

| Workload | cppgm seconds | GCC seconds | Clang seconds | vs GCC | vs Clang | A/A warning |
|---|---:|---:|---:|---:|---:|---|
| Base64::Decode | 1.885 | 0.952 | 1.000 | 0.507× | 0.526× | — |
| Base64::Encode | 1.373 | 1.198 | 1.016 | 0.882× | 0.734× | — |
| Binarytrees::Arena | 2.201 | 2.275 | 2.276 | 1.031× | 1.026× | — |
| Binarytrees::Obj | 2.160 | 2.000 | 2.151 | 0.962× | 0.995× | — |
| Brainfuck::Array | 1.849 | 2.141 | 1.521 | 1.118× | 0.834× | gcc, clang |
| Brainfuck::Recursion | 1.270 | 1.179 | 1.147 | 0.933× | 0.902× | clang |
| CLBG::Fannkuchredux | 1.019 | 0.972 | 0.893 | 0.958× | 0.881× | — |
| CLBG::Mandelbrot | 1.237 | 1.171 | 1.115 | 0.947× | 0.899× | — |
| CLBG::Nbody | 0.886 | 0.891 | 0.895 | 1.006× | 1.008× | — |
| CLBG::Spectralnorm | 0.954 | 0.927 | 1.576 | 0.972× | 1.648× | — |
| CSV::Parse | 0.858 | 0.825 | 0.907 | 0.964× | 1.057× | — |
| Calculator::Ast | 0.978 | 0.745 | 0.798 | 0.761× | 0.814× | clang |
| Calculator::Interpreter | 1.220 | 1.089 | 1.438 | 0.967× | 1.132× | gcc, clang |
| Compress::ArithDecode | 0.787 | 0.812 | 1.396 | 1.035× | 1.774× | — |
| Compress::ArithEncode | 0.938 | 0.854 | 0.953 | 0.910× | 1.015× | — |
| Compress::BWTDecode | 0.563 | 0.538 | 0.563 | 0.957× | 0.999× | — |
| Compress::BWTEncode | 0.902 | 0.856 | 0.988 | 0.949× | 1.097× | — |
| Compress::HuffDecode | 0.558 | 0.595 | 0.413 | 1.070× | 0.736× | — |
| Compress::HuffEncode | 0.736 | 0.776 | 0.917 | 1.048× | 1.248× | — |
| Compress::LZWDecode | 0.583 | 0.611 | 0.565 | 1.049× | 0.959× | — |
| Compress::LZWEncode | 1.000 | 0.979 | 0.996 | 0.972× | 0.997× | — |
| Distance::Jaro | 1.093 | 1.051 | 0.977 | 0.962× | 0.891× | — |
| Distance::NGram | 1.119 | 1.075 | 1.054 | 0.962× | 0.942× | — |
| Etc::CacheSimulation | 0.899 | 0.877 | 0.889 | 0.974× | 0.984× | — |
| Etc::GameOfLife | 1.348 | 1.031 | 1.401 | 0.824× | 0.909× | gcc, clang |
| Etc::LogParser | 0.464 | 1.740 | 0.522 | 3.720× | 1.116× | gcc |
| Etc::NeuralNet | 0.986 | 0.831 | 0.879 | 0.841× | 0.833× | — |
| Etc::Sieve | 1.183 | 1.563 | 1.192 | 1.323× | 1.007× | — |
| Etc::TextRaytracer | 1.144 | 0.881 | 1.062 | 0.768× | 0.927× | — |
| Etc::Words | 1.018 | 0.972 | 1.015 | 0.959× | 0.977× | — |
| Graph::AStar | 1.313 | 1.230 | 1.058 | 0.961× | 0.799× | gcc, clang |
| Graph::BFS | 1.968 | 1.315 | 1.294 | 0.695× | 0.664× | gcc |
| Graph::DFS | 1.459 | 1.212 | 1.295 | 0.858× | 0.886× | clang |
| Hash::CRC32 | 0.257 | 0.706 | 0.802 | 2.747× | 3.121× | — |
| Hash::SHA256 | 1.196 | 1.172 | 1.183 | 0.979× | 0.994× | — |
| Json::Generate | 0.789 | 0.772 | 0.766 | 0.988× | 0.970× | — |
| Json::ParseDom | 0.932 | 0.927 | 0.897 | 1.005× | 0.952× | gcc |
| Json::ParseMapping | 0.894 | 0.879 | 0.868 | 0.996× | 0.961× | — |
| Matmul::Single | 1.199 | 1.187 | 1.190 | 0.992× | 0.990× | — |
| Matmul::T16 | 2.355 | 2.316 | 2.325 | 0.989× | 0.981× | — |
| Matmul::T4 | 1.169 | 1.169 | 1.166 | 0.996× | 0.998× | — |
| Matmul::T8 | 1.222 | 1.204 | 1.204 | 0.987× | 0.983× | — |
| Maze::AStar | 1.385 | 1.204 | 1.598 | 0.992× | 1.098× | gcc, clang |
| Maze::BFS | 1.277 | 1.172 | 1.371 | 0.999× | 0.994× | gcc, clang |
| Maze::Generator | 1.405 | 1.332 | 1.435 | 0.978× | 0.994× | gcc, clang |
| Sort::Merge | 1.064 | 0.988 | 0.879 | 0.934× | 0.826× | — |
| Sort::Quick | 1.158 | 1.086 | 1.102 | 0.937× | 0.952× | — |
| Sort::Self | 1.127 | 1.041 | 1.058 | 0.924× | 0.938× | — |
| Template::Parse | 1.450 | 1.019 | 1.619 | 0.971× | 0.988× | gcc, clang |
| Template::Regex | 1.102 | 1.103 | 1.470 | 1.337× | 1.298× | clang |

A/A warnings flag variation above 2%; no cases are dropped. Small timing differences need confirmation.
Compile gcc A/A CPU ratio: 1.002×.
Compile clang A/A CPU ratio: 1.020×.

Hardware counters: unavailable: perf is not installed on this host.

## Toolchains

- **cppgm**: cppgm++ (host configuration: g++)
- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and header hashes](metadata.json) · [Raw compile observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039920) · [Raw runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039920)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039920)
