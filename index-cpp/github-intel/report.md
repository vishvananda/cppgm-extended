# index-cpp benchmarks

Compiler checkout: `dc908433031b1e72d07206262cb240cdedc9d13b` · CPU: INTEL(R) XEON(R) PLATINUM 8573C

Unchanged upstream: `79e0b423094d420da37fb714f20c53fc35eeb2a3`. All 50 checksums pass with cppgm, GCC and Clang. Full runtime inputs, paired ABBA/BAAB blocks with A/A calibration. Speeds are reference time / cppgm time: higher is better. The aggregate gives all 50 workloads equal weight and is separate from the public suite.

| Measurement | vs GCC | vs Clang |
|---|---:|---:|
| Generated code | 1.043× | 1.029× |
| Compile CPU | 5.994× | 4.933× |

| Compiler | Executable bytes | .text bytes | Compile peak RSS (KiB) |
|---|---:|---:|---:|
| cppgm | 714,480 | 341,330 | 228,506 |
| gcc | 540,112 | 280,921 | 485,952 |
| clang | 513,720 | 264,561 | 319,122 |

| Workload | cppgm seconds | GCC seconds | Clang seconds | vs GCC | vs Clang | A/A warning |
|---|---:|---:|---:|---:|---:|---|
| Base64::Decode | 1.272 | 0.960 | 1.071 | 0.755× | 0.842× | — |
| Base64::Encode | 1.380 | 1.260 | 1.039 | 0.914× | 0.750× | — |
| Binarytrees::Arena | 2.391 | 2.369 | 2.597 | 1.004× | 1.090× | — |
| Binarytrees::Obj | 1.242 | 1.150 | 1.625 | 0.933× | 1.305× | — |
| Brainfuck::Array | 1.950 | 2.369 | 2.199 | 1.215× | 1.123× | — |
| Brainfuck::Recursion | 1.453 | 1.551 | 1.476 | 1.068× | 1.014× | — |
| CLBG::Fannkuchredux | 1.205 | 1.288 | 1.090 | 1.060× | 0.910× | — |
| CLBG::Mandelbrot | 1.500 | 1.349 | 1.331 | 0.897× | 0.888× | — |
| CLBG::Nbody | 0.879 | 0.834 | 0.850 | 0.942× | 0.969× | — |
| CLBG::Spectralnorm | 1.247 | 1.204 | 1.684 | 0.964× | 1.349× | — |
| CSV::Parse | 1.125 | 1.055 | 1.173 | 0.934× | 1.039× | — |
| Calculator::Ast | 1.145 | 1.103 | 1.090 | 0.957× | 0.952× | — |
| Calculator::Interpreter | 1.454 | 1.425 | 1.469 | 0.969× | 1.049× | gcc |
| Compress::ArithDecode | 1.127 | 1.304 | 1.929 | 1.162× | 1.713× | — |
| Compress::ArithEncode | 1.339 | 1.300 | 1.403 | 0.969× | 1.053× | — |
| Compress::BWTDecode | 1.655 | 1.647 | 1.649 | 0.994× | 1.002× | — |
| Compress::BWTEncode | 0.954 | 1.012 | 1.182 | 1.059× | 1.248× | — |
| Compress::HuffDecode | 1.122 | 0.938 | 0.497 | 0.838× | 0.440× | — |
| Compress::HuffEncode | 1.213 | 1.300 | 1.057 | 1.067× | 0.880× | — |
| Compress::LZWDecode | 0.771 | 0.771 | 0.744 | 0.997× | 0.967× | — |
| Compress::LZWEncode | 1.232 | 1.223 | 1.228 | 0.990× | 1.001× | — |
| Distance::Jaro | 1.674 | 1.583 | 1.724 | 0.940× | 1.028× | — |
| Distance::NGram | 1.537 | 1.565 | 1.480 | 1.012× | 0.962× | — |
| Etc::CacheSimulation | 1.063 | 1.034 | 1.062 | 0.973× | 1.004× | — |
| Etc::GameOfLife | 1.377 | 1.351 | 1.302 | 0.980× | 0.951× | — |
| Etc::LogParser | 0.553 | 1.531 | 0.609 | 2.747× | 1.113× | gcc |
| Etc::NeuralNet | 1.022 | 0.944 | 0.929 | 0.926× | 0.912× | — |
| Etc::Sieve | 2.853 | 3.634 | 2.688 | 1.260× | 0.947× | — |
| Etc::TextRaytracer | 1.228 | 0.843 | 1.091 | 0.685× | 0.891× | — |
| Etc::Words | 1.188 | 1.138 | 1.170 | 0.957× | 0.985× | — |
| Graph::AStar | 1.423 | 1.399 | 1.211 | 0.981× | 0.854× | — |
| Graph::BFS | 0.997 | 1.010 | 1.001 | 0.994× | 0.991× | gcc |
| Graph::DFS | 1.244 | 1.144 | 1.181 | 0.923× | 0.954× | — |
| Hash::CRC32 | 0.317 | 0.917 | 1.272 | 2.891× | 4.016× | — |
| Hash::SHA256 | 0.442 | 1.322 | 1.103 | 2.986× | 2.491× | — |
| Json::Generate | 1.141 | 1.140 | 1.128 | 0.997× | 0.996× | — |
| Json::ParseDom | 1.369 | 1.319 | 1.299 | 0.966× | 0.948× | — |
| Json::ParseMapping | 1.304 | 1.254 | 1.235 | 0.963× | 0.944× | — |
| Matmul::Single | 0.945 | 0.944 | 0.929 | 0.996× | 0.985× | — |
| Matmul::T16 | 2.155 | 1.906 | 1.899 | 0.886× | 0.872× | — |
| Matmul::T4 | 0.956 | 0.954 | 0.954 | 0.993× | 0.997× | — |
| Matmul::T8 | 1.090 | 0.998 | 1.000 | 0.910× | 0.938× | — |
| Maze::AStar | 1.490 | 1.653 | 1.518 | 1.078× | 1.005× | gcc, clang |
| Maze::BFS | 0.970 | 0.961 | 0.963 | 1.028× | 1.027× | gcc |
| Maze::Generator | 1.650 | 1.848 | 1.822 | 1.126× | 1.116× | — |
| Sort::Merge | 1.289 | 1.389 | 1.090 | 1.075× | 0.845× | — |
| Sort::Quick | 1.487 | 1.514 | 1.466 | 1.014× | 0.989× | — |
| Sort::Self | 1.541 | 1.420 | 1.450 | 0.918× | 0.939× | — |
| Template::Parse | 1.438 | 1.476 | 1.483 | 1.022× | 1.065× | — |
| Template::Regex | 1.506 | 1.340 | 1.486 | 0.884× | 0.991× | clang |

A/A warnings flag variation above 2%; no cases are dropped. Small timing differences need confirmation.
Compile gcc A/A CPU ratio: 1.002×.
Compile clang A/A CPU ratio: 0.992×.

Hardware counters: unavailable: perf is not installed on this host.

## Toolchains

- **cppgm**: cppgm++ (host configuration: g++)
- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)

[Results](results.json) · [Provenance and header hashes](metadata.json) · [Raw compile observations](compile-observations.jsonl) · [Raw runtime observations](runtime-observations.jsonl)

[CI run and complete artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43395345398)
