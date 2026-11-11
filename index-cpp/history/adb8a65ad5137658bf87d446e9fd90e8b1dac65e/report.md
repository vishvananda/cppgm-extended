# index-cpp benchmarks

Compiler checkout: `adb8a65ad5137658bf87d446e9fd90e8b1dac65e` · CPU: AMD EPYC 7763 64-Core Processor

Unchanged upstream: `79e0b423094d420da37fb714f20c53fc35eeb2a3`. All 50 checksums pass with cppgm, GCC and Clang. Full runtime inputs, paired ABBA/BAAB blocks with A/A calibration. Speeds are reference time / cppgm time: higher is better. The aggregate gives all 50 workloads equal weight and is separate from the public suite.

| Measurement | vs GCC | vs Clang |
|---|---:|---:|
| Generated code | 0.991× | 0.986× |
| Compile CPU | 6.703× | 6.276× |

| Compiler | Executable bytes | .text bytes | Compile peak RSS (KiB) |
|---|---:|---:|---:|
| cppgm | 822,840 | 386,370 | 224,622 |
| gcc | 540,112 | 280,921 | 486,020 |
| clang | 513,720 | 264,561 | 319,074 |

| Workload | cppgm seconds | GCC seconds | Clang seconds | vs GCC | vs Clang | A/A warning |
|---|---:|---:|---:|---:|---:|---|
| Base64::Decode | 2.607 | 1.184 | 1.196 | 0.453× | 0.457× | — |
| Base64::Encode | 1.769 | 1.419 | 1.270 | 0.803× | 0.718× | — |
| Binarytrees::Arena | 2.351 | 2.433 | 2.519 | 1.036× | 1.073× | — |
| Binarytrees::Obj | 1.752 | 1.389 | 1.699 | 0.791× | 0.936× | gcc, clang |
| Brainfuck::Array | 2.820 | 2.809 | 2.835 | 0.962× | 1.023× | clang |
| Brainfuck::Recursion | 1.678 | 1.738 | 1.707 | 1.036× | 1.038× | gcc |
| CLBG::Fannkuchredux | 1.080 | 1.147 | 0.972 | 1.063× | 0.907× | — |
| CLBG::Mandelbrot | 1.381 | 1.351 | 1.335 | 0.978× | 0.966× | — |
| CLBG::Nbody | 1.024 | 1.054 | 1.056 | 1.027× | 1.032× | — |
| CLBG::Spectralnorm | 1.177 | 1.141 | 1.736 | 0.969× | 1.476× | — |
| CSV::Parse | 1.214 | 1.081 | 1.140 | 0.892× | 0.940× | — |
| Calculator::Ast | 1.179 | 0.958 | 0.986 | 0.812× | 0.838× | — |
| Calculator::Interpreter | 1.313 | 1.253 | 1.371 | 0.952× | 1.054× | — |
| Compress::ArithDecode | 0.935 | 0.947 | 1.603 | 1.012× | 1.712× | — |
| Compress::ArithEncode | 1.047 | 0.981 | 1.109 | 0.937× | 1.058× | — |
| Compress::BWTDecode | 0.653 | 0.647 | 0.655 | 0.989× | 1.003× | — |
| Compress::BWTEncode | 1.177 | 1.129 | 1.348 | 0.950× | 1.151× | — |
| Compress::HuffDecode | 0.656 | 0.772 | 0.512 | 1.179× | 0.781× | — |
| Compress::HuffEncode | 0.855 | 1.227 | 1.032 | 1.431× | 1.207× | — |
| Compress::LZWDecode | 0.721 | 0.730 | 0.695 | 1.012× | 0.969× | — |
| Compress::LZWEncode | 1.331 | 1.296 | 1.308 | 0.970× | 0.984× | — |
| Distance::Jaro | 1.222 | 1.171 | 1.148 | 0.958× | 0.939× | — |
| Distance::NGram | 1.367 | 1.320 | 1.285 | 0.967× | 0.943× | — |
| Etc::CacheSimulation | 1.137 | 1.108 | 1.137 | 0.981× | 0.997× | — |
| Etc::GameOfLife | 1.433 | 1.113 | 1.178 | 0.776× | 0.812× | — |
| Etc::LogParser | 0.592 | 1.944 | 0.666 | 3.349× | 1.125× | — |
| Etc::NeuralNet | 1.271 | 1.154 | 1.079 | 0.905× | 0.853× | — |
| Etc::Sieve | 1.382 | 1.963 | 1.452 | 1.348× | 0.997× | gcc, clang |
| Etc::TextRaytracer | 1.533 | 1.020 | 1.338 | 0.666× | 0.873× | — |
| Etc::Words | 1.341 | 1.300 | 1.333 | 0.964× | 0.998× | gcc |
| Graph::AStar | 1.297 | 1.291 | 1.046 | 0.991× | 0.806× | — |
| Graph::BFS | 1.706 | 1.131 | 1.066 | 0.659× | 0.622× | — |
| Graph::DFS | 1.322 | 1.160 | 1.216 | 0.876× | 0.917× | — |
| Hash::CRC32 | 0.293 | 0.806 | 0.915 | 2.749× | 3.123× | — |
| Hash::SHA256 | 1.651 | 1.472 | 1.378 | 0.891× | 0.837× | — |
| Json::Generate | 1.119 | 1.128 | 1.133 | 1.008× | 1.009× | — |
| Json::ParseDom | 1.258 | 1.193 | 1.185 | 0.953× | 0.941× | — |
| Json::ParseMapping | 1.196 | 1.133 | 1.129 | 0.942× | 0.943× | — |
| Matmul::Single | 1.387 | 1.387 | 1.383 | 1.001× | 0.998× | — |
| Matmul::T16 | 2.631 | 2.632 | 2.635 | 1.000× | 0.999× | — |
| Matmul::T4 | 1.354 | 1.353 | 1.354 | 0.999× | 1.000× | — |
| Matmul::T8 | 1.370 | 1.367 | 1.378 | 0.997× | 1.002× | — |
| Maze::AStar | 1.200 | 1.034 | 1.185 | 0.865× | 0.992× | — |
| Maze::BFS | 1.240 | 1.202 | 1.246 | 0.999× | 0.998× | gcc |
| Maze::Generator | 1.228 | 1.218 | 1.200 | 0.989× | 0.984× | — |
| Sort::Merge | 1.115 | 1.126 | 1.024 | 1.002× | 0.918× | — |
| Sort::Quick | 1.349 | 1.231 | 1.250 | 0.913× | 0.926× | — |
| Sort::Self | 1.324 | 1.205 | 1.228 | 0.909× | 0.928× | — |
| Template::Parse | 1.224 | 1.192 | 1.208 | 0.950× | 0.996× | gcc, clang |
| Template::Regex | 1.027 | 1.294 | 1.462 | 1.262× | 1.395× | — |

A/A warnings flag variation above 2%; no cases are dropped. Small timing differences need confirmation.
Compile gcc A/A CPU ratio: 0.996×.
Compile clang A/A CPU ratio: 1.010×.

Hardware counters: unavailable: perf is not installed on this host.

## Toolchains

- **cppgm**: cppgm++ (host configuration: g++)
- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and header hashes](metadata.json) · [Raw compile observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42414627747) · [Raw runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42414627747)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42414627747)
