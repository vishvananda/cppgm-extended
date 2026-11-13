# Public benchmarks: AMD EPYC

Measured compiler: [`72d294c91`](https://github.com/vishvananda/cppgm-extended/commit/72d294c916f0f3f43e50e812f7efa91937eb9b66) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/43348824438) · [Raw program results](results.json)

81 of 81 programs built with cppgm++ and g++ and agreed. Each number is the reference over cppgm++, so above 1× means cppgm++ is smaller or faster.

CPU: AMD EPYC 9V74 80-Core Processor.

| Measure | vs g++ | Programs | vs clang++ | Programs |
| --- | ---: | ---: | ---: | ---: |
| Frontend CPU, `-fsyntax-only` | 2.25× | 79 | 2.97× | 79 |
| Compile CPU, `-O0` | 3.07× | 79 | 3.21× | 79 |
| Compile CPU, `-O3` | 4.30× | 79 | 3.95× | 79 |
| Compile peak RSS, `-O0` | 1.51× | 81 | 2.45× | 81 |
| Compile peak RSS, `-O3` | 1.78× | 81 | 2.55× | 81 |
| Executable size, `-O0` | 0.93× | 81 | 0.87× | 81 |
| Executable size, `-O3` | 0.96× | 81 | 0.92× | 81 |
| Run CPU, `-O0` | 1.21× | 81 | 1.25× | 81 |
| Run CPU, `-O3` | 0.98× | 81 | 0.97× | 80 |
| Run instructions, `-O3` | 0.95× | 43 | - | 0 |

O3 execution uses paired ABBA/BAAB blocks. GCC A/A CPU ratios ranged from 0.909 to 1.139; raw observations and block ratios are retained in the artifact.

## Slowest generated code, by run-time instructions

| Program | vs g++ |
| --- | ---: |
| tramp3d-v4 | 0.46× |
| bg/nbody/2 | 0.64× |
| PAQ8p | 0.68× |
| bg/mandelbrot/0 | 0.68× |
| bg/spectralnorm/1 | 0.75× |
| Misc-C++/oopack_v1p8 | 0.77× |
| bg/nbody/1 | 0.83× |
| bg/mandelbrot/7 | 0.84× |
| bg/mandelbrot/9 | 0.84× |
| bg/nbody/3 | 0.86× |

## Compile CPU times

Times are CPU seconds, excluding linking. Speed is reference time divided by cppgm++ time; higher is better. Every program is listed. Missing times show a dash. Ratios are omitted for failed programs or when either time is below 10 ms.

Compilation is measured once per compiler and level; the ABBA/BAAB repetitions apply to generated-program execution and self-built compiler speed.

### O0 compilation

| Program | cppgm++ (s) | GCC (s) | Speed vs GCC ↑ | Clang (s) | Speed vs Clang ↑ | Status |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| Adobe-C++/functionobjects | 0.084 | 0.294 | 3.50× | 0.224 | 2.67× | ok |
| Adobe-C++/loop_unroll | 0.068 | 0.626 | 9.21× | 0.230 | 3.38× | ok |
| Adobe-C++/simple_types_constant_folding | 0.066 | 0.725 | 10.98× | 0.297 | 4.50× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.059 | 0.514 | 8.71× | 0.243 | 4.12× | ok |
| Adobe-C++/stepanov_abstraction | 0.051 | 0.204 | 4.00× | 0.139 | 2.73× | ok |
| Adobe-C++/stepanov_vector | 0.098 | 0.342 | 3.49× | 0.278 | 2.84× | ok |
| Bullet | 5.953 | 21.239 | 3.57× | 19.790 | 3.32× | ok |
| CLAMR | 2.935 | 10.965 | 3.74× | 9.382 | 3.20× | ok |
| CoyoteBench/almabench | 0.035 | 0.095 | 2.71× | 0.094 | 2.69× | ok |
| CoyoteBench/fftbench | 0.142 | 0.293 | 2.06× | 0.372 | 2.62× | ok |
| CoyoteBench/huffbench | 0.035 | 0.092 | 2.63× | 0.124 | 3.54× | ok |
| CoyoteBench/lpbench | 0.041 | 0.083 | 2.02× | 0.126 | 3.07× | ok |
| DOE/HACCKernels | 0.130 | 0.345 | 2.65× | 0.391 | 3.01× | ok |
| DOE/HPCCG | 0.873 | 1.890 | 2.16× | 3.112 | 3.56× | ok |
| DOE/PENNANT | 1.809 | 4.798 | 2.65× | 5.064 | 2.80× | ok |
| DOE/miniFE | 0.833 | 2.523 | 3.03× | 2.894 | 3.47× | ok |
| Misc-C++-EH/spirit | 0.559 | 3.105 | 5.55× | 2.554 | 4.57× | ok |
| Misc-C++-Large/ray | 0.134 | 0.338 | 2.52× | 0.426 | 3.18× | ok |
| Misc-C++-Large/sphereflake | 0.108 | 0.274 | 2.54× | 0.349 | 3.23× | ok |
| Misc-C++/bigfib | 0.134 | 0.413 | 3.08× | 0.487 | 3.63× | ok |
| Misc-C++/oopack_v1p8 | 0.039 | 0.110 | 2.82× | 0.118 | 3.03× | ok |
| Misc-C++/stepanov_container | 0.200 | 0.683 | 3.42× | 0.710 | 3.55× | ok |
| Misc-C++/stepanov_v1p2 | 0.038 | 0.134 | 3.53× | 0.121 | 3.18× | ok |
| PAQ8p | 0.078 | 0.515 | 6.60× | 0.280 | 3.59× | ok |
| Prolangs-C++/life | 0.004 | 0.036 | — | 0.033 | — | ok |
| Prolangs-C++/ocean | 0.013 | 0.073 | 5.62× | 0.058 | 4.46× | ok |
| Prolangs-C++/primes | 0.007 | 0.028 | — | 0.033 | — | ok |
| Shootout-C++-EH/except | 0.091 | 0.220 | 2.42× | 0.297 | 3.26× | ok |
| Shootout-C++/ackermann | 0.084 | 0.185 | 2.20× | 0.253 | 3.01× | ok |
| Shootout-C++/ary3 | 0.119 | 0.235 | 1.97× | 0.304 | 2.55× | ok |
| Shootout-C++/fibo | 0.087 | 0.181 | 2.08× | 0.255 | 2.93× | ok |
| Shootout-C++/hash | 0.126 | 0.338 | 2.68× | 0.350 | 2.78× | ok |
| Shootout-C++/hash2 | 0.124 | 0.311 | 2.51× | 0.378 | 3.05× | ok |
| Shootout-C++/heapsort | 0.105 | 0.229 | 2.18× | 0.294 | 2.80× | ok |
| Shootout-C++/lists | 0.090 | 0.252 | 2.80× | 0.346 | 3.84× | ok |
| Shootout-C++/lists1 | 0.108 | 0.262 | 2.43× | 0.312 | 2.89× | ok |
| Shootout-C++/matrix | 0.081 | 0.170 | 2.10× | 0.244 | 3.01× | ok |
| Shootout-C++/methcall | 0.090 | 0.222 | 2.47× | 0.234 | 2.60× | ok |
| Shootout-C++/moments | 0.113 | 0.288 | 2.55× | 0.323 | 2.86× | ok |
| Shootout-C++/random | 0.102 | 0.212 | 2.08× | 0.299 | 2.93× | ok |
| Shootout-C++/sieve | 0.098 | 0.237 | 2.42× | 0.304 | 3.10× | ok |
| SmallPT | 0.034 | 0.108 | 3.18× | 0.110 | 3.24× | ok |
| Stanford/Bubblesort | 0.011 | 0.032 | 2.91× | 0.037 | 3.36× | ok |
| Stanford/FloatMM | 0.011 | 0.034 | 3.09× | 0.039 | 3.55× | ok |
| Stanford/Puzzle | 0.012 | 0.045 | 3.75× | 0.042 | 3.50× | ok |
| Stanford/Treesort | 0.011 | 0.036 | 3.27× | 0.043 | 3.91× | ok |
| bg/binarytrees/2 | 0.082 | 0.184 | 2.24× | 0.252 | 3.07× | ok |
| bg/fannkuchredux/3 | 0.043 | 0.085 | 1.98× | 0.113 | 2.63× | ok |
| bg/fasta/1 | 0.063 | 0.151 | 2.40× | 0.171 | 2.71× | ok |
| bg/fasta/2 | 0.062 | 0.141 | 2.27× | 0.170 | 2.74× | ok |
| bg/fasta/3 | 0.094 | 0.216 | 2.30× | 0.277 | 2.95× | ok |
| bg/fasta/4 | 0.086 | 0.178 | 2.07× | 0.261 | 3.03× | ok |
| bg/fasta/6 | 0.151 | 0.432 | 2.86× | 0.506 | 3.35× | ok |
| bg/mandelbrot/0 | 0.170 | 0.533 | 3.14× | 0.496 | 2.92× | ok |
| bg/mandelbrot/2 | 0.099 | 0.285 | 2.88× | 0.361 | 3.65× | ok |
| bg/mandelbrot/3 | 0.113 | 0.241 | 2.13× | 0.348 | 3.08× | ok |
| bg/mandelbrot/5 | 0.052 | 0.150 | 2.88× | 0.144 | 2.77× | ok |
| bg/mandelbrot/7 | 0.055 | 0.159 | 2.89× | 0.172 | 3.13× | ok |
| bg/mandelbrot/9 | 0.050 | 0.131 | 2.62× | 0.150 | 3.00× | ok |
| bg/nbody/1 | 0.035 | 0.130 | 3.71× | 0.106 | 3.03× | ok |
| bg/nbody/2 | 0.034 | 0.075 | 2.21× | 0.099 | 2.91× | ok |
| bg/nbody/3 | 0.034 | 0.081 | 2.38× | 0.098 | 2.88× | ok |
| bg/nbody/6 | 0.036 | 0.118 | 3.28× | 0.106 | 2.94× | ok |
| bg/pidigits/4 | 0.144 | 0.353 | 2.45× | 0.475 | 3.30× | ok |
| bg/pidigits/5 | 0.090 | 0.201 | 2.23× | 0.279 | 3.10× | ok |
| bg/revcomp/1 | 0.100 | 0.243 | 2.43× | 0.289 | 2.89× | ok |
| bg/revcomp/5 | 0.100 | 0.231 | 2.31× | 0.370 | 3.70× | ok |
| bg/spectralnorm/1 | 0.131 | 0.298 | 2.27× | 0.418 | 3.19× | ok |
| box2d | 1.797 | 4.898 | 2.73× | 6.418 | 3.57× | ok |
| fmt | 0.602 | 2.749 | 4.57× | 2.589 | 4.30× | ok |
| hexxagon | 0.327 | 0.757 | 2.31× | 1.117 | 3.42× | ok |
| lambda | 0.072 | 0.359 | 4.99× | 0.297 | 4.12× | ok |
| minisat | 0.082 | 0.354 | 4.32× | 0.277 | 3.38× | ok |
| nlohmann-json | 0.436 | 2.385 | 5.47× | 1.861 | 4.27× | ok |
| pugixml | 0.302 | 1.563 | 5.18× | 1.279 | 4.24× | ok |
| pugixml-tests | 4.542 | 17.611 | 3.88× | 17.288 | 3.81× | ok |
| rapidjson | 0.146 | 0.547 | 3.75× | 0.468 | 3.21× | ok |
| re2 | 3.750 | 12.339 | 3.29× | 8.180 | 2.18× | ok |
| re2-test-re2 | 5.772 | 18.190 | 3.15× | 19.707 | 3.41× | ok |
| re2-test-search | 5.773 | 16.896 | 2.93× | 14.573 | 2.52× | ok |
| tramp3d-v4 | 0.828 | 9.774 | 11.80× | 3.010 | 3.64× | ok |

### O3 compilation

| Program | cppgm++ (s) | GCC (s) | Speed vs GCC ↑ | Clang (s) | Speed vs Clang ↑ | Status |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| Adobe-C++/functionobjects | 0.137 | 1.112 | 8.12× | 0.571 | 4.17× | ok |
| Adobe-C++/loop_unroll | 0.349 | 6.355 | 18.21× | 5.061 | 14.50× | ok |
| Adobe-C++/simple_types_constant_folding | 0.224 | 3.119 | 13.92× | 2.712 | 12.11× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.193 | 2.584 | 13.39× | 2.101 | 10.89× | ok |
| Adobe-C++/stepanov_abstraction | 0.092 | 1.286 | 13.98× | 0.667 | 7.25× | ok |
| Adobe-C++/stepanov_vector | 0.126 | 1.685 | 13.37× | 0.706 | 5.60× | ok |
| Bullet | 7.235 | 41.685 | 5.76× | 32.674 | 4.52× | ok |
| CLAMR | 4.867 | 26.487 | 5.44× | 19.519 | 4.01× | ok |
| CoyoteBench/almabench | 0.039 | 0.194 | 4.97× | 0.135 | 3.46× | ok |
| CoyoteBench/fftbench | 0.157 | 0.483 | 3.08× | 0.489 | 3.11× | ok |
| CoyoteBench/huffbench | 0.044 | 0.245 | 5.57× | 0.152 | 3.45× | ok |
| CoyoteBench/lpbench | 0.047 | 0.225 | 4.79× | 0.203 | 4.32× | ok |
| DOE/HACCKernels | 0.139 | 0.427 | 3.07× | 0.450 | 3.24× | ok |
| DOE/HPCCG | 0.864 | 2.678 | 3.10× | 3.343 | 3.87× | ok |
| DOE/PENNANT | 2.098 | 6.945 | 3.31× | 7.057 | 3.36× | ok |
| DOE/miniFE | 0.968 | 4.353 | 4.50× | 4.099 | 4.23× | ok |
| Misc-C++-EH/spirit | 0.767 | 4.151 | 5.41× | 4.092 | 5.34× | ok |
| Misc-C++-Large/ray | 0.156 | 0.582 | 3.73× | 0.550 | 3.53× | ok |
| Misc-C++-Large/sphereflake | 0.135 | 0.896 | 6.64× | 0.451 | 3.34× | ok |
| Misc-C++/bigfib | 0.160 | 0.575 | 3.59× | 0.624 | 3.90× | ok |
| Misc-C++/oopack_v1p8 | 0.046 | 0.204 | 4.43× | 0.191 | 4.15× | ok |
| Misc-C++/stepanov_container | 0.284 | 1.035 | 3.64× | 0.980 | 3.45× | ok |
| Misc-C++/stepanov_v1p2 | 0.047 | 0.272 | 5.79× | 0.210 | 4.47× | ok |
| PAQ8p | 0.307 | 4.297 | 14.00× | 1.863 | 6.07× | ok |
| Prolangs-C++/life | 0.009 | 0.110 | — | 0.073 | — | ok |
| Prolangs-C++/ocean | 0.023 | 0.217 | 9.43× | 0.130 | 5.65× | ok |
| Prolangs-C++/primes | 0.008 | 0.041 | — | 0.045 | — | ok |
| Shootout-C++-EH/except | 0.097 | 0.253 | 2.61× | 0.314 | 3.24× | ok |
| Shootout-C++/ackermann | 0.087 | 0.229 | 2.63× | 0.266 | 3.06× | ok |
| Shootout-C++/ary3 | 0.110 | 0.329 | 2.99× | 0.360 | 3.27× | ok |
| Shootout-C++/fibo | 0.090 | 0.231 | 2.57× | 0.312 | 3.47× | ok |
| Shootout-C++/hash | 0.143 | 0.440 | 3.08× | 0.437 | 3.06× | ok |
| Shootout-C++/hash2 | 0.134 | 0.472 | 3.52× | 0.526 | 3.93× | ok |
| Shootout-C++/heapsort | 0.107 | 0.257 | 2.40× | 0.315 | 2.94× | ok |
| Shootout-C++/lists | 0.104 | 0.266 | 2.56× | 0.319 | 3.07× | ok |
| Shootout-C++/lists1 | 0.119 | 0.306 | 2.57× | 0.342 | 2.87× | ok |
| Shootout-C++/matrix | 0.086 | 0.228 | 2.65× | 0.345 | 4.01× | ok |
| Shootout-C++/methcall | 0.099 | 0.219 | 2.21× | 0.268 | 2.71× | ok |
| Shootout-C++/moments | 0.128 | 0.348 | 2.72× | 0.385 | 3.01× | ok |
| Shootout-C++/random | 0.118 | 0.241 | 2.04× | 0.320 | 2.71× | ok |
| Shootout-C++/sieve | 0.119 | 0.265 | 2.23× | 0.357 | 3.00× | ok |
| SmallPT | 0.048 | 0.332 | 6.92× | 0.186 | 3.88× | ok |
| Stanford/Bubblesort | 0.013 | 0.061 | 4.69× | 0.055 | 4.23× | ok |
| Stanford/FloatMM | 0.014 | 0.069 | 4.93× | 0.081 | 5.79× | ok |
| Stanford/Puzzle | 0.019 | 0.137 | 7.21× | 0.128 | 6.74× | ok |
| Stanford/Treesort | 0.016 | 0.094 | 5.88× | 0.064 | 4.00× | ok |
| bg/binarytrees/2 | 0.088 | 0.474 | 5.39× | 0.291 | 3.31× | ok |
| bg/fannkuchredux/3 | 0.044 | 0.142 | 3.23× | 0.141 | 3.20× | ok |
| bg/fasta/1 | 0.067 | 0.185 | 2.76× | 0.201 | 3.00× | ok |
| bg/fasta/2 | 0.067 | 0.184 | 2.75× | 0.203 | 3.03× | ok |
| bg/fasta/3 | 0.099 | 0.252 | 2.55× | 0.323 | 3.26× | ok |
| bg/fasta/4 | 0.090 | 0.206 | 2.29× | 0.265 | 2.94× | ok |
| bg/fasta/6 | 0.164 | 0.489 | 2.98× | 0.587 | 3.58× | ok |
| bg/mandelbrot/0 | 0.206 | 0.792 | 3.84× | 0.824 | 4.00× | ok |
| bg/mandelbrot/2 | 0.107 | 0.315 | 2.94× | 0.426 | 3.98× | ok |
| bg/mandelbrot/3 | 0.115 | 0.264 | 2.30× | 0.342 | 2.97× | ok |
| bg/mandelbrot/5 | 0.058 | 0.183 | 3.16× | 0.174 | 3.00× | ok |
| bg/mandelbrot/7 | 0.063 | 0.225 | 3.57× | 0.198 | 3.14× | ok |
| bg/mandelbrot/9 | 0.059 | 0.238 | 4.03× | 0.190 | 3.22× | ok |
| bg/nbody/1 | 0.048 | 0.172 | 3.58× | 0.199 | 4.15× | ok |
| bg/nbody/2 | 0.037 | 0.146 | 3.95× | 0.138 | 3.73× | ok |
| bg/nbody/3 | 0.039 | 0.137 | 3.51× | 0.125 | 3.21× | ok |
| bg/nbody/6 | 0.047 | 0.186 | 3.96× | 0.187 | 3.98× | ok |
| bg/pidigits/4 | 0.150 | 0.382 | 2.55× | 0.514 | 3.43× | ok |
| bg/pidigits/5 | 0.094 | 0.248 | 2.64× | 0.351 | 3.73× | ok |
| bg/revcomp/1 | 0.102 | 0.273 | 2.68× | 0.336 | 3.29× | ok |
| bg/revcomp/5 | 0.100 | 0.313 | 3.13× | 0.378 | 3.78× | ok |
| bg/spectralnorm/1 | 0.139 | 0.388 | 2.79× | 0.472 | 3.40× | ok |
| box2d | 2.204 | 8.260 | 3.75× | 8.552 | 3.88× | ok |
| fmt | 1.048 | 5.983 | 5.71× | 4.553 | 4.34× | ok |
| hexxagon | 0.355 | 1.305 | 3.68× | 1.510 | 4.25× | ok |
| lambda | 0.146 | 1.293 | 8.86× | 0.649 | 4.45× | ok |
| minisat | 0.158 | 1.136 | 7.19× | 0.708 | 4.48× | ok |
| nlohmann-json | 0.968 | 4.036 | 4.17× | 3.271 | 3.38× | ok |
| pugixml | 0.690 | 6.137 | 8.89× | 4.369 | 6.33× | ok |
| pugixml-tests | 6.201 | 39.931 | 6.44× | 29.191 | 4.71× | ok |
| rapidjson | 0.246 | 1.394 | 5.67× | 0.849 | 3.45× | ok |
| re2 | 3.744 | 20.726 | 5.54× | 11.546 | 3.08× | ok |
| re2-test-re2 | 6.846 | 30.857 | 4.51× | 26.055 | 3.81× | ok |
| re2-test-search | 6.794 | 27.255 | 4.01× | 22.076 | 3.25× | ok |
| tramp3d-v4 | 2.306 | 19.799 | 8.59× | 14.835 | 6.43× | ok |

### Frontend only: -fsyntax-only

| Program | cppgm++ (s) | GCC (s) | Speed vs GCC ↑ | Clang (s) | Speed vs Clang ↑ | Status |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| Adobe-C++/functionobjects | 0.080 | 0.167 | 2.09× | 0.210 | 2.62× | ok |
| Adobe-C++/loop_unroll | 0.050 | 0.121 | 2.42× | 0.134 | 2.68× | ok |
| Adobe-C++/simple_types_constant_folding | 0.047 | 0.127 | 2.70× | 0.145 | 3.09× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.045 | 0.115 | 2.56× | 0.130 | 2.89× | ok |
| Adobe-C++/stepanov_abstraction | 0.043 | 0.085 | 1.98× | 0.108 | 2.51× | ok |
| Adobe-C++/stepanov_vector | 0.082 | 0.157 | 1.91× | 0.222 | 2.71× | ok |
| Bullet | 5.551 | 13.464 | 2.43× | 16.973 | 3.06× | ok |
| CLAMR | 2.573 | 6.144 | 2.39× | 7.955 | 3.09× | ok |
| CoyoteBench/almabench | 0.034 | 0.078 | 2.29× | 0.088 | 2.59× | ok |
| CoyoteBench/fftbench | 0.136 | 0.259 | 1.90× | 0.346 | 2.54× | ok |
| CoyoteBench/huffbench | 0.035 | 0.080 | 2.29× | 0.112 | 3.20× | ok |
| CoyoteBench/lpbench | 0.037 | 0.068 | 1.84× | 0.112 | 3.03× | ok |
| DOE/HACCKernels | 0.127 | 0.281 | 2.21× | 0.363 | 2.86× | ok |
| DOE/HPCCG | 0.871 | 1.652 | 1.90× | 2.903 | 3.33× | ok |
| DOE/PENNANT | 1.694 | 3.721 | 2.20× | 4.759 | 2.81× | ok |
| DOE/miniFE | 0.760 | 1.697 | 2.23× | 2.089 | 2.75× | ok |
| Misc-C++-EH/spirit | 0.358 | 1.283 | 3.58× | 1.876 | 5.24× | ok |
| Misc-C++-Large/ray | 0.131 | 0.268 | 2.05× | 0.431 | 3.29× | ok |
| Misc-C++-Large/sphereflake | 0.105 | 0.234 | 2.23× | 0.324 | 3.09× | ok |
| Misc-C++/bigfib | 0.125 | 0.284 | 2.27× | 0.432 | 3.46× | ok |
| Misc-C++/oopack_v1p8 | 0.037 | 0.081 | 2.19× | 0.103 | 2.78× | ok |
| Misc-C++/stepanov_container | 0.186 | 0.406 | 2.18× | 0.592 | 3.18× | ok |
| Misc-C++/stepanov_v1p2 | 0.035 | 0.077 | 2.20× | 0.093 | 2.66× | ok |
| PAQ8p | 0.055 | 0.142 | 2.58× | 0.167 | 3.04× | ok |
| Prolangs-C++/life | 0.004 | 0.012 | — | 0.024 | — | ok |
| Prolangs-C++/ocean | 0.011 | 0.028 | 2.55× | 0.039 | 3.55× | ok |
| Prolangs-C++/primes | 0.007 | 0.020 | — | 0.029 | — | ok |
| Shootout-C++-EH/except | 0.091 | 0.222 | 2.44× | 0.286 | 3.14× | ok |
| Shootout-C++/ackermann | 0.084 | 0.189 | 2.25× | 0.243 | 2.89× | ok |
| Shootout-C++/ary3 | 0.114 | 0.224 | 1.96× | 0.285 | 2.50× | ok |
| Shootout-C++/fibo | 0.086 | 0.182 | 2.12× | 0.246 | 2.86× | ok |
| Shootout-C++/hash | 0.122 | 0.258 | 2.11× | 0.311 | 2.55× | ok |
| Shootout-C++/hash2 | 0.118 | 0.232 | 1.97× | 0.338 | 2.86× | ok |
| Shootout-C++/heapsort | 0.105 | 0.234 | 2.23× | 0.282 | 2.69× | ok |
| Shootout-C++/lists | 0.090 | 0.208 | 2.31× | 0.314 | 3.49× | ok |
| Shootout-C++/lists1 | 0.104 | 0.215 | 2.07× | 0.291 | 2.80× | ok |
| Shootout-C++/matrix | 0.081 | 0.169 | 2.09× | 0.234 | 2.89× | ok |
| Shootout-C++/methcall | 0.082 | 0.221 | 2.70× | 0.222 | 2.71× | ok |
| Shootout-C++/moments | 0.109 | 0.230 | 2.11× | 0.293 | 2.69× | ok |
| Shootout-C++/random | 0.102 | 0.214 | 2.10× | 0.286 | 2.80× | ok |
| Shootout-C++/sieve | 0.094 | 0.195 | 2.07× | 0.280 | 2.98× | ok |
| SmallPT | 0.032 | 0.072 | 2.25× | 0.099 | 3.09× | ok |
| Stanford/Bubblesort | 0.011 | 0.029 | 2.64× | 0.036 | 3.27× | ok |
| Stanford/FloatMM | 0.011 | 0.030 | 2.73× | 0.036 | 3.27× | ok |
| Stanford/Puzzle | 0.011 | 0.033 | 3.00× | 0.038 | 3.45× | ok |
| Stanford/Treesort | 0.012 | 0.030 | 2.50× | 0.040 | 3.33× | ok |
| bg/binarytrees/2 | 0.082 | 0.180 | 2.20× | 0.238 | 2.90× | ok |
| bg/fannkuchredux/3 | 0.043 | 0.073 | 1.70× | 0.106 | 2.47× | ok |
| bg/fasta/1 | 0.061 | 0.123 | 2.02× | 0.157 | 2.57× | ok |
| bg/fasta/2 | 0.059 | 0.113 | 1.92× | 0.155 | 2.63× | ok |
| bg/fasta/3 | 0.094 | 0.205 | 2.18× | 0.266 | 2.83× | ok |
| bg/fasta/4 | 0.085 | 0.171 | 2.01× | 0.278 | 3.27× | ok |
| bg/fasta/6 | 0.140 | 0.310 | 2.21× | 0.567 | 4.05× | ok |
| bg/mandelbrot/0 | 0.156 | 0.355 | 2.28× | 0.433 | 2.78× | ok |
| bg/mandelbrot/2 | 0.099 | 0.244 | 2.46× | 0.305 | 3.08× | ok |
| bg/mandelbrot/3 | 0.111 | 0.227 | 2.05× | 0.320 | 2.88× | ok |
| bg/mandelbrot/5 | 0.050 | 0.121 | 2.42× | 0.132 | 2.64× | ok |
| bg/mandelbrot/7 | 0.052 | 0.119 | 2.29× | 0.176 | 3.38× | ok |
| bg/mandelbrot/9 | 0.047 | 0.097 | 2.06× | 0.130 | 2.77× | ok |
| bg/nbody/1 | 0.034 | 0.076 | 2.24× | 0.089 | 2.62× | ok |
| bg/nbody/2 | 0.033 | 0.068 | 2.06× | 0.087 | 2.64× | ok |
| bg/nbody/3 | 0.037 | 0.067 | 1.81× | 0.092 | 2.49× | ok |
| bg/nbody/6 | 0.034 | 0.072 | 2.12× | 0.092 | 2.71× | ok |
| bg/pidigits/4 | 0.154 | 0.319 | 2.07× | 0.461 | 2.99× | ok |
| bg/pidigits/5 | 0.089 | 0.195 | 2.19× | 0.264 | 2.97× | ok |
| bg/revcomp/1 | 0.095 | 0.198 | 2.08× | 0.261 | 2.75× | ok |
| bg/revcomp/5 | 0.096 | 0.192 | 2.00× | 0.342 | 3.56× | ok |
| bg/spectralnorm/1 | 0.132 | 0.264 | 2.00× | 0.391 | 2.96× | ok |
| box2d | 1.685 | 3.453 | 2.05× | 5.720 | 3.39× | ok |
| fmt | 0.523 | 1.308 | 2.50× | 1.994 | 3.81× | ok |
| hexxagon | 0.320 | 0.712 | 2.22× | 1.062 | 3.32× | ok |
| lambda | 0.057 | 0.171 | 3.00× | 0.212 | 3.72× | ok |
| minisat | 0.068 | 0.211 | 3.10× | 0.204 | 3.00× | ok |
| nlohmann-json | 0.422 | 1.161 | 2.75× | 1.438 | 3.41× | ok |
| pugixml | 0.260 | 0.579 | 2.23× | 1.030 | 3.96× | ok |
| pugixml-tests | 3.920 | 8.732 | 2.23× | 12.902 | 3.29× | ok |
| rapidjson | 0.118 | 0.301 | 2.55× | 0.373 | 3.16× | ok |
| re2 | 3.752 | 8.169 | 2.18× | 7.052 | 1.88× | ok |
| re2-test-re2 | 5.522 | 12.432 | 2.25× | 16.789 | 3.04× | ok |
| re2-test-search | 5.490 | 12.157 | 2.21× | 13.449 | 2.45× | ok |
| tramp3d-v4 | 0.806 | 2.675 | 3.32× | 1.794 | 2.23× | ok |


## Toolchains

- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)
- **valgrind**: valgrind-3.26.0

[Compiler build and codegen results](self-build.md) have their own measured revision.
