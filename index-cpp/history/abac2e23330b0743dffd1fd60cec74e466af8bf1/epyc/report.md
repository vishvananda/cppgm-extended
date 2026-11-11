# index-cpp benchmarks

Compiler checkout: `abac2e23330b0743dffd1fd60cec74e466af8bf1` · CPU: AMD EPYC 7763 64-Core Processor

Unchanged upstream: `79e0b423094d420da37fb714f20c53fc35eeb2a3`. All 50 checksums pass with cppgm, GCC and Clang. Full runtime inputs, paired ABBA/BAAB blocks with A/A calibration. Speeds are reference time / cppgm time: higher is better. The aggregate gives all 50 workloads equal weight and is separate from the public suite.

| Measurement | vs GCC | vs Clang |
|---|---:|---:|
| Generated code | 0.995× | 0.994× |
| Compile CPU | 6.538× | 6.048× |

| Compiler | Executable bytes | .text bytes | Compile peak RSS (KiB) |
|---|---:|---:|---:|
| cppgm | 798,176 | 388,834 | 228,982 |
| gcc | 540,112 | 280,921 | 485,992 |
| clang | 513,720 | 264,561 | 319,322 |

| Workload | cppgm seconds | GCC seconds | Clang seconds | vs GCC | vs Clang | A/A warning |
|---|---:|---:|---:|---:|---:|---|
| Base64::Decode | 2.984 | 1.200 | 1.221 | 0.404× | 0.411× | — |
| Base64::Encode | 1.778 | 1.447 | 1.319 | 0.816× | 0.739× | — |
| Binarytrees::Arena | 2.403 | 2.464 | 2.558 | 1.028× | 1.064× | — |
| Binarytrees::Obj | 1.809 | 1.458 | 1.627 | 0.782× | 0.864× | clang |
| Brainfuck::Array | 3.345 | 2.807 | 2.853 | 0.839× | 0.856× | clang |
| Brainfuck::Recursion | 1.652 | 1.740 | 1.704 | 1.055× | 1.037× | clang |
| CLBG::Fannkuchredux | 1.068 | 1.147 | 0.981 | 1.070× | 0.917× | — |
| CLBG::Mandelbrot | 1.381 | 1.351 | 1.336 | 0.978× | 0.968× | — |
| CLBG::Nbody | 1.026 | 1.054 | 1.057 | 1.029× | 1.031× | — |
| CLBG::Spectralnorm | 1.185 | 1.140 | 1.739 | 0.962× | 1.468× | — |
| CSV::Parse | 1.125 | 1.083 | 1.145 | 0.962× | 1.018× | — |
| Calculator::Ast | 1.204 | 0.963 | 0.986 | 0.798× | 0.818× | — |
| Calculator::Interpreter | 1.433 | 1.327 | 1.508 | 0.933× | 1.039× | clang |
| Compress::ArithDecode | 0.905 | 0.946 | 1.605 | 1.046× | 1.771× | — |
| Compress::ArithEncode | 1.045 | 0.982 | 1.111 | 0.937× | 1.064× | — |
| Compress::BWTDecode | 0.659 | 0.648 | 0.661 | 0.989× | 0.997× | clang |
| Compress::BWTEncode | 1.187 | 1.106 | 1.370 | 0.935× | 1.139× | gcc |
| Compress::HuffDecode | 0.646 | 0.778 | 0.518 | 1.217× | 0.801× | clang |
| Compress::HuffEncode | 0.856 | 1.220 | 1.032 | 1.433× | 1.206× | — |
| Compress::LZWDecode | 0.718 | 0.728 | 0.696 | 1.012× | 0.969× | — |
| Compress::LZWEncode | 1.337 | 1.292 | 1.302 | 0.969× | 0.973× | — |
| Distance::Jaro | 1.205 | 1.171 | 1.149 | 0.973× | 0.952× | — |
| Distance::NGram | 1.360 | 1.324 | 1.293 | 0.973× | 0.946× | — |
| Etc::CacheSimulation | 1.130 | 1.105 | 1.123 | 0.995× | 0.990× | — |
| Etc::GameOfLife | 1.495 | 1.229 | 1.347 | 0.838× | 0.908× | gcc |
| Etc::LogParser | 0.581 | 1.947 | 0.663 | 3.406× | 1.140× | — |
| Etc::NeuralNet | 1.288 | 1.105 | 1.083 | 0.846× | 0.843× | — |
| Etc::Sieve | 1.397 | 1.827 | 1.429 | 1.288× | 1.034× | gcc, clang |
| Etc::TextRaytracer | 1.154 | 1.024 | 1.339 | 0.886× | 1.160× | — |
| Etc::Words | 1.343 | 1.292 | 1.353 | 0.958× | 1.005× | — |
| Graph::AStar | 1.321 | 1.306 | 1.078 | 0.989× | 0.812× | — |
| Graph::BFS | 1.759 | 1.227 | 1.196 | 0.704× | 0.675× | gcc, clang |
| Graph::DFS | 1.389 | 1.175 | 1.265 | 0.855× | 0.910× | clang |
| Hash::CRC32 | 0.293 | 0.806 | 0.916 | 2.751× | 3.126× | — |
| Hash::SHA256 | 1.649 | 1.473 | 1.378 | 0.893× | 0.835× | — |
| Json::Generate | 1.107 | 1.129 | 1.136 | 1.015× | 1.029× | — |
| Json::ParseDom | 1.314 | 1.188 | 1.204 | 0.912× | 0.916× | — |
| Json::ParseMapping | 1.241 | 1.145 | 1.155 | 0.922× | 0.928× | — |
| Matmul::Single | 1.386 | 1.386 | 1.382 | 1.000× | 0.998× | — |
| Matmul::T16 | 2.632 | 2.628 | 2.640 | 1.000× | 1.001× | — |
| Matmul::T4 | 1.356 | 1.353 | 1.355 | 0.998× | 1.000× | — |
| Matmul::T8 | 1.370 | 1.372 | 1.370 | 1.000× | 0.999× | — |
| Maze::AStar | 1.393 | 1.265 | 1.475 | 0.929× | 1.062× | gcc, clang |
| Maze::BFS | 1.473 | 1.627 | 1.708 | 1.130× | 1.155× | gcc |
| Maze::Generator | 1.353 | 1.286 | 1.321 | 0.966× | 0.979× | — |
| Sort::Merge | 1.212 | 1.129 | 1.028 | 0.932× | 0.849× | — |
| Sort::Quick | 1.337 | 1.232 | 1.253 | 0.923× | 0.937× | — |
| Sort::Self | 1.311 | 1.207 | 1.231 | 0.921× | 0.939× | — |
| Template::Parse | 1.504 | 1.440 | 1.781 | 0.968× | 1.059× | gcc, clang |
| Template::Regex | 1.294 | 1.470 | 1.791 | 1.177× | 1.310× | clang |

A/A warnings flag variation above 2%; no cases are dropped. Small timing differences need confirmation.
Compile gcc A/A CPU ratio: 1.000×.
Compile clang A/A CPU ratio: 0.992×.

Hardware counters: unavailable: perf is not installed on this host.

## Toolchains

- **cppgm**: cppgm++ (host configuration: g++)
- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and header hashes](metadata.json) · [Raw compile observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924193) · [Raw runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924193)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924193)
