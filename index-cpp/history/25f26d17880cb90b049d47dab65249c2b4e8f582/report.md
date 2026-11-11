# index-cpp benchmarks

Compiler checkout: `25f26d17880cb90b049d47dab65249c2b4e8f582` · CPU: AMD EPYC 7763 64-Core Processor

Unchanged upstream: `79e0b423094d420da37fb714f20c53fc35eeb2a3`. All 50 checksums pass with cppgm, GCC and Clang. Full runtime inputs, paired ABBA/BAAB blocks with A/A calibration. Speeds are reference time / cppgm time: higher is better. The aggregate gives all 50 workloads equal weight and is separate from the public suite.

| Measurement | vs GCC | vs Clang |
|---|---:|---:|
| Generated code | 0.990× | 0.983× |
| Compile CPU | 6.735× | 6.251× |

| Compiler | Executable bytes | .text bytes | Compile peak RSS (KiB) |
|---|---:|---:|---:|
| cppgm | 807,560 | 385,970 | 224,558 |
| gcc | 540,112 | 280,921 | 486,024 |
| clang | 513,720 | 264,561 | 319,114 |

| Workload | cppgm seconds | GCC seconds | Clang seconds | vs GCC | vs Clang | A/A warning |
|---|---:|---:|---:|---:|---:|---|
| Base64::Decode | 2.331 | 1.189 | 1.202 | 0.510× | 0.515× | — |
| Base64::Encode | 1.782 | 1.432 | 1.282 | 0.803× | 0.719× | — |
| Binarytrees::Arena | 2.371 | 2.465 | 2.537 | 1.037× | 1.070× | — |
| Binarytrees::Obj | 1.713 | 1.335 | 1.518 | 0.767× | 0.893× | — |
| Brainfuck::Array | 3.208 | 2.813 | 2.827 | 0.875× | 0.879× | clang |
| Brainfuck::Recursion | 1.653 | 1.740 | 1.691 | 1.053× | 1.026× | clang |
| CLBG::Fannkuchredux | 1.052 | 1.141 | 0.977 | 1.088× | 0.931× | — |
| CLBG::Mandelbrot | 1.381 | 1.351 | 1.336 | 0.978× | 0.967× | — |
| CLBG::Nbody | 1.025 | 1.058 | 1.057 | 1.033× | 1.031× | — |
| CLBG::Spectralnorm | 1.177 | 1.139 | 1.737 | 0.969× | 1.477× | — |
| CSV::Parse | 1.149 | 1.084 | 1.143 | 0.933× | 0.997× | clang |
| Calculator::Ast | 1.227 | 0.970 | 0.980 | 0.782× | 0.798× | — |
| Calculator::Interpreter | 1.310 | 1.296 | 1.372 | 0.986× | 1.048× | — |
| Compress::ArithDecode | 0.936 | 0.946 | 1.605 | 1.011× | 1.714× | — |
| Compress::ArithEncode | 1.050 | 0.982 | 1.110 | 0.935× | 1.057× | — |
| Compress::BWTDecode | 0.657 | 0.645 | 0.657 | 0.979× | 1.003× | — |
| Compress::BWTEncode | 1.183 | 1.098 | 1.356 | 0.927× | 1.146× | — |
| Compress::HuffDecode | 0.657 | 0.774 | 0.514 | 1.169× | 0.783× | — |
| Compress::HuffEncode | 0.859 | 1.231 | 1.033 | 1.434× | 1.204× | — |
| Compress::LZWDecode | 0.724 | 0.729 | 0.697 | 1.004× | 0.966× | — |
| Compress::LZWEncode | 1.331 | 1.290 | 1.303 | 0.977× | 0.976× | — |
| Distance::Jaro | 1.222 | 1.171 | 1.150 | 0.958× | 0.941× | — |
| Distance::NGram | 1.373 | 1.320 | 1.294 | 0.960× | 0.939× | — |
| Etc::CacheSimulation | 1.128 | 1.106 | 1.116 | 0.985× | 0.980× | — |
| Etc::GameOfLife | 1.454 | 1.116 | 1.201 | 0.779× | 0.831× | gcc |
| Etc::LogParser | 0.596 | 1.959 | 0.663 | 3.275× | 1.106× | gcc |
| Etc::NeuralNet | 1.266 | 1.119 | 1.080 | 0.883× | 0.854× | clang |
| Etc::Sieve | 1.389 | 1.820 | 1.383 | 1.306× | 0.997× | — |
| Etc::TextRaytracer | 1.534 | 1.022 | 1.339 | 0.664× | 0.873× | — |
| Etc::Words | 1.361 | 1.296 | 1.324 | 0.953× | 0.968× | — |
| Graph::AStar | 1.320 | 1.301 | 1.054 | 0.986× | 0.794× | — |
| Graph::BFS | 1.810 | 1.207 | 1.107 | 0.662× | 0.625× | — |
| Graph::DFS | 1.340 | 1.173 | 1.216 | 0.869× | 0.913× | — |
| Hash::CRC32 | 0.293 | 0.806 | 0.916 | 2.746× | 3.119× | — |
| Hash::SHA256 | 1.654 | 1.474 | 1.381 | 0.891× | 0.835× | — |
| Json::Generate | 1.145 | 1.129 | 1.136 | 0.985× | 0.993× | — |
| Json::ParseDom | 1.244 | 1.204 | 1.194 | 0.973× | 0.958× | — |
| Json::ParseMapping | 1.183 | 1.143 | 1.135 | 0.972× | 0.961× | — |
| Matmul::Single | 1.391 | 1.391 | 1.388 | 1.000× | 0.998× | — |
| Matmul::T16 | 2.633 | 2.613 | 2.649 | 0.990× | 1.001× | — |
| Matmul::T4 | 1.357 | 1.356 | 1.360 | 0.999× | 1.002× | — |
| Matmul::T8 | 1.379 | 1.381 | 1.381 | 0.980× | 1.001× | — |
| Maze::AStar | 1.267 | 1.129 | 1.251 | 0.910× | 0.974× | gcc, clang |
| Maze::BFS | 1.367 | 1.341 | 1.361 | 0.994× | 1.006× | gcc, clang |
| Maze::Generator | 1.265 | 1.260 | 1.252 | 0.993× | 0.983× | — |
| Sort::Merge | 1.163 | 1.128 | 1.028 | 0.969× | 0.885× | — |
| Sort::Quick | 1.368 | 1.236 | 1.253 | 0.902× | 0.917× | — |
| Sort::Self | 1.324 | 1.208 | 1.232 | 0.914× | 0.931× | — |
| Template::Parse | 1.222 | 1.245 | 1.222 | 1.027× | 1.014× | gcc, clang |
| Template::Regex | 1.078 | 1.306 | 1.496 | 1.204× | 1.379× | clang |

A/A warnings flag variation above 2%; no cases are dropped. Small timing differences need confirmation.
Compile gcc A/A CPU ratio: 1.003×.
Compile clang A/A CPU ratio: 1.005×.

Hardware counters: unavailable: perf is not installed on this host.

## Toolchains

- **cppgm**: cppgm++ (host configuration: g++)
- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and header hashes](metadata.json) · [Raw compile observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42299582443) · [Raw runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42299582443)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42299582443)
