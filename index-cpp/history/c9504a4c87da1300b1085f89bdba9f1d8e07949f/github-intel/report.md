# index-cpp benchmarks

Compiler checkout: `c9504a4c87da1300b1085f89bdba9f1d8e07949f` · CPU: Intel(R) Xeon(R) Platinum 8370C CPU @ 2.80GHz

Unchanged upstream: `79e0b423094d420da37fb714f20c53fc35eeb2a3`. All 50 checksums pass with cppgm, GCC and Clang. Full runtime inputs, paired ABBA/BAAB blocks with A/A calibration. Speeds are reference time / cppgm time: higher is better. The aggregate gives all 50 workloads equal weight and is separate from the public suite.

| Measurement | vs GCC | vs Clang |
|---|---:|---:|
| Generated code | 1.028× | 1.015× |
| Compile CPU | 6.708× | 5.660× |

| Compiler | Executable bytes | .text bytes | Compile peak RSS (KiB) |
|---|---:|---:|---:|
| cppgm | 823,072 | 385,154 | 229,144 |
| gcc | 540,112 | 280,921 | 486,000 |
| clang | 513,720 | 264,561 | 319,220 |

| Workload | cppgm seconds | GCC seconds | Clang seconds | vs GCC | vs Clang | A/A warning |
|---|---:|---:|---:|---:|---:|---|
| Base64::Decode | 1.681 | 1.046 | 1.163 | 0.623× | 0.692× | — |
| Base64::Encode | 1.586 | 1.513 | 1.216 | 0.957× | 0.766× | gcc |
| Binarytrees::Arena | 2.223 | 2.284 | 2.336 | 1.032× | 1.052× | clang |
| Binarytrees::Obj | 1.669 | 1.520 | 1.727 | 0.912× | 1.027× | gcc |
| Brainfuck::Array | 1.701 | 2.075 | 1.978 | 1.221× | 1.160× | — |
| Brainfuck::Recursion | 1.364 | 1.608 | 1.359 | 1.180× | 0.996× | clang |
| CLBG::Fannkuchredux | 0.960 | 1.080 | 0.955 | 1.126× | 0.984× | — |
| CLBG::Mandelbrot | 1.696 | 1.645 | 1.655 | 0.969× | 0.974× | — |
| CLBG::Nbody | 0.919 | 0.883 | 0.903 | 0.961× | 0.985× | — |
| CLBG::Spectralnorm | 1.284 | 1.341 | 1.802 | 1.044× | 1.399× | — |
| CSV::Parse | 1.102 | 1.016 | 1.094 | 0.925× | 0.995× | — |
| Calculator::Ast | 1.397 | 1.089 | 1.097 | 0.777× | 0.783× | — |
| Calculator::Interpreter | 1.989 | 1.710 | 1.970 | 0.886× | 0.962× | clang |
| Compress::ArithDecode | 1.046 | 1.138 | 1.644 | 1.092× | 1.571× | — |
| Compress::ArithEncode | 1.181 | 1.190 | 1.252 | 1.006× | 1.060× | — |
| Compress::BWTDecode | 1.394 | 1.354 | 1.454 | 0.987× | 1.026× | — |
| Compress::BWTEncode | 1.105 | 1.074 | 1.199 | 0.972× | 1.082× | — |
| Compress::HuffDecode | 0.754 | 0.889 | 0.511 | 1.196× | 0.675× | — |
| Compress::HuffEncode | 0.776 | 0.998 | 1.082 | 1.280× | 1.394× | gcc |
| Compress::LZWDecode | 0.754 | 0.761 | 0.728 | 1.001× | 0.965× | gcc, clang |
| Compress::LZWEncode | 1.118 | 1.107 | 1.129 | 0.993× | 1.011× | — |
| Distance::Jaro | 1.365 | 1.288 | 1.295 | 0.944× | 0.951× | — |
| Distance::NGram | 1.482 | 1.414 | 1.363 | 0.955× | 0.917× | — |
| Etc::CacheSimulation | 1.034 | 0.987 | 1.034 | 0.954× | 1.014× | — |
| Etc::GameOfLife | 2.178 | 1.762 | 2.134 | 0.925× | 0.921× | gcc, clang |
| Etc::LogParser | 0.601 | 1.368 | 0.577 | 2.254× | 0.961× | — |
| Etc::NeuralNet | 1.206 | 1.040 | 1.035 | 0.857× | 0.863× | — |
| Etc::Sieve | 2.700 | 3.413 | 2.783 | 1.260× | 1.044× | gcc, clang |
| Etc::TextRaytracer | 1.134 | 0.995 | 1.206 | 0.877× | 1.064× | — |
| Etc::Words | 1.193 | 1.159 | 1.198 | 0.971× | 1.012× | — |
| Graph::AStar | 1.144 | 1.148 | 1.026 | 1.005× | 0.892× | clang |
| Graph::BFS | 0.960 | 0.909 | 0.915 | 0.947× | 0.954× | — |
| Graph::DFS | 1.010 | 1.027 | 1.035 | 1.037× | 1.021× | — |
| Hash::CRC32 | 0.281 | 0.829 | 0.958 | 2.966× | 3.409× | — |
| Hash::SHA256 | 1.264 | 1.357 | 1.278 | 1.074× | 1.011× | — |
| Json::Generate | 1.147 | 1.137 | 1.150 | 1.000× | 0.989× | — |
| Json::ParseDom | 1.465 | 1.304 | 1.347 | 0.913× | 0.898× | — |
| Json::ParseMapping | 1.397 | 1.252 | 1.288 | 0.917× | 0.917× | — |
| Matmul::Single | 1.661 | 1.654 | 1.643 | 0.997× | 0.992× | — |
| Matmul::T16 | 3.276 | 3.237 | 3.272 | 1.004× | 0.985× | gcc, clang |
| Matmul::T4 | 1.646 | 1.644 | 1.648 | 0.999× | 1.000× | — |
| Matmul::T8 | 1.702 | 1.683 | 1.725 | 1.000× | 0.986× | gcc |
| Maze::AStar | 1.554 | 1.497 | 1.661 | 0.974× | 1.064× | clang |
| Maze::BFS | 1.303 | 1.273 | 1.337 | 0.984× | 1.023× | — |
| Maze::Generator | 1.482 | 1.417 | 1.429 | 0.969× | 0.956× | gcc, clang |
| Sort::Merge | 1.137 | 1.201 | 0.984 | 1.054× | 0.866× | — |
| Sort::Quick | 1.324 | 1.334 | 1.312 | 1.008× | 0.994× | — |
| Sort::Self | 1.372 | 1.294 | 1.287 | 0.944× | 0.939× | — |
| Template::Parse | 2.093 | 2.034 | 2.178 | 1.000× | 1.029× | gcc, clang |
| Template::Regex | 1.465 | 1.567 | 1.781 | 1.093× | 1.209× | gcc |

A/A warnings flag variation above 2%; no cases are dropped. Small timing differences need confirmation.
Compile gcc A/A CPU ratio: 1.017×.
Compile clang A/A CPU ratio: 1.013×.

Hardware counters: unavailable: perf is not installed on this host.

## Toolchains

- **cppgm**: cppgm++ (host configuration: g++)
- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and header hashes](metadata.json) · [Raw compile observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42570601123) · [Raw runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42570601123)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42570601123)
