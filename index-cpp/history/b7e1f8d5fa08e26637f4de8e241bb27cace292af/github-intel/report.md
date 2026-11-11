# index-cpp benchmarks

Compiler checkout: `b7e1f8d5fa08e26637f4de8e241bb27cace292af` · CPU: Intel(R) Xeon(R) Platinum 8370C CPU @ 2.80GHz

Unchanged upstream: `79e0b423094d420da37fb714f20c53fc35eeb2a3`. All 50 checksums pass with cppgm, GCC and Clang. Full runtime inputs, paired ABBA/BAAB blocks with A/A calibration. Speeds are reference time / cppgm time: higher is better. The aggregate gives all 50 workloads equal weight and is separate from the public suite.

| Measurement | vs GCC | vs Clang |
|---|---:|---:|
| Generated code | 1.020× | 1.005× |
| Compile CPU | 6.544× | 5.668× |

| Compiler | Executable bytes | .text bytes | Compile peak RSS (KiB) |
|---|---:|---:|---:|
| cppgm | 822,840 | 385,282 | 227,528 |
| gcc | 540,112 | 280,921 | 486,052 |
| clang | 513,720 | 264,561 | 319,050 |

| Workload | cppgm seconds | GCC seconds | Clang seconds | vs GCC | vs Clang | A/A warning |
|---|---:|---:|---:|---:|---:|---|
| Base64::Decode | 1.677 | 1.043 | 1.159 | 0.622× | 0.693× | — |
| Base64::Encode | 1.583 | 1.501 | 1.211 | 0.945× | 0.759× | — |
| Binarytrees::Arena | 2.187 | 2.253 | 2.320 | 1.035× | 1.070× | clang |
| Binarytrees::Obj | 1.652 | 1.512 | 1.707 | 0.925× | 1.030× | — |
| Brainfuck::Array | 1.700 | 2.075 | 1.979 | 1.221× | 1.164× | — |
| Brainfuck::Recursion | 1.368 | 1.606 | 1.360 | 1.168× | 0.993× | — |
| CLBG::Fannkuchredux | 0.958 | 1.087 | 0.952 | 1.134× | 0.985× | clang |
| CLBG::Mandelbrot | 1.690 | 1.644 | 1.655 | 0.973× | 0.979× | — |
| CLBG::Nbody | 0.918 | 0.884 | 0.903 | 0.962× | 0.985× | — |
| CLBG::Spectralnorm | 1.284 | 1.354 | 1.812 | 1.051× | 1.409× | — |
| CSV::Parse | 1.090 | 1.014 | 1.091 | 0.936× | 1.003× | — |
| Calculator::Ast | 1.421 | 1.080 | 1.087 | 0.762× | 0.762× | — |
| Calculator::Interpreter | 1.901 | 1.711 | 1.774 | 0.915× | 0.918× | gcc, clang |
| Compress::ArithDecode | 1.044 | 1.138 | 1.643 | 1.090× | 1.574× | — |
| Compress::ArithEncode | 1.172 | 1.192 | 1.258 | 1.017× | 1.073× | — |
| Compress::BWTDecode | 1.421 | 1.366 | 1.414 | 1.001× | 0.997× | — |
| Compress::BWTEncode | 1.101 | 1.064 | 1.179 | 0.959× | 1.072× | — |
| Compress::HuffDecode | 0.718 | 0.915 | 0.504 | 1.270× | 0.700× | gcc |
| Compress::HuffEncode | 0.909 | 0.997 | 1.082 | 1.101× | 1.186× | — |
| Compress::LZWDecode | 0.742 | 0.746 | 0.710 | 0.996× | 0.958× | — |
| Compress::LZWEncode | 1.120 | 1.105 | 1.128 | 0.986× | 1.013× | — |
| Distance::Jaro | 1.372 | 1.289 | 1.290 | 0.939× | 0.943× | — |
| Distance::NGram | 1.467 | 1.414 | 1.363 | 0.962× | 0.929× | — |
| Etc::CacheSimulation | 1.036 | 0.986 | 1.039 | 0.945× | 1.000× | — |
| Etc::GameOfLife | 2.112 | 1.777 | 1.766 | 0.821× | 0.874× | clang |
| Etc::LogParser | 0.603 | 1.348 | 0.575 | 2.253× | 0.953× | gcc |
| Etc::NeuralNet | 1.206 | 1.036 | 1.043 | 0.853× | 0.869× | — |
| Etc::Sieve | 2.641 | 3.312 | 2.550 | 1.236× | 1.006× | gcc, clang |
| Etc::TextRaytracer | 1.254 | 0.994 | 1.206 | 0.792× | 0.961× | — |
| Etc::Words | 1.193 | 1.144 | 1.204 | 0.951× | 1.009× | — |
| Graph::AStar | 1.141 | 1.147 | 1.015 | 1.007× | 0.887× | — |
| Graph::BFS | 0.952 | 0.911 | 0.896 | 0.956× | 0.941× | — |
| Graph::DFS | 0.996 | 1.025 | 1.027 | 1.029× | 1.031× | clang |
| Hash::CRC32 | 0.281 | 0.840 | 0.958 | 2.961× | 3.407× | gcc |
| Hash::SHA256 | 1.264 | 1.356 | 1.277 | 1.074× | 1.015× | — |
| Json::Generate | 1.145 | 1.137 | 1.138 | 1.001× | 0.990× | — |
| Json::ParseDom | 1.454 | 1.304 | 1.317 | 0.894× | 0.906× | — |
| Json::ParseMapping | 1.379 | 1.239 | 1.257 | 0.902× | 0.909× | clang |
| Matmul::Single | 1.663 | 1.659 | 1.643 | 0.998× | 0.984× | — |
| Matmul::T16 | 3.241 | 3.240 | 3.236 | 0.997× | 1.002× | — |
| Matmul::T4 | 1.647 | 1.639 | 1.640 | 0.996× | 0.995× | — |
| Matmul::T8 | 1.688 | 1.675 | 1.682 | 0.998× | 0.998× | — |
| Maze::AStar | 1.528 | 1.485 | 1.605 | 0.967× | 1.051× | gcc |
| Maze::BFS | 1.292 | 1.268 | 1.295 | 0.979× | 1.005× | gcc |
| Maze::Generator | 1.466 | 1.406 | 1.378 | 0.962× | 0.952× | gcc, clang |
| Sort::Merge | 1.136 | 1.200 | 0.982 | 1.056× | 0.864× | — |
| Sort::Quick | 1.323 | 1.326 | 1.313 | 1.001× | 0.992× | — |
| Sort::Self | 1.365 | 1.293 | 1.288 | 0.947× | 0.944× | — |
| Template::Parse | 2.061 | 2.072 | 2.040 | 1.020× | 0.983× | clang |
| Template::Regex | 1.435 | 1.556 | 1.734 | 1.092× | 1.213× | — |

A/A warnings flag variation above 2%; no cases are dropped. Small timing differences need confirmation.
Compile gcc A/A CPU ratio: 1.006×.
Compile clang A/A CPU ratio: 0.993×.

Hardware counters: unavailable: perf is not installed on this host.

## Toolchains

- **cppgm**: cppgm++ (host configuration: g++)
- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and header hashes](metadata.json) · [Raw compile observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42557497539) · [Raw runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42557497539)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42557497539)
