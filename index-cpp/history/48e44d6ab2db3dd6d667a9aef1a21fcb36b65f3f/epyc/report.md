# index-cpp benchmarks

Compiler checkout: `48e44d6ab2db3dd6d667a9aef1a21fcb36b65f3f` · CPU: AMD EPYC 7763 64-Core Processor

Unchanged upstream: `79e0b423094d420da37fb714f20c53fc35eeb2a3`. All 50 checksums pass with cppgm, GCC and Clang. Full runtime inputs, paired ABBA/BAAB blocks with A/A calibration. Speeds are reference time / cppgm time: higher is better. The aggregate gives all 50 workloads equal weight and is separate from the public suite.

| Measurement | vs GCC | vs Clang |
|---|---:|---:|
| Generated code | 0.988× | 0.985× |
| Compile CPU | 6.747× | 6.184× |

| Compiler | Executable bytes | .text bytes | Compile peak RSS (KiB) |
|---|---:|---:|---:|
| cppgm | 839,456 | 401,266 | 229,178 |
| gcc | 540,112 | 280,921 | 486,002 |
| clang | 513,720 | 264,561 | 319,212 |

| Workload | cppgm seconds | GCC seconds | Clang seconds | vs GCC | vs Clang | A/A warning |
|---|---:|---:|---:|---:|---:|---|
| Base64::Decode | 2.978 | 1.200 | 1.210 | 0.405× | 0.406× | — |
| Base64::Encode | 1.838 | 1.449 | 1.294 | 0.793× | 0.696× | — |
| Binarytrees::Arena | 2.399 | 2.486 | 2.546 | 1.033× | 1.054× | — |
| Binarytrees::Obj | 1.845 | 1.411 | 1.692 | 0.772× | 0.916× | gcc |
| Brainfuck::Array | 3.289 | 2.811 | 2.843 | 0.857× | 0.844× | clang |
| Brainfuck::Recursion | 1.647 | 1.741 | 1.681 | 1.060× | 1.022× | — |
| CLBG::Fannkuchredux | 1.064 | 1.137 | 0.979 | 1.087× | 0.921× | — |
| CLBG::Mandelbrot | 1.381 | 1.351 | 1.336 | 0.978× | 0.967× | — |
| CLBG::Nbody | 1.026 | 1.058 | 1.058 | 1.031× | 1.031× | — |
| CLBG::Spectralnorm | 1.180 | 1.140 | 1.738 | 0.965× | 1.472× | — |
| CSV::Parse | 1.199 | 1.081 | 1.140 | 0.898× | 0.948× | — |
| Calculator::Ast | 1.240 | 0.951 | 0.984 | 0.765× | 0.794× | — |
| Calculator::Interpreter | 1.399 | 1.304 | 1.476 | 0.942× | 1.048× | — |
| Compress::ArithDecode | 0.907 | 0.946 | 1.604 | 1.043× | 1.769× | — |
| Compress::ArithEncode | 1.047 | 0.981 | 1.110 | 0.937× | 1.061× | — |
| Compress::BWTDecode | 0.655 | 0.647 | 0.661 | 0.985× | 0.998× | — |
| Compress::BWTEncode | 1.189 | 1.094 | 1.355 | 0.920× | 1.129× | gcc |
| Compress::HuffDecode | 0.652 | 0.774 | 0.516 | 1.183× | 0.790× | — |
| Compress::HuffEncode | 0.856 | 1.227 | 1.032 | 1.440× | 1.203× | — |
| Compress::LZWDecode | 0.722 | 0.725 | 0.696 | 1.004× | 0.965× | — |
| Compress::LZWEncode | 1.333 | 1.289 | 1.308 | 0.964× | 0.982× | — |
| Distance::Jaro | 1.203 | 1.171 | 1.149 | 0.973× | 0.956× | — |
| Distance::NGram | 1.368 | 1.325 | 1.296 | 0.964× | 0.954× | — |
| Etc::CacheSimulation | 1.138 | 1.100 | 1.121 | 0.969× | 0.986× | clang |
| Etc::GameOfLife | 1.476 | 1.163 | 1.216 | 0.786× | 0.826× | gcc, clang |
| Etc::LogParser | 0.599 | 2.006 | 0.657 | 3.327× | 1.098× | gcc |
| Etc::NeuralNet | 1.288 | 1.103 | 1.086 | 0.814× | 0.842× | gcc |
| Etc::Sieve | 1.386 | 1.893 | 1.456 | 1.370× | 0.993× | gcc, clang |
| Etc::TextRaytracer | 1.137 | 1.024 | 1.339 | 0.900× | 1.177× | — |
| Etc::Words | 1.340 | 1.284 | 1.329 | 0.969× | 0.987× | — |
| Graph::AStar | 1.345 | 1.332 | 1.088 | 0.990× | 0.801× | — |
| Graph::BFS | 1.897 | 1.281 | 1.203 | 0.671× | 0.634× | gcc |
| Graph::DFS | 1.377 | 1.216 | 1.263 | 0.884× | 0.902× | — |
| Hash::CRC32 | 0.293 | 0.806 | 0.916 | 2.751× | 3.126× | — |
| Hash::SHA256 | 1.655 | 1.473 | 1.379 | 0.890× | 0.834× | — |
| Json::Generate | 1.111 | 1.127 | 1.132 | 1.012× | 1.019× | — |
| Json::ParseDom | 1.286 | 1.195 | 1.191 | 0.929× | 0.927× | — |
| Json::ParseMapping | 1.223 | 1.136 | 1.137 | 0.930× | 0.932× | — |
| Matmul::Single | 1.388 | 1.387 | 1.383 | 0.999× | 0.997× | — |
| Matmul::T16 | 2.633 | 2.631 | 2.641 | 0.995× | 1.004× | — |
| Matmul::T4 | 1.353 | 1.353 | 1.353 | 1.000× | 1.000× | — |
| Matmul::T8 | 1.370 | 1.364 | 1.373 | 0.996× | 1.003× | — |
| Maze::AStar | 1.405 | 1.204 | 1.490 | 0.873× | 0.988× | clang |
| Maze::BFS | 1.597 | 1.526 | 1.664 | 0.957× | 1.001× | gcc, clang |
| Maze::Generator | 1.369 | 1.324 | 1.331 | 0.981× | 0.953× | gcc |
| Sort::Merge | 1.161 | 1.123 | 1.027 | 0.970× | 0.884× | — |
| Sort::Quick | 1.335 | 1.234 | 1.253 | 0.925× | 0.939× | — |
| Sort::Self | 1.320 | 1.208 | 1.232 | 0.916× | 0.933× | — |
| Template::Parse | 1.376 | 1.343 | 1.434 | 0.955× | 1.070× | gcc, clang |
| Template::Regex | 1.122 | 1.409 | 1.603 | 1.274× | 1.476× | gcc, clang |

A/A warnings flag variation above 2%; no cases are dropped. Small timing differences need confirmation.
Compile gcc A/A CPU ratio: 1.020×.
Compile clang A/A CPU ratio: 0.996×.

Hardware counters: unavailable: perf is not installed on this host.

## Toolchains

- **cppgm**: cppgm++ (host configuration: g++)
- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and header hashes](metadata.json) · [Raw compile observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204744) · [Raw runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204744)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204744)
