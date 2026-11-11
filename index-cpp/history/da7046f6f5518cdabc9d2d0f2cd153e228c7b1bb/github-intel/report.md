# index-cpp benchmarks

Compiler checkout: `da7046f6f5518cdabc9d2d0f2cd153e228c7b1bb` · CPU: INTEL(R) XEON(R) PLATINUM 8573C

Unchanged upstream: `79e0b423094d420da37fb714f20c53fc35eeb2a3`. All 50 checksums pass with cppgm, GCC and Clang. Full runtime inputs, paired ABBA/BAAB blocks with A/A calibration. Speeds are reference time / cppgm time: higher is better. The aggregate gives all 50 workloads equal weight and is separate from the public suite.

| Measurement | vs GCC | vs Clang |
|---|---:|---:|
| Generated code | 1.027× | 1.006× |
| Compile CPU | 6.219× | 5.076× |

| Compiler | Executable bytes | .text bytes | Compile peak RSS (KiB) |
|---|---:|---:|---:|
| cppgm | 798,176 | 388,834 | 229,106 |
| gcc | 540,112 | 280,921 | 485,998 |
| clang | 513,720 | 264,561 | 318,970 |

| Workload | cppgm seconds | GCC seconds | Clang seconds | vs GCC | vs Clang | A/A warning |
|---|---:|---:|---:|---:|---:|---|
| Base64::Decode | 1.129 | 0.800 | 0.891 | 0.711× | 0.788× | clang |
| Base64::Encode | 1.146 | 1.052 | 0.863 | 0.918× | 0.751× | — |
| Binarytrees::Arena | 1.859 | 1.796 | 1.984 | 0.961× | 1.083× | — |
| Binarytrees::Obj | 1.051 | 0.873 | 1.130 | 0.817× | 1.068× | gcc, clang |
| Brainfuck::Array | 1.695 | 1.985 | 1.850 | 1.171× | 1.092× | — |
| Brainfuck::Recursion | 1.129 | 1.312 | 1.249 | 1.164× | 1.105× | — |
| CLBG::Fannkuchredux | 1.039 | 1.095 | 0.933 | 1.055× | 0.909× | clang |
| CLBG::Mandelbrot | 1.260 | 1.146 | 1.119 | 0.894× | 0.889× | — |
| CLBG::Nbody | 0.719 | 0.702 | 0.716 | 0.978× | 0.996× | — |
| CLBG::Spectralnorm | 1.058 | 1.012 | 1.417 | 0.956× | 1.340× | — |
| CSV::Parse | 0.956 | 0.888 | 0.991 | 0.929× | 1.037× | — |
| Calculator::Ast | 1.030 | 0.867 | 0.865 | 0.842× | 0.845× | clang |
| Calculator::Interpreter | 1.206 | 1.095 | 1.131 | 0.905× | 0.938× | clang |
| Compress::ArithDecode | 0.948 | 1.095 | 1.618 | 1.153× | 1.704× | — |
| Compress::ArithEncode | 1.144 | 1.095 | 1.178 | 0.953× | 1.030× | — |
| Compress::BWTDecode | 1.393 | 1.382 | 1.390 | 0.992× | 0.999× | — |
| Compress::BWTEncode | 0.891 | 0.851 | 0.998 | 0.949× | 1.123× | — |
| Compress::HuffDecode | 0.721 | 0.794 | 0.420 | 1.104× | 0.585× | gcc |
| Compress::HuffEncode | 0.792 | 1.091 | 0.891 | 1.378× | 1.130× | — |
| Compress::LZWDecode | 0.641 | 0.653 | 0.624 | 1.001× | 0.979× | — |
| Compress::LZWEncode | 1.044 | 1.038 | 1.039 | 1.006× | 0.994× | — |
| Distance::Jaro | 1.429 | 1.334 | 1.453 | 0.935× | 1.014× | — |
| Distance::NGram | 1.341 | 1.317 | 1.240 | 0.983× | 0.927× | — |
| Etc::CacheSimulation | 0.904 | 0.872 | 0.895 | 0.967× | 0.983× | clang |
| Etc::GameOfLife | 1.260 | 1.151 | 1.113 | 0.917× | 0.888× | — |
| Etc::LogParser | 0.470 | 1.296 | 0.470 | 2.732× | 0.999× | gcc, clang |
| Etc::NeuralNet | 0.868 | 0.794 | 0.780 | 0.918× | 0.900× | — |
| Etc::Sieve | 2.476 | 3.131 | 2.316 | 1.250× | 0.936× | — |
| Etc::TextRaytracer | 0.807 | 0.707 | 0.917 | 0.877× | 1.134× | — |
| Etc::Words | 1.013 | 0.968 | 0.986 | 0.955× | 0.976× | — |
| Graph::AStar | 1.060 | 1.066 | 0.841 | 1.005× | 0.827× | — |
| Graph::BFS | 0.697 | 0.649 | 0.631 | 0.918× | 0.904× | clang |
| Graph::DFS | 0.898 | 0.867 | 0.879 | 0.959× | 1.012× | gcc, clang |
| Hash::CRC32 | 0.266 | 0.770 | 1.077 | 2.889× | 4.053× | — |
| Hash::SHA256 | 0.988 | 1.114 | 0.929 | 1.131× | 0.940× | — |
| Json::Generate | 0.947 | 0.957 | 0.954 | 1.021× | 1.012× | — |
| Json::ParseDom | 1.167 | 1.111 | 1.097 | 0.952× | 0.934× | — |
| Json::ParseMapping | 1.111 | 1.056 | 1.046 | 0.951× | 0.938× | — |
| Matmul::Single | 0.794 | 0.796 | 0.783 | 1.003× | 0.984× | clang |
| Matmul::T16 | 1.626 | 1.629 | 1.601 | 1.002× | 0.976× | gcc |
| Matmul::T4 | 0.807 | 0.806 | 0.806 | 0.998× | 0.996× | — |
| Matmul::T8 | 0.852 | 0.841 | 0.841 | 0.987× | 0.984× | — |
| Maze::AStar | 0.908 | 0.874 | 0.844 | 0.954× | 0.930× | — |
| Maze::BFS | 0.732 | 0.736 | 0.736 | 1.007× | 1.008× | — |
| Maze::Generator | 0.971 | 1.231 | 1.000 | 1.202× | 1.058× | gcc, clang |
| Sort::Merge | 1.245 | 1.171 | 0.913 | 0.945× | 0.734× | — |
| Sort::Quick | 1.300 | 1.268 | 1.236 | 0.975× | 0.951× | — |
| Sort::Self | 1.302 | 1.192 | 1.215 | 0.917× | 0.929× | — |
| Template::Parse | 1.078 | 1.093 | 1.089 | 0.967× | 1.025× | gcc, clang |
| Template::Regex | 0.983 | 1.065 | 1.184 | 1.047× | 1.220× | clang |

A/A warnings flag variation above 2%; no cases are dropped. Small timing differences need confirmation.
Compile gcc A/A CPU ratio: 1.007×.
Compile clang A/A CPU ratio: 1.001×.

Hardware counters: unavailable: perf is not installed on this host.

## Toolchains

- **cppgm**: cppgm++ (host configuration: g++)
- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and header hashes](metadata.json) · [Raw compile observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893648) · [Raw runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893648)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893648)
