# index-cpp benchmarks

Compiler checkout: `8287f480c7b5bdb2074c8941689aa36f9c233bd7` · CPU: AMD EPYC 7763 64-Core Processor

Unchanged upstream: `79e0b423094d420da37fb714f20c53fc35eeb2a3`. All 50 checksums pass with cppgm, GCC and Clang. Full runtime inputs, paired ABBA/BAAB blocks with A/A calibration. Speeds are reference time / cppgm time: higher is better. The aggregate gives all 50 workloads equal weight and is separate from the public suite.

| Measurement | vs GCC | vs Clang |
|---|---:|---:|
| Generated code | 0.992× | 0.994× |
| Compile CPU | 6.606× | 5.949× |

| Compiler | Executable bytes | .text bytes | Compile peak RSS (KiB) |
|---|---:|---:|---:|
| cppgm | 798,176 | 391,330 | 229,046 |
| gcc | 540,112 | 280,921 | 486,000 |
| clang | 513,720 | 264,561 | 319,120 |

| Workload | cppgm seconds | GCC seconds | Clang seconds | vs GCC | vs Clang | A/A warning |
|---|---:|---:|---:|---:|---:|---|
| Base64::Decode | 2.974 | 1.191 | 1.210 | 0.401× | 0.405× | clang |
| Base64::Encode | 1.750 | 1.429 | 1.293 | 0.815× | 0.740× | — |
| Binarytrees::Arena | 2.391 | 2.460 | 2.566 | 1.029× | 1.068× | — |
| Binarytrees::Obj | 1.760 | 1.322 | 1.593 | 0.761× | 0.901× | gcc |
| Brainfuck::Array | 3.348 | 2.811 | 2.880 | 0.841× | 0.846× | — |
| Brainfuck::Recursion | 1.652 | 1.738 | 1.704 | 1.054× | 1.026× | — |
| CLBG::Fannkuchredux | 1.070 | 1.155 | 0.976 | 1.071× | 0.918× | — |
| CLBG::Mandelbrot | 1.381 | 1.351 | 1.336 | 0.978× | 0.967× | — |
| CLBG::Nbody | 1.027 | 1.058 | 1.058 | 1.029× | 1.030× | — |
| CLBG::Spectralnorm | 1.186 | 1.141 | 1.738 | 0.963× | 1.465× | — |
| CSV::Parse | 1.139 | 1.081 | 1.140 | 0.946× | 1.000× | — |
| Calculator::Ast | 1.216 | 0.960 | 0.988 | 0.787× | 0.816× | gcc |
| Calculator::Interpreter | 1.357 | 1.275 | 1.433 | 0.954× | 1.045× | gcc |
| Compress::ArithDecode | 0.905 | 0.947 | 1.605 | 1.048× | 1.768× | — |
| Compress::ArithEncode | 1.046 | 0.981 | 1.122 | 0.939× | 1.072× | — |
| Compress::BWTDecode | 0.653 | 0.648 | 0.662 | 0.992× | 1.011× | — |
| Compress::BWTEncode | 1.205 | 1.136 | 1.365 | 0.941× | 1.129× | — |
| Compress::HuffDecode | 0.643 | 0.775 | 0.515 | 1.205× | 0.800× | — |
| Compress::HuffEncode | 0.860 | 1.222 | 1.036 | 1.424× | 1.201× | — |
| Compress::LZWDecode | 0.724 | 0.730 | 0.700 | 1.010× | 0.963× | gcc |
| Compress::LZWEncode | 1.340 | 1.311 | 1.304 | 0.978× | 0.973× | — |
| Distance::Jaro | 1.207 | 1.171 | 1.150 | 0.971× | 0.953× | — |
| Distance::NGram | 1.360 | 1.333 | 1.299 | 0.977× | 0.956× | gcc |
| Etc::CacheSimulation | 1.123 | 1.106 | 1.129 | 0.982× | 1.002× | gcc |
| Etc::GameOfLife | 1.445 | 1.131 | 1.242 | 0.782× | 0.858× | — |
| Etc::LogParser | 0.587 | 1.940 | 0.665 | 3.336× | 1.135× | — |
| Etc::NeuralNet | 1.357 | 1.121 | 1.081 | 0.812× | 0.828× | gcc |
| Etc::Sieve | 1.544 | 1.969 | 1.381 | 1.291× | 0.944× | clang |
| Etc::TextRaytracer | 1.155 | 1.022 | 1.339 | 0.882× | 1.159× | — |
| Etc::Words | 1.345 | 1.297 | 1.330 | 0.972× | 0.987× | — |
| Graph::AStar | 1.321 | 1.319 | 1.087 | 1.000× | 0.815× | — |
| Graph::BFS | 1.756 | 1.194 | 1.188 | 0.702× | 0.670× | clang |
| Graph::DFS | 1.405 | 1.185 | 1.272 | 0.849× | 0.901× | — |
| Hash::CRC32 | 0.293 | 0.806 | 0.916 | 2.751× | 3.125× | — |
| Hash::SHA256 | 1.649 | 1.475 | 1.385 | 0.893× | 0.840× | — |
| Json::Generate | 1.109 | 1.131 | 1.132 | 1.023× | 1.020× | gcc |
| Json::ParseDom | 1.304 | 1.191 | 1.196 | 0.912× | 0.917× | gcc |
| Json::ParseMapping | 1.241 | 1.132 | 1.137 | 0.905× | 0.916× | gcc |
| Matmul::Single | 1.389 | 1.387 | 1.387 | 1.000× | 0.992× | — |
| Matmul::T16 | 2.638 | 2.627 | 2.655 | 0.993× | 1.005× | — |
| Matmul::T4 | 1.357 | 1.356 | 1.357 | 0.999× | 0.993× | — |
| Matmul::T8 | 1.378 | 1.377 | 1.381 | 0.999× | 1.003× | — |
| Maze::AStar | 1.252 | 1.121 | 1.442 | 0.931× | 1.070× | gcc, clang |
| Maze::BFS | 1.275 | 1.416 | 1.723 | 1.104× | 1.221× | gcc, clang |
| Maze::Generator | 1.335 | 1.288 | 1.319 | 0.963× | 0.978× | clang |
| Sort::Merge | 1.209 | 1.127 | 1.027 | 0.932× | 0.857× | — |
| Sort::Quick | 1.331 | 1.231 | 1.251 | 0.925× | 0.939× | — |
| Sort::Self | 1.313 | 1.207 | 1.229 | 0.921× | 0.934× | — |
| Template::Parse | 1.234 | 1.235 | 1.431 | 1.000× | 1.072× | gcc, clang |
| Template::Regex | 1.087 | 1.325 | 1.624 | 1.217× | 1.441× | clang |

A/A warnings flag variation above 2%; no cases are dropped. Small timing differences need confirmation.
Compile gcc A/A CPU ratio: 1.006×.
Compile clang A/A CPU ratio: 0.983×.

Hardware counters: unavailable: perf is not installed on this host.

## Toolchains

- **cppgm**: cppgm++ (host configuration: g++)
- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and header hashes](metadata.json) · [Raw compile observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42735799865) · [Raw runtime observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42735799865)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/42735799865)
