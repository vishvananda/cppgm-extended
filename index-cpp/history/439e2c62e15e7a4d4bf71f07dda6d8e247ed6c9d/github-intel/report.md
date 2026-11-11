# index-cpp benchmarks

Compiler checkout: `439e2c62e15e7a4d4bf71f07dda6d8e247ed6c9d` · CPU: Intel(R) Xeon(R) Platinum 8370C CPU @ 2.80GHz

Unchanged upstream: `79e0b423094d420da37fb714f20c53fc35eeb2a3`. All 50 checksums pass with cppgm, GCC and Clang. Full runtime inputs, paired ABBA/BAAB blocks with A/A calibration. Speeds are reference time / cppgm time: higher is better. The aggregate gives all 50 workloads equal weight and is separate from the public suite.

| Measurement | vs GCC | vs Clang |
|---|---:|---:|
| Generated code | 1.027× | 1.014× |
| Compile CPU | 6.653× | 5.645× |

| Compiler | Executable bytes | .text bytes | Compile peak RSS (KiB) |
|---|---:|---:|---:|
| cppgm | 823,072 | 385,154 | 229,114 |
| gcc | 540,112 | 280,921 | 486,088 |
| clang | 513,720 | 264,561 | 319,076 |

| Workload | cppgm seconds | GCC seconds | Clang seconds | vs GCC | vs Clang | A/A warning |
|---|---:|---:|---:|---:|---:|---|
| Base64::Decode | 1.671 | 1.038 | 1.160 | 0.623× | 0.693× | — |
| Base64::Encode | 1.570 | 1.448 | 1.205 | 0.921× | 0.766× | — |
| Binarytrees::Arena | 2.167 | 2.242 | 2.304 | 1.035× | 1.069× | — |
| Binarytrees::Obj | 1.631 | 1.493 | 1.703 | 0.917× | 1.042× | — |
| Brainfuck::Array | 1.698 | 2.074 | 1.978 | 1.221× | 1.164× | — |
| Brainfuck::Recursion | 1.364 | 1.606 | 1.359 | 1.176× | 0.997× | — |
| CLBG::Fannkuchredux | 0.956 | 1.080 | 0.948 | 1.141× | 0.991× | — |
| CLBG::Mandelbrot | 1.695 | 1.643 | 1.655 | 0.970× | 0.976× | — |
| CLBG::Nbody | 0.918 | 0.882 | 0.902 | 0.962× | 0.983× | — |
| CLBG::Spectralnorm | 1.282 | 1.349 | 1.808 | 1.056× | 1.410× | — |
| CSV::Parse | 1.095 | 1.011 | 1.088 | 0.926× | 0.995× | — |
| Calculator::Ast | 1.400 | 1.079 | 1.091 | 0.769× | 0.784× | — |
| Calculator::Interpreter | 1.728 | 1.551 | 1.720 | 0.895× | 0.968× | clang |
| Compress::ArithDecode | 1.045 | 1.139 | 1.640 | 1.087× | 1.570× | — |
| Compress::ArithEncode | 1.179 | 1.190 | 1.252 | 1.009× | 1.061× | — |
| Compress::BWTDecode | 1.300 | 1.324 | 1.394 | 1.018× | 1.076× | — |
| Compress::BWTEncode | 1.095 | 1.077 | 1.203 | 0.979× | 1.097× | gcc |
| Compress::HuffDecode | 0.743 | 0.895 | 0.506 | 1.216× | 0.680× | — |
| Compress::HuffEncode | 0.775 | 0.986 | 1.079 | 1.274× | 1.389× | — |
| Compress::LZWDecode | 0.750 | 0.750 | 0.728 | 0.989× | 0.971× | — |
| Compress::LZWEncode | 1.118 | 1.107 | 1.128 | 0.990× | 1.010× | — |
| Distance::Jaro | 1.363 | 1.287 | 1.290 | 0.949× | 0.949× | — |
| Distance::NGram | 1.476 | 1.414 | 1.363 | 0.956× | 0.924× | — |
| Etc::CacheSimulation | 1.031 | 0.983 | 1.035 | 0.953× | 1.004× | — |
| Etc::GameOfLife | 1.617 | 1.460 | 1.418 | 0.897× | 0.836× | gcc |
| Etc::LogParser | 0.600 | 1.340 | 0.571 | 2.236× | 0.954× | — |
| Etc::NeuralNet | 1.203 | 1.032 | 1.039 | 0.858× | 0.855× | clang |
| Etc::Sieve | 2.755 | 3.401 | 2.751 | 1.255× | 1.007× | clang |
| Etc::TextRaytracer | 1.133 | 0.994 | 1.207 | 0.877× | 1.064× | — |
| Etc::Words | 1.191 | 1.155 | 1.199 | 0.970× | 1.008× | — |
| Graph::AStar | 1.132 | 1.143 | 1.014 | 1.010× | 0.877× | — |
| Graph::BFS | 0.950 | 0.906 | 0.896 | 0.954× | 0.944× | — |
| Graph::DFS | 1.003 | 1.019 | 1.027 | 1.019× | 1.023× | — |
| Hash::CRC32 | 0.281 | 0.857 | 0.958 | 2.936× | 3.409× | — |
| Hash::SHA256 | 1.264 | 1.356 | 1.276 | 1.072× | 1.010× | — |
| Json::Generate | 1.134 | 1.128 | 1.125 | 0.995× | 0.987× | — |
| Json::ParseDom | 1.400 | 1.277 | 1.284 | 0.913× | 0.909× | — |
| Json::ParseMapping | 1.331 | 1.215 | 1.223 | 0.913× | 0.910× | — |
| Matmul::Single | 1.660 | 1.653 | 1.640 | 0.996× | 0.989× | — |
| Matmul::T16 | 3.223 | 3.234 | 3.209 | 1.009× | 0.998× | — |
| Matmul::T4 | 1.643 | 1.638 | 1.640 | 0.996× | 0.999× | — |
| Matmul::T8 | 1.679 | 1.673 | 1.676 | 0.996× | 0.999× | — |
| Maze::AStar | 1.489 | 1.436 | 1.585 | 0.968× | 1.054× | clang |
| Maze::BFS | 1.222 | 1.204 | 1.281 | 1.004× | 1.024× | gcc, clang |
| Maze::Generator | 1.435 | 1.367 | 1.383 | 0.961× | 0.954× | clang |
| Sort::Merge | 1.132 | 1.200 | 0.980 | 1.059× | 0.865× | — |
| Sort::Quick | 1.321 | 1.326 | 1.312 | 1.003× | 0.994× | — |
| Sort::Self | 1.370 | 1.290 | 1.288 | 0.942× | 0.942× | — |
| Template::Parse | 1.763 | 1.759 | 1.740 | 0.984× | 0.970× | gcc, clang |
| Template::Regex | 1.318 | 1.463 | 1.663 | 1.115× | 1.268× | gcc |

A/A warnings flag variation above 2%; no cases are dropped. Small timing differences need confirmation.
Compile gcc A/A CPU ratio: 1.001×.
Compile clang A/A CPU ratio: 1.003×.

Hardware counters: unavailable: perf is not installed on this host.

## Toolchains

- **cppgm**: cppgm++ (host configuration: g++)
- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and header hashes](metadata.json) · [Raw compile observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42568872210) · [Raw runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42568872210)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42568872210)
