# index-cpp benchmarks

Compiler checkout: `6c589867bdd5c99bc35b90267a46778f59e1127e` · CPU: Intel(R) Xeon(R) CPU E5-2696 v4 @ 2.20GHz

Unchanged upstream: `79e0b423094d420da37fb714f20c53fc35eeb2a3`. All 50 checksums pass with cppgm, GCC and Clang. Full runtime inputs, paired ABBA/BAAB blocks with A/A calibration. Speeds are reference time / cppgm time: higher is better. The aggregate gives all 50 workloads equal weight and is separate from the public suite.

| Measurement | vs GCC | vs Clang |
|---|---:|---:|
| Generated code | 1.017× | 1.040× |
| Compile CPU | 6.659× | 5.787× |

| Compiler | Executable bytes | .text bytes | Compile peak RSS (KiB) |
|---|---:|---:|---:|
| cppgm | 798,176 | 388,834 | 227,638 |
| gcc | 540,112 | 280,921 | 484,656 |
| clang | 513,720 | 264,561 | 315,864 |

| Workload | cppgm seconds | GCC seconds | Clang seconds | vs GCC | vs Clang | A/A warning |
|---|---:|---:|---:|---:|---:|---|
| Base64::Decode | 2.700 | 1.522 | 1.523 | 0.552× | 0.595× | clang |
| Base64::Encode | 2.170 | 2.090 | 1.682 | 0.892× | 0.794× | — |
| Binarytrees::Arena | 3.279 | 3.513 | 3.553 | 1.061× | 1.095× | clang |
| Binarytrees::Obj | 2.181 | 1.961 | 2.109 | 0.899× | 0.986× | — |
| Brainfuck::Array | 2.438 | 2.679 | 2.730 | 1.074× | 1.162× | clang |
| Brainfuck::Recursion | 2.087 | 1.958 | 1.784 | 0.908× | 0.849× | gcc, clang |
| CLBG::Fannkuchredux | 1.081 | 1.212 | 1.045 | 1.057× | 1.078× | clang |
| CLBG::Mandelbrot | 1.806 | 1.649 | 1.658 | 0.862× | 0.969× | — |
| CLBG::Nbody | 1.300 | 1.343 | 1.328 | 0.944× | 1.034× | — |
| CLBG::Spectralnorm | 1.503 | 1.558 | 2.144 | 1.020× | 1.455× | — |
| CSV::Parse | 1.809 | 1.793 | 1.699 | 0.967× | 0.952× | gcc |
| Calculator::Ast | 1.949 | 1.542 | 1.591 | 0.748× | 0.890× | gcc |
| Calculator::Interpreter | 2.631 | 2.687 | 2.729 | 0.993× | 1.092× | gcc, clang |
| Compress::ArithDecode | 1.923 | 2.054 | 2.220 | 1.012× | 1.204× | clang |
| Compress::ArithEncode | 1.788 | 1.965 | 1.842 | 1.027× | 1.078× | clang |
| Compress::BWTDecode | 1.450 | 1.629 | 1.567 | 1.057× | 1.141× | clang |
| Compress::BWTEncode | 2.128 | 2.292 | 2.320 | 1.056× | 1.127× | gcc, clang |
| Compress::HuffDecode | 1.003 | 1.135 | 0.752 | 1.083× | 0.831× | clang |
| Compress::HuffEncode | 1.131 | 1.442 | 1.531 | 1.201× | 1.444× | — |
| Compress::LZWDecode | 1.180 | 1.228 | 1.141 | 1.018× | 0.999× | clang |
| Compress::LZWEncode | 1.921 | 1.947 | 1.838 | 1.004× | 1.004× | clang |
| Distance::Jaro | 1.677 | 1.509 | 1.466 | 0.861× | 0.937× | clang |
| Distance::NGram | 2.577 | 2.547 | 2.250 | 0.963× | 0.880× | gcc |
| Etc::CacheSimulation | 1.903 | 1.851 | 1.885 | 0.948× | 0.991× | clang |
| Etc::GameOfLife | 2.112 | 2.050 | 1.873 | 0.974× | 0.963× | gcc, clang |
| Etc::LogParser | 0.812 | 2.491 | 0.986 | 2.890× | 1.245× | gcc, clang |
| Etc::NeuralNet | 1.859 | 1.701 | 1.574 | 0.883× | 0.879× | clang |
| Etc::Sieve | 3.444 | 4.338 | 3.531 | 1.245× | 1.006× | clang |
| Etc::TextRaytracer | 1.481 | 1.363 | 1.499 | 0.895× | 1.036× | clang |
| Etc::Words | 2.083 | 2.162 | 2.202 | 0.982× | 1.118× | gcc, clang |
| Graph::AStar | 1.493 | 1.770 | 1.256 | 1.160× | 0.850× | — |
| Graph::BFS | 1.235 | 1.250 | 1.155 | 1.019× | 1.017× | gcc, clang |
| Graph::DFS | 1.391 | 1.454 | 1.385 | 0.988× | 1.064× | gcc, clang |
| Hash::CRC32 | 0.306 | 1.063 | 0.980 | 3.183× | 3.153× | — |
| Hash::SHA256 | 1.838 | 2.078 | 1.730 | 1.124× | 0.982× | clang |
| Json::Generate | 1.633 | 1.738 | 1.558 | 1.012× | 0.981× | — |
| Json::ParseDom | 2.188 | 2.146 | 1.977 | 0.948× | 0.951× | clang |
| Json::ParseMapping | 2.136 | 2.024 | 1.885 | 0.912× | 0.941× | gcc, clang |
| Matmul::Single | 1.483 | 1.627 | 1.516 | 1.001× | 1.061× | clang |
| Matmul::T16 | 1.605 | 1.595 | 1.634 | 0.954× | 1.064× | gcc |
| Matmul::T4 | 1.569 | 1.602 | 1.520 | 1.010× | 0.993× | gcc, clang |
| Matmul::T8 | 1.651 | 1.622 | 1.545 | 0.978× | 0.938× | — |
| Maze::AStar | 1.724 | 1.688 | 1.615 | 0.913× | 1.054× | gcc, clang |
| Maze::BFS | 1.514 | 1.476 | 1.603 | 0.907× | 1.155× | gcc, clang |
| Maze::Generator | 2.067 | 2.037 | 1.937 | 0.970× | 0.994× | gcc, clang |
| Sort::Merge | 1.317 | 1.335 | 1.093 | 0.921× | 0.887× | gcc, clang |
| Sort::Quick | 1.406 | 1.515 | 1.297 | 1.018× | 1.010× | gcc, clang |
| Sort::Self | 1.407 | 1.430 | 1.269 | 0.935× | 0.940× | gcc, clang |
| Template::Parse | 2.022 | 2.332 | 1.929 | 0.968× | 1.293× | gcc, clang |
| Template::Regex | 1.724 | 2.104 | 2.058 | 1.059× | 1.366× | gcc |

A/A warnings flag variation above 2%; no cases are dropped. Small timing differences need confirmation.
Compile gcc A/A CPU ratio: 1.032×.
Compile clang A/A CPU ratio: 0.955×.

Hardware counters: unavailable: perf is not installed on this host.

## Toolchains

- **cppgm**: cppgm++ (host configuration: g++)
- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and header hashes](metadata.json) · [Raw compile observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42683414911) · [Raw runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42683414911)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42683414911)
