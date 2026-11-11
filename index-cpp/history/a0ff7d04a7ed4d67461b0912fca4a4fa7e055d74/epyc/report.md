# index-cpp benchmarks

Compiler checkout: `a0ff7d04a7ed4d67461b0912fca4a4fa7e055d74` · CPU: AMD EPYC 7763 64-Core Processor

Unchanged upstream: `79e0b423094d420da37fb714f20c53fc35eeb2a3`. All 50 checksums pass with cppgm, GCC and Clang. Full runtime inputs, paired ABBA/BAAB blocks with A/A calibration. Speeds are reference time / cppgm time: higher is better. The aggregate gives all 50 workloads equal weight and is separate from the public suite.

| Measurement | vs GCC | vs Clang |
|---|---:|---:|
| Generated code | 0.989× | 0.985× |
| Compile CPU | 6.447× | 5.841× |

| Compiler | Executable bytes | .text bytes | Compile peak RSS (KiB) |
|---|---:|---:|---:|
| cppgm | 801,632 | 389,890 | 228,138 |
| gcc | 540,112 | 280,921 | 485,972 |
| clang | 513,720 | 264,561 | 319,232 |

| Workload | cppgm seconds | GCC seconds | Clang seconds | vs GCC | vs Clang | A/A warning |
|---|---:|---:|---:|---:|---:|---|
| Base64::Decode | 2.975 | 1.208 | 1.239 | 0.410× | 0.416× | — |
| Base64::Encode | 1.720 | 1.455 | 1.318 | 0.850× | 0.756× | clang |
| Binarytrees::Arena | 2.622 | 2.691 | 2.769 | 1.023× | 1.074× | — |
| Binarytrees::Obj | 1.924 | 1.568 | 1.750 | 0.808× | 0.926× | gcc, clang |
| Brainfuck::Array | 3.234 | 2.815 | 2.853 | 0.873× | 0.861× | clang |
| Brainfuck::Recursion | 1.673 | 1.744 | 1.711 | 1.044× | 1.023× | — |
| CLBG::Fannkuchredux | 1.109 | 1.143 | 0.982 | 1.033× | 0.886× | — |
| CLBG::Mandelbrot | 1.387 | 1.352 | 1.337 | 0.974× | 0.962× | — |
| CLBG::Nbody | 1.026 | 1.057 | 1.058 | 1.031× | 1.030× | — |
| CLBG::Spectralnorm | 1.187 | 1.140 | 1.736 | 0.959× | 1.465× | — |
| CSV::Parse | 1.143 | 1.094 | 1.159 | 0.958× | 1.013× | gcc |
| Calculator::Ast | 1.264 | 0.990 | 1.003 | 0.780× | 0.798× | gcc |
| Calculator::Interpreter | 1.359 | 1.397 | 1.424 | 0.961× | 1.053× | clang |
| Compress::ArithDecode | 0.939 | 0.954 | 1.610 | 1.015× | 1.713× | — |
| Compress::ArithEncode | 1.110 | 0.990 | 1.115 | 0.895× | 1.002× | — |
| Compress::BWTDecode | 0.656 | 0.647 | 0.653 | 0.989× | 0.981× | — |
| Compress::BWTEncode | 1.179 | 1.103 | 1.347 | 0.932× | 1.143× | clang |
| Compress::HuffDecode | 0.645 | 0.782 | 0.524 | 1.205× | 0.816× | — |
| Compress::HuffEncode | 0.877 | 1.230 | 1.046 | 1.412× | 1.187× | — |
| Compress::LZWDecode | 0.732 | 0.739 | 0.700 | 1.012× | 0.958× | gcc |
| Compress::LZWEncode | 1.306 | 1.291 | 1.312 | 0.986× | 1.000× | clang |
| Distance::Jaro | 1.238 | 1.184 | 1.157 | 0.959× | 0.932× | — |
| Distance::NGram | 1.409 | 1.352 | 1.323 | 0.963× | 0.935× | — |
| Etc::CacheSimulation | 1.155 | 1.109 | 1.143 | 0.959× | 0.977× | gcc |
| Etc::GameOfLife | 1.436 | 1.164 | 1.180 | 0.803× | 0.829× | clang |
| Etc::LogParser | 0.613 | 1.924 | 0.675 | 3.168× | 1.104× | — |
| Etc::NeuralNet | 1.288 | 1.102 | 1.079 | 0.837× | 0.834× | clang |
| Etc::Sieve | 1.361 | 1.804 | 1.363 | 1.325× | 0.999× | — |
| Etc::TextRaytracer | 1.431 | 1.022 | 1.341 | 0.715× | 0.935× | — |
| Etc::Words | 1.352 | 1.303 | 1.343 | 0.964× | 0.992× | — |
| Graph::AStar | 1.454 | 1.399 | 1.123 | 0.961× | 0.781× | gcc, clang |
| Graph::BFS | 1.664 | 1.313 | 1.189 | 0.812× | 0.761× | gcc, clang |
| Graph::DFS | 1.429 | 1.240 | 1.237 | 0.876× | 0.902× | gcc, clang |
| Hash::CRC32 | 0.293 | 0.806 | 0.916 | 2.751× | 3.126× | — |
| Hash::SHA256 | 1.652 | 1.472 | 1.380 | 0.891× | 0.837× | — |
| Json::Generate | 1.176 | 1.153 | 1.179 | 0.985× | 0.994× | — |
| Json::ParseDom | 1.324 | 1.224 | 1.234 | 0.927× | 0.932× | — |
| Json::ParseMapping | 1.260 | 1.160 | 1.178 | 0.925× | 0.933× | — |
| Matmul::Single | 1.388 | 1.389 | 1.383 | 1.001× | 0.996× | — |
| Matmul::T16 | 2.645 | 2.633 | 2.635 | 0.995× | 1.000× | — |
| Matmul::T4 | 1.357 | 1.354 | 1.354 | 0.999× | 0.997× | — |
| Matmul::T8 | 1.377 | 1.372 | 1.381 | 0.996× | 1.005× | — |
| Maze::AStar | 1.214 | 1.142 | 1.232 | 0.884× | 1.030× | gcc, clang |
| Maze::BFS | 1.204 | 1.256 | 1.241 | 1.015× | 1.032× | gcc, clang |
| Maze::Generator | 1.550 | 1.603 | 1.558 | 1.034× | 1.050× | gcc, clang |
| Sort::Merge | 1.196 | 1.147 | 1.038 | 0.959× | 0.868× | — |
| Sort::Quick | 1.355 | 1.254 | 1.272 | 0.926× | 0.939× | — |
| Sort::Self | 1.324 | 1.224 | 1.248 | 0.926× | 0.943× | — |
| Template::Parse | 1.141 | 1.151 | 1.167 | 0.978× | 1.034× | gcc, clang |
| Template::Regex | 1.049 | 1.258 | 1.432 | 1.205× | 1.356× | gcc, clang |

A/A warnings flag variation above 2%; no cases are dropped. Small timing differences need confirmation.
Compile gcc A/A CPU ratio: 1.003×.
Compile clang A/A CPU ratio: 0.985×.

Hardware counters: unavailable: perf is not installed on this host.

## Toolchains

- **cppgm**: cppgm++ (host configuration: g++)
- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and header hashes](metadata.json) · [Raw compile observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42974658011) · [Raw runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42974658011)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42974658011)
