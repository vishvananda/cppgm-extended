# index-cpp benchmarks

Compiler checkout: `9ca649bf59cb1e14979655d1669ade353a9ae881` · CPU: AMD EPYC 7763 64-Core Processor

Unchanged upstream: `79e0b423094d420da37fb714f20c53fc35eeb2a3`. All 50 checksums pass with cppgm, GCC and Clang. Full runtime inputs, paired ABBA/BAAB blocks with A/A calibration. Speeds are reference time / cppgm time: higher is better. The aggregate gives all 50 workloads equal weight and is separate from the public suite.

| Measurement | vs GCC | vs Clang |
|---|---:|---:|
| Generated code | 0.993× | 0.989× |
| Compile CPU | 6.513× | 6.053× |

| Compiler | Executable bytes | .text bytes | Compile peak RSS (KiB) |
|---|---:|---:|---:|
| cppgm | 798,176 | 391,330 | 229,112 |
| gcc | 540,112 | 280,921 | 486,000 |
| clang | 513,720 | 264,561 | 319,274 |

| Workload | cppgm seconds | GCC seconds | Clang seconds | vs GCC | vs Clang | A/A warning |
|---|---:|---:|---:|---:|---:|---|
| Base64::Decode | 2.958 | 1.184 | 1.196 | 0.400× | 0.404× | — |
| Base64::Encode | 1.742 | 1.425 | 1.276 | 0.819× | 0.730× | — |
| Binarytrees::Arena | 2.369 | 2.446 | 2.508 | 1.030× | 1.058× | — |
| Binarytrees::Obj | 1.694 | 1.280 | 1.494 | 0.761× | 0.882× | — |
| Brainfuck::Array | 3.343 | 2.808 | 2.796 | 0.845× | 0.822× | clang |
| Brainfuck::Recursion | 1.651 | 1.737 | 1.688 | 1.057× | 1.022× | — |
| CLBG::Fannkuchredux | 1.066 | 1.145 | 0.984 | 1.083× | 0.923× | — |
| CLBG::Mandelbrot | 1.381 | 1.350 | 1.335 | 0.978× | 0.967× | — |
| CLBG::Nbody | 1.025 | 1.055 | 1.056 | 1.028× | 1.031× | — |
| CLBG::Spectralnorm | 1.184 | 1.139 | 1.736 | 0.962× | 1.466× | — |
| CSV::Parse | 1.140 | 1.076 | 1.136 | 0.939× | 0.998× | — |
| Calculator::Ast | 1.208 | 0.948 | 0.980 | 0.788× | 0.806× | — |
| Calculator::Interpreter | 1.338 | 1.305 | 1.373 | 0.972× | 1.020× | — |
| Compress::ArithDecode | 0.905 | 0.945 | 1.601 | 1.043× | 1.769× | — |
| Compress::ArithEncode | 1.046 | 0.980 | 1.110 | 0.937× | 1.062× | — |
| Compress::BWTDecode | 0.650 | 0.647 | 0.655 | 0.998× | 1.008× | — |
| Compress::BWTEncode | 1.204 | 1.112 | 1.344 | 0.936× | 1.126× | — |
| Compress::HuffDecode | 0.642 | 0.768 | 0.514 | 1.198× | 0.801× | — |
| Compress::HuffEncode | 0.855 | 1.229 | 1.031 | 1.439× | 1.205× | — |
| Compress::LZWDecode | 0.720 | 0.730 | 0.694 | 1.012× | 0.968× | — |
| Compress::LZWEncode | 1.339 | 1.292 | 1.300 | 0.965× | 0.968× | — |
| Distance::Jaro | 1.206 | 1.170 | 1.147 | 0.970× | 0.953× | — |
| Distance::NGram | 1.357 | 1.321 | 1.290 | 0.975× | 0.948× | — |
| Etc::CacheSimulation | 1.118 | 1.103 | 1.113 | 0.992× | 0.997× | — |
| Etc::GameOfLife | 1.439 | 1.120 | 1.187 | 0.775× | 0.823× | — |
| Etc::LogParser | 0.584 | 2.017 | 0.658 | 3.381× | 1.128× | gcc, clang |
| Etc::NeuralNet | 1.285 | 1.098 | 1.079 | 0.853× | 0.840× | gcc, clang |
| Etc::Sieve | 1.456 | 1.819 | 1.372 | 1.249× | 0.998× | gcc, clang |
| Etc::TextRaytracer | 1.153 | 1.015 | 1.337 | 0.882× | 1.159× | — |
| Etc::Words | 1.340 | 1.295 | 1.322 | 0.971× | 0.983× | — |
| Graph::AStar | 1.304 | 1.300 | 1.059 | 0.996× | 0.812× | — |
| Graph::BFS | 1.719 | 1.174 | 1.103 | 0.691× | 0.644× | gcc |
| Graph::DFS | 1.340 | 1.183 | 1.230 | 0.882× | 0.927× | — |
| Hash::CRC32 | 0.293 | 0.806 | 0.915 | 2.747× | 3.125× | — |
| Hash::SHA256 | 1.646 | 1.472 | 1.377 | 0.892× | 0.837× | — |
| Json::Generate | 1.095 | 1.119 | 1.121 | 1.020× | 1.024× | — |
| Json::ParseDom | 1.297 | 1.183 | 1.185 | 0.914× | 0.908× | — |
| Json::ParseMapping | 1.235 | 1.127 | 1.131 | 0.913× | 0.914× | — |
| Matmul::Single | 1.384 | 1.383 | 1.381 | 0.999× | 0.998× | — |
| Matmul::T16 | 2.620 | 2.635 | 2.623 | 0.998× | 1.003× | — |
| Matmul::T4 | 1.353 | 1.352 | 1.352 | 0.999× | 1.000× | — |
| Matmul::T8 | 1.368 | 1.365 | 1.369 | 0.994× | 1.002× | gcc |
| Maze::AStar | 1.192 | 1.079 | 1.241 | 0.901× | 1.061× | clang |
| Maze::BFS | 1.188 | 1.309 | 1.419 | 1.108× | 1.202× | clang |
| Maze::Generator | 1.319 | 1.264 | 1.242 | 0.962× | 0.960× | — |
| Sort::Merge | 1.208 | 1.126 | 1.024 | 0.930× | 0.848× | — |
| Sort::Quick | 1.329 | 1.232 | 1.250 | 0.926× | 0.941× | — |
| Sort::Self | 1.312 | 1.206 | 1.228 | 0.919× | 0.937× | — |
| Template::Parse | 1.202 | 1.205 | 1.212 | 1.007× | 1.015× | — |
| Template::Regex | 1.047 | 1.296 | 1.504 | 1.228× | 1.458× | — |

A/A warnings flag variation above 2%; no cases are dropped. Small timing differences need confirmation.
Compile gcc A/A CPU ratio: 1.000×.
Compile clang A/A CPU ratio: 0.997×.

Hardware counters: unavailable: perf is not installed on this host.

## Toolchains

- **cppgm**: cppgm++ (host configuration: g++)
- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and header hashes](metadata.json) · [Raw compile observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42709012196) · [Raw runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42709012196)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42709012196)
