# index-cpp benchmarks

Compiler checkout: `6c589867bdd5c99bc35b90267a46778f59e1127e` · CPU: AMD EPYC 7763 64-Core Processor

Unchanged upstream: `79e0b423094d420da37fb714f20c53fc35eeb2a3`. All 50 checksums pass with cppgm, GCC and Clang. Full runtime inputs, paired ABBA/BAAB blocks with A/A calibration. Speeds are reference time / cppgm time: higher is better. The aggregate gives all 50 workloads equal weight and is separate from the public suite.

| Measurement | vs GCC | vs Clang |
|---|---:|---:|
| Generated code | 0.998× | 0.992× |
| Compile CPU | 6.510× | 6.009× |

| Compiler | Executable bytes | .text bytes | Compile peak RSS (KiB) |
|---|---:|---:|---:|
| cppgm | 798,176 | 388,834 | 229,142 |
| gcc | 540,112 | 280,921 | 486,032 |
| clang | 513,720 | 264,561 | 319,298 |

| Workload | cppgm seconds | GCC seconds | Clang seconds | vs GCC | vs Clang | A/A warning |
|---|---:|---:|---:|---:|---:|---|
| Base64::Decode | 2.976 | 1.192 | 1.204 | 0.402× | 0.406× | — |
| Base64::Encode | 1.751 | 1.437 | 1.292 | 0.821× | 0.734× | — |
| Binarytrees::Arena | 2.391 | 2.486 | 2.567 | 1.037× | 1.073× | — |
| Binarytrees::Obj | 1.780 | 1.399 | 1.650 | 0.791× | 0.936× | — |
| Brainfuck::Array | 3.345 | 2.809 | 2.851 | 0.840× | 0.837× | gcc, clang |
| Brainfuck::Recursion | 1.652 | 1.739 | 1.704 | 1.053× | 1.030× | clang |
| CLBG::Fannkuchredux | 1.067 | 1.137 | 0.981 | 1.071× | 0.921× | — |
| CLBG::Mandelbrot | 1.380 | 1.350 | 1.335 | 0.978× | 0.967× | — |
| CLBG::Nbody | 1.025 | 1.054 | 1.056 | 1.030× | 1.030× | — |
| CLBG::Spectralnorm | 1.184 | 1.139 | 1.736 | 0.961× | 1.465× | — |
| CSV::Parse | 1.129 | 1.079 | 1.137 | 0.950× | 1.009× | — |
| Calculator::Ast | 1.206 | 0.954 | 0.982 | 0.790× | 0.813× | — |
| Calculator::Interpreter | 1.356 | 1.264 | 1.393 | 0.934× | 1.033× | — |
| Compress::ArithDecode | 0.905 | 0.945 | 1.603 | 1.044× | 1.774× | — |
| Compress::ArithEncode | 1.046 | 0.984 | 1.110 | 0.941× | 1.060× | — |
| Compress::BWTDecode | 0.653 | 0.648 | 0.658 | 0.993× | 1.005× | — |
| Compress::BWTEncode | 1.181 | 1.098 | 1.346 | 0.938× | 1.138× | — |
| Compress::HuffDecode | 0.643 | 0.773 | 0.514 | 1.199× | 0.793× | — |
| Compress::HuffEncode | 0.856 | 1.226 | 1.031 | 1.431× | 1.206× | — |
| Compress::LZWDecode | 0.720 | 0.764 | 0.693 | 1.067× | 0.962× | — |
| Compress::LZWEncode | 1.335 | 1.292 | 1.295 | 0.966× | 0.967× | — |
| Distance::Jaro | 1.206 | 1.171 | 1.148 | 0.970× | 0.953× | — |
| Distance::NGram | 1.357 | 1.323 | 1.291 | 0.974× | 0.951× | — |
| Etc::CacheSimulation | 1.131 | 1.106 | 1.118 | 0.981× | 0.988× | — |
| Etc::GameOfLife | 1.433 | 1.185 | 1.242 | 0.822× | 0.858× | clang |
| Etc::LogParser | 0.581 | 1.954 | 0.672 | 3.325× | 1.153× | gcc |
| Etc::NeuralNet | 1.282 | 1.218 | 1.141 | 0.947× | 0.890× | — |
| Etc::Sieve | 1.563 | 1.905 | 1.379 | 1.290× | 0.938× | — |
| Etc::TextRaytracer | 1.153 | 1.018 | 1.339 | 0.883× | 1.159× | — |
| Etc::Words | 1.327 | 1.288 | 1.327 | 0.968× | 0.991× | — |
| Graph::AStar | 1.316 | 1.310 | 1.074 | 1.001× | 0.811× | — |
| Graph::BFS | 1.736 | 1.214 | 1.151 | 0.705× | 0.665× | gcc |
| Graph::DFS | 1.348 | 1.181 | 1.244 | 0.884× | 0.915× | — |
| Hash::CRC32 | 0.293 | 0.806 | 0.915 | 2.744× | 3.123× | — |
| Hash::SHA256 | 1.648 | 1.472 | 1.379 | 0.894× | 0.835× | — |
| Json::Generate | 1.099 | 1.117 | 1.117 | 1.016× | 1.017× | — |
| Json::ParseDom | 1.292 | 1.186 | 1.188 | 0.918× | 0.917× | — |
| Json::ParseMapping | 1.228 | 1.129 | 1.131 | 0.918× | 0.924× | — |
| Matmul::Single | 1.385 | 1.385 | 1.381 | 1.000× | 0.997× | — |
| Matmul::T16 | 2.642 | 2.619 | 2.639 | 0.988× | 1.002× | — |
| Matmul::T4 | 1.353 | 1.352 | 1.352 | 0.999× | 1.000× | — |
| Matmul::T8 | 1.368 | 1.362 | 1.368 | 0.995× | 1.001× | — |
| Maze::AStar | 1.274 | 1.144 | 1.319 | 0.913× | 1.038× | clang |
| Maze::BFS | 1.309 | 1.413 | 1.515 | 1.070× | 1.169× | gcc, clang |
| Maze::Generator | 1.329 | 1.266 | 1.278 | 0.959× | 0.961× | — |
| Sort::Merge | 1.209 | 1.126 | 1.026 | 0.931× | 0.849× | — |
| Sort::Quick | 1.335 | 1.232 | 1.252 | 0.923× | 0.936× | — |
| Sort::Self | 1.310 | 1.207 | 1.229 | 0.921× | 0.938× | — |
| Template::Parse | 1.261 | 1.308 | 1.352 | 1.059× | 1.047× | gcc, clang |
| Template::Regex | 1.107 | 1.359 | 1.539 | 1.235× | 1.383× | gcc |

A/A warnings flag variation above 2%; no cases are dropped. Small timing differences need confirmation.
Compile gcc A/A CPU ratio: 0.998×.
Compile clang A/A CPU ratio: 1.006×.

Hardware counters: unavailable: perf is not installed on this host.

## Toolchains

- **cppgm**: cppgm++ (host configuration: g++)
- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and header hashes](metadata.json) · [Raw compile observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42674013073) · [Raw runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42674013073)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42674013073)
