# index-cpp benchmarks

Compiler checkout: `01bac61953a982df28933dfde0fa03eb65838ea4` · CPU: Intel(R) Xeon(R) 6975P-C

Unchanged upstream: `79e0b423094d420da37fb714f20c53fc35eeb2a3`. All 50 checksums pass with cppgm, GCC and Clang. Full runtime inputs, paired ABBA/BAAB blocks with A/A calibration. Speeds are reference time / cppgm time: higher is better. The aggregate gives all 50 workloads equal weight and is separate from the public suite.

| Measurement | vs GCC | vs Clang |
|---|---:|---:|
| Generated code | 1.053× | 1.063× |
| Compile CPU | 6.201× | 4.985× |

| Compiler | Executable bytes | .text bytes | Compile peak RSS (KiB) |
|---|---:|---:|---:|
| cppgm | 710,384 | 336,434 | 228,438 |
| gcc | 540,112 | 280,921 | 486,062 |
| clang | 513,720 | 264,561 | 319,674 |

| Workload | cppgm seconds | GCC seconds | Clang seconds | vs GCC | vs Clang | A/A warning |
|---|---:|---:|---:|---:|---:|---|
| Base64::Decode | 0.869 | 0.745 | 0.873 | 0.850× | 1.013× | — |
| Base64::Encode | 0.972 | 1.016 | 0.804 | 1.043× | 0.827× | — |
| Binarytrees::Arena | 2.093 | 2.212 | 2.237 | 1.065× | 1.072× | gcc |
| Binarytrees::Obj | 1.017 | 0.948 | 1.242 | 0.920× | 1.218× | gcc |
| Brainfuck::Array | 1.635 | 1.663 | 1.719 | 1.022× | 1.051× | — |
| Brainfuck::Recursion | 1.292 | 1.220 | 1.429 | 0.944× | 1.126× | gcc |
| CLBG::Fannkuchredux | 0.904 | 0.968 | 0.838 | 1.068× | 0.929× | — |
| CLBG::Mandelbrot | 1.067 | 0.985 | 0.979 | 0.924× | 0.918× | — |
| CLBG::Nbody | 0.663 | 0.628 | 0.640 | 0.947× | 0.964× | — |
| CLBG::Spectralnorm | 0.983 | 0.944 | 1.324 | 0.961× | 1.350× | — |
| CSV::Parse | 0.919 | 0.800 | 0.936 | 0.872× | 1.016× | — |
| Calculator::Ast | 0.906 | 0.849 | 0.869 | 0.941× | 0.956× | — |
| Calculator::Interpreter | 1.494 | 1.361 | 1.532 | 0.916× | 1.055× | gcc |
| Compress::ArithDecode | 0.865 | 1.020 | 1.502 | 1.177× | 1.735× | — |
| Compress::ArithEncode | 1.035 | 1.005 | 1.083 | 0.970× | 1.045× | — |
| Compress::BWTDecode | 1.313 | 1.310 | 1.316 | 0.999× | 1.002× | — |
| Compress::BWTEncode | 0.754 | 0.782 | 0.929 | 1.037× | 1.232× | — |
| Compress::HuffDecode | 0.800 | 0.730 | 0.390 | 0.912× | 0.489× | clang |
| Compress::HuffEncode | 0.816 | 0.935 | 0.832 | 1.137× | 1.020× | — |
| Compress::LZWDecode | 0.571 | 0.574 | 0.559 | 1.006× | 0.985× | — |
| Compress::LZWEncode | 0.964 | 0.956 | 0.962 | 0.990× | 0.998× | — |
| Distance::Jaro | 1.176 | 1.196 | 1.210 | 1.017× | 1.032× | — |
| Distance::NGram | 1.174 | 1.195 | 1.131 | 1.019× | 0.968× | — |
| Etc::CacheSimulation | 0.820 | 0.790 | 0.806 | 0.966× | 0.982× | — |
| Etc::GameOfLife | 1.123 | 1.142 | 1.107 | 1.024× | 0.989× | — |
| Etc::LogParser | 0.474 | 1.188 | 0.444 | 2.506× | 0.931× | gcc, clang |
| Etc::NeuralNet | 0.786 | 0.732 | 0.714 | 0.934× | 0.906× | — |
| Etc::Sieve | 2.458 | 3.152 | 2.439 | 1.288× | 0.987× | — |
| Etc::TextRaytracer | 0.922 | 0.637 | 0.834 | 0.691× | 0.905× | — |
| Etc::Words | 0.929 | 0.909 | 0.916 | 0.983× | 0.985× | — |
| Graph::AStar | 1.197 | 1.248 | 1.091 | 1.050× | 0.916× | — |
| Graph::BFS | 0.986 | 0.962 | 0.950 | 0.970× | 0.969× | gcc |
| Graph::DFS | 1.118 | 1.026 | 1.038 | 0.919× | 0.938× | — |
| Hash::CRC32 | 0.246 | 0.721 | 1.012 | 2.931× | 4.118× | — |
| Hash::SHA256 | 0.337 | 0.922 | 0.786 | 2.741× | 2.337× | — |
| Json::Generate | 0.865 | 0.862 | 0.861 | 0.997× | 0.993× | — |
| Json::ParseDom | 1.027 | 0.989 | 0.982 | 0.964× | 0.967× | — |
| Json::ParseMapping | 0.976 | 0.941 | 0.936 | 0.964× | 0.971× | — |
| Matmul::Single | 0.766 | 0.754 | 0.767 | 0.984× | 0.998× | — |
| Matmul::T16 | 1.632 | 1.636 | 1.636 | 1.006× | 1.000× | — |
| Matmul::T4 | 0.784 | 0.785 | 0.782 | 1.006× | 0.993× | — |
| Matmul::T8 | 0.843 | 0.842 | 0.838 | 0.998× | 1.001× | — |
| Maze::AStar | 1.576 | 1.602 | 1.668 | 1.011× | 1.114× | gcc, clang |
| Maze::BFS | 0.935 | 0.998 | 1.151 | 0.984× | 1.488× | gcc, clang |
| Maze::Generator | 1.515 | 1.927 | 1.906 | 1.251× | 1.276× | clang |
| Sort::Merge | 0.978 | 1.065 | 0.857 | 1.091× | 0.873× | — |
| Sort::Quick | 1.110 | 1.158 | 1.134 | 1.047× | 1.016× | — |
| Sort::Self | 1.163 | 1.094 | 1.110 | 0.944× | 0.953× | — |
| Template::Parse | 1.309 | 1.296 | 1.356 | 0.954× | 1.081× | gcc, clang |
| Template::Regex | 0.962 | 1.050 | 1.214 | 1.042× | 1.246× | gcc, clang |

A/A warnings flag variation above 2%; no cases are dropped. Small timing differences need confirmation.
Compile gcc A/A CPU ratio: 1.014×.
Compile clang A/A CPU ratio: 1.010×.

Hardware counters: unavailable: perf is not installed on this host.

## Toolchains

- **cppgm**: cppgm++ (host configuration: g++)
- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and header hashes](metadata.json) · [Raw compile observations](compile-observations.jsonl) · [Raw runtime observations](runtime-observations.jsonl)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43559814525)
