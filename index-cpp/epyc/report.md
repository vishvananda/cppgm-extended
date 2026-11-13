# index-cpp benchmarks

Compiler checkout: `393d1b1d09089ffdade676f4e7cbad8759c15d2a` · CPU: AMD EPYC 9V74 80-Core Processor

Unchanged upstream: `79e0b423094d420da37fb714f20c53fc35eeb2a3`. All 50 checksums pass with cppgm, GCC and Clang. Full runtime inputs, paired ABBA/BAAB blocks with A/A calibration. Speeds are reference time / cppgm time: higher is better. The aggregate gives all 50 workloads equal weight and is separate from the public suite.

| Measurement | vs GCC | vs Clang |
|---|---:|---:|
| Generated code | 1.005× | 1.004× |
| Compile CPU | 6.306× | 5.287× |

| Compiler | Executable bytes | .text bytes | Compile peak RSS (KiB) |
|---|---:|---:|---:|
| cppgm | 718,576 | 343,554 | 228,492 |
| gcc | 540,112 | 280,921 | 486,100 |
| clang | 513,720 | 264,561 | 319,134 |

| Workload | cppgm seconds | GCC seconds | Clang seconds | vs GCC | vs Clang | A/A warning |
|---|---:|---:|---:|---:|---:|---|
| Base64::Decode | 1.292 | 0.953 | 0.998 | 0.737× | 0.773× | — |
| Base64::Encode | 1.358 | 1.195 | 1.010 | 0.882× | 0.747× | — |
| Binarytrees::Arena | 2.125 | 2.214 | 2.187 | 1.043× | 1.016× | — |
| Binarytrees::Obj | 2.040 | 1.956 | 2.099 | 0.952× | 1.013× | gcc |
| Brainfuck::Array | 1.457 | 2.146 | 1.513 | 1.459× | 1.033× | gcc, clang |
| Brainfuck::Recursion | 1.155 | 1.180 | 1.148 | 1.034× | 0.992× | clang |
| CLBG::Fannkuchredux | 0.936 | 0.978 | 0.913 | 1.055× | 0.976× | — |
| CLBG::Mandelbrot | 1.239 | 1.171 | 1.115 | 0.945× | 0.900× | — |
| CLBG::Nbody | 0.889 | 0.891 | 0.895 | 1.002× | 1.007× | — |
| CLBG::Spectralnorm | 0.960 | 0.927 | 1.575 | 0.965× | 1.640× | — |
| CSV::Parse | 0.916 | 0.821 | 0.905 | 0.897× | 0.990× | — |
| Calculator::Ast | 0.831 | 0.748 | 0.794 | 0.897× | 0.958× | — |
| Calculator::Interpreter | 1.255 | 1.137 | 1.245 | 0.884× | 0.983× | — |
| Compress::ArithDecode | 0.794 | 0.812 | 1.395 | 1.019× | 1.756× | — |
| Compress::ArithEncode | 0.966 | 0.854 | 0.952 | 0.885× | 0.987× | — |
| Compress::BWTDecode | 0.554 | 0.540 | 0.562 | 0.971× | 1.011× | — |
| Compress::BWTEncode | 0.825 | 0.857 | 0.993 | 1.036× | 1.207× | — |
| Compress::HuffDecode | 0.726 | 0.597 | 0.413 | 0.821× | 0.570× | — |
| Compress::HuffEncode | 0.837 | 0.775 | 0.919 | 0.925× | 1.098× | — |
| Compress::LZWDecode | 0.603 | 0.628 | 0.560 | 1.030× | 0.930× | — |
| Compress::LZWEncode | 0.994 | 0.979 | 0.994 | 0.983× | 0.995× | — |
| Distance::Jaro | 1.076 | 1.051 | 0.976 | 0.979× | 0.908× | — |
| Distance::NGram | 1.104 | 1.071 | 1.050 | 0.968× | 0.951× | — |
| Etc::CacheSimulation | 0.900 | 0.883 | 0.887 | 0.982× | 0.982× | — |
| Etc::GameOfLife | 1.238 | 1.040 | 1.203 | 0.811× | 1.003× | gcc, clang |
| Etc::LogParser | 0.407 | 1.740 | 0.506 | 4.278× | 1.236× | — |
| Etc::NeuralNet | 0.976 | 0.833 | 0.819 | 0.859× | 0.810× | clang |
| Etc::Sieve | 1.181 | 1.563 | 1.189 | 1.323× | 1.005× | — |
| Etc::TextRaytracer | 1.130 | 0.881 | 1.060 | 0.780× | 0.938× | — |
| Etc::Words | 1.004 | 0.964 | 1.004 | 0.956× | 1.006× | — |
| Graph::AStar | 1.291 | 1.248 | 1.035 | 0.965× | 0.803× | clang |
| Graph::BFS | 1.732 | 1.350 | 1.240 | 0.777× | 0.723× | gcc, clang |
| Graph::DFS | 1.490 | 1.233 | 1.259 | 0.823× | 0.845× | gcc |
| Hash::CRC32 | 0.257 | 0.706 | 0.802 | 2.745× | 3.121× | — |
| Hash::SHA256 | 1.198 | 1.172 | 1.179 | 0.980× | 0.988× | — |
| Json::Generate | 0.770 | 0.770 | 0.774 | 1.007× | 1.006× | — |
| Json::ParseDom | 0.956 | 0.917 | 0.900 | 0.952× | 0.946× | — |
| Json::ParseMapping | 0.915 | 0.878 | 0.857 | 0.948× | 0.938× | — |
| Matmul::Single | 1.184 | 1.181 | 1.186 | 0.997× | 0.997× | — |
| Matmul::T16 | 2.373 | 2.305 | 2.298 | 0.972× | 0.977× | — |
| Matmul::T4 | 1.168 | 1.169 | 1.164 | 1.000× | 0.997× | — |
| Matmul::T8 | 1.229 | 1.202 | 1.200 | 0.979× | 0.981× | — |
| Maze::AStar | 1.298 | 1.211 | 1.497 | 0.886× | 1.107× | gcc |
| Maze::BFS | 1.292 | 1.121 | 1.277 | 0.814× | 0.997× | gcc, clang |
| Maze::Generator | 1.320 | 1.316 | 1.357 | 0.991× | 1.021× | gcc |
| Sort::Merge | 0.978 | 0.986 | 0.879 | 1.013× | 0.898× | — |
| Sort::Quick | 1.126 | 1.086 | 1.100 | 0.964× | 0.976× | — |
| Sort::Self | 1.085 | 1.041 | 1.058 | 0.959× | 0.976× | — |
| Template::Parse | 1.027 | 1.211 | 1.224 | 1.112× | 1.086× | gcc, clang |
| Template::Regex | 1.198 | 1.185 | 1.287 | 0.959× | 1.116× | gcc, clang |

A/A warnings flag variation above 2%; no cases are dropped. Small timing differences need confirmation.
Compile gcc A/A CPU ratio: 0.996×.
Compile clang A/A CPU ratio: 0.995×.

Hardware counters: unavailable: perf is not installed on this host.

## Toolchains

- **cppgm**: cppgm++ (host configuration: g++)
- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and header hashes](metadata.json) · [Raw compile observations](compile-observations.jsonl) · [Raw runtime observations](runtime-observations.jsonl)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43357224966)
