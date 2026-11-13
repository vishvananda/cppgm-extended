# index-cpp benchmarks

Compiler checkout: `b81455bc7d42635d12519c11c3788a20b5fad2f4` · CPU: AMD EPYC 7763 64-Core Processor

Unchanged upstream: `79e0b423094d420da37fb714f20c53fc35eeb2a3`. All 50 checksums pass with cppgm, GCC and Clang. Full runtime inputs, paired ABBA/BAAB blocks with A/A calibration. Speeds are reference time / cppgm time: higher is better. The aggregate gives all 50 workloads equal weight and is separate from the public suite.

| Measurement | vs GCC | vs Clang |
|---|---:|---:|
| Generated code | 1.034× | 1.023× |
| Compile CPU | 6.311× | 5.783× |

| Compiler | Executable bytes | .text bytes | Compile peak RSS (KiB) |
|---|---:|---:|---:|
| cppgm | 714,480 | 341,490 | 228,320 |
| gcc | 540,112 | 280,921 | 486,082 |
| clang | 513,720 | 264,561 | 319,256 |

| Workload | cppgm seconds | GCC seconds | Clang seconds | vs GCC | vs Clang | A/A warning |
|---|---:|---:|---:|---:|---:|---|
| Base64::Decode | 1.667 | 1.204 | 1.216 | 0.725× | 0.732× | — |
| Base64::Encode | 1.744 | 1.458 | 1.324 | 0.835× | 0.757× | — |
| Binarytrees::Arena | 2.615 | 2.624 | 2.732 | 1.009× | 1.042× | — |
| Binarytrees::Obj | 1.582 | 1.547 | 1.681 | 1.011× | 1.049× | clang |
| Brainfuck::Array | 2.654 | 2.808 | 2.768 | 1.065× | 1.026× | — |
| Brainfuck::Recursion | 1.979 | 1.740 | 1.707 | 0.877× | 0.863× | — |
| CLBG::Fannkuchredux | 1.006 | 1.147 | 0.978 | 1.137× | 0.978× | — |
| CLBG::Mandelbrot | 1.381 | 1.350 | 1.335 | 0.977× | 0.967× | — |
| CLBG::Nbody | 1.036 | 1.055 | 1.057 | 1.018× | 1.020× | — |
| CLBG::Spectralnorm | 1.178 | 1.139 | 1.736 | 0.967× | 1.475× | — |
| CSV::Parse | 1.206 | 1.083 | 1.155 | 0.897× | 0.958× | — |
| Calculator::Ast | 1.042 | 0.957 | 0.989 | 0.923× | 0.945× | — |
| Calculator::Interpreter | 1.474 | 1.369 | 1.455 | 0.925× | 0.982× | gcc, clang |
| Compress::ArithDecode | 0.934 | 0.948 | 1.605 | 1.015× | 1.719× | — |
| Compress::ArithEncode | 1.086 | 0.980 | 1.110 | 0.902× | 1.023× | — |
| Compress::BWTDecode | 0.648 | 0.655 | 0.665 | 1.018× | 1.028× | — |
| Compress::BWTEncode | 1.135 | 1.144 | 1.371 | 1.008× | 1.203× | — |
| Compress::HuffDecode | 0.774 | 0.778 | 0.518 | 1.005× | 0.670× | — |
| Compress::HuffEncode | 0.901 | 1.228 | 1.034 | 1.362× | 1.148× | — |
| Compress::LZWDecode | 0.740 | 0.755 | 0.704 | 1.024× | 0.954× | — |
| Compress::LZWEncode | 1.330 | 1.293 | 1.298 | 0.967× | 0.983× | — |
| Distance::Jaro | 1.264 | 1.172 | 1.149 | 0.923× | 0.912× | — |
| Distance::NGram | 1.373 | 1.324 | 1.300 | 0.963× | 0.947× | gcc, clang |
| Etc::CacheSimulation | 1.141 | 1.103 | 1.120 | 0.976× | 0.973× | — |
| Etc::GameOfLife | 1.279 | 1.212 | 1.307 | 0.965× | 0.997× | gcc, clang |
| Etc::LogParser | 0.581 | 2.041 | 0.667 | 3.506× | 1.136× | gcc |
| Etc::NeuralNet | 1.256 | 1.105 | 1.080 | 0.876× | 0.859× | gcc |
| Etc::Sieve | 1.546 | 1.997 | 1.558 | 1.283× | 0.994× | — |
| Etc::TextRaytracer | 1.395 | 1.024 | 1.340 | 0.734× | 0.961× | — |
| Etc::Words | 1.330 | 1.292 | 1.351 | 0.980× | 1.005× | gcc |
| Graph::AStar | 1.441 | 1.359 | 1.129 | 0.940× | 0.787× | — |
| Graph::BFS | 1.796 | 1.331 | 1.272 | 0.735× | 0.715× | clang |
| Graph::DFS | 1.567 | 1.250 | 1.309 | 0.794× | 0.835× | clang |
| Hash::CRC32 | 0.294 | 0.806 | 0.915 | 2.740× | 3.119× | — |
| Hash::SHA256 | 0.452 | 1.473 | 1.380 | 3.257× | 3.049× | — |
| Json::Generate | 1.146 | 1.132 | 1.160 | 0.990× | 1.016× | — |
| Json::ParseDom | 1.288 | 1.198 | 1.204 | 0.937× | 0.925× | — |
| Json::ParseMapping | 1.219 | 1.141 | 1.144 | 0.944× | 0.930× | — |
| Matmul::Single | 1.391 | 1.388 | 1.385 | 0.998× | 0.995× | — |
| Matmul::T16 | 2.634 | 2.627 | 2.638 | 0.998× | 1.000× | — |
| Matmul::T4 | 1.356 | 1.355 | 1.355 | 0.999× | 0.999× | — |
| Matmul::T8 | 1.376 | 1.383 | 1.383 | 1.007× | 1.005× | — |
| Maze::AStar | 1.502 | 1.296 | 1.523 | 0.865× | 1.008× | gcc, clang |
| Maze::BFS | 1.778 | 1.783 | 1.796 | 1.057× | 0.957× | gcc, clang |
| Maze::Generator | 1.401 | 1.426 | 1.398 | 1.015× | 1.005× | gcc, clang |
| Sort::Merge | 1.147 | 1.138 | 1.040 | 0.993× | 0.903× | — |
| Sort::Quick | 1.367 | 1.244 | 1.272 | 0.915× | 0.929× | — |
| Sort::Self | 1.268 | 1.221 | 1.250 | 0.966× | 0.983× | — |
| Template::Parse | 1.497 | 1.546 | 1.519 | 0.999× | 1.057× | gcc, clang |
| Template::Regex | 1.486 | 1.482 | 1.635 | 1.043× | 1.152× | — |

A/A warnings flag variation above 2%; no cases are dropped. Small timing differences need confirmation.
Compile gcc A/A CPU ratio: 0.994×.
Compile clang A/A CPU ratio: 0.988×.

Hardware counters: unavailable: perf is not installed on this host.

## Toolchains

- **cppgm**: cppgm++ (host configuration: g++)
- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and header hashes](metadata.json) · [Raw compile observations](compile-observations.jsonl) · [Raw runtime observations](runtime-observations.jsonl)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43493409829)
