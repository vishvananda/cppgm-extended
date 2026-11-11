# index-cpp benchmarks

Compiler checkout: `6a6a15f446cf4249b207924c70da749b98b10d1a` · CPU: AMD EPYC 7763 64-Core Processor

Unchanged upstream: `79e0b423094d420da37fb714f20c53fc35eeb2a3`. All 50 checksums pass with cppgm, GCC and Clang. Full runtime inputs, paired ABBA/BAAB blocks with A/A calibration. Speeds are reference time / cppgm time: higher is better. The aggregate gives all 50 workloads equal weight and is separate from the public suite.

| Measurement | vs GCC | vs Clang |
|---|---:|---:|
| Generated code | 0.999× | 0.981× |
| Compile CPU | 6.639× | 6.329× |

| Compiler | Executable bytes | .text bytes | Compile peak RSS (KiB) |
|---|---:|---:|---:|
| cppgm | 822,840 | 385,058 | 227,502 |
| gcc | 540,112 | 280,921 | 485,998 |
| clang | 513,720 | 264,561 | 319,222 |

| Workload | cppgm seconds | GCC seconds | Clang seconds | vs GCC | vs Clang | A/A warning |
|---|---:|---:|---:|---:|---:|---|
| Base64::Decode | 2.638 | 1.208 | 1.195 | 0.455× | 0.455× | — |
| Base64::Encode | 1.818 | 1.471 | 1.266 | 0.802× | 0.705× | — |
| Binarytrees::Arena | 2.375 | 2.468 | 2.508 | 1.033× | 1.063× | — |
| Binarytrees::Obj | 1.797 | 1.442 | 1.603 | 0.808× | 0.921× | — |
| Brainfuck::Array | 2.825 | 2.809 | 2.841 | 1.005× | 0.965× | gcc |
| Brainfuck::Recursion | 1.510 | 1.740 | 1.694 | 1.154× | 1.114× | clang |
| CLBG::Fannkuchredux | 1.073 | 1.152 | 0.981 | 1.072× | 0.913× | clang |
| CLBG::Mandelbrot | 1.381 | 1.352 | 1.337 | 0.978× | 0.968× | — |
| CLBG::Nbody | 1.024 | 1.057 | 1.056 | 1.031× | 1.031× | — |
| CLBG::Spectralnorm | 1.186 | 1.140 | 1.736 | 0.960× | 1.464× | — |
| CSV::Parse | 1.213 | 1.079 | 1.140 | 0.890× | 0.941× | — |
| Calculator::Ast | 1.210 | 0.969 | 0.980 | 0.801× | 0.810× | — |
| Calculator::Interpreter | 1.348 | 1.406 | 1.382 | 0.995× | 1.035× | clang |
| Compress::ArithDecode | 0.937 | 0.944 | 1.603 | 1.009× | 1.715× | — |
| Compress::ArithEncode | 1.049 | 0.981 | 1.110 | 0.936× | 1.058× | — |
| Compress::BWTDecode | 0.661 | 0.660 | 0.657 | 0.989× | 1.003× | — |
| Compress::BWTEncode | 1.211 | 1.098 | 1.348 | 0.909× | 1.118× | — |
| Compress::HuffDecode | 0.661 | 0.786 | 0.516 | 1.184× | 0.778× | — |
| Compress::HuffEncode | 0.854 | 1.224 | 1.032 | 1.432× | 1.206× | — |
| Compress::LZWDecode | 0.724 | 0.753 | 0.697 | 1.041× | 0.961× | — |
| Compress::LZWEncode | 1.347 | 1.296 | 1.313 | 0.973× | 0.971× | — |
| Distance::Jaro | 1.204 | 1.170 | 1.151 | 0.971× | 0.962× | — |
| Distance::NGram | 1.361 | 1.328 | 1.292 | 0.978× | 0.946× | — |
| Etc::CacheSimulation | 1.157 | 1.102 | 1.128 | 0.956× | 0.971× | — |
| Etc::GameOfLife | 1.508 | 1.278 | 1.196 | 0.824× | 0.817× | gcc |
| Etc::LogParser | 0.591 | 2.001 | 0.658 | 3.405× | 1.126× | gcc |
| Etc::NeuralNet | 1.266 | 1.212 | 1.151 | 0.923× | 0.900× | clang |
| Etc::Sieve | 1.400 | 1.839 | 1.375 | 1.249× | 0.988× | clang |
| Etc::TextRaytracer | 1.532 | 1.024 | 1.338 | 0.668× | 0.872× | — |
| Etc::Words | 1.333 | 1.295 | 1.337 | 0.966× | 1.000× | — |
| Graph::AStar | 1.309 | 1.325 | 1.050 | 0.991× | 0.807× | clang |
| Graph::BFS | 1.826 | 1.261 | 1.111 | 0.677× | 0.624× | clang |
| Graph::DFS | 1.346 | 1.199 | 1.220 | 0.879× | 0.915× | clang |
| Hash::CRC32 | 0.293 | 0.806 | 0.916 | 2.751× | 3.119× | — |
| Hash::SHA256 | 1.650 | 1.473 | 1.379 | 0.893× | 0.836× | — |
| Json::Generate | 1.106 | 1.123 | 1.119 | 1.014× | 1.009× | — |
| Json::ParseDom | 1.266 | 1.208 | 1.187 | 0.951× | 0.941× | clang |
| Json::ParseMapping | 1.204 | 1.151 | 1.131 | 0.949× | 0.926× | clang |
| Matmul::Single | 1.387 | 1.386 | 1.384 | 0.998× | 0.997× | — |
| Matmul::T16 | 2.639 | 2.626 | 2.627 | 0.999× | 1.000× | — |
| Matmul::T4 | 1.354 | 1.353 | 1.352 | 1.000× | 0.994× | — |
| Matmul::T8 | 1.369 | 1.369 | 1.369 | 1.016× | 0.999× | clang |
| Maze::AStar | 1.391 | 1.316 | 1.321 | 0.907× | 1.004× | gcc, clang |
| Maze::BFS | 1.435 | 1.599 | 1.457 | 1.075× | 0.994× | gcc, clang |
| Maze::Generator | 1.284 | 1.329 | 1.232 | 0.999× | 0.964× | clang |
| Sort::Merge | 1.125 | 1.129 | 1.025 | 1.002× | 0.913× | — |
| Sort::Quick | 1.351 | 1.235 | 1.250 | 0.913× | 0.926× | — |
| Sort::Self | 1.344 | 1.208 | 1.231 | 0.898× | 0.915× | — |
| Template::Parse | 1.537 | 1.687 | 1.365 | 1.012× | 0.948× | gcc, clang |
| Template::Regex | 1.212 | 1.569 | 1.524 | 1.194× | 1.286× | gcc, clang |

A/A warnings flag variation above 2%; no cases are dropped. Small timing differences need confirmation.
Compile gcc A/A CPU ratio: 1.008×.
Compile clang A/A CPU ratio: 1.005×.

Hardware counters: unavailable: perf is not installed on this host.

## Toolchains

- **cppgm**: cppgm++ (host configuration: g++)
- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and header hashes](metadata.json) · [Raw compile observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42517562075) · [Raw runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42517562075)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42517562075)
