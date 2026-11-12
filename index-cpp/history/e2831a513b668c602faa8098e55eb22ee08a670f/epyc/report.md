# index-cpp benchmarks

Compiler checkout: `e2831a513b668c602faa8098e55eb22ee08a670f` · CPU: AMD EPYC 9V74 80-Core Processor

Unchanged upstream: `79e0b423094d420da37fb714f20c53fc35eeb2a3`. All 50 checksums pass with cppgm, GCC and Clang. Full runtime inputs, paired ABBA/BAAB blocks with A/A calibration. Speeds are reference time / cppgm time: higher is better. The aggregate gives all 50 workloads equal weight and is separate from the public suite.

| Measurement | vs GCC | vs Clang |
|---|---:|---:|
| Generated code | 0.995× | 0.983× |
| Compile CPU | 6.484× | 5.394× |

| Compiler | Executable bytes | .text bytes | Compile peak RSS (KiB) |
|---|---:|---:|---:|
| cppgm | 713,168 | 342,450 | 228,166 |
| gcc | 540,112 | 280,921 | 486,018 |
| clang | 513,720 | 264,561 | 318,850 |

| Workload | cppgm seconds | GCC seconds | Clang seconds | vs GCC | vs Clang | A/A warning |
|---|---:|---:|---:|---:|---:|---|
| Base64::Decode | 2.304 | 1.215 | 1.269 | 0.527× | 0.551× | — |
| Base64::Encode | 1.724 | 1.518 | 1.272 | 0.882× | 0.740× | — |
| Binarytrees::Arena | 2.596 | 2.694 | 2.660 | 1.037× | 1.028× | — |
| Binarytrees::Obj | 2.317 | 2.216 | 2.372 | 0.955× | 1.018× | clang |
| Brainfuck::Array | 2.827 | 2.756 | 1.922 | 0.975× | 0.685× | gcc, clang |
| Brainfuck::Recursion | 1.418 | 1.519 | 1.480 | 1.073× | 1.049× | — |
| CLBG::Fannkuchredux | 1.190 | 1.260 | 1.157 | 1.056× | 0.979× | — |
| CLBG::Mandelbrot | 1.597 | 1.509 | 1.438 | 0.945× | 0.900× | — |
| CLBG::Nbody | 1.145 | 1.149 | 1.154 | 1.002× | 1.007× | — |
| CLBG::Spectralnorm | 1.208 | 1.171 | 2.011 | 0.968× | 1.657× | — |
| CSV::Parse | 1.246 | 1.054 | 1.164 | 0.847× | 0.935× | — |
| Calculator::Ast | 1.280 | 0.955 | 0.997 | 0.743× | 0.780× | — |
| Calculator::Interpreter | 1.395 | 1.314 | 1.456 | 0.944× | 1.044× | — |
| Compress::ArithDecode | 1.016 | 1.054 | 1.800 | 1.035× | 1.773× | — |
| Compress::ArithEncode | 1.205 | 1.101 | 1.227 | 0.915× | 1.017× | — |
| Compress::BWTDecode | 0.714 | 0.693 | 0.722 | 0.971× | 1.011× | — |
| Compress::BWTEncode | 1.082 | 1.098 | 1.270 | 1.015× | 1.173× | — |
| Compress::HuffDecode | 0.887 | 0.767 | 0.532 | 0.863× | 0.601× | — |
| Compress::HuffEncode | 1.163 | 1.001 | 1.183 | 0.860× | 1.015× | — |
| Compress::LZWDecode | 0.739 | 0.757 | 0.700 | 1.026× | 0.948× | — |
| Compress::LZWEncode | 1.289 | 1.260 | 1.274 | 0.980× | 0.991× | — |
| Distance::Jaro | 1.373 | 1.356 | 1.260 | 0.988× | 0.918× | — |
| Distance::NGram | 1.425 | 1.381 | 1.359 | 0.966× | 0.954× | — |
| Etc::CacheSimulation | 1.172 | 1.137 | 1.152 | 0.968× | 0.981× | — |
| Etc::GameOfLife | 1.306 | 1.185 | 1.256 | 0.912× | 0.962× | — |
| Etc::LogParser | 0.552 | 2.255 | 0.660 | 4.051× | 1.201× | — |
| Etc::NeuralNet | 1.228 | 1.065 | 1.054 | 0.866× | 0.857× | clang |
| Etc::Sieve | 1.525 | 2.016 | 1.534 | 1.322× | 1.008× | — |
| Etc::TextRaytracer | 1.445 | 1.129 | 1.300 | 0.782× | 0.907× | — |
| Etc::Words | 1.301 | 1.261 | 1.323 | 0.972× | 1.013× | — |
| Graph::AStar | 1.469 | 1.446 | 1.143 | 0.983× | 0.776× | — |
| Graph::BFS | 1.645 | 1.338 | 1.230 | 0.805× | 0.738× | — |
| Graph::DFS | 1.652 | 1.382 | 1.425 | 0.834× | 0.864× | — |
| Hash::CRC32 | 0.331 | 0.910 | 1.034 | 2.749× | 3.124× | — |
| Hash::SHA256 | 1.544 | 1.510 | 1.520 | 0.974× | 0.985× | — |
| Json::Generate | 1.016 | 0.996 | 0.983 | 0.977× | 0.973× | — |
| Json::ParseDom | 1.231 | 1.198 | 1.155 | 0.966× | 0.940× | — |
| Json::ParseMapping | 1.171 | 1.143 | 1.103 | 0.965× | 0.948× | — |
| Matmul::Single | 1.516 | 1.512 | 1.515 | 0.998× | 0.999× | — |
| Matmul::T16 | 2.956 | 2.931 | 2.930 | 0.996× | 0.991× | — |
| Matmul::T4 | 1.502 | 1.502 | 1.495 | 1.000× | 0.994× | — |
| Matmul::T8 | 1.550 | 1.539 | 1.539 | 0.993× | 0.990× | — |
| Maze::AStar | 1.197 | 1.118 | 1.265 | 0.939× | 1.058× | — |
| Maze::BFS | 1.050 | 0.991 | 1.022 | 0.946× | 0.972× | gcc |
| Maze::Generator | 1.312 | 1.317 | 1.341 | 1.018× | 1.022× | — |
| Sort::Merge | 1.347 | 1.272 | 1.132 | 0.945× | 0.851× | — |
| Sort::Quick | 1.457 | 1.399 | 1.417 | 0.961× | 0.971× | — |
| Sort::Self | 1.385 | 1.341 | 1.363 | 0.968× | 0.984× | — |
| Template::Parse | 1.185 | 1.212 | 1.252 | 1.034× | 1.050× | — |
| Template::Regex | 1.326 | 1.353 | 1.436 | 1.029× | 1.086× | — |

A/A warnings flag variation above 2%; no cases are dropped. Small timing differences need confirmation.
Compile gcc A/A CPU ratio: 1.001×.
Compile clang A/A CPU ratio: 0.997×.

Hardware counters: unavailable: perf is not installed on this host.

## Toolchains

- **cppgm**: cppgm++ (host configuration: g++)
- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and header hashes](metadata.json) · [Raw compile observations](compile-observations.jsonl) · [Raw runtime observations](runtime-observations.jsonl)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43133898491)
