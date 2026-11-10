# Issue 115: store predication, paired full-run table

Intel Xeon E5-2696 v4, generic x86-64, -O3. Ratios are higher-is-better.
Each column uses its corresponding ABBA block; absolute times from different
blocks must not be compared directly. See [method and qualification](STORE_PREDICATION.md).

| Workload | cppgm (s) | Clang (s) | cppgm / Clang speed ↑ | cppgm / main speed ↑ |
|---|---:|---:|---:|---:|
| Compress::HuffDecode | 1.173 | 0.835 | 0.712× | 1.001× |
| Etc::LogParser | 1.271 | 0.988 | 0.777× | 1.114× |
| Compress::BWTEncode | 2.237 | 1.798 | 0.804× | 0.950× |
| Compress::ArithDecode | 2.373 | 1.915 | 0.807× | 0.980× |
| Etc::NeuralNet | 2.163 | 1.751 | 0.810× | 1.034× |
| Etc::GameOfLife | 1.252 | 1.031 | 0.823× | 1.048× |
| Etc::TextRaytracer | 1.852 | 1.575 | 0.851× | 1.009× |
| Brainfuck::Array | 2.032 | 1.738 | 0.855× | 0.923× |
| Graph::AStar | 1.524 | 1.332 | 0.874× | 0.977× |
| CLBG::Mandelbrot | 1.767 | 1.562 | 0.884× | 1.082× |
| Sort::Quick | 1.450 | 1.284 | 0.885× | 1.048× |
| Maze::AStar | 2.243 | 2.011 | 0.897× | 1.043× |
| Distance::NGram | 2.542 | 2.314 | 0.911× | 0.996× |
| Compress::ArithEncode | 1.809 | 1.655 | 0.915× | 1.015× |
| Hash::SHA256 | 1.813 | 1.659 | 0.915× | 0.997× |
| Template::Parse | 2.435 | 2.247 | 0.923× | 1.012× |
| Compress::BWTDecode | 1.467 | 1.356 | 0.924× | 0.989× |
| Compress::LZWEncode | 2.077 | 1.942 | 0.935× | 0.989× |
| CLBG::Spectralnorm | 1.675 | 1.568 | 0.936× | 0.979× |
| Sort::Self | 1.381 | 1.294 | 0.937× | 0.967× |
| CLBG::Nbody | 1.391 | 1.305 | 0.938× | 1.069× |
| Compress::LZWDecode | 2.198 | 2.079 | 0.946× | 1.000× |
| Json::ParseDom | 2.142 | 2.035 | 0.950× | 0.977× |
| Json::ParseMapping | 2.034 | 1.942 | 0.955× | 0.979× |
| Matmul::T8 | 1.684 | 1.626 | 0.965× | 0.955× |
| Maze::Generator | 1.875 | 1.818 | 0.970× | 1.038× |
| Etc::Sieve | 3.457 | 3.354 | 0.970× | 0.939× |
| Etc::Words | 2.107 | 2.053 | 0.975× | 0.988× |
| Sort::Merge | 1.054 | 1.053 | 1.000× | 1.430× |
| Distance::Jaro | 1.450 | 1.450 | 1.001× | 1.046× |
| Graph::BFS | 1.216 | 1.222 | 1.005× | 1.056× |
| Matmul::T4 | 1.737 | 1.786 | 1.028× | 0.955× |
| Etc::CacheSimulation | 1.556 | 1.601 | 1.028× | 0.989× |
| Calculator::Interpreter | 2.813 | 2.933 | 1.042× | 0.910× |
| Calculator::Ast | 2.308 | 2.411 | 1.045× | 0.960× |
| Hash::CRC32 | 0.322 | 0.340 | 1.058× | 1.030× |
| Matmul::T16 | 2.294 | 2.444 | 1.065× | 0.861× |
| Matmul::Single | 1.559 | 1.665 | 1.068× | 0.960× |
| CLBG::Fannkuchredux | 1.384 | 1.500 | 1.084× | 1.023× |
| Binarytrees::Obj | 1.821 | 1.980 | 1.087× | 0.991× |
| Json::Generate | 1.616 | 1.763 | 1.091× | 1.022× |
| Graph::DFS | 1.378 | 1.508 | 1.094× | 0.952× |
| Maze::BFS | 1.612 | 1.780 | 1.104× | 1.027× |
| Base64::Encode | 0.192 | 0.216 | 1.125× | 1.016× |
| Brainfuck::Recursion | 1.958 | 2.208 | 1.128× | 0.885× |
| Template::Regex | 1.777 | 2.054 | 1.156× | 1.063× |
| CSV::Parse | 1.498 | 1.775 | 1.185× | 1.034× |
| Binarytrees::Arena | 2.391 | 2.851 | 1.192× | 0.952× |
| Compress::HuffEncode | 1.479 | 1.867 | 1.262× | 1.000× |
| Base64::Decode | 1.156 | 1.812 | 1.567× | 1.063× |
