# index-cpp benchmarks

Compiler checkout: `b858d4082cddcb8e1e930d786f3f1139c6996e6d` · CPU: AMD EPYC 9V74 80-Core Processor

Unchanged upstream: `79e0b423094d420da37fb714f20c53fc35eeb2a3`. All 50 checksums pass with cppgm, GCC and Clang. Full runtime inputs, paired ABBA/BAAB blocks with A/A calibration. Speeds are reference time / cppgm time: higher is better. The aggregate gives all 50 workloads equal weight and is separate from the public suite.

| Measurement | vs GCC | vs Clang |
|---|---:|---:|
| Generated code | 1.032× | 1.018× |
| Compile CPU | 6.359× | 5.252× |

| Compiler | Executable bytes | .text bytes | Compile peak RSS (KiB) |
|---|---:|---:|---:|
| cppgm | 714,480 | 341,490 | 228,376 |
| gcc | 540,112 | 280,921 | 486,088 |
| clang | 513,720 | 264,561 | 319,276 |

| Workload | cppgm seconds | GCC seconds | Clang seconds | vs GCC | vs Clang | A/A warning |
|---|---:|---:|---:|---:|---:|---|
| Base64::Decode | 1.286 | 0.949 | 0.993 | 0.738× | 0.770× | — |
| Base64::Encode | 1.352 | 1.195 | 1.003 | 0.884× | 0.743× | — |
| Binarytrees::Arena | 2.138 | 2.229 | 2.201 | 1.043× | 1.028× | — |
| Binarytrees::Obj | 2.088 | 1.991 | 2.071 | 0.955× | 0.971× | gcc, clang |
| Brainfuck::Array | 1.530 | 2.138 | 1.584 | 1.448× | 1.048× | gcc, clang |
| Brainfuck::Recursion | 1.490 | 1.179 | 1.132 | 0.854× | 0.750× | — |
| CLBG::Fannkuchredux | 0.936 | 0.980 | 0.915 | 1.039× | 0.990× | — |
| CLBG::Mandelbrot | 1.240 | 1.171 | 1.115 | 0.945× | 0.901× | — |
| CLBG::Nbody | 0.889 | 0.892 | 0.895 | 1.002× | 1.007× | — |
| CLBG::Spectralnorm | 0.959 | 0.928 | 1.577 | 0.966× | 1.645× | — |
| CSV::Parse | 0.909 | 0.822 | 0.907 | 0.900× | 0.999× | — |
| Calculator::Ast | 0.821 | 0.747 | 0.792 | 0.911× | 0.958× | — |
| Calculator::Interpreter | 1.195 | 1.083 | 1.206 | 0.904× | 0.997× | gcc |
| Compress::ArithDecode | 0.799 | 0.815 | 1.396 | 1.021× | 1.739× | — |
| Compress::ArithEncode | 0.925 | 0.855 | 0.952 | 0.926× | 1.033× | — |
| Compress::BWTDecode | 0.554 | 0.538 | 0.560 | 0.972× | 1.012× | — |
| Compress::BWTEncode | 0.833 | 0.857 | 0.993 | 1.029× | 1.183× | — |
| Compress::HuffDecode | 0.694 | 0.598 | 0.413 | 0.863× | 0.595× | — |
| Compress::HuffEncode | 0.752 | 0.776 | 0.920 | 1.033× | 1.221× | — |
| Compress::LZWDecode | 0.600 | 0.615 | 0.573 | 1.026× | 0.955× | — |
| Compress::LZWEncode | 0.994 | 0.976 | 1.001 | 0.984× | 1.003× | gcc, clang |
| Distance::Jaro | 1.067 | 1.051 | 0.978 | 0.986× | 0.915× | — |
| Distance::NGram | 1.113 | 1.073 | 1.054 | 0.962× | 0.950× | — |
| Etc::CacheSimulation | 0.902 | 0.885 | 0.891 | 0.976× | 0.985× | — |
| Etc::GameOfLife | 1.099 | 1.015 | 1.058 | 0.935× | 0.964× | clang |
| Etc::LogParser | 0.470 | 1.744 | 0.506 | 3.670× | 1.084× | gcc, clang |
| Etc::NeuralNet | 0.975 | 0.829 | 0.822 | 0.805× | 0.844× | gcc, clang |
| Etc::Sieve | 1.182 | 1.565 | 1.194 | 1.324× | 1.011× | — |
| Etc::TextRaytracer | 1.130 | 0.882 | 1.060 | 0.780× | 0.939× | — |
| Etc::Words | 1.007 | 0.970 | 1.007 | 0.963× | 1.000× | gcc |
| Graph::AStar | 1.296 | 1.260 | 1.039 | 0.970× | 0.796× | — |
| Graph::BFS | 1.696 | 1.304 | 1.210 | 0.768× | 0.706× | — |
| Graph::DFS | 1.521 | 1.230 | 1.256 | 0.805× | 0.827× | gcc |
| Hash::CRC32 | 0.257 | 0.706 | 0.802 | 2.747× | 3.121× | — |
| Hash::SHA256 | 0.383 | 1.173 | 1.180 | 3.047× | 3.073× | — |
| Json::Generate | 0.782 | 0.770 | 0.776 | 0.980× | 0.992× | — |
| Json::ParseDom | 0.966 | 0.906 | 0.903 | 0.933× | 0.933× | — |
| Json::ParseMapping | 0.922 | 0.864 | 0.860 | 0.944× | 0.931× | — |
| Matmul::Single | 1.185 | 1.184 | 1.185 | 1.000× | 0.999× | — |
| Matmul::T16 | 2.353 | 2.306 | 2.306 | 0.981× | 0.977× | — |
| Matmul::T4 | 1.169 | 1.168 | 1.164 | 1.000× | 0.996× | — |
| Matmul::T8 | 1.224 | 1.203 | 1.200 | 0.981× | 0.980× | — |
| Maze::AStar | 1.197 | 1.143 | 1.308 | 0.979× | 1.081× | — |
| Maze::BFS | 1.118 | 1.067 | 1.075 | 0.981× | 0.935× | gcc |
| Maze::Generator | 1.322 | 1.319 | 1.337 | 1.015× | 1.014× | — |
| Sort::Merge | 0.995 | 0.988 | 0.879 | 0.996× | 0.888× | — |
| Sort::Quick | 1.148 | 1.086 | 1.100 | 0.946× | 0.958× | — |
| Sort::Self | 1.084 | 1.040 | 1.058 | 0.959× | 0.976× | — |
| Template::Parse | 0.980 | 0.984 | 1.002 | 1.009× | 1.030× | — |
| Template::Regex | 1.019 | 1.072 | 1.166 | 1.058× | 1.148× | clang |

A/A warnings flag variation above 2%; no cases are dropped. Small timing differences need confirmation.
Compile gcc A/A CPU ratio: 1.001×.
Compile clang A/A CPU ratio: 1.003×.

Hardware counters: unavailable: perf is not installed on this host.

## Toolchains

- **cppgm**: cppgm++ (host configuration: g++)
- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and header hashes](metadata.json) · [Raw compile observations](compile-observations.jsonl) · [Raw runtime observations](runtime-observations.jsonl)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43479737988)
