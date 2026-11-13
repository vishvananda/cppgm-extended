# index-cpp benchmarks

Compiler checkout: `72d294c916f0f3f43e50e812f7efa91937eb9b66` · CPU: AMD EPYC 9V45 96-Core Processor

Unchanged upstream: `79e0b423094d420da37fb714f20c53fc35eeb2a3`. All 50 checksums pass with cppgm, GCC and Clang. Full runtime inputs, paired ABBA/BAAB blocks with A/A calibration. Speeds are reference time / cppgm time: higher is better. The aggregate gives all 50 workloads equal weight and is separate from the public suite.

| Measurement | vs GCC | vs Clang |
|---|---:|---:|
| Generated code | 1.014× | 0.992× |
| Compile CPU | 6.442× | 4.952× |

| Compiler | Executable bytes | .text bytes | Compile peak RSS (KiB) |
|---|---:|---:|---:|
| cppgm | 718,576 | 343,554 | 228,178 |
| gcc | 540,112 | 280,921 | 486,010 |
| clang | 513,720 | 264,561 | 319,066 |

| Workload | cppgm seconds | GCC seconds | Clang seconds | vs GCC | vs Clang | A/A warning |
|---|---:|---:|---:|---:|---:|---|
| Base64::Decode | 0.970 | 0.719 | 0.751 | 0.731× | 0.784× | clang |
| Base64::Encode | 0.972 | 0.909 | 0.698 | 0.942× | 0.733× | gcc |
| Binarytrees::Arena | 1.691 | 1.788 | 1.753 | 1.053× | 1.042× | gcc |
| Binarytrees::Obj | 0.733 | 0.674 | 0.800 | 0.904× | 1.091× | gcc, clang |
| Brainfuck::Array | 1.313 | 1.493 | 1.301 | 1.106× | 0.996× | gcc |
| Brainfuck::Recursion | 0.909 | 0.776 | 0.885 | 0.853× | 0.990× | gcc |
| CLBG::Fannkuchredux | 0.807 | 0.810 | 0.834 | 0.992× | 1.048× | — |
| CLBG::Mandelbrot | 0.960 | 0.896 | 0.915 | 0.918× | 0.981× | — |
| CLBG::Nbody | 0.663 | 0.667 | 0.663 | 1.007× | 0.996× | — |
| CLBG::Spectralnorm | 0.778 | 0.760 | 1.051 | 0.964× | 1.376× | — |
| CSV::Parse | 0.667 | 0.629 | 0.663 | 0.943× | 0.986× | gcc, clang |
| Calculator::Ast | 0.635 | 0.645 | 0.601 | 1.013× | 0.942× | gcc, clang |
| Calculator::Interpreter | 0.818 | 0.801 | 0.805 | 0.994× | 1.020× | clang |
| Compress::ArithDecode | 0.742 | 0.710 | 1.273 | 0.968× | 1.794× | — |
| Compress::ArithEncode | 0.927 | 0.817 | 0.813 | 0.873× | 0.911× | — |
| Compress::BWTDecode | 0.442 | 0.436 | 0.427 | 0.975× | 0.982× | gcc |
| Compress::BWTEncode | 0.627 | 0.643 | 0.758 | 1.036× | 1.217× | gcc |
| Compress::HuffDecode | 0.420 | 0.405 | 0.267 | 0.964× | 0.653× | gcc, clang |
| Compress::HuffEncode | 0.585 | 0.546 | 0.715 | 0.908× | 1.268× | gcc |
| Compress::LZWDecode | 0.441 | 0.446 | 0.419 | 0.965× | 0.950× | gcc |
| Compress::LZWEncode | 0.787 | 0.775 | 0.780 | 0.987× | 0.991× | gcc |
| Distance::Jaro | 0.996 | 0.965 | 0.927 | 0.961× | 0.976× | — |
| Distance::NGram | 0.950 | 0.912 | 0.919 | 0.969× | 0.964× | — |
| Etc::CacheSimulation | 0.671 | 0.673 | 0.655 | 1.010× | 0.968× | gcc, clang |
| Etc::GameOfLife | 1.118 | 0.671 | 0.706 | 0.629× | 0.652× | gcc, clang |
| Etc::LogParser | 0.267 | 0.921 | 0.303 | 3.396× | 1.114× | gcc, clang |
| Etc::NeuralNet | 0.722 | 0.653 | 0.619 | 0.904× | 0.864× | gcc |
| Etc::Sieve | 0.962 | 1.349 | 0.903 | 1.406× | 0.918× | gcc |
| Etc::TextRaytracer | 0.852 | 0.617 | 0.756 | 0.733× | 0.890× | clang |
| Etc::Words | 0.768 | 0.757 | 0.768 | 0.988× | 1.006× | gcc, clang |
| Graph::AStar | 1.139 | 1.140 | 0.972 | 1.012× | 0.861× | gcc, clang |
| Graph::BFS | 1.271 | 1.375 | 1.129 | 1.110× | 0.916× | — |
| Graph::DFS | 1.292 | 1.151 | 1.143 | 0.862× | 0.917× | clang |
| Hash::CRC32 | 0.221 | 0.623 | 0.684 | 2.782× | 3.084× | clang |
| Hash::SHA256 | 0.976 | 0.962 | 0.958 | 0.991× | 0.990× | clang |
| Json::Generate | 0.687 | 0.694 | 0.682 | 1.009× | 1.019× | gcc |
| Json::ParseDom | 0.827 | 0.790 | 0.788 | 0.954× | 0.950× | gcc, clang |
| Json::ParseMapping | 0.790 | 0.747 | 0.756 | 0.949× | 0.913× | gcc |
| Matmul::Single | 0.666 | 0.671 | 0.662 | 1.007× | 0.989× | gcc |
| Matmul::T16 | 1.349 | 1.357 | 1.369 | 1.006× | 1.017× | gcc |
| Matmul::T4 | 0.665 | 0.663 | 0.655 | 0.999× | 0.984× | gcc |
| Matmul::T8 | 0.695 | 0.696 | 0.721 | 1.002× | 1.036× | gcc |
| Maze::AStar | 1.167 | 1.273 | 1.065 | 1.054× | 0.992× | gcc |
| Maze::BFS | 1.125 | 1.162 | 0.861 | 1.003× | 0.877× | — |
| Maze::Generator | 1.294 | 1.276 | 1.179 | 0.969× | 0.912× | gcc, clang |
| Sort::Merge | 0.927 | 0.986 | 0.710 | 1.078× | 0.770× | — |
| Sort::Quick | 1.114 | 1.093 | 1.085 | 0.962× | 1.000× | clang |
| Sort::Self | 1.120 | 1.056 | 0.996 | 0.925× | 0.888× | — |
| Template::Parse | 0.789 | 1.009 | 0.776 | 1.159× | 0.950× | gcc, clang |
| Template::Regex | 0.848 | 0.943 | 0.909 | 1.056× | 1.060× | gcc, clang |

A/A warnings flag variation above 2%; no cases are dropped. Small timing differences need confirmation.
Compile gcc A/A CPU ratio: 0.932×.
Compile clang A/A CPU ratio: 0.987×.

Hardware counters: unavailable: perf is not installed on this host.

## Toolchains

- **cppgm**: cppgm++ (host configuration: g++)
- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and header hashes](metadata.json) · [Raw compile observations](compile-observations.jsonl) · [Raw runtime observations](runtime-observations.jsonl)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43348824459)
