# index-cpp benchmarks

Compiler checkout: `67f175d753e64b5f1c0b3ce7f3f5733a7e077c7a` · CPU: AMD EPYC 7763 64-Core Processor

Unchanged upstream: `79e0b423094d420da37fb714f20c53fc35eeb2a3`. All 50 checksums pass with cppgm, GCC and Clang. Full runtime inputs, paired ABBA/BAAB blocks with A/A calibration. Speeds are reference time / cppgm time: higher is better. The aggregate gives all 50 workloads equal weight and is separate from the public suite.

| Measurement | vs GCC | vs Clang |
|---|---:|---:|
| Generated code | 0.997× | 0.989× |
| Compile CPU | 6.728× | 6.199× |

| Compiler | Executable bytes | .text bytes | Compile peak RSS (KiB) |
|---|---:|---:|---:|
| cppgm | 822,840 | 385,058 | 226,380 |
| gcc | 540,112 | 280,921 | 485,994 |
| clang | 513,720 | 264,561 | 319,036 |

| Workload | cppgm seconds | GCC seconds | Clang seconds | vs GCC | vs Clang | A/A warning |
|---|---:|---:|---:|---:|---:|---|
| Base64::Decode | 2.639 | 1.201 | 1.212 | 0.455× | 0.456× | — |
| Base64::Encode | 1.805 | 1.447 | 1.293 | 0.809× | 0.717× | — |
| Binarytrees::Arena | 2.420 | 2.492 | 2.581 | 1.026× | 1.074× | — |
| Binarytrees::Obj | 1.978 | 1.528 | 1.754 | 0.774× | 0.887× | gcc, clang |
| Brainfuck::Array | 2.816 | 2.815 | 2.862 | 1.015× | 1.039× | — |
| Brainfuck::Recursion | 1.514 | 1.743 | 1.692 | 1.152× | 1.114× | — |
| CLBG::Fannkuchredux | 1.079 | 1.144 | 0.976 | 1.061× | 0.906× | — |
| CLBG::Mandelbrot | 1.381 | 1.353 | 1.335 | 0.979× | 0.966× | — |
| CLBG::Nbody | 1.024 | 1.054 | 1.058 | 1.029× | 1.034× | — |
| CLBG::Spectralnorm | 1.187 | 1.139 | 1.737 | 0.960× | 1.463× | — |
| CSV::Parse | 1.210 | 1.087 | 1.141 | 0.899× | 0.943× | — |
| Calculator::Ast | 1.201 | 0.970 | 0.986 | 0.811× | 0.824× | — |
| Calculator::Interpreter | 1.382 | 1.326 | 1.462 | 0.958× | 1.042× | gcc |
| Compress::ArithDecode | 0.927 | 0.945 | 1.603 | 1.022× | 1.730× | — |
| Compress::ArithEncode | 1.047 | 0.980 | 1.110 | 0.935× | 1.060× | — |
| Compress::BWTDecode | 0.661 | 0.653 | 0.664 | 0.977× | 1.006× | — |
| Compress::BWTEncode | 1.204 | 1.105 | 1.364 | 0.912× | 1.118× | gcc |
| Compress::HuffDecode | 0.661 | 0.778 | 0.518 | 1.170× | 0.785× | — |
| Compress::HuffEncode | 0.857 | 1.230 | 1.032 | 1.440× | 1.206× | — |
| Compress::LZWDecode | 0.728 | 0.732 | 0.696 | 1.010× | 0.952× | — |
| Compress::LZWEncode | 1.338 | 1.288 | 1.310 | 0.958× | 0.981× | — |
| Distance::Jaro | 1.204 | 1.171 | 1.148 | 0.971× | 0.953× | — |
| Distance::NGram | 1.361 | 1.326 | 1.293 | 0.971× | 0.952× | — |
| Etc::CacheSimulation | 1.145 | 1.106 | 1.131 | 0.965× | 0.982× | — |
| Etc::GameOfLife | 1.518 | 1.240 | 1.260 | 0.825× | 0.831× | gcc |
| Etc::LogParser | 0.593 | 1.980 | 0.663 | 3.330× | 1.115× | gcc |
| Etc::NeuralNet | 1.268 | 1.115 | 1.079 | 0.869× | 0.863× | gcc |
| Etc::Sieve | 1.385 | 1.831 | 1.471 | 1.387× | 0.999× | gcc |
| Etc::TextRaytracer | 1.531 | 1.022 | 1.339 | 0.668× | 0.879× | — |
| Etc::Words | 1.327 | 1.289 | 1.337 | 0.970× | 1.003× | — |
| Graph::AStar | 1.325 | 1.316 | 1.066 | 0.995× | 0.811× | — |
| Graph::BFS | 1.872 | 1.293 | 1.175 | 0.669× | 0.641× | gcc, clang |
| Graph::DFS | 1.370 | 1.212 | 1.239 | 0.867× | 0.919× | gcc, clang |
| Hash::CRC32 | 0.293 | 0.806 | 0.915 | 2.751× | 3.123× | — |
| Hash::SHA256 | 1.649 | 1.472 | 1.379 | 0.893× | 0.835× | — |
| Json::Generate | 1.123 | 1.133 | 1.141 | 1.008× | 1.014× | — |
| Json::ParseDom | 1.264 | 1.189 | 1.192 | 0.950× | 0.933× | — |
| Json::ParseMapping | 1.200 | 1.133 | 1.136 | 0.950× | 0.946× | — |
| Matmul::Single | 1.390 | 1.389 | 1.387 | 0.999× | 0.998× | — |
| Matmul::T16 | 2.632 | 2.649 | 2.639 | 1.006× | 1.005× | — |
| Matmul::T4 | 1.355 | 1.356 | 1.355 | 1.001× | 1.000× | — |
| Matmul::T8 | 1.373 | 1.368 | 1.373 | 0.991× | 1.000× | — |
| Maze::AStar | 1.429 | 1.263 | 1.413 | 0.875× | 1.008× | gcc, clang |
| Maze::BFS | 1.624 | 1.628 | 1.641 | 1.066× | 1.037× | gcc |
| Maze::Generator | 1.312 | 1.341 | 1.288 | 1.005× | 1.003× | gcc |
| Sort::Merge | 1.124 | 1.131 | 1.027 | 1.009× | 0.917× | — |
| Sort::Quick | 1.352 | 1.236 | 1.253 | 0.913× | 0.927× | — |
| Sort::Self | 1.348 | 1.212 | 1.231 | 0.899× | 0.911× | — |
| Template::Parse | 1.497 | 1.542 | 1.511 | 1.026× | 1.006× | gcc |
| Template::Regex | 1.196 | 1.488 | 1.639 | 1.241× | 1.397× | gcc |

A/A warnings flag variation above 2%; no cases are dropped. Small timing differences need confirmation.
Compile gcc A/A CPU ratio: 1.001×.
Compile clang A/A CPU ratio: 0.999×.

Hardware counters: unavailable: perf is not installed on this host.

## Toolchains

- **cppgm**: cppgm++ (host configuration: g++)
- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and header hashes](metadata.json) · [Raw compile observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42506478683) · [Raw runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42506478683)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42506478683)
