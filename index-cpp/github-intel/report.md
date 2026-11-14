# index-cpp benchmarks

Compiler checkout: `9ed2f1eea641727dd56efede0b4ca09c045e9f7f` · CPU: Intel(R) Xeon(R) 6975P-C

Unchanged upstream: `79e0b423094d420da37fb714f20c53fc35eeb2a3`. All 50 checksums pass with cppgm, GCC and Clang. Full runtime inputs, paired ABBA/BAAB blocks with A/A calibration. Speeds are reference time / cppgm time: higher is better. The aggregate gives all 50 workloads equal weight and is separate from the public suite.

| Measurement | vs GCC | vs Clang |
|---|---:|---:|
| Generated code | 1.072× | 1.051× |
| Compile CPU | 6.143× | 4.971× |

| Compiler | Executable bytes | .text bytes | Compile peak RSS (KiB) |
|---|---:|---:|---:|
| cppgm | 714,480 | 340,002 | 228,612 |
| gcc | 540,112 | 280,921 | 486,096 |
| clang | 513,720 | 264,561 | 319,044 |

| Workload | cppgm seconds | GCC seconds | Clang seconds | vs GCC | vs Clang | A/A warning |
|---|---:|---:|---:|---:|---:|---|
| Base64::Decode | 0.833 | 0.730 | 0.833 | 0.875× | 0.999× | — |
| Base64::Encode | 0.842 | 0.998 | 0.786 | 1.183× | 0.933× | — |
| Binarytrees::Arena | 1.974 | 2.115 | 1.968 | 1.055× | 1.027× | gcc |
| Binarytrees::Obj | 0.913 | 0.865 | 1.097 | 0.937× | 1.231× | clang |
| Brainfuck::Array | 1.907 | 1.651 | 1.702 | 0.865× | 0.890× | — |
| Brainfuck::Recursion | 1.145 | 1.195 | 1.419 | 1.045× | 1.244× | — |
| CLBG::Fannkuchredux | 0.899 | 0.961 | 0.830 | 1.069× | 0.924× | — |
| CLBG::Mandelbrot | 1.081 | 0.977 | 0.970 | 0.904× | 0.898× | — |
| CLBG::Nbody | 0.659 | 0.622 | 0.634 | 0.946× | 0.962× | — |
| CLBG::Spectralnorm | 0.982 | 0.936 | 1.315 | 0.954× | 1.340× | — |
| CSV::Parse | 0.839 | 0.790 | 0.927 | 0.941× | 1.108× | — |
| Calculator::Ast | 0.840 | 0.829 | 0.754 | 0.959× | 0.919× | clang |
| Calculator::Interpreter | 1.141 | 1.241 | 1.155 | 1.035× | 1.026× | gcc |
| Compress::ArithDecode | 0.880 | 1.011 | 1.489 | 1.147× | 1.692× | — |
| Compress::ArithEncode | 1.008 | 0.996 | 1.073 | 0.986× | 1.065× | — |
| Compress::BWTDecode | 1.301 | 1.300 | 1.302 | 0.999× | 1.002× | — |
| Compress::BWTEncode | 0.741 | 0.776 | 0.919 | 1.047× | 1.242× | — |
| Compress::HuffDecode | 0.496 | 0.721 | 0.387 | 1.444× | 0.779× | clang |
| Compress::HuffEncode | 0.782 | 0.926 | 0.825 | 1.178× | 1.057× | — |
| Compress::LZWDecode | 0.567 | 0.569 | 0.548 | 1.000× | 0.972× | — |
| Compress::LZWEncode | 0.960 | 0.948 | 0.953 | 0.984× | 0.992× | — |
| Distance::Jaro | 1.157 | 1.188 | 1.204 | 1.028× | 1.041× | — |
| Distance::NGram | 1.166 | 1.185 | 1.123 | 1.016× | 0.963× | — |
| Etc::CacheSimulation | 0.809 | 0.783 | 0.804 | 0.971× | 0.994× | — |
| Etc::GameOfLife | 1.094 | 1.107 | 1.094 | 1.021× | 0.998× | — |
| Etc::LogParser | 0.420 | 1.179 | 0.439 | 2.806× | 1.050× | clang |
| Etc::NeuralNet | 0.782 | 0.723 | 0.708 | 0.926× | 0.906× | — |
| Etc::Sieve | 2.418 | 3.107 | 2.402 | 1.283× | 0.995× | — |
| Etc::TextRaytracer | 0.915 | 0.633 | 0.827 | 0.691× | 0.904× | — |
| Etc::Words | 0.929 | 0.915 | 0.915 | 0.985× | 0.978× | — |
| Graph::AStar | 1.157 | 1.219 | 0.863 | 1.040× | 0.820× | gcc |
| Graph::BFS | 0.793 | 0.877 | 0.682 | 0.981× | 0.899× | — |
| Graph::DFS | 1.034 | 0.997 | 0.916 | 0.950× | 0.928× | gcc, clang |
| Hash::CRC32 | 0.244 | 0.715 | 1.003 | 2.930× | 4.113× | — |
| Hash::SHA256 | 0.353 | 0.913 | 0.778 | 2.583× | 2.217× | — |
| Json::Generate | 0.853 | 0.855 | 0.851 | 1.002× | 0.999× | — |
| Json::ParseDom | 1.014 | 0.980 | 0.968 | 0.968× | 0.953× | — |
| Json::ParseMapping | 0.965 | 0.931 | 0.921 | 0.966× | 0.955× | — |
| Matmul::Single | 0.760 | 0.760 | 0.762 | 1.001× | 0.997× | — |
| Matmul::T16 | 1.647 | 1.651 | 1.637 | 0.997× | 0.998× | gcc |
| Matmul::T4 | 0.777 | 0.778 | 0.773 | 0.999× | 0.996× | — |
| Matmul::T8 | 0.837 | 0.839 | 0.837 | 0.996× | 1.013× | — |
| Maze::AStar | 0.974 | 1.172 | 0.847 | 0.941× | 0.880× | gcc |
| Maze::BFS | 0.724 | 0.727 | 0.752 | 0.998× | 1.046× | — |
| Maze::Generator | 1.185 | 1.776 | 1.183 | 1.410× | 1.096× | gcc, clang |
| Sort::Merge | 0.975 | 1.053 | 0.847 | 1.080× | 0.868× | — |
| Sort::Quick | 1.110 | 1.143 | 1.122 | 1.029× | 1.011× | — |
| Sort::Self | 1.152 | 1.083 | 1.099 | 0.940× | 0.954× | — |
| Template::Parse | 1.107 | 1.057 | 1.093 | 0.957× | 0.987× | — |
| Template::Regex | 0.954 | 0.954 | 1.095 | 0.999× | 1.154× | — |

A/A warnings flag variation above 2%; no cases are dropped. Small timing differences need confirmation.
Compile gcc A/A CPU ratio: 0.993×.
Compile clang A/A CPU ratio: 0.994×.

Hardware counters: unavailable: perf is not installed on this host.

## Toolchains

- **cppgm**: cppgm++ (host configuration: g++)
- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and header hashes](metadata.json) · [Raw compile observations](compile-observations.jsonl) · [Raw runtime observations](runtime-observations.jsonl)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43601267096)
