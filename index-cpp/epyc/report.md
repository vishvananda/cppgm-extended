# index-cpp benchmarks

Compiler checkout: `a0093889216091a98b4ab4561769544b02092c58` · CPU: AMD EPYC 9V45 96-Core Processor

Unchanged upstream: `79e0b423094d420da37fb714f20c53fc35eeb2a3`. All 50 checksums pass with cppgm, GCC and Clang. Full runtime inputs, paired ABBA/BAAB blocks with A/A calibration. Speeds are reference time / cppgm time: higher is better. The aggregate gives all 50 workloads equal weight and is separate from the public suite.

| Measurement | vs GCC | vs Clang |
|---|---:|---:|
| Generated code | 1.021× | 1.016× |
| Compile CPU | 6.463× | 4.875× |

| Compiler | Executable bytes | .text bytes | Compile peak RSS (KiB) |
|---|---:|---:|---:|
| cppgm | 714,480 | 341,490 | 228,466 |
| gcc | 540,112 | 280,921 | 486,042 |
| clang | 513,720 | 264,561 | 318,846 |

| Workload | cppgm seconds | GCC seconds | Clang seconds | vs GCC | vs Clang | A/A warning |
|---|---:|---:|---:|---:|---:|---|
| Base64::Decode | 0.919 | 0.677 | 0.709 | 0.738× | 0.773× | gcc |
| Base64::Encode | 0.937 | 0.873 | 0.669 | 0.932× | 0.716× | gcc, clang |
| Binarytrees::Arena | 1.579 | 1.653 | 1.695 | 1.048× | 1.085× | clang |
| Binarytrees::Obj | 0.703 | 0.655 | 0.750 | 0.926× | 1.063× | gcc, clang |
| Brainfuck::Array | 1.227 | 1.393 | 1.243 | 1.127× | 1.026× | gcc |
| Brainfuck::Recursion | 0.875 | 0.736 | 0.845 | 0.847× | 0.973× | gcc |
| CLBG::Fannkuchredux | 0.794 | 0.768 | 0.798 | 0.990× | 1.011× | gcc, clang |
| CLBG::Mandelbrot | 0.911 | 0.841 | 0.869 | 0.926× | 0.951× | gcc, clang |
| CLBG::Nbody | 0.631 | 0.631 | 0.638 | 1.004× | 1.003× | gcc |
| CLBG::Spectralnorm | 0.730 | 0.719 | 1.013 | 0.986× | 1.383× | — |
| CSV::Parse | 0.649 | 0.593 | 0.626 | 0.907× | 0.970× | gcc |
| Calculator::Ast | 0.599 | 0.601 | 0.586 | 0.994× | 0.995× | gcc |
| Calculator::Interpreter | 0.742 | 0.692 | 0.771 | 0.935× | 1.059× | gcc, clang |
| Compress::ArithDecode | 0.711 | 0.669 | 1.212 | 0.942× | 1.690× | gcc |
| Compress::ArithEncode | 0.831 | 0.778 | 0.785 | 0.940× | 0.934× | gcc |
| Compress::BWTDecode | 0.417 | 0.407 | 0.410 | 0.980× | 0.988× | gcc |
| Compress::BWTEncode | 0.591 | 0.613 | 0.728 | 1.032× | 1.237× | gcc |
| Compress::HuffDecode | 0.381 | 0.387 | 0.260 | 1.012× | 0.678× | gcc |
| Compress::HuffEncode | 0.511 | 0.504 | 0.688 | 0.985× | 1.339× | gcc |
| Compress::LZWDecode | 0.407 | 0.416 | 0.392 | 1.021× | 0.963× | gcc |
| Compress::LZWEncode | 0.744 | 0.736 | 0.732 | 0.978× | 0.986× | gcc |
| Distance::Jaro | 0.950 | 0.903 | 0.894 | 0.947× | 0.941× | gcc |
| Distance::NGram | 0.911 | 0.875 | 0.871 | 0.970× | 0.958× | gcc |
| Etc::CacheSimulation | 0.638 | 0.621 | 0.639 | 0.956× | 0.989× | gcc |
| Etc::GameOfLife | 0.843 | 0.586 | 0.610 | 0.721× | 0.718× | clang |
| Etc::LogParser | 0.293 | 0.895 | 0.286 | 3.008× | 0.950× | clang |
| Etc::NeuralNet | 0.702 | 0.609 | 0.619 | 0.868× | 0.895× | gcc, clang |
| Etc::Sieve | 0.918 | 1.282 | 0.851 | 1.378× | 0.927× | clang |
| Etc::TextRaytracer | 0.804 | 0.583 | 0.720 | 0.730× | 0.896× | gcc |
| Etc::Words | 0.738 | 0.707 | 0.752 | 0.951× | 1.002× | — |
| Graph::AStar | 1.075 | 1.058 | 0.907 | 0.972× | 0.864× | — |
| Graph::BFS | 1.071 | 1.161 | 1.007 | 1.121× | 0.948× | gcc, clang |
| Graph::DFS | 1.140 | 0.994 | 1.014 | 0.877× | 0.906× | — |
| Hash::CRC32 | 0.209 | 0.577 | 0.654 | 2.745× | 3.148× | — |
| Hash::SHA256 | 0.464 | 0.902 | 0.923 | 1.933× | 2.000× | — |
| Json::Generate | 0.667 | 0.655 | 0.653 | 0.996× | 0.979× | — |
| Json::ParseDom | 0.790 | 0.754 | 0.748 | 0.948× | 0.937× | — |
| Json::ParseMapping | 0.750 | 0.716 | 0.713 | 0.953× | 0.942× | — |
| Matmul::Single | 0.629 | 0.625 | 0.626 | 0.984× | 0.994× | — |
| Matmul::T16 | 1.294 | 1.315 | 1.316 | 1.017× | 1.011× | — |
| Matmul::T4 | 0.630 | 0.637 | 0.629 | 1.012× | 0.984× | — |
| Matmul::T8 | 0.657 | 0.655 | 0.677 | 0.998× | 1.012× | gcc |
| Maze::AStar | 0.972 | 0.898 | 0.924 | 0.910× | 0.987× | gcc, clang |
| Maze::BFS | 0.829 | 0.796 | 0.764 | 0.969× | 0.972× | gcc, clang |
| Maze::Generator | 1.184 | 1.172 | 1.137 | 0.994× | 0.966× | gcc, clang |
| Sort::Merge | 0.877 | 0.943 | 0.682 | 1.056× | 0.786× | — |
| Sort::Quick | 1.071 | 1.038 | 1.071 | 0.966× | 0.999× | gcc, clang |
| Sort::Self | 1.071 | 0.989 | 1.002 | 0.918× | 0.951× | — |
| Template::Parse | 0.600 | 0.603 | 0.619 | 1.008× | 1.026× | gcc |
| Template::Regex | 0.679 | 0.734 | 0.774 | 1.065× | 1.151× | gcc, clang |

A/A warnings flag variation above 2%; no cases are dropped. Small timing differences need confirmation.
Compile gcc A/A CPU ratio: 1.006×.
Compile clang A/A CPU ratio: 0.989×.

Hardware counters: unavailable: perf is not installed on this host.

## Toolchains

- **cppgm**: cppgm++ (host configuration: g++)
- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and header hashes](metadata.json) · [Raw compile observations](compile-observations.jsonl) · [Raw runtime observations](runtime-observations.jsonl)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43509589179)
