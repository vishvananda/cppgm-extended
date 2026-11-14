# index-cpp benchmarks

Compiler checkout: `a649ddb21dd810338ac87d93e7d2e713d0f98ab1` · CPU: AMD EPYC 7763 64-Core Processor

Unchanged upstream: `79e0b423094d420da37fb714f20c53fc35eeb2a3`. All 50 checksums pass with cppgm, GCC and Clang. Full runtime inputs, paired ABBA/BAAB blocks with A/A calibration. Speeds are reference time / cppgm time: higher is better. The aggregate gives all 50 workloads equal weight and is separate from the public suite.

| Measurement | vs GCC | vs Clang |
|---|---:|---:|
| Generated code | 1.046× | 1.036× |
| Compile CPU | 6.271× | 5.769× |

| Compiler | Executable bytes | .text bytes | Compile peak RSS (KiB) |
|---|---:|---:|---:|
| cppgm | 714,480 | 339,490 | 228,586 |
| gcc | 540,112 | 280,921 | 486,046 |
| clang | 513,720 | 264,561 | 319,224 |

| Workload | cppgm seconds | GCC seconds | Clang seconds | vs GCC | vs Clang | A/A warning |
|---|---:|---:|---:|---:|---:|---|
| Base64::Decode | 1.422 | 1.184 | 1.220 | 0.834× | 0.842× | — |
| Base64::Encode | 1.726 | 1.419 | 1.301 | 0.825× | 0.731× | — |
| Binarytrees::Arena | 2.437 | 2.459 | 2.558 | 1.013× | 1.050× | — |
| Binarytrees::Obj | 1.356 | 1.278 | 1.567 | 0.963× | 1.147× | — |
| Brainfuck::Array | 2.815 | 2.814 | 2.800 | 0.999× | 0.964× | clang |
| Brainfuck::Recursion | 1.781 | 1.741 | 1.704 | 0.980× | 0.974× | clang |
| CLBG::Fannkuchredux | 0.999 | 1.153 | 0.975 | 1.146× | 0.980× | — |
| CLBG::Mandelbrot | 1.379 | 1.351 | 1.336 | 0.979× | 0.968× | — |
| CLBG::Nbody | 1.036 | 1.058 | 1.057 | 1.020× | 1.020× | — |
| CLBG::Spectralnorm | 1.185 | 1.139 | 1.739 | 0.960× | 1.467× | — |
| CSV::Parse | 1.195 | 1.082 | 1.155 | 0.903× | 0.970× | — |
| Calculator::Ast | 1.034 | 0.961 | 0.984 | 0.926× | 0.953× | — |
| Calculator::Interpreter | 1.360 | 1.269 | 1.417 | 0.946× | 1.027× | — |
| Compress::ArithDecode | 0.942 | 0.946 | 1.603 | 1.004× | 1.701× | — |
| Compress::ArithEncode | 1.087 | 0.981 | 1.111 | 0.903× | 1.023× | — |
| Compress::BWTDecode | 0.640 | 0.647 | 0.663 | 1.013× | 1.022× | — |
| Compress::BWTEncode | 1.118 | 1.098 | 1.353 | 0.993× | 1.184× | — |
| Compress::HuffDecode | 0.698 | 0.774 | 0.514 | 1.113× | 0.736× | — |
| Compress::HuffEncode | 0.976 | 1.227 | 1.033 | 1.259× | 1.059× | — |
| Compress::LZWDecode | 0.739 | 0.734 | 0.696 | 0.990× | 0.940× | — |
| Compress::LZWEncode | 1.326 | 1.296 | 1.307 | 0.975× | 0.989× | — |
| Distance::Jaro | 1.202 | 1.169 | 1.148 | 0.972× | 0.957× | — |
| Distance::NGram | 1.364 | 1.325 | 1.294 | 0.973× | 0.953× | — |
| Etc::CacheSimulation | 1.134 | 1.107 | 1.122 | 0.971× | 0.989× | — |
| Etc::GameOfLife | 1.171 | 1.110 | 1.255 | 0.946× | 1.005× | — |
| Etc::LogParser | 0.547 | 2.028 | 0.667 | 3.691× | 1.222× | — |
| Etc::NeuralNet | 1.254 | 1.101 | 1.089 | 0.877× | 0.897× | — |
| Etc::Sieve | 1.421 | 1.819 | 1.381 | 1.319× | 0.936× | gcc, clang |
| Etc::TextRaytracer | 1.050 | 1.025 | 1.339 | 0.972× | 1.276× | — |
| Etc::Words | 1.320 | 1.283 | 1.333 | 0.970× | 1.011× | — |
| Graph::AStar | 1.354 | 1.300 | 1.071 | 0.964× | 0.786× | — |
| Graph::BFS | 1.661 | 1.144 | 1.181 | 0.690× | 0.676× | clang |
| Graph::DFS | 1.334 | 1.159 | 1.243 | 0.874× | 0.901× | clang |
| Hash::CRC32 | 0.293 | 0.805 | 0.915 | 2.747× | 3.123× | — |
| Hash::SHA256 | 0.452 | 1.473 | 1.379 | 3.254× | 3.053× | — |
| Json::Generate | 1.147 | 1.131 | 1.143 | 0.989× | 0.997× | — |
| Json::ParseDom | 1.249 | 1.195 | 1.196 | 0.960× | 0.945× | — |
| Json::ParseMapping | 1.191 | 1.131 | 1.141 | 0.952× | 0.951× | — |
| Matmul::Single | 1.389 | 1.388 | 1.386 | 0.999× | 0.998× | — |
| Matmul::T16 | 2.642 | 2.630 | 2.638 | 1.001× | 0.999× | — |
| Matmul::T4 | 1.356 | 1.356 | 1.356 | 1.000× | 1.000× | — |
| Matmul::T8 | 1.379 | 1.375 | 1.379 | 0.999× | 1.001× | — |
| Maze::AStar | 1.325 | 1.030 | 1.410 | 0.903× | 0.934× | clang |
| Maze::BFS | 1.339 | 1.228 | 1.551 | 1.027× | 0.964× | gcc, clang |
| Maze::Generator | 1.256 | 1.260 | 1.310 | 1.024× | 1.009× | — |
| Sort::Merge | 1.103 | 1.128 | 1.028 | 1.019× | 0.929× | — |
| Sort::Quick | 1.347 | 1.232 | 1.252 | 0.915× | 0.929× | — |
| Sort::Self | 1.273 | 1.207 | 1.230 | 0.948× | 0.965× | — |
| Template::Parse | 1.208 | 1.178 | 1.455 | 1.000× | 1.000× | clang |
| Template::Regex | 1.298 | 1.269 | 1.609 | 0.989× | 1.131× | clang |

A/A warnings flag variation above 2%; no cases are dropped. Small timing differences need confirmation.
Compile gcc A/A CPU ratio: 1.000×.
Compile clang A/A CPU ratio: 1.005×.

Hardware counters: unavailable: perf is not installed on this host.

## Toolchains

- **cppgm**: cppgm++ (host configuration: g++)
- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and header hashes](metadata.json) · [Raw compile observations](compile-observations.jsonl) · [Raw runtime observations](runtime-observations.jsonl)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43609274587)
