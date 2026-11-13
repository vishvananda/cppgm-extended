# Public benchmarks: AMD EPYC

Measured compiler: [`b81455bc7`](https://github.com/vishvananda/cppgm-extended/commit/b81455bc7d42635d12519c11c3788a20b5fad2f4) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/43493408977) · [Raw program results](results.json)

81 of 81 programs built with cppgm++ and g++ and agreed. Each number is the reference over cppgm++, so above 1× means cppgm++ is smaller or faster.

CPU: AMD EPYC 9V74 80-Core Processor.

| Measure | vs g++ | Programs | vs clang++ | Programs |
| --- | ---: | ---: | ---: | ---: |
| Frontend CPU, `-fsyntax-only` | 2.26× | 79 | 2.99× | 79 |
| Compile CPU, `-O0` | 3.01× | 79 | 3.17× | 79 |
| Compile CPU, `-O3` | 4.26× | 79 | 3.96× | 79 |
| Compile peak RSS, `-O0` | 1.50× | 81 | 2.41× | 81 |
| Compile peak RSS, `-O3` | 1.77× | 81 | 2.52× | 81 |
| Executable size, `-O0` | 0.93× | 81 | 0.87× | 81 |
| Executable size, `-O3` | 0.96× | 81 | 0.92× | 81 |
| Run CPU, `-O0` | 1.21× | 81 | 1.24× | 81 |
| Run CPU, `-O3` | 0.98× | 81 | 0.97× | 80 |
| Run instructions, `-O3` | 0.95× | 43 | - | 0 |

O3 execution uses paired ABBA/BAAB blocks. GCC A/A CPU ratios ranged from 0.933 to 1.047; raw observations and block ratios are retained in the artifact.

## Slowest generated code, by run-time instructions

| Program | vs g++ |
| --- | ---: |
| tramp3d-v4 | 0.46× |
| bg/nbody/2 | 0.64× |
| bg/mandelbrot/0 | 0.68× |
| PAQ8p | 0.69× |
| bg/spectralnorm/1 | 0.75× |
| bg/nbody/1 | 0.83× |
| bg/mandelbrot/7 | 0.84× |
| bg/mandelbrot/9 | 0.84× |
| bg/nbody/3 | 0.86× |
| bg/mandelbrot/5 | 0.86× |

## Compile CPU times

Times are CPU seconds, excluding linking. Speed is reference time divided by cppgm++ time; higher is better. Every program is listed. Missing times show a dash. Ratios are omitted for failed programs or when either time is below 10 ms.

Compilation is measured once per compiler and level; the ABBA/BAAB repetitions apply to generated-program execution and self-built compiler speed.

### O0 compilation

| Program | cppgm++ (s) | GCC (s) | Speed vs GCC ↑ | Clang (s) | Speed vs Clang ↑ | Status |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| Adobe-C++/functionobjects | 0.090 | 0.301 | 3.34× | 0.230 | 2.56× | ok |
| Adobe-C++/loop_unroll | 0.070 | 0.622 | 8.89× | 0.248 | 3.54× | ok |
| Adobe-C++/simple_types_constant_folding | 0.070 | 0.710 | 10.14× | 0.305 | 4.36× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.062 | 0.537 | 8.66× | 0.250 | 4.03× | ok |
| Adobe-C++/stepanov_abstraction | 0.048 | 0.211 | 4.40× | 0.186 | 3.88× | ok |
| Adobe-C++/stepanov_vector | 0.094 | 0.356 | 3.79× | 0.267 | 2.84× | ok |
| Bullet | 5.945 | 21.198 | 3.57× | 19.686 | 3.31× | ok |
| CLAMR | 3.311 | 11.166 | 3.37× | 11.525 | 3.48× | ok |
| CoyoteBench/almabench | 0.044 | 0.088 | 2.00× | 0.134 | 3.05× | ok |
| CoyoteBench/fftbench | 0.136 | 0.322 | 2.37× | 0.390 | 2.87× | ok |
| CoyoteBench/huffbench | 0.039 | 0.086 | 2.21× | 0.116 | 2.97× | ok |
| CoyoteBench/lpbench | 0.036 | 0.090 | 2.50× | 0.095 | 2.64× | ok |
| DOE/HACCKernels | 0.137 | 0.325 | 2.37× | 0.425 | 3.10× | ok |
| DOE/HPCCG | 0.810 | 1.917 | 2.37× | 2.560 | 3.16× | ok |
| DOE/PENNANT | 1.958 | 4.638 | 2.37× | 5.224 | 2.67× | ok |
| DOE/miniFE | 0.804 | 2.328 | 2.90× | 3.045 | 3.79× | ok |
| Misc-C++-EH/spirit | 0.491 | 3.365 | 6.85× | 2.179 | 4.44× | ok |
| Misc-C++-Large/ray | 0.144 | 0.326 | 2.26× | 0.365 | 2.53× | ok |
| Misc-C++-Large/sphereflake | 0.116 | 0.259 | 2.23× | 0.347 | 2.99× | ok |
| Misc-C++/bigfib | 0.165 | 0.517 | 3.13× | 0.528 | 3.20× | ok |
| Misc-C++/oopack_v1p8 | 0.040 | 0.112 | 2.80× | 0.152 | 3.80× | ok |
| Misc-C++/stepanov_container | 0.213 | 0.741 | 3.48× | 0.702 | 3.30× | ok |
| Misc-C++/stepanov_v1p2 | 0.038 | 0.154 | 4.05× | 0.114 | 3.00× | ok |
| PAQ8p | 0.085 | 0.522 | 6.14× | 0.314 | 3.69× | ok |
| Prolangs-C++/life | 0.005 | 0.038 | — | 0.035 | — | ok |
| Prolangs-C++/ocean | 0.013 | 0.075 | 5.77× | 0.061 | 4.69× | ok |
| Prolangs-C++/primes | 0.007 | 0.030 | — | 0.036 | — | ok |
| Shootout-C++-EH/except | 0.088 | 0.191 | 2.17× | 0.271 | 3.08× | ok |
| Shootout-C++/ackermann | 0.089 | 0.212 | 2.38× | 0.271 | 3.04× | ok |
| Shootout-C++/ary3 | 0.099 | 0.234 | 2.36× | 0.306 | 3.09× | ok |
| Shootout-C++/fibo | 0.106 | 0.221 | 2.08× | 0.271 | 2.56× | ok |
| Shootout-C++/hash | 0.134 | 0.353 | 2.63× | 0.397 | 2.96× | ok |
| Shootout-C++/hash2 | 0.117 | 0.347 | 2.97× | 0.375 | 3.21× | ok |
| Shootout-C++/heapsort | 0.105 | 0.224 | 2.13× | 0.294 | 2.80× | ok |
| Shootout-C++/lists | 0.097 | 0.239 | 2.46× | 0.303 | 3.12× | ok |
| Shootout-C++/lists1 | 0.115 | 0.281 | 2.44× | 0.327 | 2.84× | ok |
| Shootout-C++/matrix | 0.083 | 0.191 | 2.30× | 0.264 | 3.18× | ok |
| Shootout-C++/methcall | 0.081 | 0.183 | 2.26× | 0.252 | 3.11× | ok |
| Shootout-C++/moments | 0.118 | 0.298 | 2.53× | 0.325 | 2.75× | ok |
| Shootout-C++/random | 0.101 | 0.211 | 2.09× | 0.298 | 2.95× | ok |
| Shootout-C++/sieve | 0.106 | 0.260 | 2.45× | 0.369 | 3.48× | ok |
| SmallPT | 0.036 | 0.108 | 3.00× | 0.101 | 2.81× | ok |
| Stanford/Bubblesort | 0.011 | 0.033 | 3.00× | 0.036 | 3.27× | ok |
| Stanford/FloatMM | 0.011 | 0.033 | 3.00× | 0.036 | 3.27× | ok |
| Stanford/Puzzle | 0.013 | 0.042 | 3.23× | 0.041 | 3.15× | ok |
| Stanford/Treesort | 0.011 | 0.036 | 3.27× | 0.040 | 3.64× | ok |
| bg/binarytrees/2 | 0.083 | 0.188 | 2.27× | 0.259 | 3.12× | ok |
| bg/fannkuchredux/3 | 0.039 | 0.084 | 2.15× | 0.103 | 2.64× | ok |
| bg/fasta/1 | 0.060 | 0.141 | 2.35× | 0.170 | 2.83× | ok |
| bg/fasta/2 | 0.061 | 0.149 | 2.44× | 0.162 | 2.66× | ok |
| bg/fasta/3 | 0.096 | 0.216 | 2.25× | 0.277 | 2.89× | ok |
| bg/fasta/4 | 0.092 | 0.194 | 2.11× | 0.295 | 3.21× | ok |
| bg/fasta/6 | 0.158 | 0.431 | 2.73× | 0.563 | 3.56× | ok |
| bg/mandelbrot/0 | 0.208 | 0.490 | 2.36× | 0.536 | 2.58× | ok |
| bg/mandelbrot/2 | 0.101 | 0.245 | 2.43× | 0.352 | 3.49× | ok |
| bg/mandelbrot/3 | 0.118 | 0.243 | 2.06× | 0.322 | 2.73× | ok |
| bg/mandelbrot/5 | 0.050 | 0.126 | 2.52× | 0.145 | 2.90× | ok |
| bg/mandelbrot/7 | 0.056 | 0.146 | 2.61× | 0.177 | 3.16× | ok |
| bg/mandelbrot/9 | 0.050 | 0.131 | 2.62× | 0.141 | 2.82× | ok |
| bg/nbody/1 | 0.033 | 0.115 | 3.48× | 0.110 | 3.33× | ok |
| bg/nbody/2 | 0.033 | 0.077 | 2.33× | 0.092 | 2.79× | ok |
| bg/nbody/3 | 0.035 | 0.102 | 2.91× | 0.096 | 2.74× | ok |
| bg/nbody/6 | 0.036 | 0.114 | 3.17× | 0.124 | 3.44× | ok |
| bg/pidigits/4 | 0.143 | 0.359 | 2.51× | 0.444 | 3.10× | ok |
| bg/pidigits/5 | 0.103 | 0.199 | 1.93× | 0.283 | 2.75× | ok |
| bg/revcomp/1 | 0.099 | 0.242 | 2.44× | 0.358 | 3.62× | ok |
| bg/revcomp/5 | 0.085 | 0.215 | 2.53× | 0.353 | 4.15× | ok |
| bg/spectralnorm/1 | 0.133 | 0.288 | 2.17× | 0.417 | 3.14× | ok |
| box2d | 2.080 | 5.039 | 2.42× | 6.249 | 3.00× | ok |
| fmt | 0.663 | 2.739 | 4.13× | 2.446 | 3.69× | ok |
| hexxagon | 0.307 | 0.795 | 2.59× | 1.101 | 3.59× | ok |
| lambda | 0.070 | 0.366 | 5.23× | 0.284 | 4.06× | ok |
| minisat | 0.083 | 0.348 | 4.19× | 0.274 | 3.30× | ok |
| nlohmann-json | 0.437 | 2.171 | 4.97× | 2.257 | 5.16× | ok |
| pugixml | 0.356 | 1.508 | 4.24× | 1.258 | 3.53× | ok |
| pugixml-tests | 4.651 | 16.868 | 3.63× | 14.480 | 3.11× | ok |
| rapidjson | 0.154 | 0.538 | 3.49× | 0.436 | 2.83× | ok |
| re2 | 4.325 | 12.244 | 2.83× | 8.762 | 2.03× | ok |
| re2-test-re2 | 6.017 | 18.015 | 2.99× | 19.675 | 3.27× | ok |
| re2-test-search | 5.742 | 16.465 | 2.87× | 12.124 | 2.11× | ok |
| tramp3d-v4 | 0.815 | 9.283 | 11.39× | 3.373 | 4.14× | ok |

### O3 compilation

| Program | cppgm++ (s) | GCC (s) | Speed vs GCC ↑ | Clang (s) | Speed vs Clang ↑ | Status |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| Adobe-C++/functionobjects | 0.145 | 1.156 | 7.97× | 0.642 | 4.43× | ok |
| Adobe-C++/loop_unroll | 0.420 | 6.484 | 15.44× | 6.045 | 14.39× | ok |
| Adobe-C++/simple_types_constant_folding | 0.236 | 3.248 | 13.76× | 2.853 | 12.09× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.214 | 2.740 | 12.80× | 2.181 | 10.19× | ok |
| Adobe-C++/stepanov_abstraction | 0.105 | 1.193 | 11.36× | 0.612 | 5.83× | ok |
| Adobe-C++/stepanov_vector | 0.135 | 1.775 | 13.15× | 0.639 | 4.73× | ok |
| Bullet | 7.403 | 47.510 | 6.42× | 31.597 | 4.27× | ok |
| CLAMR | 5.488 | 28.008 | 5.10× | 22.579 | 4.11× | ok |
| CoyoteBench/almabench | 0.048 | 0.184 | 3.83× | 0.141 | 2.94× | ok |
| CoyoteBench/fftbench | 0.156 | 0.499 | 3.20× | 0.519 | 3.33× | ok |
| CoyoteBench/huffbench | 0.045 | 0.252 | 5.60× | 0.190 | 4.22× | ok |
| CoyoteBench/lpbench | 0.042 | 0.203 | 4.83× | 0.230 | 5.48× | ok |
| DOE/HACCKernels | 0.149 | 0.446 | 2.99× | 0.483 | 3.24× | ok |
| DOE/HPCCG | 0.861 | 2.740 | 3.18× | 2.919 | 3.39× | ok |
| DOE/PENNANT | 2.091 | 7.462 | 3.57× | 7.050 | 3.37× | ok |
| DOE/miniFE | 1.067 | 4.442 | 4.16× | 3.766 | 3.53× | ok |
| Misc-C++-EH/spirit | 0.839 | 4.704 | 5.61× | 3.773 | 4.50× | ok |
| Misc-C++-Large/ray | 0.135 | 0.589 | 4.36× | 0.543 | 4.02× | ok |
| Misc-C++-Large/sphereflake | 0.135 | 0.824 | 6.10× | 0.469 | 3.47× | ok |
| Misc-C++/bigfib | 0.202 | 0.674 | 3.34× | 0.650 | 3.22× | ok |
| Misc-C++/oopack_v1p8 | 0.048 | 0.204 | 4.25× | 0.243 | 5.06× | ok |
| Misc-C++/stepanov_container | 0.284 | 1.080 | 3.80× | 1.089 | 3.83× | ok |
| Misc-C++/stepanov_v1p2 | 0.049 | 0.242 | 4.94× | 0.232 | 4.73× | ok |
| PAQ8p | 0.320 | 4.486 | 14.02× | 2.123 | 6.63× | ok |
| Prolangs-C++/life | 0.009 | 0.117 | — | 0.084 | — | ok |
| Prolangs-C++/ocean | 0.026 | 0.223 | 8.58× | 0.138 | 5.31× | ok |
| Prolangs-C++/primes | 0.009 | 0.041 | — | 0.044 | — | ok |
| Shootout-C++-EH/except | 0.088 | 0.227 | 2.58× | 0.298 | 3.39× | ok |
| Shootout-C++/ackermann | 0.094 | 0.262 | 2.79× | 0.340 | 3.62× | ok |
| Shootout-C++/ary3 | 0.108 | 0.272 | 2.52× | 0.364 | 3.37× | ok |
| Shootout-C++/fibo | 0.091 | 0.274 | 3.01× | 0.276 | 3.03× | ok |
| Shootout-C++/hash | 0.172 | 0.461 | 2.68× | 0.506 | 2.94× | ok |
| Shootout-C++/hash2 | 0.143 | 0.451 | 3.15× | 0.500 | 3.50× | ok |
| Shootout-C++/heapsort | 0.108 | 0.281 | 2.60× | 0.312 | 2.89× | ok |
| Shootout-C++/lists | 0.105 | 0.277 | 2.64× | 0.324 | 3.09× | ok |
| Shootout-C++/lists1 | 0.123 | 0.327 | 2.66× | 0.374 | 3.04× | ok |
| Shootout-C++/matrix | 0.092 | 0.236 | 2.57× | 0.357 | 3.88× | ok |
| Shootout-C++/methcall | 0.087 | 0.219 | 2.52× | 0.275 | 3.16× | ok |
| Shootout-C++/moments | 0.127 | 0.361 | 2.84× | 0.388 | 3.06× | ok |
| Shootout-C++/random | 0.106 | 0.258 | 2.43× | 0.309 | 2.92× | ok |
| Shootout-C++/sieve | 0.104 | 0.254 | 2.44× | 0.361 | 3.47× | ok |
| SmallPT | 0.045 | 0.361 | 8.02× | 0.187 | 4.16× | ok |
| Stanford/Bubblesort | 0.013 | 0.062 | 4.77× | 0.054 | 4.15× | ok |
| Stanford/FloatMM | 0.013 | 0.070 | 5.38× | 0.076 | 5.85× | ok |
| Stanford/Puzzle | 0.020 | 0.137 | 6.85× | 0.130 | 6.50× | ok |
| Stanford/Treesort | 0.016 | 0.094 | 5.88× | 0.061 | 3.81× | ok |
| bg/binarytrees/2 | 0.086 | 0.459 | 5.34× | 0.276 | 3.21× | ok |
| bg/fannkuchredux/3 | 0.044 | 0.135 | 3.07× | 0.137 | 3.11× | ok |
| bg/fasta/1 | 0.074 | 0.174 | 2.35× | 0.213 | 2.88× | ok |
| bg/fasta/2 | 0.067 | 0.188 | 2.81× | 0.203 | 3.03× | ok |
| bg/fasta/3 | 0.098 | 0.246 | 2.51× | 0.342 | 3.49× | ok |
| bg/fasta/4 | 0.097 | 0.213 | 2.20× | 0.260 | 2.68× | ok |
| bg/fasta/6 | 0.167 | 0.522 | 3.13× | 0.640 | 3.83× | ok |
| bg/mandelbrot/0 | 0.211 | 0.724 | 3.43× | 0.773 | 3.66× | ok |
| bg/mandelbrot/2 | 0.109 | 0.301 | 2.76× | 0.372 | 3.41× | ok |
| bg/mandelbrot/3 | 0.118 | 0.267 | 2.26× | 0.332 | 2.81× | ok |
| bg/mandelbrot/5 | 0.053 | 0.184 | 3.47× | 0.178 | 3.36× | ok |
| bg/mandelbrot/7 | 0.062 | 0.215 | 3.47× | 0.202 | 3.26× | ok |
| bg/mandelbrot/9 | 0.060 | 0.202 | 3.37× | 0.234 | 3.90× | ok |
| bg/nbody/1 | 0.045 | 0.171 | 3.80× | 0.213 | 4.73× | ok |
| bg/nbody/2 | 0.036 | 0.157 | 4.36× | 0.143 | 3.97× | ok |
| bg/nbody/3 | 0.039 | 0.135 | 3.46× | 0.127 | 3.26× | ok |
| bg/nbody/6 | 0.048 | 0.193 | 4.02× | 0.193 | 4.02× | ok |
| bg/pidigits/4 | 0.144 | 0.400 | 2.78× | 0.489 | 3.40× | ok |
| bg/pidigits/5 | 0.091 | 0.233 | 2.56× | 0.334 | 3.67× | ok |
| bg/revcomp/1 | 0.103 | 0.294 | 2.85× | 0.337 | 3.27× | ok |
| bg/revcomp/5 | 0.090 | 0.249 | 2.77× | 0.389 | 4.32× | ok |
| bg/spectralnorm/1 | 0.141 | 0.401 | 2.84× | 0.412 | 2.92× | ok |
| box2d | 2.261 | 7.966 | 3.52× | 8.651 | 3.83× | ok |
| fmt | 1.058 | 5.667 | 5.36× | 4.535 | 4.29× | ok |
| hexxagon | 0.372 | 1.307 | 3.51× | 1.456 | 3.91× | ok |
| lambda | 0.141 | 1.290 | 9.15× | 0.583 | 4.13× | ok |
| minisat | 0.164 | 1.188 | 7.24× | 0.671 | 4.09× | ok |
| nlohmann-json | 0.763 | 3.394 | 4.45× | 3.068 | 4.02× | ok |
| pugixml | 0.685 | 6.220 | 9.08× | 4.306 | 6.29× | ok |
| pugixml-tests | 6.193 | 40.166 | 6.49× | 28.656 | 4.63× | ok |
| rapidjson | 0.240 | 1.378 | 5.74× | 0.931 | 3.88× | ok |
| re2 | 3.999 | 20.229 | 5.06× | 12.567 | 3.14× | ok |
| re2-test-re2 | 6.987 | 31.139 | 4.46× | 27.705 | 3.97× | ok |
| re2-test-search | 6.665 | 28.387 | 4.26× | 17.685 | 2.65× | ok |
| tramp3d-v4 | 2.424 | 19.120 | 7.89× | 16.602 | 6.85× | ok |

### Frontend only: -fsyntax-only

| Program | cppgm++ (s) | GCC (s) | Speed vs GCC ↑ | Clang (s) | Speed vs Clang ↑ | Status |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| Adobe-C++/functionobjects | 0.083 | 0.174 | 2.10× | 0.214 | 2.58× | ok |
| Adobe-C++/loop_unroll | 0.050 | 0.129 | 2.58× | 0.146 | 2.92× | ok |
| Adobe-C++/simple_types_constant_folding | 0.049 | 0.122 | 2.49× | 0.153 | 3.12× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.047 | 0.120 | 2.55× | 0.136 | 2.89× | ok |
| Adobe-C++/stepanov_abstraction | 0.040 | 0.087 | 2.17× | 0.137 | 3.43× | ok |
| Adobe-C++/stepanov_vector | 0.086 | 0.166 | 1.93× | 0.214 | 2.49× | ok |
| Bullet | 5.994 | 14.364 | 2.40× | 16.395 | 2.74× | ok |
| CLAMR | 3.013 | 6.603 | 2.19× | 9.585 | 3.18× | ok |
| CoyoteBench/almabench | 0.043 | 0.073 | 1.70× | 0.112 | 2.60× | ok |
| CoyoteBench/fftbench | 0.134 | 0.285 | 2.13× | 0.389 | 2.90× | ok |
| CoyoteBench/huffbench | 0.039 | 0.074 | 1.90× | 0.123 | 3.15× | ok |
| CoyoteBench/lpbench | 0.036 | 0.074 | 2.06× | 0.086 | 2.39× | ok |
| DOE/HACCKernels | 0.132 | 0.294 | 2.23× | 0.391 | 2.96× | ok |
| DOE/HPCCG | 0.786 | 1.871 | 2.38× | 2.326 | 2.96× | ok |
| DOE/PENNANT | 1.773 | 3.674 | 2.07× | 5.068 | 2.86× | ok |
| DOE/miniFE | 0.790 | 1.622 | 2.05× | 2.821 | 3.57× | ok |
| Misc-C++-EH/spirit | 0.440 | 1.246 | 2.83× | 1.406 | 3.20× | ok |
| Misc-C++-Large/ray | 0.136 | 0.279 | 2.05× | 0.344 | 2.53× | ok |
| Misc-C++-Large/sphereflake | 0.113 | 0.234 | 2.07× | 0.327 | 2.89× | ok |
| Misc-C++/bigfib | 0.155 | 0.361 | 2.33× | 0.460 | 2.97× | ok |
| Misc-C++/oopack_v1p8 | 0.040 | 0.081 | 2.02× | 0.146 | 3.65× | ok |
| Misc-C++/stepanov_container | 0.201 | 0.469 | 2.33× | 0.591 | 2.94× | ok |
| Misc-C++/stepanov_v1p2 | 0.035 | 0.092 | 2.63× | 0.096 | 2.74× | ok |
| PAQ8p | 0.058 | 0.149 | 2.57× | 0.189 | 3.26× | ok |
| Prolangs-C++/life | 0.004 | 0.013 | — | 0.026 | — | ok |
| Prolangs-C++/ocean | 0.011 | 0.028 | 2.55× | 0.041 | 3.73× | ok |
| Prolangs-C++/primes | 0.007 | 0.022 | — | 0.032 | — | ok |
| Shootout-C++-EH/except | 0.088 | 0.192 | 2.18× | 0.253 | 2.88× | ok |
| Shootout-C++/ackermann | 0.089 | 0.218 | 2.45× | 0.262 | 2.94× | ok |
| Shootout-C++/ary3 | 0.097 | 0.212 | 2.19× | 0.293 | 3.02× | ok |
| Shootout-C++/fibo | 0.098 | 0.227 | 2.32× | 0.255 | 2.60× | ok |
| Shootout-C++/hash | 0.128 | 0.270 | 2.11× | 0.360 | 2.81× | ok |
| Shootout-C++/hash2 | 0.110 | 0.264 | 2.40× | 0.335 | 3.05× | ok |
| Shootout-C++/heapsort | 0.109 | 0.221 | 2.03× | 0.279 | 2.56× | ok |
| Shootout-C++/lists | 0.093 | 0.208 | 2.24× | 0.276 | 2.97× | ok |
| Shootout-C++/lists1 | 0.110 | 0.230 | 2.09× | 0.299 | 2.72× | ok |
| Shootout-C++/matrix | 0.083 | 0.190 | 2.29× | 0.247 | 2.98× | ok |
| Shootout-C++/methcall | 0.083 | 0.182 | 2.19× | 0.240 | 2.89× | ok |
| Shootout-C++/moments | 0.113 | 0.240 | 2.12× | 0.295 | 2.61× | ok |
| Shootout-C++/random | 0.102 | 0.213 | 2.09× | 0.287 | 2.81× | ok |
| Shootout-C++/sieve | 0.102 | 0.214 | 2.10× | 0.303 | 2.97× | ok |
| SmallPT | 0.034 | 0.067 | 1.97× | 0.093 | 2.74× | ok |
| Stanford/Bubblesort | 0.011 | 0.029 | 2.64× | 0.036 | 3.27× | ok |
| Stanford/FloatMM | 0.011 | 0.028 | 2.55× | 0.034 | 3.09× | ok |
| Stanford/Puzzle | 0.012 | 0.030 | 2.50× | 0.037 | 3.08× | ok |
| Stanford/Treesort | 0.011 | 0.029 | 2.64× | 0.037 | 3.36× | ok |
| bg/binarytrees/2 | 0.082 | 0.183 | 2.23× | 0.263 | 3.21× | ok |
| bg/fannkuchredux/3 | 0.039 | 0.073 | 1.87× | 0.096 | 2.46× | ok |
| bg/fasta/1 | 0.059 | 0.114 | 1.93× | 0.157 | 2.66× | ok |
| bg/fasta/2 | 0.058 | 0.119 | 2.05× | 0.149 | 2.57× | ok |
| bg/fasta/3 | 0.095 | 0.205 | 2.16× | 0.259 | 2.73× | ok |
| bg/fasta/4 | 0.088 | 0.167 | 1.90× | 0.286 | 3.25× | ok |
| bg/fasta/6 | 0.139 | 0.315 | 2.27× | 0.532 | 3.83× | ok |
| bg/mandelbrot/0 | 0.170 | 0.355 | 2.09× | 0.471 | 2.77× | ok |
| bg/mandelbrot/2 | 0.099 | 0.233 | 2.35× | 0.319 | 3.22× | ok |
| bg/mandelbrot/3 | 0.120 | 0.247 | 2.06× | 0.309 | 2.58× | ok |
| bg/mandelbrot/5 | 0.048 | 0.105 | 2.19× | 0.133 | 2.77× | ok |
| bg/mandelbrot/7 | 0.053 | 0.108 | 2.04× | 0.143 | 2.70× | ok |
| bg/mandelbrot/9 | 0.048 | 0.097 | 2.02× | 0.127 | 2.65× | ok |
| bg/nbody/1 | 0.032 | 0.070 | 2.19× | 0.090 | 2.81× | ok |
| bg/nbody/2 | 0.032 | 0.065 | 2.03× | 0.086 | 2.69× | ok |
| bg/nbody/3 | 0.034 | 0.087 | 2.56× | 0.090 | 2.65× | ok |
| bg/nbody/6 | 0.034 | 0.069 | 2.03× | 0.099 | 2.91× | ok |
| bg/pidigits/4 | 0.136 | 0.325 | 2.39× | 0.410 | 3.01× | ok |
| bg/pidigits/5 | 0.090 | 0.193 | 2.14× | 0.263 | 2.92× | ok |
| bg/revcomp/1 | 0.095 | 0.197 | 2.07× | 0.312 | 3.28× | ok |
| bg/revcomp/5 | 0.081 | 0.181 | 2.23× | 0.321 | 3.96× | ok |
| bg/spectralnorm/1 | 0.136 | 0.321 | 2.36× | 0.395 | 2.90× | ok |
| box2d | 1.706 | 3.636 | 2.13× | 5.544 | 3.25× | ok |
| fmt | 0.495 | 1.282 | 2.59× | 1.648 | 3.33× | ok |
| hexxagon | 0.322 | 0.696 | 2.16× | 1.033 | 3.21× | ok |
| lambda | 0.054 | 0.154 | 2.85× | 0.230 | 4.26× | ok |
| minisat | 0.072 | 0.176 | 2.44× | 0.208 | 2.89× | ok |
| nlohmann-json | 0.354 | 1.152 | 3.25× | 1.712 | 4.84× | ok |
| pugixml | 0.254 | 0.608 | 2.39× | 0.924 | 3.64× | ok |
| pugixml-tests | 3.880 | 8.543 | 2.20× | 12.670 | 3.27× | ok |
| rapidjson | 0.128 | 0.302 | 2.36× | 0.396 | 3.09× | ok |
| re2 | 3.661 | 8.057 | 2.20× | 7.389 | 2.02× | ok |
| re2-test-re2 | 5.386 | 11.634 | 2.16× | 16.928 | 3.14× | ok |
| re2-test-search | 5.385 | 11.639 | 2.16× | 16.515 | 3.07× | ok |
| tramp3d-v4 | 0.539 | 2.830 | 5.25× | 2.028 | 3.76× | ok |


## Toolchains

- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)
- **valgrind**: valgrind-3.26.0

[Compiler build and codegen results](self-build.md) have their own measured revision.
