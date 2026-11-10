# All 50 original-input cases after the fixes

Same frozen LowIR and immutable binaries as [FIXES.md](FIXES.md). Baseline and
candidate are paired means; Clang is the preceding A/A mean, GCC a single
context run. Small cross-compiler timing differences are exploratory under host
load. See the focused checks and timing caveats in the report.

| Case | Before (s) | After (s) | Speedup ↑ | vs GCC ↑ | vs Clang ↑ |
|---|---:|---:|---:|---:|---:|
| Maze::Generator | 3.8450 | 3.3425 | 1.150× | 0.567× | 0.545× |
| Compress::HuffDecode | 2.1605 | 2.3190 | 0.932× | 0.446× | 0.368× |
| Compress::BWTDecode | 1.4800 | 2.5705 | 0.576× | 0.552× | 0.543× |
| Maze::BFS | 2.4210 | 2.1135 | 1.145× | 0.767× | 0.634× |
| Compress::LZWDecode | 2.2605 | 2.7065 | 0.835× | 0.756× | 0.714× |
| Distance::NGram | 2.6410 | 2.7250 | 0.969× | 0.876× | 0.734× |
| Etc::NeuralNet | 2.3690 | 2.3540 | 1.006× | 0.726× | 0.696× |
| Compress::LZWEncode | 2.3095 | 2.4280 | 0.951× | 0.808× | 0.717× |
| Maze::AStar | 3.1930 | 2.7625 | 1.156× | 0.695× | 0.755× |
| Compress::HuffEncode | 1.5290 | 2.4785 | 0.617× | 0.751× | 0.734× |
| Brainfuck::Array | 2.1745 | 2.2030 | 0.987× | 0.914× | 0.715× |
| Sort::Merge | 1.5165 | 1.5390 | 0.985× | 0.828× | 0.661× |
| Etc::Sieve | 4.0650 | 3.7395 | 1.087× | 1.072× | 0.872× |
| CLBG::Mandelbrot | 2.0500 | 1.9450 | 1.054× | 0.867× | 0.765× |
| Etc::TextRaytracer | 13.1995 | 1.9940 | 6.620× | 0.716× | 0.776× |
| Etc::LogParser | 2.2480 | 1.4185 | 1.585× | 1.736× | 0.692× |
| Calculator::Ast | 2.7675 | 2.6125 | 1.059× | 0.842× | 0.841× |
| Compress::ArithDecode | 2.3360 | 2.3510 | 0.994× | 0.857× | 0.824× |
| Etc::GameOfLife | 1.6470 | 1.4160 | 1.163× | 0.668× | 0.755× |
| CLBG::Spectralnorm | 1.8030 | 1.9065 | 0.946× | 0.912× | 0.825× |
| Template::Parse | 2.5950 | 2.4405 | 1.063× | 0.784× | 0.866× |
| CLBG::Nbody | 1.5360 | 1.5640 | 0.982× | 0.871× | 0.793× |
| Compress::BWTEncode | 2.5405 | 2.1615 | 1.175× | 1.084× | 0.855× |
| Json::ParseMapping | 2.4865 | 2.2345 | 1.113× | 0.877× | 0.860× |
| Graph::AStar | 1.8280 | 1.5890 | 1.150× | 0.829× | 0.806× |
| Sort::Quick | 1.5285 | 1.5210 | 1.005× | 0.964× | 0.798× |
| Json::ParseDom | 2.5235 | 2.2605 | 1.116× | 0.921× | 0.885× |
| Graph::BFS | 1.5885 | 1.3775 | 1.153× | 1.053× | 0.816× |
| Hash::SHA256 | 2.8220 | 1.8380 | 1.535× | 0.974× | 0.876× |
| Calculator::Interpreter | 3.3815 | 2.8840 | 1.173× | 0.909× | 0.923× |
| Matmul::Single | 1.5680 | 1.7250 | 0.909× | 0.926× | 0.884× |
| Json::Generate | 1.9795 | 1.8595 | 1.065× | 0.997× | 0.898× |
| Etc::Words | 2.2770 | 2.2680 | 1.004× | 0.899× | 0.920× |
| Graph::DFS | 1.8910 | 1.5825 | 1.195× | 0.889× | 0.889× |
| Distance::Jaro | 1.6285 | 1.5245 | 1.068× | 0.971× | 0.885× |
| Etc::CacheSimulation | 1.7540 | 1.7570 | 0.998× | 0.889× | 0.901× |
| Sort::Self | 1.4315 | 1.4100 | 1.015× | 0.889× | 0.879× |
| Compress::ArithEncode | 1.7935 | 1.7855 | 1.004× | 1.035× | 0.922× |
| Matmul::T4 | 1.7700 | 1.8545 | 0.954× | 1.036× | 0.926× |
| CSV::Parse | 1.8225 | 1.8420 | 0.989× | 1.094× | 0.930× |
| CLBG::Fannkuchredux | 1.4695 | 1.5120 | 0.972× | 1.209× | 0.950× |
| Binarytrees::Obj | 1.8650 | 1.9345 | 0.964× | 0.854× | 0.980× |
| Brainfuck::Recursion | 1.9545 | 2.0390 | 0.959× | 0.953× | 0.985× |
| Base64::Encode | 2.5190 | 0.2200 | 11.450× | 8.950× | 0.911× |
| Hash::CRC32 | 0.3380 | 0.3235 | 1.045× | 3.196× | 0.983× |
| Binarytrees::Arena | 2.9490 | 2.5240 | 1.168× | 0.959× | 1.066× |
| Template::Regex | 2.2985 | 1.9330 | 1.189× | 0.907× | 1.092× |
| Matmul::T8 | 2.1765 | 1.7070 | 1.275× | 1.103× | 1.243× |
| Base64::Decode | 2.4105 | 1.2380 | 1.947× | 1.508× | 1.435× |
| Matmul::T16 | 3.2000 | 1.8725 | 1.709× | 0.974× | 1.459× |
