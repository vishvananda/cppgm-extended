# index-cpp benchmarks

Compiler checkout: `30adb3542505ff7adba0599734ecfb804f15734a` · CPU: AMD EPYC 7763 64-Core Processor

Unchanged upstream: `79e0b423094d420da37fb714f20c53fc35eeb2a3`. All 50 checksums pass with cppgm, GCC and Clang. Full runtime inputs, paired ABBA/BAAB blocks with A/A calibration. Speeds are reference time / cppgm time: higher is better. The aggregate gives all 50 workloads equal weight and is separate from the public suite.

| Measurement | vs GCC | vs Clang |
|---|---:|---:|
| Generated code | 1.012× | 0.995× |
| Compile CPU | 6.844× | 6.156× |

| Compiler | Executable bytes | .text bytes | Compile peak RSS (KiB) |
|---|---:|---:|---:|
| cppgm | 823,072 | 385,154 | 228,978 |
| gcc | 540,112 | 280,921 | 485,960 |
| clang | 513,720 | 264,561 | 319,000 |

| Workload | cppgm seconds | GCC seconds | Clang seconds | vs GCC | vs Clang | A/A warning |
|---|---:|---:|---:|---:|---:|---|
| Base64::Decode | 2.646 | 1.192 | 1.212 | 0.455× | 0.459× | — |
| Base64::Encode | 1.792 | 1.432 | 1.292 | 0.798× | 0.724× | — |
| Binarytrees::Arena | 2.377 | 2.437 | 2.527 | 1.014× | 1.062× | — |
| Binarytrees::Obj | 1.793 | 1.428 | 1.707 | 0.790× | 0.957× | clang |
| Brainfuck::Array | 3.026 | 2.808 | 2.860 | 0.907× | 0.923× | gcc, clang |
| Brainfuck::Recursion | 1.677 | 1.738 | 1.712 | 1.037× | 1.037× | clang |
| CLBG::Fannkuchredux | 1.079 | 1.147 | 0.978 | 1.066× | 0.909× | — |
| CLBG::Mandelbrot | 1.381 | 1.350 | 1.337 | 0.978× | 0.967× | — |
| CLBG::Nbody | 1.025 | 1.056 | 1.056 | 1.030× | 1.032× | — |
| CLBG::Spectralnorm | 1.187 | 1.139 | 1.739 | 0.960× | 1.465× | — |
| CSV::Parse | 1.219 | 1.078 | 1.139 | 0.889× | 0.936× | — |
| Calculator::Ast | 1.193 | 0.956 | 0.984 | 0.799× | 0.824× | — |
| Calculator::Interpreter | 1.320 | 1.292 | 1.439 | 0.932× | 1.132× | gcc |
| Compress::ArithDecode | 0.927 | 0.946 | 1.603 | 1.020× | 1.731× | — |
| Compress::ArithEncode | 1.049 | 0.981 | 1.109 | 0.936× | 1.058× | — |
| Compress::BWTDecode | 0.656 | 1.437 | 0.657 | 2.185× | 1.002× | gcc |
| Compress::BWTEncode | 1.180 | 1.126 | 1.352 | 0.949× | 1.148× | — |
| Compress::HuffDecode | 0.659 | 0.778 | 0.520 | 1.178× | 0.792× | — |
| Compress::HuffEncode | 0.857 | 1.230 | 1.032 | 1.434× | 1.206× | clang |
| Compress::LZWDecode | 0.724 | 0.726 | 0.696 | 1.002× | 0.958× | — |
| Compress::LZWEncode | 1.336 | 1.288 | 1.306 | 0.958× | 0.983× | — |
| Distance::Jaro | 1.205 | 1.169 | 1.148 | 0.971× | 0.953× | — |
| Distance::NGram | 1.378 | 1.325 | 1.293 | 0.960× | 0.944× | — |
| Etc::CacheSimulation | 1.123 | 1.111 | 1.120 | 0.988× | 1.007× | — |
| Etc::GameOfLife | 1.470 | 1.202 | 1.219 | 0.818× | 0.839× | gcc, clang |
| Etc::LogParser | 0.591 | 1.981 | 0.658 | 3.318× | 1.098× | gcc |
| Etc::NeuralNet | 1.267 | 1.099 | 1.090 | 0.868× | 0.859× | gcc, clang |
| Etc::Sieve | 1.382 | 1.815 | 1.374 | 1.307× | 1.000× | gcc, clang |
| Etc::TextRaytracer | 1.152 | 1.019 | 1.338 | 0.885× | 1.162× | — |
| Etc::Words | 1.325 | 1.300 | 1.329 | 0.979× | 1.007× | — |
| Graph::AStar | 1.308 | 1.296 | 1.058 | 0.991× | 0.801× | — |
| Graph::BFS | 1.817 | 1.192 | 1.156 | 0.648× | 0.639× | clang |
| Graph::DFS | 1.336 | 1.163 | 1.229 | 0.865× | 0.930× | — |
| Hash::CRC32 | 0.293 | 0.806 | 0.916 | 2.744× | 3.124× | — |
| Hash::SHA256 | 1.653 | 1.473 | 1.379 | 0.891× | 0.834× | — |
| Json::Generate | 1.103 | 1.123 | 1.117 | 1.022× | 1.016× | — |
| Json::ParseDom | 1.254 | 1.188 | 1.188 | 0.939× | 0.947× | — |
| Json::ParseMapping | 1.193 | 1.128 | 1.134 | 0.936× | 0.951× | gcc |
| Matmul::Single | 1.387 | 1.384 | 1.382 | 0.998× | 0.995× | — |
| Matmul::T16 | 2.631 | 2.633 | 2.637 | 1.006× | 1.004× | — |
| Matmul::T4 | 1.355 | 1.355 | 1.358 | 1.002× | 1.001× | — |
| Matmul::T8 | 1.375 | 1.369 | 1.373 | 0.993× | 0.999× | — |
| Maze::AStar | 1.358 | 1.179 | 1.355 | 0.912× | 0.908× | — |
| Maze::BFS | 1.368 | 1.455 | 1.459 | 1.030× | 1.046× | gcc, clang |
| Maze::Generator | 1.271 | 1.268 | 1.243 | 1.000× | 0.975× | — |
| Sort::Merge | 1.163 | 1.126 | 1.026 | 0.968× | 0.883× | — |
| Sort::Quick | 1.364 | 1.232 | 1.251 | 0.905× | 0.919× | — |
| Sort::Self | 1.319 | 1.207 | 1.229 | 0.916× | 0.932× | — |
| Template::Parse | 1.323 | 1.361 | 1.470 | 1.013× | 1.151× | gcc, clang |
| Template::Regex | 1.088 | 1.377 | 1.527 | 1.320× | 1.426× | gcc, clang |

A/A warnings flag variation above 2%; no cases are dropped. Small timing differences need confirmation.
Compile gcc A/A CPU ratio: 0.985×.
Compile clang A/A CPU ratio: 1.009×.

Hardware counters: unavailable: perf is not installed on this host.

## Toolchains

- **cppgm**: cppgm++ (host configuration: g++)
- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and header hashes](metadata.json) · [Raw compile observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42573958661) · [Raw runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42573958661)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42573958661)
