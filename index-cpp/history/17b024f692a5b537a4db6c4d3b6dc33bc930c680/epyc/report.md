# index-cpp benchmarks

Compiler checkout: `17b024f692a5b537a4db6c4d3b6dc33bc930c680` · CPU: AMD EPYC 9V74 80-Core Processor

Unchanged upstream: `79e0b423094d420da37fb714f20c53fc35eeb2a3`. All 50 checksums pass with cppgm, GCC and Clang. Full runtime inputs, paired ABBA/BAAB blocks with A/A calibration. Speeds are reference time / cppgm time: higher is better. The aggregate gives all 50 workloads equal weight and is separate from the public suite.

| Measurement | vs GCC | vs Clang |
|---|---:|---:|
| Generated code | 0.998× | 0.985× |
| Compile CPU | 6.641× | 5.563× |

| Compiler | Executable bytes | .text bytes | Compile peak RSS (KiB) |
|---|---:|---:|---:|
| cppgm | 822,840 | 385,058 | 227,678 |
| gcc | 540,112 | 280,921 | 485,986 |
| clang | 513,720 | 264,561 | 319,172 |

| Workload | cppgm seconds | GCC seconds | Clang seconds | vs GCC | vs Clang | A/A warning |
|---|---:|---:|---:|---:|---:|---|
| Base64::Decode | 1.878 | 0.949 | 1.000 | 0.506× | 0.535× | — |
| Base64::Encode | 1.432 | 1.200 | 1.012 | 0.837× | 0.704× | — |
| Binarytrees::Arena | 2.092 | 2.206 | 2.169 | 1.052× | 1.043× | — |
| Binarytrees::Obj | 2.078 | 1.983 | 2.077 | 0.955× | 0.988× | — |
| Brainfuck::Array | 2.349 | 2.138 | 1.484 | 0.949× | 0.616× | clang |
| Brainfuck::Recursion | 1.386 | 1.173 | 1.151 | 0.847× | 0.850× | — |
| CLBG::Fannkuchredux | 0.980 | 0.972 | 0.895 | 0.993× | 0.922× | gcc |
| CLBG::Mandelbrot | 1.240 | 1.171 | 1.115 | 0.945× | 0.904× | — |
| CLBG::Nbody | 0.886 | 0.891 | 0.895 | 1.006× | 1.009× | — |
| CLBG::Spectralnorm | 0.954 | 0.925 | 1.578 | 0.969× | 1.655× | — |
| CSV::Parse | 0.963 | 0.823 | 0.905 | 0.851× | 0.945× | — |
| Calculator::Ast | 0.974 | 0.746 | 0.789 | 0.765× | 0.808× | — |
| Calculator::Interpreter | 1.165 | 1.186 | 1.265 | 1.010× | 1.083× | gcc, clang |
| Compress::ArithDecode | 0.764 | 0.812 | 1.399 | 1.063× | 1.831× | — |
| Compress::ArithEncode | 0.938 | 0.853 | 0.952 | 0.908× | 1.016× | — |
| Compress::BWTDecode | 0.564 | 0.538 | 0.560 | 0.956× | 0.990× | — |
| Compress::BWTEncode | 0.919 | 0.857 | 0.993 | 0.937× | 1.079× | — |
| Compress::HuffDecode | 0.515 | 0.596 | 0.413 | 1.157× | 0.802× | — |
| Compress::HuffEncode | 0.702 | 0.776 | 0.917 | 1.089× | 1.310× | — |
| Compress::LZWDecode | 0.593 | 0.630 | 0.561 | 1.072× | 0.944× | — |
| Compress::LZWEncode | 0.998 | 0.985 | 0.996 | 0.983× | 0.996× | — |
| Distance::Jaro | 1.068 | 1.050 | 0.978 | 0.984× | 0.917× | — |
| Distance::NGram | 1.098 | 1.074 | 1.052 | 0.980× | 0.958× | — |
| Etc::CacheSimulation | 0.895 | 0.879 | 0.891 | 0.986× | 0.994× | clang |
| Etc::GameOfLife | 1.478 | 1.067 | 1.335 | 0.757× | 0.922× | gcc, clang |
| Etc::LogParser | 0.460 | 1.740 | 0.522 | 3.739× | 1.111× | clang |
| Etc::NeuralNet | 1.011 | 0.835 | 0.829 | 0.820× | 0.825× | gcc, clang |
| Etc::Sieve | 1.183 | 1.567 | 1.193 | 1.324× | 1.022× | — |
| Etc::TextRaytracer | 1.149 | 0.881 | 1.060 | 0.766× | 0.922× | — |
| Etc::Words | 1.018 | 0.970 | 1.006 | 0.956× | 0.987× | — |
| Graph::AStar | 1.243 | 1.239 | 1.020 | 1.002× | 0.825× | gcc, clang |
| Graph::BFS | 1.938 | 1.324 | 1.235 | 0.690× | 0.635× | gcc, clang |
| Graph::DFS | 1.406 | 1.222 | 1.261 | 0.869× | 0.899× | — |
| Hash::CRC32 | 0.257 | 0.706 | 0.802 | 2.745× | 3.121× | — |
| Hash::SHA256 | 1.196 | 1.168 | 1.176 | 0.970× | 0.984× | — |
| Json::Generate | 0.765 | 0.773 | 0.766 | 1.007× | 1.002× | — |
| Json::ParseDom | 0.940 | 0.943 | 0.895 | 0.983× | 0.958× | — |
| Json::ParseMapping | 0.897 | 0.891 | 0.857 | 0.995× | 0.941× | — |
| Matmul::Single | 1.196 | 1.186 | 1.187 | 0.991× | 0.992× | — |
| Matmul::T16 | 2.354 | 2.314 | 2.309 | 0.984× | 0.977× | — |
| Matmul::T4 | 1.169 | 1.168 | 1.163 | 0.999× | 0.994× | — |
| Matmul::T8 | 1.211 | 1.197 | 1.194 | 0.990× | 0.986× | — |
| Maze::AStar | 1.329 | 1.308 | 1.529 | 1.051× | 1.085× | gcc, clang |
| Maze::BFS | 1.211 | 1.239 | 1.304 | 1.160× | 0.945× | gcc, clang |
| Maze::Generator | 1.344 | 1.330 | 1.324 | 1.002× | 0.965× | — |
| Sort::Merge | 0.972 | 0.988 | 0.879 | 1.009× | 0.904× | — |
| Sort::Quick | 1.166 | 1.085 | 1.100 | 0.931× | 0.945× | — |
| Sort::Self | 1.147 | 1.041 | 1.057 | 0.907× | 0.922× | — |
| Template::Parse | 1.264 | 1.117 | 1.365 | 1.000× | 1.157× | gcc, clang |
| Template::Regex | 1.047 | 1.218 | 1.389 | 1.143× | 1.334× | gcc, clang |

A/A warnings flag variation above 2%; no cases are dropped. Small timing differences need confirmation.
Compile gcc A/A CPU ratio: 1.018×.
Compile clang A/A CPU ratio: 1.001×.

Hardware counters: unavailable: perf is not installed on this host.

## Toolchains

- **cppgm**: cppgm++ (host configuration: g++)
- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and header hashes](metadata.json) · [Raw compile observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42516011682) · [Raw runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42516011682)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42516011682)
