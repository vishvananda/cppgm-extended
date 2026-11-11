# index-cpp benchmarks

Compiler checkout: `043e34737771945b5b28325856dadffa564bb383` · CPU: AMD EPYC 7763 64-Core Processor

Unchanged upstream: `79e0b423094d420da37fb714f20c53fc35eeb2a3`. All 50 checksums pass with cppgm, GCC and Clang. Full runtime inputs, paired ABBA/BAAB blocks with A/A calibration. Speeds are reference time / cppgm time: higher is better. The aggregate gives all 50 workloads equal weight and is separate from the public suite.

| Measurement | vs GCC | vs Clang |
|---|---:|---:|
| Generated code | 1.021× | 0.982× |
| Compile CPU | 6.475× | 6.044× |

| Compiler | Executable bytes | .text bytes | Compile peak RSS (KiB) |
|---|---:|---:|---:|
| cppgm | 798,176 | 388,834 | 229,052 |
| gcc | 540,112 | 280,921 | 485,968 |
| clang | 513,720 | 264,561 | 318,996 |

| Workload | cppgm seconds | GCC seconds | Clang seconds | vs GCC | vs Clang | A/A warning |
|---|---:|---:|---:|---:|---:|---|
| Base64::Decode | 2.965 | 1.215 | 1.192 | 0.407× | 0.404× | — |
| Base64::Encode | 1.776 | 1.480 | 1.268 | 0.829× | 0.728× | gcc |
| Binarytrees::Arena | 2.407 | 2.507 | 2.523 | 1.034× | 1.069× | gcc |
| Binarytrees::Obj | 1.784 | 1.397 | 1.503 | 0.759× | 0.905× | clang |
| Brainfuck::Array | 3.346 | 2.811 | 2.859 | 0.846× | 0.861× | clang |
| Brainfuck::Recursion | 1.653 | 1.738 | 1.703 | 1.059× | 1.032× | — |
| CLBG::Fannkuchredux | 1.066 | 1.152 | 0.980 | 1.085× | 0.923× | — |
| CLBG::Mandelbrot | 1.381 | 1.351 | 1.336 | 0.978× | 0.967× | — |
| CLBG::Nbody | 1.026 | 1.056 | 1.056 | 1.029× | 1.030× | — |
| CLBG::Spectralnorm | 1.185 | 1.149 | 1.737 | 0.975× | 1.466× | — |
| CSV::Parse | 1.140 | 1.087 | 1.140 | 0.958× | 1.001× | clang |
| Calculator::Ast | 1.207 | 0.962 | 0.982 | 0.798× | 0.817× | — |
| Calculator::Interpreter | 1.399 | 1.401 | 1.381 | 0.970× | 1.033× | gcc |
| Compress::ArithDecode | 0.905 | 0.948 | 1.603 | 1.047× | 1.765× | — |
| Compress::ArithEncode | 1.047 | 0.980 | 1.110 | 0.936× | 1.059× | — |
| Compress::BWTDecode | 0.655 | 1.370 | 0.655 | 2.136× | 1.009× | gcc |
| Compress::BWTEncode | 1.194 | 1.103 | 1.348 | 0.931× | 1.134× | — |
| Compress::HuffDecode | 0.645 | 0.782 | 0.513 | 1.200× | 0.796× | — |
| Compress::HuffEncode | 0.855 | 1.232 | 1.032 | 1.441× | 1.209× | — |
| Compress::LZWDecode | 0.722 | 0.766 | 0.696 | 1.062× | 0.969× | — |
| Compress::LZWEncode | 1.340 | 1.298 | 1.309 | 0.963× | 0.977× | — |
| Distance::Jaro | 1.205 | 1.172 | 1.149 | 0.973× | 0.950× | — |
| Distance::NGram | 1.363 | 1.328 | 1.292 | 0.976× | 0.946× | — |
| Etc::CacheSimulation | 1.134 | 1.108 | 1.110 | 0.970× | 0.977× | — |
| Etc::GameOfLife | 1.495 | 1.357 | 1.216 | 0.924× | 0.847× | gcc, clang |
| Etc::LogParser | 0.581 | 1.994 | 0.657 | 3.438× | 1.129× | gcc |
| Etc::NeuralNet | 1.283 | 1.214 | 1.079 | 0.945× | 0.839× | clang |
| Etc::Sieve | 1.383 | 1.883 | 1.373 | 1.369× | 0.941× | — |
| Etc::TextRaytracer | 1.153 | 1.020 | 1.338 | 0.886× | 1.159× | — |
| Etc::Words | 1.337 | 1.305 | 1.332 | 0.975× | 1.001× | — |
| Graph::AStar | 1.319 | 1.337 | 1.030 | 1.006× | 0.807× | — |
| Graph::BFS | 1.774 | 1.300 | 1.060 | 0.718× | 0.651× | gcc |
| Graph::DFS | 1.377 | 1.215 | 1.188 | 0.879× | 0.893× | clang |
| Hash::CRC32 | 0.293 | 0.806 | 0.915 | 2.746× | 3.125× | — |
| Hash::SHA256 | 1.648 | 1.473 | 1.382 | 0.894× | 0.839× | — |
| Json::Generate | 1.107 | 1.131 | 1.137 | 1.021× | 1.028× | — |
| Json::ParseDom | 1.297 | 1.223 | 1.199 | 0.931× | 0.933× | gcc |
| Json::ParseMapping | 1.235 | 1.175 | 1.144 | 0.940× | 0.932× | gcc |
| Matmul::Single | 1.387 | 1.386 | 1.383 | 0.999× | 0.997× | — |
| Matmul::T16 | 2.633 | 2.644 | 2.621 | 1.004× | 0.995× | — |
| Matmul::T4 | 1.356 | 1.356 | 1.355 | 1.000× | 0.999× | — |
| Matmul::T8 | 1.371 | 1.368 | 1.373 | 0.995× | 1.004× | — |
| Maze::AStar | 1.344 | 1.333 | 1.067 | 0.935× | 0.929× | gcc, clang |
| Maze::BFS | 1.385 | 1.695 | 1.167 | 1.053× | 0.982× | clang |
| Maze::Generator | 1.361 | 1.373 | 1.248 | 0.974× | 0.956× | clang |
| Sort::Merge | 1.210 | 1.133 | 1.024 | 0.933× | 0.848× | — |
| Sort::Quick | 1.339 | 1.235 | 1.250 | 0.925× | 0.928× | — |
| Sort::Self | 1.310 | 1.209 | 1.229 | 0.923× | 0.939× | — |
| Template::Parse | 1.663 | 1.831 | 1.151 | 1.000× | 0.997× | gcc |
| Template::Regex | 1.216 | 1.659 | 1.412 | 1.281× | 1.340× | gcc |

A/A warnings flag variation above 2%; no cases are dropped. Small timing differences need confirmation.
Compile gcc A/A CPU ratio: 1.010×.
Compile clang A/A CPU ratio: 0.983×.

Hardware counters: unavailable: perf is not installed on this host.

## Toolchains

- **cppgm**: cppgm++ (host configuration: g++)
- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and header hashes](metadata.json) · [Raw compile observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42700385877) · [Raw runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42700385877)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42700385877)
