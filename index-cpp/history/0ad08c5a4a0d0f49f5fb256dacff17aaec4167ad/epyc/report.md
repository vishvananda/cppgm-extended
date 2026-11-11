# index-cpp benchmarks

Compiler checkout: `0ad08c5a4a0d0f49f5fb256dacff17aaec4167ad` · CPU: AMD EPYC 9V74 80-Core Processor

Unchanged upstream: `79e0b423094d420da37fb714f20c53fc35eeb2a3`. All 50 checksums pass with cppgm, GCC and Clang. Full runtime inputs, paired ABBA/BAAB blocks with A/A calibration. Speeds are reference time / cppgm time: higher is better. The aggregate gives all 50 workloads equal weight and is separate from the public suite.

| Measurement | vs GCC | vs Clang |
|---|---:|---:|
| Generated code | 1.002× | 0.995× |
| Compile CPU | 6.604× | 5.608× |

| Compiler | Executable bytes | .text bytes | Compile peak RSS (KiB) |
|---|---:|---:|---:|
| cppgm | 771,352 | 389,474 | 228,276 |
| gcc | 540,112 | 280,921 | 485,970 |
| clang | 513,720 | 264,561 | 319,162 |

| Workload | cppgm seconds | GCC seconds | Clang seconds | vs GCC | vs Clang | A/A warning |
|---|---:|---:|---:|---:|---:|---|
| Base64::Decode | 1.873 | 0.950 | 0.994 | 0.510× | 0.532× | — |
| Base64::Encode | 1.352 | 1.194 | 1.011 | 0.891× | 0.747× | clang |
| Binarytrees::Arena | 2.136 | 2.197 | 2.191 | 1.040× | 1.030× | — |
| Binarytrees::Obj | 2.035 | 1.980 | 2.056 | 0.999× | 0.992× | gcc |
| Brainfuck::Array | 1.958 | 2.135 | 1.530 | 1.078× | 0.813× | gcc |
| Brainfuck::Recursion | 1.205 | 1.177 | 1.147 | 0.935× | 0.985× | clang |
| CLBG::Fannkuchredux | 0.996 | 0.983 | 0.891 | 0.988× | 0.924× | — |
| CLBG::Mandelbrot | 1.239 | 1.171 | 1.115 | 0.945× | 0.900× | — |
| CLBG::Nbody | 0.886 | 0.891 | 0.894 | 1.005× | 1.010× | — |
| CLBG::Spectralnorm | 0.954 | 0.925 | 1.575 | 0.970× | 1.648× | — |
| CSV::Parse | 0.899 | 0.823 | 0.907 | 0.919× | 1.008× | — |
| Calculator::Ast | 0.972 | 0.746 | 0.790 | 0.767× | 0.812× | — |
| Calculator::Interpreter | 1.163 | 1.052 | 1.257 | 0.923× | 1.021× | gcc, clang |
| Compress::ArithDecode | 0.768 | 0.817 | 1.395 | 1.059× | 1.818× | — |
| Compress::ArithEncode | 0.913 | 0.854 | 0.951 | 0.935× | 1.041× | — |
| Compress::BWTDecode | 0.562 | 0.538 | 0.560 | 0.957× | 0.996× | — |
| Compress::BWTEncode | 0.912 | 0.857 | 0.996 | 0.945× | 1.087× | — |
| Compress::HuffDecode | 0.556 | 0.597 | 0.413 | 1.071× | 0.742× | — |
| Compress::HuffEncode | 0.702 | 0.774 | 0.920 | 1.085× | 1.308× | — |
| Compress::LZWDecode | 0.589 | 0.655 | 0.565 | 1.116× | 0.955× | — |
| Compress::LZWEncode | 0.999 | 0.977 | 0.994 | 0.977× | 0.995× | — |
| Distance::Jaro | 1.064 | 1.051 | 0.977 | 0.989× | 0.917× | — |
| Distance::NGram | 1.098 | 1.071 | 1.050 | 0.973× | 0.957× | — |
| Etc::CacheSimulation | 0.899 | 0.877 | 0.889 | 0.970× | 0.988× | — |
| Etc::GameOfLife | 1.276 | 1.050 | 1.080 | 0.850× | 0.826× | gcc, clang |
| Etc::LogParser | 0.464 | 1.761 | 0.522 | 3.807× | 1.103× | clang |
| Etc::NeuralNet | 1.148 | 0.829 | 0.877 | 0.764× | 0.804× | gcc |
| Etc::Sieve | 1.183 | 1.563 | 1.192 | 1.322× | 1.007× | — |
| Etc::TextRaytracer | 0.923 | 0.882 | 1.060 | 0.951× | 1.149× | — |
| Etc::Words | 1.017 | 0.968 | 1.014 | 0.960× | 0.992× | — |
| Graph::AStar | 1.232 | 1.232 | 1.030 | 1.003× | 0.846× | gcc, clang |
| Graph::BFS | 1.824 | 1.323 | 1.238 | 0.729× | 0.663× | gcc, clang |
| Graph::DFS | 1.405 | 1.230 | 1.261 | 0.876× | 0.888× | clang |
| Hash::CRC32 | 0.257 | 0.706 | 0.802 | 2.747× | 3.121× | — |
| Hash::SHA256 | 1.196 | 1.169 | 1.176 | 0.979× | 0.978× | — |
| Json::Generate | 0.740 | 0.772 | 0.772 | 1.049× | 1.040× | — |
| Json::ParseDom | 0.942 | 0.925 | 0.901 | 0.982× | 0.959× | — |
| Json::ParseMapping | 0.897 | 0.882 | 0.856 | 0.987× | 0.952× | clang |
| Matmul::Single | 1.199 | 1.184 | 1.189 | 0.987× | 0.993× | — |
| Matmul::T16 | 2.359 | 2.303 | 2.297 | 0.976× | 0.975× | — |
| Matmul::T4 | 1.168 | 1.171 | 1.164 | 1.002× | 0.997× | — |
| Matmul::T8 | 1.216 | 1.196 | 1.194 | 0.984× | 0.980× | — |
| Maze::AStar | 1.297 | 1.069 | 1.400 | 0.866× | 1.121× | gcc, clang |
| Maze::BFS | 1.130 | 0.956 | 1.103 | 0.880× | 1.014× | gcc, clang |
| Maze::Generator | 1.341 | 1.280 | 1.341 | 0.965× | 0.959× | gcc, clang |
| Sort::Merge | 1.063 | 0.988 | 0.879 | 0.932× | 0.825× | — |
| Sort::Quick | 1.146 | 1.085 | 1.100 | 0.948× | 0.960× | — |
| Sort::Self | 1.135 | 1.041 | 1.058 | 0.917× | 0.932× | — |
| Template::Parse | 1.056 | 1.042 | 1.153 | 1.007× | 1.110× | gcc, clang |
| Template::Regex | 0.923 | 1.125 | 1.157 | 1.256× | 1.241× | gcc, clang |

A/A warnings flag variation above 2%; no cases are dropped. Small timing differences need confirmation.
Compile gcc A/A CPU ratio: 0.997×.
Compile clang A/A CPU ratio: 1.011×.

Hardware counters: unavailable: perf is not installed on this host.

## Toolchains

- **cppgm**: cppgm++ (host configuration: g++)
- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and header hashes](metadata.json) · [Raw compile observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42770606087) · [Raw runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42770606087)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42770606087)
