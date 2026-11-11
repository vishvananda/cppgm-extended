# index-cpp benchmarks

Compiler checkout: `783dbfcf01360d923ff590d2048a0c1764761cc3` · CPU: AMD EPYC 7763 64-Core Processor

Unchanged upstream: `79e0b423094d420da37fb714f20c53fc35eeb2a3`. All 50 checksums pass with cppgm, GCC and Clang. Full runtime inputs, paired ABBA/BAAB blocks with A/A calibration. Speeds are reference time / cppgm time: higher is better. The aggregate gives all 50 workloads equal weight and is separate from the public suite.

| Measurement | vs GCC | vs Clang |
|---|---:|---:|
| Generated code | 0.993× | 0.981× |
| Compile CPU | 6.706× | 6.153× |

| Compiler | Executable bytes | .text bytes | Compile peak RSS (KiB) |
|---|---:|---:|---:|
| cppgm | 822,840 | 385,058 | 226,414 |
| gcc | 540,112 | 280,921 | 486,014 |
| clang | 513,720 | 264,561 | 319,026 |

| Workload | cppgm seconds | GCC seconds | Clang seconds | vs GCC | vs Clang | A/A warning |
|---|---:|---:|---:|---:|---:|---|
| Base64::Decode | 2.609 | 1.184 | 1.199 | 0.454× | 0.456× | — |
| Base64::Encode | 1.771 | 1.422 | 1.276 | 0.802× | 0.720× | — |
| Binarytrees::Arena | 2.352 | 2.454 | 2.501 | 1.055× | 1.061× | — |
| Binarytrees::Obj | 1.644 | 1.290 | 1.524 | 0.788× | 0.910× | — |
| Brainfuck::Array | 2.865 | 2.812 | 2.841 | 1.061× | 0.950× | clang |
| Brainfuck::Recursion | 1.514 | 1.738 | 1.706 | 1.149× | 1.125× | — |
| CLBG::Fannkuchredux | 1.077 | 1.155 | 0.978 | 1.068× | 0.909× | — |
| CLBG::Mandelbrot | 1.381 | 1.350 | 1.335 | 0.978× | 0.967× | — |
| CLBG::Nbody | 1.024 | 1.057 | 1.056 | 1.030× | 1.032× | — |
| CLBG::Spectralnorm | 1.186 | 1.139 | 1.736 | 0.961× | 1.464× | — |
| CSV::Parse | 1.212 | 1.085 | 1.142 | 0.897× | 0.937× | — |
| Calculator::Ast | 1.203 | 0.965 | 0.981 | 0.803× | 0.811× | — |
| Calculator::Interpreter | 1.292 | 1.247 | 1.347 | 0.961× | 1.046× | — |
| Compress::ArithDecode | 0.925 | 0.946 | 1.602 | 1.020× | 1.731× | — |
| Compress::ArithEncode | 1.049 | 0.981 | 1.110 | 0.935× | 1.061× | — |
| Compress::BWTDecode | 0.655 | 0.646 | 0.659 | 0.985× | 1.004× | — |
| Compress::BWTEncode | 1.202 | 1.098 | 1.349 | 0.915× | 1.124× | — |
| Compress::HuffDecode | 0.657 | 0.773 | 0.513 | 1.175× | 0.781× | — |
| Compress::HuffEncode | 0.855 | 1.226 | 1.032 | 1.437× | 1.208× | — |
| Compress::LZWDecode | 0.723 | 0.730 | 0.697 | 1.014× | 0.969× | — |
| Compress::LZWEncode | 1.331 | 1.289 | 1.306 | 0.961× | 0.984× | — |
| Distance::Jaro | 1.207 | 1.171 | 1.148 | 0.974× | 0.952× | — |
| Distance::NGram | 1.358 | 1.321 | 1.290 | 0.970× | 0.949× | — |
| Etc::CacheSimulation | 1.147 | 1.105 | 1.124 | 0.959× | 0.979× | — |
| Etc::GameOfLife | 1.423 | 1.107 | 1.167 | 0.771× | 0.814× | gcc |
| Etc::LogParser | 0.592 | 2.028 | 0.662 | 3.434× | 1.118× | — |
| Etc::NeuralNet | 1.268 | 1.156 | 1.081 | 0.897× | 0.814× | clang |
| Etc::Sieve | 1.383 | 1.813 | 1.377 | 1.311× | 0.994× | — |
| Etc::TextRaytracer | 1.532 | 1.023 | 1.338 | 0.668× | 0.874× | — |
| Etc::Words | 1.329 | 1.284 | 1.334 | 0.964× | 1.008× | — |
| Graph::AStar | 1.301 | 1.286 | 1.046 | 0.992× | 0.799× | — |
| Graph::BFS | 1.760 | 1.127 | 1.063 | 0.642× | 0.612× | gcc |
| Graph::DFS | 1.320 | 1.146 | 1.210 | 0.873× | 0.915× | gcc |
| Hash::CRC32 | 0.293 | 0.806 | 0.915 | 2.749× | 3.125× | — |
| Hash::SHA256 | 1.651 | 1.472 | 1.378 | 0.893× | 0.835× | — |
| Json::Generate | 1.103 | 1.118 | 1.119 | 1.015× | 1.010× | — |
| Json::ParseDom | 1.254 | 1.185 | 1.184 | 0.946× | 0.941× | — |
| Json::ParseMapping | 1.194 | 1.127 | 1.129 | 0.944× | 0.944× | — |
| Matmul::Single | 1.391 | 1.388 | 1.386 | 0.999× | 0.996× | — |
| Matmul::T16 | 2.638 | 2.630 | 2.639 | 0.994× | 1.004× | — |
| Matmul::T4 | 1.357 | 1.355 | 1.357 | 1.000× | 1.000× | — |
| Matmul::T8 | 1.375 | 1.367 | 1.373 | 0.996× | 0.983× | — |
| Maze::AStar | 1.212 | 1.085 | 1.140 | 0.873× | 0.951× | gcc, clang |
| Maze::BFS | 1.211 | 1.271 | 1.207 | 1.002× | 1.004× | gcc, clang |
| Maze::Generator | 1.234 | 1.204 | 1.192 | 0.984× | 0.963× | — |
| Sort::Merge | 1.131 | 1.127 | 1.024 | 0.991× | 0.903× | — |
| Sort::Quick | 1.349 | 1.231 | 1.251 | 0.912× | 0.927× | — |
| Sort::Self | 1.341 | 1.206 | 1.229 | 0.899× | 0.915× | — |
| Template::Parse | 1.197 | 1.171 | 1.177 | 0.991× | 0.986× | gcc, clang |
| Template::Regex | 1.016 | 1.296 | 1.442 | 1.255× | 1.427× | — |

A/A warnings flag variation above 2%; no cases are dropped. Small timing differences need confirmation.
Compile gcc A/A CPU ratio: 0.986×.
Compile clang A/A CPU ratio: 1.007×.

Hardware counters: unavailable: perf is not installed on this host.

## Toolchains

- **cppgm**: cppgm++ (host configuration: g++)
- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and header hashes](metadata.json) · [Raw compile observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42493504572) · [Raw runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42493504572)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42493504572)
