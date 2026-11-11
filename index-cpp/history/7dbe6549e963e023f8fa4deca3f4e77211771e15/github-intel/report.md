# index-cpp benchmarks

Compiler checkout: `7dbe6549e963e023f8fa4deca3f4e77211771e15` · CPU: Intel(R) Xeon(R) Platinum 8370C CPU @ 2.80GHz

Unchanged upstream: `79e0b423094d420da37fb714f20c53fc35eeb2a3`. All 50 checksums pass with cppgm, GCC and Clang. Full runtime inputs, paired ABBA/BAAB blocks with A/A calibration. Speeds are reference time / cppgm time: higher is better. The aggregate gives all 50 workloads equal weight and is separate from the public suite.

| Measurement | vs GCC | vs Clang |
|---|---:|---:|
| Generated code | 1.030× | 1.013× |
| Compile CPU | 6.698× | 5.674× |

| Compiler | Executable bytes | .text bytes | Compile peak RSS (KiB) |
|---|---:|---:|---:|
| cppgm | 822,840 | 386,370 | 224,698 |
| gcc | 540,112 | 280,921 | 486,032 |
| clang | 513,720 | 264,561 | 318,984 |

| Workload | cppgm seconds | GCC seconds | Clang seconds | vs GCC | vs Clang | A/A warning |
|---|---:|---:|---:|---:|---:|---|
| Base64::Decode | 1.679 | 1.046 | 1.161 | 0.621× | 0.690× | — |
| Base64::Encode | 1.582 | 1.512 | 1.210 | 0.959× | 0.767× | — |
| Binarytrees::Arena | 2.180 | 2.262 | 2.303 | 1.044× | 1.056× | — |
| Binarytrees::Obj | 1.656 | 1.514 | 1.714 | 0.918× | 1.033× | — |
| Brainfuck::Array | 1.699 | 2.074 | 1.978 | 1.218× | 1.165× | — |
| Brainfuck::Recursion | 1.365 | 1.609 | 1.359 | 1.178× | 0.995× | — |
| CLBG::Fannkuchredux | 0.958 | 1.088 | 0.946 | 1.144× | 0.982× | gcc, clang |
| CLBG::Mandelbrot | 1.690 | 1.643 | 1.655 | 0.972× | 0.979× | — |
| CLBG::Nbody | 0.920 | 0.882 | 0.903 | 0.959× | 0.983× | — |
| CLBG::Spectralnorm | 1.282 | 1.434 | 1.856 | 1.102× | 1.443× | — |
| CSV::Parse | 1.088 | 1.011 | 1.094 | 0.931× | 1.008× | — |
| Calculator::Ast | 1.413 | 1.087 | 1.092 | 0.773× | 0.770× | — |
| Calculator::Interpreter | 1.778 | 1.620 | 1.745 | 0.919× | 0.975× | clang |
| Compress::ArithDecode | 1.049 | 1.138 | 1.640 | 1.084× | 1.564× | — |
| Compress::ArithEncode | 1.181 | 1.191 | 1.251 | 1.008× | 1.059× | — |
| Compress::BWTDecode | 1.320 | 1.345 | 1.413 | 1.020× | 1.054× | — |
| Compress::BWTEncode | 1.095 | 1.069 | 1.188 | 0.975× | 1.083× | — |
| Compress::HuffDecode | 0.744 | 0.893 | 0.506 | 1.204× | 0.676× | — |
| Compress::HuffEncode | 0.776 | 1.002 | 1.083 | 1.286× | 1.379× | — |
| Compress::LZWDecode | 0.742 | 0.755 | 0.716 | 1.015× | 0.965× | — |
| Compress::LZWEncode | 1.123 | 1.104 | 1.128 | 0.981× | 1.007× | — |
| Distance::Jaro | 1.370 | 1.289 | 1.291 | 0.941× | 0.941× | — |
| Distance::NGram | 1.456 | 1.412 | 1.362 | 0.971× | 0.936× | — |
| Etc::CacheSimulation | 1.029 | 0.984 | 1.035 | 0.955× | 1.004× | — |
| Etc::GameOfLife | 1.822 | 1.590 | 1.603 | 0.888× | 0.856× | gcc |
| Etc::LogParser | 0.602 | 1.353 | 0.577 | 2.293× | 0.957× | — |
| Etc::NeuralNet | 1.208 | 1.036 | 1.041 | 0.855× | 0.861× | gcc |
| Etc::Sieve | 2.747 | 3.486 | 2.731 | 1.303× | 1.018× | gcc, clang |
| Etc::TextRaytracer | 1.252 | 0.994 | 1.204 | 0.795× | 0.961× | — |
| Etc::Words | 1.188 | 1.151 | 1.204 | 0.968× | 1.009× | — |
| Graph::AStar | 1.135 | 1.148 | 1.016 | 1.014× | 0.893× | — |
| Graph::BFS | 0.952 | 0.911 | 0.897 | 0.959× | 0.948× | — |
| Graph::DFS | 0.994 | 1.024 | 1.028 | 1.033× | 1.035× | — |
| Hash::CRC32 | 0.281 | 0.808 | 0.958 | 2.874× | 3.407× | gcc |
| Hash::SHA256 | 1.264 | 1.355 | 1.275 | 1.066× | 1.009× | — |
| Json::Generate | 1.129 | 1.131 | 1.126 | 1.003× | 0.999× | — |
| Json::ParseDom | 1.424 | 1.288 | 1.299 | 0.905× | 0.906× | — |
| Json::ParseMapping | 1.361 | 1.224 | 1.228 | 0.904× | 0.900× | — |
| Matmul::Single | 1.663 | 1.652 | 1.641 | 0.995× | 0.987× | — |
| Matmul::T16 | 3.232 | 3.183 | 3.224 | 0.987× | 1.005× | — |
| Matmul::T4 | 1.643 | 1.638 | 1.641 | 0.998× | 0.997× | — |
| Matmul::T8 | 1.683 | 1.677 | 1.683 | 0.997× | 0.998× | — |
| Maze::AStar | 1.509 | 1.457 | 1.605 | 0.975× | 1.061× | — |
| Maze::BFS | 1.253 | 1.240 | 1.281 | 0.990× | 1.014× | — |
| Maze::Generator | 1.438 | 1.393 | 1.376 | 0.969× | 0.962× | — |
| Sort::Merge | 1.134 | 1.198 | 0.981 | 1.056× | 0.864× | — |
| Sort::Quick | 1.326 | 1.325 | 1.312 | 0.999× | 0.990× | — |
| Sort::Self | 1.365 | 1.291 | 1.287 | 0.947× | 0.942× | — |
| Template::Parse | 1.774 | 1.803 | 1.913 | 1.020× | 1.024× | gcc, clang |
| Template::Regex | 1.328 | 1.479 | 1.696 | 1.147× | 1.261× | gcc |

A/A warnings flag variation above 2%; no cases are dropped. Small timing differences need confirmation.
Compile gcc A/A CPU ratio: 0.994×.
Compile clang A/A CPU ratio: 0.980×.

Hardware counters: unavailable: perf is not installed on this host.

## Toolchains

- **cppgm**: cppgm++ (host configuration: g++)
- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and header hashes](metadata.json) · [Raw compile observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42416405140) · [Raw runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42416405140)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42416405140)
