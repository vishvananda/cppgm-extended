# index-cpp benchmarks

Compiler checkout: `93343ea0ebdcf068d87b88027a6eca8abf32111f` · CPU: AMD EPYC 9V45 96-Core Processor

Unchanged upstream: `79e0b423094d420da37fb714f20c53fc35eeb2a3`. All 50 checksums pass with cppgm, GCC and Clang. Full runtime inputs, paired ABBA/BAAB blocks with A/A calibration. Speeds are reference time / cppgm time: higher is better. The aggregate gives all 50 workloads equal weight and is separate from the public suite.

| Measurement | vs GCC | vs Clang |
|---|---:|---:|
| Generated code | 0.994× | 0.984× |
| Compile CPU | 6.665× | 5.319× |

| Compiler | Executable bytes | .text bytes | Compile peak RSS (KiB) |
|---|---:|---:|---:|
| cppgm | 798,176 | 388,834 | 229,102 |
| gcc | 540,112 | 280,921 | 486,036 |
| clang | 513,720 | 264,561 | 319,084 |

| Workload | cppgm seconds | GCC seconds | Clang seconds | vs GCC | vs Clang | A/A warning |
|---|---:|---:|---:|---:|---:|---|
| Base64::Decode | 1.203 | 0.716 | 0.761 | 0.609× | 0.629× | — |
| Base64::Encode | 0.996 | 0.925 | 0.724 | 0.939× | 0.732× | gcc |
| Binarytrees::Arena | 1.730 | 1.776 | 1.756 | 1.028× | 1.011× | clang |
| Binarytrees::Obj | 0.841 | 0.685 | 0.797 | 0.823× | 0.948× | clang |
| Brainfuck::Array | 1.425 | 1.505 | 1.325 | 1.034× | 0.941× | — |
| Brainfuck::Recursion | 0.923 | 0.780 | 0.907 | 0.840× | 0.987× | gcc |
| CLBG::Fannkuchredux | 0.871 | 0.822 | 0.857 | 0.948× | 0.981× | — |
| CLBG::Mandelbrot | 0.976 | 0.909 | 0.931 | 0.929× | 0.939× | — |
| CLBG::Nbody | 0.675 | 0.680 | 0.678 | 0.998× | 1.006× | — |
| CLBG::Spectralnorm | 0.782 | 0.770 | 1.086 | 0.989× | 1.386× | — |
| CSV::Parse | 0.686 | 0.621 | 0.679 | 0.903× | 0.985× | clang |
| Calculator::Ast | 0.770 | 0.639 | 0.631 | 0.820× | 0.808× | — |
| Calculator::Interpreter | 0.982 | 0.787 | 0.905 | 0.794× | 0.875× | gcc |
| Compress::ArithDecode | 0.720 | 0.709 | 1.311 | 0.999× | 1.814× | — |
| Compress::ArithEncode | 0.907 | 0.813 | 0.845 | 0.881× | 0.939× | — |
| Compress::BWTDecode | 0.460 | 0.442 | 0.442 | 0.964× | 0.972× | — |
| Compress::BWTEncode | 0.694 | 0.653 | 0.782 | 0.955× | 1.118× | gcc |
| Compress::HuffDecode | 0.344 | 0.408 | 0.279 | 1.222× | 0.811× | gcc |
| Compress::HuffEncode | 0.556 | 0.542 | 0.740 | 0.980× | 1.322× | — |
| Compress::LZWDecode | 0.430 | 0.441 | 0.422 | 1.038× | 0.980× | — |
| Compress::LZWEncode | 0.790 | 0.778 | 0.788 | 0.996× | 0.983× | — |
| Distance::Jaro | 1.030 | 0.964 | 0.955 | 0.929× | 0.904× | — |
| Distance::NGram | 0.964 | 0.933 | 0.936 | 0.972× | 0.951× | gcc, clang |
| Etc::CacheSimulation | 0.680 | 0.633 | 0.677 | 0.928× | 0.986× | — |
| Etc::GameOfLife | 1.030 | 0.809 | 0.877 | 0.752× | 0.723× | gcc, clang |
| Etc::LogParser | 0.334 | 0.913 | 0.309 | 2.776× | 0.988× | clang |
| Etc::NeuralNet | 0.801 | 0.690 | 0.637 | 0.833× | 0.823× | clang |
| Etc::Sieve | 0.988 | 1.345 | 0.931 | 1.357× | 0.924× | gcc |
| Etc::TextRaytracer | 0.684 | 0.622 | 0.778 | 0.908× | 1.139× | — |
| Etc::Words | 0.789 | 0.746 | 0.804 | 0.950× | 1.026× | — |
| Graph::AStar | 1.155 | 1.134 | 0.994 | 0.988× | 0.862× | gcc |
| Graph::BFS | 1.319 | 1.405 | 1.247 | 1.043× | 0.926× | gcc, clang |
| Graph::DFS | 1.219 | 1.149 | 1.205 | 0.950× | 0.950× | — |
| Hash::CRC32 | 0.226 | 0.627 | 0.704 | 2.780× | 3.126× | — |
| Hash::SHA256 | 0.999 | 0.966 | 0.996 | 0.956× | 0.974× | — |
| Json::Generate | 0.688 | 0.693 | 0.707 | 1.018× | 0.969× | — |
| Json::ParseDom | 0.844 | 0.798 | 0.805 | 0.951× | 0.955× | clang |
| Json::ParseMapping | 0.798 | 0.768 | 0.770 | 0.963× | 0.948× | — |
| Matmul::Single | 0.677 | 0.672 | 0.678 | 0.998× | 0.979× | — |
| Matmul::T16 | 1.367 | 1.361 | 1.415 | 1.001× | 1.033× | — |
| Matmul::T4 | 0.679 | 0.671 | 0.680 | 0.984× | 0.997× | — |
| Matmul::T8 | 0.712 | 0.696 | 0.734 | 0.988× | 1.039× | — |
| Maze::AStar | 1.278 | 1.177 | 1.342 | 0.972× | 1.004× | clang |
| Maze::BFS | 1.127 | 1.010 | 1.074 | 0.942× | 0.881× | clang |
| Maze::Generator | 1.351 | 1.262 | 1.300 | 0.913× | 0.981× | gcc, clang |
| Sort::Merge | 0.983 | 1.004 | 0.744 | 1.025× | 0.750× | clang |
| Sort::Quick | 1.174 | 1.086 | 1.139 | 0.933× | 0.962× | — |
| Sort::Self | 1.165 | 1.084 | 1.052 | 0.943× | 0.881× | gcc |
| Template::Parse | 0.917 | 0.805 | 0.883 | 0.982× | 0.859× | clang |
| Template::Regex | 0.766 | 0.883 | 1.120 | 1.169× | 1.201× | gcc, clang |

A/A warnings flag variation above 2%; no cases are dropped. Small timing differences need confirmation.
Compile gcc A/A CPU ratio: 1.038×.
Compile clang A/A CPU ratio: 0.972×.

Hardware counters: unavailable: perf is not installed on this host.

## Toolchains

- **cppgm**: cppgm++ (host configuration: g++)
- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and header hashes](metadata.json) · [Raw compile observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42682903810) · [Raw runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42682903810)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42682903810)
