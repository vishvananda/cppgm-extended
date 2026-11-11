# index-cpp benchmarks

Compiler checkout: `1ddd7507ee861649d43d1e72dcde7bb17a85a691` · CPU: AMD EPYC 9V74 80-Core Processor

Unchanged upstream: `79e0b423094d420da37fb714f20c53fc35eeb2a3`. All 50 checksums pass with cppgm, GCC and Clang. Full runtime inputs, paired ABBA/BAAB blocks with A/A calibration. Speeds are reference time / cppgm time: higher is better. The aggregate gives all 50 workloads equal weight and is separate from the public suite.

| Measurement | vs GCC | vs Clang |
|---|---:|---:|
| Generated code | 0.998× | 0.981× |
| Compile CPU | 6.542× | 5.487× |

| Compiler | Executable bytes | .text bytes | Compile peak RSS (KiB) |
|---|---:|---:|---:|
| cppgm | 801,632 | 389,890 | 228,108 |
| gcc | 540,112 | 280,921 | 486,028 |
| clang | 513,720 | 264,561 | 318,274 |

| Workload | cppgm seconds | GCC seconds | Clang seconds | vs GCC | vs Clang | A/A warning |
|---|---:|---:|---:|---:|---:|---|
| Base64::Decode | 1.883 | 0.951 | 0.995 | 0.506× | 0.529× | — |
| Base64::Encode | 1.360 | 1.198 | 1.005 | 0.886× | 0.736× | — |
| Binarytrees::Arena | 2.120 | 2.212 | 2.188 | 1.043× | 1.029× | — |
| Binarytrees::Obj | 2.056 | 1.947 | 2.037 | 0.958× | 0.993× | gcc, clang |
| Brainfuck::Array | 2.061 | 2.137 | 1.507 | 1.023× | 0.791× | clang |
| Brainfuck::Recursion | 1.283 | 1.180 | 1.147 | 0.924× | 0.895× | clang |
| CLBG::Fannkuchredux | 1.018 | 0.982 | 0.900 | 0.969× | 0.890× | — |
| CLBG::Mandelbrot | 1.241 | 1.171 | 1.115 | 0.942× | 0.900× | — |
| CLBG::Nbody | 0.887 | 0.891 | 0.895 | 1.001× | 1.009× | — |
| CLBG::Spectralnorm | 0.955 | 0.927 | 1.578 | 0.971× | 1.653× | — |
| CSV::Parse | 0.897 | 0.822 | 0.908 | 0.920× | 1.012× | — |
| Calculator::Ast | 0.988 | 0.747 | 0.790 | 0.764× | 0.779× | — |
| Calculator::Interpreter | 1.218 | 1.069 | 1.232 | 0.916× | 0.980× | gcc, clang |
| Compress::ArithDecode | 0.796 | 0.811 | 1.396 | 1.026× | 1.752× | — |
| Compress::ArithEncode | 0.946 | 0.854 | 0.954 | 0.902× | 1.005× | — |
| Compress::BWTDecode | 0.562 | 0.538 | 0.560 | 0.958× | 0.997× | — |
| Compress::BWTEncode | 0.900 | 0.853 | 0.999 | 0.945× | 1.105× | — |
| Compress::HuffDecode | 0.539 | 0.597 | 0.414 | 1.105× | 0.769× | — |
| Compress::HuffEncode | 0.696 | 0.774 | 0.917 | 1.110× | 1.316× | — |
| Compress::LZWDecode | 0.595 | 0.616 | 0.561 | 1.037× | 0.931× | — |
| Compress::LZWEncode | 1.005 | 0.977 | 0.994 | 0.974× | 0.984× | — |
| Distance::Jaro | 1.093 | 1.050 | 0.977 | 0.961× | 0.894× | — |
| Distance::NGram | 1.124 | 1.071 | 1.053 | 0.955× | 0.936× | — |
| Etc::CacheSimulation | 0.901 | 0.875 | 0.890 | 0.975× | 0.987× | — |
| Etc::GameOfLife | 1.349 | 1.175 | 1.302 | 0.914× | 0.858× | gcc, clang |
| Etc::LogParser | 0.460 | 1.780 | 0.520 | 3.793× | 1.108× | gcc |
| Etc::NeuralNet | 0.987 | 0.831 | 0.817 | 0.845× | 0.842× | gcc, clang |
| Etc::Sieve | 1.184 | 1.567 | 1.194 | 1.321× | 1.012× | — |
| Etc::TextRaytracer | 1.145 | 0.881 | 1.059 | 0.768× | 0.926× | — |
| Etc::Words | 1.019 | 0.972 | 1.018 | 0.949× | 1.003× | — |
| Graph::AStar | 1.298 | 1.238 | 1.020 | 0.960× | 0.780× | — |
| Graph::BFS | 1.793 | 1.319 | 1.206 | 0.768× | 0.672× | — |
| Graph::DFS | 1.417 | 1.231 | 1.255 | 0.884× | 0.884× | — |
| Hash::CRC32 | 0.257 | 0.706 | 0.802 | 2.749× | 3.121× | — |
| Hash::SHA256 | 1.199 | 1.171 | 1.184 | 0.981× | 0.990× | — |
| Json::Generate | 0.790 | 0.774 | 0.774 | 0.979× | 0.976× | — |
| Json::ParseDom | 0.931 | 0.916 | 0.894 | 0.987× | 0.949× | — |
| Json::ParseMapping | 0.891 | 0.867 | 0.853 | 0.971× | 0.948× | clang |
| Matmul::Single | 1.198 | 1.182 | 1.188 | 0.986× | 0.990× | — |
| Matmul::T16 | 2.373 | 2.316 | 2.300 | 0.984× | 0.967× | — |
| Matmul::T4 | 1.170 | 1.170 | 1.167 | 1.001× | 1.000× | — |
| Matmul::T8 | 1.225 | 1.203 | 1.204 | 0.982× | 0.978× | — |
| Maze::AStar | 1.384 | 1.194 | 1.369 | 0.878× | 1.080× | clang |
| Maze::BFS | 1.252 | 1.070 | 1.196 | 0.893× | 0.970× | gcc, clang |
| Maze::Generator | 1.381 | 1.298 | 1.300 | 0.971× | 0.907× | clang |
| Sort::Merge | 1.062 | 0.986 | 0.879 | 0.927× | 0.829× | — |
| Sort::Quick | 1.149 | 1.086 | 1.100 | 0.943× | 0.957× | — |
| Sort::Self | 1.127 | 1.040 | 1.057 | 0.924× | 0.938× | — |
| Template::Parse | 1.195 | 1.199 | 1.353 | 1.105× | 1.069× | gcc, clang |
| Template::Regex | 0.940 | 1.193 | 1.384 | 1.316× | 1.336× | gcc, clang |

A/A warnings flag variation above 2%; no cases are dropped. Small timing differences need confirmation.
Compile gcc A/A CPU ratio: 1.007×.
Compile clang A/A CPU ratio: 1.003×.

Hardware counters: unavailable: perf is not installed on this host.

## Toolchains

- **cppgm**: cppgm++ (host configuration: g++)
- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and header hashes](metadata.json) · [Raw compile observations](compile-observations.jsonl) · [Raw runtime observations](runtime-observations.jsonl)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42988175320)
