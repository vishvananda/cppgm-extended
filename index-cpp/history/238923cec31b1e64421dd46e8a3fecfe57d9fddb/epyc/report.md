# index-cpp benchmarks

Compiler checkout: `238923cec31b1e64421dd46e8a3fecfe57d9fddb` · CPU: AMD EPYC 9V74 80-Core Processor

Unchanged upstream: `79e0b423094d420da37fb714f20c53fc35eeb2a3`. All 50 checksums pass with cppgm, GCC and Clang. Full runtime inputs, paired ABBA/BAAB blocks with A/A calibration. Speeds are reference time / cppgm time: higher is better. The aggregate gives all 50 workloads equal weight and is separate from the public suite.

| Measurement | vs GCC | vs Clang |
|---|---:|---:|
| Generated code | 0.995× | 0.984× |
| Compile CPU | 6.820× | 5.552× |

| Compiler | Executable bytes | .text bytes | Compile peak RSS (KiB) |
|---|---:|---:|---:|
| cppgm | 823,072 | 385,154 | 229,096 |
| gcc | 540,112 | 280,921 | 486,020 |
| clang | 513,720 | 264,561 | 319,096 |

| Workload | cppgm seconds | GCC seconds | Clang seconds | vs GCC | vs Clang | A/A warning |
|---|---:|---:|---:|---:|---:|---|
| Base64::Decode | 1.875 | 0.961 | 0.998 | 0.515× | 0.535× | — |
| Base64::Encode | 1.422 | 1.207 | 1.004 | 0.851× | 0.709× | — |
| Binarytrees::Arena | 2.075 | 2.200 | 2.173 | 1.059× | 1.048× | — |
| Binarytrees::Obj | 2.069 | 1.905 | 2.094 | 0.922× | 1.043× | clang |
| Brainfuck::Array | 2.383 | 2.138 | 1.516 | 0.862× | 0.728× | clang |
| Brainfuck::Recursion | 1.363 | 1.178 | 1.153 | 0.907× | 0.869× | — |
| CLBG::Fannkuchredux | 0.980 | 0.979 | 0.903 | 0.998× | 0.915× | — |
| CLBG::Mandelbrot | 1.239 | 1.171 | 1.114 | 0.944× | 0.899× | — |
| CLBG::Nbody | 0.886 | 0.891 | 0.896 | 1.005× | 1.011× | — |
| CLBG::Spectralnorm | 0.954 | 0.927 | 1.577 | 0.972× | 1.652× | — |
| CSV::Parse | 0.953 | 0.825 | 0.909 | 0.870× | 0.952× | — |
| Calculator::Ast | 0.960 | 0.750 | 0.792 | 0.776× | 0.823× | — |
| Calculator::Interpreter | 1.173 | 1.107 | 1.251 | 0.908× | 1.071× | gcc |
| Compress::ArithDecode | 0.764 | 0.812 | 1.395 | 1.067× | 1.827× | — |
| Compress::ArithEncode | 0.940 | 0.855 | 0.952 | 0.910× | 1.013× | — |
| Compress::BWTDecode | 0.563 | 0.542 | 0.558 | 0.959× | 0.993× | — |
| Compress::BWTEncode | 0.911 | 0.855 | 0.995 | 0.942× | 1.094× | — |
| Compress::HuffDecode | 0.514 | 0.597 | 0.413 | 1.162× | 0.802× | — |
| Compress::HuffEncode | 0.700 | 0.773 | 0.917 | 1.105× | 1.310× | — |
| Compress::LZWDecode | 0.591 | 0.615 | 0.560 | 1.038× | 0.942× | — |
| Compress::LZWEncode | 1.003 | 0.976 | 0.996 | 0.973× | 0.995× | — |
| Distance::Jaro | 1.067 | 1.051 | 0.977 | 0.986× | 0.916× | — |
| Distance::NGram | 1.106 | 1.074 | 1.050 | 0.973× | 0.955× | — |
| Etc::CacheSimulation | 0.892 | 0.879 | 0.893 | 0.987× | 0.993× | — |
| Etc::GameOfLife | 1.306 | 1.116 | 1.071 | 0.838× | 0.843× | clang |
| Etc::LogParser | 0.460 | 1.753 | 0.520 | 3.800× | 1.139× | clang |
| Etc::NeuralNet | 1.022 | 0.830 | 0.835 | 0.815× | 0.804× | gcc, clang |
| Etc::Sieve | 1.181 | 1.562 | 1.190 | 1.325× | 1.006× | — |
| Etc::TextRaytracer | 0.923 | 0.881 | 1.059 | 0.957× | 1.147× | — |
| Etc::Words | 1.015 | 0.967 | 1.010 | 0.960× | 0.994× | — |
| Graph::AStar | 1.245 | 1.239 | 1.026 | 0.996× | 0.828× | — |
| Graph::BFS | 1.946 | 1.339 | 1.214 | 0.680× | 0.647× | clang |
| Graph::DFS | 1.430 | 1.225 | 1.256 | 0.857× | 0.867× | — |
| Hash::CRC32 | 0.257 | 0.706 | 0.802 | 2.747× | 3.119× | — |
| Hash::SHA256 | 1.198 | 1.168 | 1.176 | 0.973× | 0.982× | — |
| Json::Generate | 0.780 | 0.770 | 0.764 | 0.983× | 0.979× | — |
| Json::ParseDom | 0.935 | 0.920 | 0.893 | 0.975× | 0.952× | — |
| Json::ParseMapping | 0.893 | 0.871 | 0.849 | 0.972× | 0.952× | clang |
| Matmul::Single | 1.199 | 1.183 | 1.187 | 0.987× | 0.991× | — |
| Matmul::T16 | 2.349 | 2.307 | 2.287 | 0.978× | 0.983× | — |
| Matmul::T4 | 1.168 | 1.168 | 1.163 | 1.004× | 0.996× | — |
| Matmul::T8 | 1.214 | 1.199 | 1.195 | 0.993× | 0.980× | — |
| Maze::AStar | 1.361 | 1.256 | 1.359 | 0.888× | 1.008× | clang |
| Maze::BFS | 1.200 | 1.194 | 1.130 | 0.997× | 0.942× | gcc, clang |
| Maze::Generator | 1.363 | 1.341 | 1.327 | 0.976× | 0.973× | — |
| Sort::Merge | 0.998 | 0.988 | 0.879 | 0.990× | 0.881× | — |
| Sort::Quick | 1.173 | 1.090 | 1.100 | 0.928× | 0.938× | — |
| Sort::Self | 1.143 | 1.040 | 1.056 | 0.909× | 0.923× | — |
| Template::Parse | 1.156 | 1.198 | 1.108 | 0.981× | 0.926× | gcc, clang |
| Template::Regex | 0.948 | 1.188 | 1.233 | 1.227× | 1.246× | clang |

A/A warnings flag variation above 2%; no cases are dropped. Small timing differences need confirmation.
Compile gcc A/A CPU ratio: 0.987×.
Compile clang A/A CPU ratio: 1.014×.

Hardware counters: unavailable: perf is not installed on this host.

## Toolchains

- **cppgm**: cppgm++ (host configuration: g++)
- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and header hashes](metadata.json) · [Raw compile observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42562778935) · [Raw runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42562778935)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42562778935)
