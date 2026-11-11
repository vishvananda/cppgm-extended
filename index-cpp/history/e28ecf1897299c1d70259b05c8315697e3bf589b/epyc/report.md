# index-cpp benchmarks

Compiler checkout: `e28ecf1897299c1d70259b05c8315697e3bf589b` · CPU: AMD EPYC 9V74 80-Core Processor

Unchanged upstream: `79e0b423094d420da37fb714f20c53fc35eeb2a3`. All 50 checksums pass with cppgm, GCC and Clang. Full runtime inputs, paired ABBA/BAAB blocks with A/A calibration. Speeds are reference time / cppgm time: higher is better. The aggregate gives all 50 workloads equal weight and is separate from the public suite.

| Measurement | vs GCC | vs Clang |
|---|---:|---:|
| Generated code | 0.996× | 0.979× |
| Compile CPU | 6.808× | 5.627× |

| Compiler | Executable bytes | .text bytes | Compile peak RSS (KiB) |
|---|---:|---:|---:|
| cppgm | 822,840 | 385,058 | 227,532 |
| gcc | 540,112 | 280,921 | 485,982 |
| clang | 513,720 | 264,561 | 319,156 |

| Workload | cppgm seconds | GCC seconds | Clang seconds | vs GCC | vs Clang | A/A warning |
|---|---:|---:|---:|---:|---:|---|
| Base64::Decode | 1.881 | 0.960 | 0.998 | 0.511× | 0.530× | — |
| Base64::Encode | 1.441 | 1.219 | 1.015 | 0.838× | 0.705× | — |
| Binarytrees::Arena | 2.112 | 2.226 | 2.204 | 1.052× | 1.046× | gcc |
| Binarytrees::Obj | 2.066 | 2.007 | 2.048 | 0.972× | 0.977× | clang |
| Brainfuck::Array | 2.365 | 2.127 | 1.525 | 0.876× | 0.643× | clang |
| Brainfuck::Recursion | 1.389 | 1.180 | 1.149 | 0.850× | 0.844× | clang |
| CLBG::Fannkuchredux | 0.988 | 0.976 | 0.900 | 0.981× | 0.914× | — |
| CLBG::Mandelbrot | 1.240 | 1.171 | 1.114 | 0.944× | 0.898× | — |
| CLBG::Nbody | 0.887 | 0.891 | 0.895 | 1.004× | 1.008× | — |
| CLBG::Spectralnorm | 0.954 | 0.926 | 1.579 | 0.971× | 1.648× | — |
| CSV::Parse | 0.956 | 0.824 | 0.907 | 0.861× | 0.951× | — |
| Calculator::Ast | 0.974 | 0.752 | 0.789 | 0.772× | 0.811× | gcc |
| Calculator::Interpreter | 1.289 | 1.215 | 1.261 | 0.939× | 1.023× | gcc, clang |
| Compress::ArithDecode | 0.764 | 0.812 | 1.396 | 1.066× | 1.827× | — |
| Compress::ArithEncode | 0.939 | 0.854 | 0.952 | 0.910× | 1.013× | — |
| Compress::BWTDecode | 0.567 | 0.543 | 0.561 | 0.955× | 0.992× | — |
| Compress::BWTEncode | 0.915 | 0.856 | 0.996 | 0.931× | 1.092× | — |
| Compress::HuffDecode | 0.516 | 0.598 | 0.414 | 1.157× | 0.803× | — |
| Compress::HuffEncode | 0.702 | 0.774 | 0.917 | 1.102× | 1.307× | — |
| Compress::LZWDecode | 0.593 | 0.615 | 0.562 | 1.042× | 0.952× | — |
| Compress::LZWEncode | 0.998 | 0.985 | 0.998 | 0.990× | 0.997× | gcc |
| Distance::Jaro | 1.068 | 1.051 | 0.978 | 0.986× | 0.916× | — |
| Distance::NGram | 1.099 | 1.073 | 1.058 | 0.976× | 0.965× | — |
| Etc::CacheSimulation | 0.895 | 0.881 | 0.897 | 0.988× | 1.021× | — |
| Etc::GameOfLife | 1.562 | 1.517 | 1.336 | 0.964× | 0.845× | gcc, clang |
| Etc::LogParser | 0.459 | 1.727 | 0.522 | 3.812× | 1.133× | — |
| Etc::NeuralNet | 1.013 | 0.831 | 0.817 | 0.826× | 0.807× | gcc |
| Etc::Sieve | 1.186 | 1.570 | 1.195 | 1.326× | 1.007× | — |
| Etc::TextRaytracer | 1.149 | 0.881 | 1.060 | 0.767× | 0.923× | — |
| Etc::Words | 1.023 | 0.968 | 1.007 | 0.947× | 0.979× | — |
| Graph::AStar | 1.255 | 1.256 | 1.022 | 1.002× | 0.812× | — |
| Graph::BFS | 1.984 | 1.375 | 1.228 | 0.690× | 0.623× | clang |
| Graph::DFS | 1.426 | 1.235 | 1.253 | 0.857× | 0.882× | — |
| Hash::CRC32 | 0.257 | 0.706 | 0.802 | 2.747× | 3.121× | — |
| Hash::SHA256 | 1.197 | 1.171 | 1.176 | 0.977× | 0.979× | — |
| Json::Generate | 0.762 | 0.774 | 0.767 | 1.012× | 1.004× | — |
| Json::ParseDom | 0.946 | 0.918 | 0.891 | 0.964× | 0.935× | gcc |
| Json::ParseMapping | 0.905 | 0.877 | 0.853 | 0.964× | 0.946× | — |
| Matmul::Single | 1.197 | 1.187 | 1.186 | 0.990× | 0.990× | — |
| Matmul::T16 | 2.370 | 2.329 | 2.330 | 0.983× | 0.986× | — |
| Matmul::T4 | 1.170 | 1.171 | 1.164 | 1.000× | 0.997× | — |
| Matmul::T8 | 1.217 | 1.209 | 1.202 | 0.989× | 0.991× | gcc |
| Maze::AStar | 1.431 | 1.367 | 1.534 | 0.951× | 1.126× | clang |
| Maze::BFS | 1.356 | 1.337 | 1.349 | 0.966× | 1.019× | gcc |
| Maze::Generator | 1.418 | 1.406 | 1.361 | 0.980× | 0.981× | gcc, clang |
| Sort::Merge | 0.969 | 0.989 | 0.879 | 1.022× | 0.907× | — |
| Sort::Quick | 1.167 | 1.086 | 1.101 | 0.931× | 0.944× | — |
| Sort::Self | 1.149 | 1.042 | 1.058 | 0.907× | 0.921× | — |
| Template::Parse | 1.514 | 1.688 | 1.272 | 1.040× | 0.918× | gcc, clang |
| Template::Regex | 1.112 | 1.450 | 1.356 | 1.244× | 1.298× | clang |

A/A warnings flag variation above 2%; no cases are dropped. Small timing differences need confirmation.
Compile gcc A/A CPU ratio: 1.010×.
Compile clang A/A CPU ratio: 0.992×.

Hardware counters: unavailable: perf is not installed on this host.

## Toolchains

- **cppgm**: cppgm++ (host configuration: g++)
- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and header hashes](metadata.json) · [Raw compile observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42538308624) · [Raw runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42538308624)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42538308624)
