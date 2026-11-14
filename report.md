# Public benchmarks: AMD EPYC

Measured compiler: [`1597e573a`](https://github.com/vishvananda/cppgm-extended/commit/1597e573a4882a000810eb15f86d4cb230ce4afd) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/43572026851) · [Raw program results](results.json)

81 of 81 programs built with cppgm++ and g++ and agreed. Each number is the reference over cppgm++, so above 1× means cppgm++ is smaller or faster.

CPU: AMD EPYC 9V45 96-Core Processor.

| Measure | vs g++ | Programs | vs clang++ | Programs |
| --- | ---: | ---: | ---: | ---: |
| Frontend CPU, `-fsyntax-only` | 2.23× | 75 | 2.73× | 75 |
| Compile CPU, `-O0` | 3.06× | 76 | 2.94× | 76 |
| Compile CPU, `-O3` | 4.19× | 79 | 3.74× | 79 |
| Compile peak RSS, `-O0` | 1.44× | 81 | 2.20× | 81 |
| Compile peak RSS, `-O3` | 1.71× | 81 | 2.30× | 81 |
| Executable size, `-O0` | 0.93× | 81 | 0.87× | 81 |
| Executable size, `-O3` | 0.96× | 81 | 0.91× | 81 |
| Run CPU, `-O0` | 1.11× | 81 | 1.10× | 81 |
| Run CPU, `-O3` | 0.98× | 80 | 0.99× | 80 |
| Run instructions, `-O3` | 0.95× | 43 | - | 0 |

O3 execution uses paired ABBA/BAAB blocks. GCC A/A CPU ratios ranged from 0.772 to 1.132; raw observations and block ratios are retained in the artifact.

## Slowest generated code, by run-time instructions

| Program | vs g++ |
| --- | ---: |
| tramp3d-v4 | 0.46× |
| bg/nbody/2 | 0.63× |
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
| Adobe-C++/functionobjects | 0.062 | 0.224 | 3.61× | 0.157 | 2.53× | ok |
| Adobe-C++/loop_unroll | 0.048 | 0.478 | 9.96× | 0.155 | 3.23× | ok |
| Adobe-C++/simple_types_constant_folding | 0.050 | 0.560 | 11.20× | 0.195 | 3.90× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.044 | 0.407 | 9.25× | 0.156 | 3.55× | ok |
| Adobe-C++/stepanov_abstraction | 0.037 | 0.156 | 4.22× | 0.106 | 2.86× | ok |
| Adobe-C++/stepanov_vector | 0.064 | 0.261 | 4.08× | 0.213 | 3.33× | ok |
| Bullet | 4.692 | 15.951 | 3.40× | 14.403 | 3.07× | ok |
| CLAMR | 2.395 | 8.223 | 3.43× | 7.029 | 2.93× | ok |
| CoyoteBench/almabench | 0.026 | 0.069 | 2.65× | 0.075 | 2.88× | ok |
| CoyoteBench/fftbench | 0.094 | 0.223 | 2.37× | 0.297 | 3.16× | ok |
| CoyoteBench/huffbench | 0.027 | 0.070 | 2.59× | 0.076 | 2.81× | ok |
| CoyoteBench/lpbench | 0.030 | 0.073 | 2.43× | 0.069 | 2.30× | ok |
| DOE/HACCKernels | 0.112 | 0.239 | 2.13× | 0.327 | 2.92× | ok |
| DOE/HPCCG | 0.604 | 1.520 | 2.52× | 1.777 | 2.94× | ok |
| DOE/PENNANT | 1.267 | 3.400 | 2.68× | 3.656 | 2.89× | ok |
| DOE/miniFE | 0.582 | 1.888 | 3.24× | 1.839 | 3.16× | ok |
| Misc-C++-EH/spirit | 0.402 | 2.223 | 5.53× | 1.777 | 4.42× | ok |
| Misc-C++-Large/ray | 0.097 | 0.252 | 2.60× | 0.320 | 3.30× | ok |
| Misc-C++-Large/sphereflake | 0.088 | 0.217 | 2.47× | 0.276 | 3.14× | ok |
| Misc-C++/bigfib | 0.110 | 0.373 | 3.39× | 0.336 | 3.05× | ok |
| Misc-C++/oopack_v1p8 | 0.030 | 0.089 | 2.97× | 0.083 | 2.77× | ok |
| Misc-C++/stepanov_container | 0.164 | 0.494 | 3.01× | 0.419 | 2.55× | ok |
| Misc-C++/stepanov_v1p2 | 0.033 | 0.113 | 3.42× | 0.079 | 2.39× | ok |
| PAQ8p | 0.061 | 0.405 | 6.64× | 0.210 | 3.44× | ok |
| Prolangs-C++/life | 0.004 | 0.029 | — | 0.027 | — | ok |
| Prolangs-C++/ocean | 0.011 | 0.059 | 5.36× | 0.043 | 3.91× | ok |
| Prolangs-C++/primes | 0.006 | 0.023 | — | 0.025 | — | ok |
| Shootout-C++-EH/except | 0.065 | 0.139 | 2.14× | 0.188 | 2.89× | ok |
| Shootout-C++/ackermann | 0.069 | 0.148 | 2.14× | 0.195 | 2.83× | ok |
| Shootout-C++/ary3 | 0.074 | 0.167 | 2.26× | 0.217 | 2.93× | ok |
| Shootout-C++/fibo | 0.066 | 0.139 | 2.11× | 0.185 | 2.80× | ok |
| Shootout-C++/hash | 0.099 | 0.257 | 2.60× | 0.285 | 2.88× | ok |
| Shootout-C++/hash2 | 0.091 | 0.270 | 2.97× | 0.261 | 2.87× | ok |
| Shootout-C++/heapsort | 0.077 | 0.170 | 2.21× | 0.208 | 2.70× | ok |
| Shootout-C++/lists | 0.080 | 0.184 | 2.30× | 0.228 | 2.85× | ok |
| Shootout-C++/lists1 | 0.083 | 0.203 | 2.45× | 0.239 | 2.88× | ok |
| Shootout-C++/matrix | 0.066 | 0.149 | 2.26× | 0.179 | 2.71× | ok |
| Shootout-C++/methcall | 0.061 | 0.141 | 2.31× | 0.181 | 2.97× | ok |
| Shootout-C++/moments | 0.089 | 0.236 | 2.65× | 0.231 | 2.60× | ok |
| Shootout-C++/random | 0.086 | 0.167 | 1.94× | 0.239 | 2.78× | ok |
| Shootout-C++/sieve | 0.084 | 0.234 | 2.79× | 0.229 | 2.73× | ok |
| SmallPT | 0.026 | 0.080 | 3.08× | 0.078 | 3.00× | ok |
| Stanford/Bubblesort | 0.009 | 0.027 | — | 0.030 | — | ok |
| Stanford/FloatMM | 0.009 | 0.028 | — | 0.032 | — | ok |
| Stanford/Puzzle | 0.010 | 0.035 | 3.50× | 0.032 | 3.20× | ok |
| Stanford/Treesort | 0.009 | 0.030 | — | 0.032 | — | ok |
| bg/binarytrees/2 | 0.060 | 0.143 | 2.38× | 0.189 | 3.15× | ok |
| bg/fannkuchredux/3 | 0.028 | 0.067 | 2.39× | 0.078 | 2.79× | ok |
| bg/fasta/1 | 0.060 | 0.111 | 1.85× | 0.120 | 2.00× | ok |
| bg/fasta/2 | 0.051 | 0.104 | 2.04× | 0.117 | 2.29× | ok |
| bg/fasta/3 | 0.075 | 0.166 | 2.21× | 0.237 | 3.16× | ok |
| bg/fasta/4 | 0.068 | 0.142 | 2.09× | 0.168 | 2.47× | ok |
| bg/fasta/6 | 0.124 | 0.329 | 2.65× | 0.378 | 3.05× | ok |
| bg/mandelbrot/0 | 0.139 | 0.416 | 2.99× | 0.370 | 2.66× | ok |
| bg/mandelbrot/2 | 0.086 | 0.201 | 2.34× | 0.247 | 2.87× | ok |
| bg/mandelbrot/3 | 0.088 | 0.184 | 2.09× | 0.232 | 2.64× | ok |
| bg/mandelbrot/5 | 0.042 | 0.100 | 2.38× | 0.111 | 2.64× | ok |
| bg/mandelbrot/7 | 0.045 | 0.112 | 2.49× | 0.117 | 2.60× | ok |
| bg/mandelbrot/9 | 0.041 | 0.103 | 2.51× | 0.108 | 2.63× | ok |
| bg/nbody/1 | 0.027 | 0.093 | 3.44× | 0.086 | 3.19× | ok |
| bg/nbody/2 | 0.026 | 0.065 | 2.50× | 0.068 | 2.62× | ok |
| bg/nbody/3 | 0.026 | 0.067 | 2.58× | 0.070 | 2.69× | ok |
| bg/nbody/6 | 0.027 | 0.095 | 3.52× | 0.088 | 3.26× | ok |
| bg/pidigits/4 | 0.111 | 0.269 | 2.42× | 0.320 | 2.88× | ok |
| bg/pidigits/5 | 0.070 | 0.152 | 2.17× | 0.221 | 3.16× | ok |
| bg/revcomp/1 | 0.102 | 0.190 | 1.86× | 0.227 | 2.23× | ok |
| bg/revcomp/5 | 0.069 | 0.205 | 2.97× | 0.203 | 2.94× | ok |
| bg/spectralnorm/1 | 0.107 | 0.233 | 2.18× | 0.279 | 2.61× | ok |
| box2d | 1.398 | 4.075 | 2.91× | 4.580 | 3.28× | ok |
| fmt | 0.444 | 2.226 | 5.01× | 1.551 | 3.49× | ok |
| hexxagon | 0.257 | 0.612 | 2.38× | 0.776 | 3.02× | ok |
| lambda | 0.049 | 0.297 | 6.06× | 0.214 | 4.37× | ok |
| minisat | 0.057 | 0.259 | 4.54× | 0.173 | 3.04× | ok |
| nlohmann-json | 0.367 | 1.616 | 4.40× | 1.547 | 4.22× | ok |
| pugixml | 0.249 | 1.212 | 4.87× | 0.849 | 3.41× | ok |
| pugixml-tests | 3.349 | 13.207 | 3.94× | 11.211 | 3.35× | ok |
| rapidjson | 0.112 | 0.416 | 3.71× | 0.323 | 2.88× | ok |
| re2 | 2.897 | 9.368 | 3.23× | 5.847 | 2.02× | ok |
| re2-test-re2 | 4.267 | 13.648 | 3.20× | 13.276 | 3.11× | ok |
| re2-test-search | 4.587 | 12.400 | 2.70× | 13.800 | 3.01× | ok |
| tramp3d-v4 | 0.646 | 8.136 | 12.59× | 2.182 | 3.38× | ok |

### O3 compilation

| Program | cppgm++ (s) | GCC (s) | Speed vs GCC ↑ | Clang (s) | Speed vs Clang ↑ | Status |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| Adobe-C++/functionobjects | 0.115 | 0.743 | 6.46× | 0.430 | 3.74× | ok |
| Adobe-C++/loop_unroll | 0.330 | 4.882 | 14.79× | 3.773 | 11.43× | ok |
| Adobe-C++/simple_types_constant_folding | 0.186 | 2.548 | 13.70× | 2.061 | 11.08× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.154 | 1.970 | 12.79× | 1.850 | 12.01× | ok |
| Adobe-C++/stepanov_abstraction | 0.081 | 0.929 | 11.47× | 0.452 | 5.58× | ok |
| Adobe-C++/stepanov_vector | 0.099 | 1.090 | 11.01× | 0.468 | 4.73× | ok |
| Bullet | 6.016 | 34.383 | 5.72× | 23.302 | 3.87× | ok |
| CLAMR | 3.557 | 19.864 | 5.58× | 15.903 | 4.47× | ok |
| CoyoteBench/almabench | 0.033 | 0.154 | 4.67× | 0.114 | 3.45× | ok |
| CoyoteBench/fftbench | 0.114 | 0.352 | 3.09× | 0.412 | 3.61× | ok |
| CoyoteBench/huffbench | 0.038 | 0.189 | 4.97× | 0.120 | 3.16× | ok |
| CoyoteBench/lpbench | 0.036 | 0.173 | 4.81× | 0.172 | 4.78× | ok |
| DOE/HACCKernels | 0.120 | 0.361 | 3.01× | 0.352 | 2.93× | ok |
| DOE/HPCCG | 0.636 | 2.170 | 3.41× | 2.010 | 3.16× | ok |
| DOE/PENNANT | 1.662 | 5.283 | 3.18× | 4.904 | 2.95× | ok |
| DOE/miniFE | 0.781 | 3.492 | 4.47× | 3.164 | 4.05× | ok |
| Misc-C++-EH/spirit | 0.582 | 3.167 | 5.44× | 3.147 | 5.41× | ok |
| Misc-C++-Large/ray | 0.108 | 0.436 | 4.04× | 0.371 | 3.44× | ok |
| Misc-C++-Large/sphereflake | 0.100 | 0.681 | 6.81× | 0.346 | 3.46× | ok |
| Misc-C++/bigfib | 0.134 | 0.528 | 3.94× | 0.434 | 3.24× | ok |
| Misc-C++/oopack_v1p8 | 0.036 | 0.167 | 4.64× | 0.148 | 4.11× | ok |
| Misc-C++/stepanov_container | 0.215 | 0.782 | 3.64× | 0.753 | 3.50× | ok |
| Misc-C++/stepanov_v1p2 | 0.039 | 0.206 | 5.28× | 0.172 | 4.41× | ok |
| PAQ8p | 0.263 | 3.400 | 12.93× | 1.397 | 5.31× | ok |
| Prolangs-C++/life | 0.008 | 0.093 | — | 0.070 | — | ok |
| Prolangs-C++/ocean | 0.020 | 0.181 | 9.05× | 0.106 | 5.30× | ok |
| Prolangs-C++/primes | 0.007 | 0.035 | — | 0.035 | — | ok |
| Shootout-C++-EH/except | 0.087 | 0.178 | 2.05× | 0.190 | 2.18× | ok |
| Shootout-C++/ackermann | 0.075 | 0.186 | 2.48× | 0.198 | 2.64× | ok |
| Shootout-C++/ary3 | 0.080 | 0.201 | 2.51× | 0.249 | 3.11× | ok |
| Shootout-C++/fibo | 0.068 | 0.174 | 2.56× | 0.189 | 2.78× | ok |
| Shootout-C++/hash | 0.116 | 0.339 | 2.92× | 0.411 | 3.54× | ok |
| Shootout-C++/hash2 | 0.108 | 0.385 | 3.56× | 0.395 | 3.66× | ok |
| Shootout-C++/heapsort | 0.084 | 0.193 | 2.30× | 0.230 | 2.74× | ok |
| Shootout-C++/lists | 0.079 | 0.225 | 2.85× | 0.223 | 2.82× | ok |
| Shootout-C++/lists1 | 0.094 | 0.239 | 2.54× | 0.291 | 3.10× | ok |
| Shootout-C++/matrix | 0.071 | 0.185 | 2.61× | 0.265 | 3.73× | ok |
| Shootout-C++/methcall | 0.065 | 0.168 | 2.58× | 0.194 | 2.98× | ok |
| Shootout-C++/moments | 0.098 | 0.293 | 2.99× | 0.297 | 3.03× | ok |
| Shootout-C++/random | 0.081 | 0.186 | 2.30× | 0.220 | 2.72× | ok |
| Shootout-C++/sieve | 0.088 | 0.225 | 2.56× | 0.254 | 2.89× | ok |
| SmallPT | 0.037 | 0.254 | 6.86× | 0.139 | 3.76× | ok |
| Stanford/Bubblesort | 0.011 | 0.050 | 4.55× | 0.045 | 4.09× | ok |
| Stanford/FloatMM | 0.012 | 0.056 | 4.67× | 0.063 | 5.25× | ok |
| Stanford/Puzzle | 0.017 | 0.112 | 6.59× | 0.106 | 6.24× | ok |
| Stanford/Treesort | 0.013 | 0.074 | 5.69× | 0.051 | 3.92× | ok |
| bg/binarytrees/2 | 0.064 | 0.352 | 5.50× | 0.203 | 3.17× | ok |
| bg/fannkuchredux/3 | 0.034 | 0.099 | 2.91× | 0.101 | 2.97× | ok |
| bg/fasta/1 | 0.056 | 0.146 | 2.61× | 0.143 | 2.55× | ok |
| bg/fasta/2 | 0.055 | 0.144 | 2.62× | 0.166 | 3.02× | ok |
| bg/fasta/3 | 0.079 | 0.209 | 2.65× | 0.255 | 3.23× | ok |
| bg/fasta/4 | 0.072 | 0.174 | 2.42× | 0.199 | 2.76× | ok |
| bg/fasta/6 | 0.127 | 0.399 | 3.14× | 0.458 | 3.61× | ok |
| bg/mandelbrot/0 | 0.171 | 0.564 | 3.30× | 0.595 | 3.48× | ok |
| bg/mandelbrot/2 | 0.085 | 0.238 | 2.80× | 0.273 | 3.21× | ok |
| bg/mandelbrot/3 | 0.089 | 0.230 | 2.58× | 0.265 | 2.98× | ok |
| bg/mandelbrot/5 | 0.047 | 0.146 | 3.11× | 0.132 | 2.81× | ok |
| bg/mandelbrot/7 | 0.053 | 0.165 | 3.11× | 0.155 | 2.92× | ok |
| bg/mandelbrot/9 | 0.056 | 0.154 | 2.75× | 0.147 | 2.62× | ok |
| bg/nbody/1 | 0.037 | 0.139 | 3.76× | 0.150 | 4.05× | ok |
| bg/nbody/2 | 0.029 | 0.117 | 4.03× | 0.109 | 3.76× | ok |
| bg/nbody/3 | 0.030 | 0.112 | 3.73× | 0.096 | 3.20× | ok |
| bg/nbody/6 | 0.035 | 0.147 | 4.20× | 0.150 | 4.29× | ok |
| bg/pidigits/4 | 0.111 | 0.301 | 2.71× | 0.401 | 3.61× | ok |
| bg/pidigits/5 | 0.069 | 0.186 | 2.70× | 0.240 | 3.48× | ok |
| bg/revcomp/1 | 0.085 | 0.213 | 2.51× | 0.279 | 3.28× | ok |
| bg/revcomp/5 | 0.075 | 0.220 | 2.93× | 0.271 | 3.61× | ok |
| bg/spectralnorm/1 | 0.117 | 0.310 | 2.65× | 0.375 | 3.21× | ok |
| box2d | 1.713 | 7.054 | 4.12× | 6.228 | 3.64× | ok |
| fmt | 0.797 | 4.686 | 5.88× | 3.210 | 4.03× | ok |
| hexxagon | 0.297 | 0.964 | 3.25× | 0.999 | 3.36× | ok |
| lambda | 0.106 | 1.041 | 9.82× | 0.529 | 4.99× | ok |
| minisat | 0.128 | 0.892 | 6.97× | 0.599 | 4.68× | ok |
| nlohmann-json | 0.643 | 2.930 | 4.56× | 2.372 | 3.69× | ok |
| pugixml | 0.541 | 4.265 | 7.88× | 3.287 | 6.08× | ok |
| pugixml-tests | 4.446 | 29.951 | 6.74× | 20.007 | 4.50× | ok |
| rapidjson | 0.207 | 1.010 | 4.88× | 0.555 | 2.68× | ok |
| re2 | 2.962 | 15.597 | 5.27× | 8.513 | 2.87× | ok |
| re2-test-re2 | 5.366 | 23.223 | 4.33× | 20.101 | 3.75× | ok |
| re2-test-search | 5.432 | 20.757 | 3.82× | 16.402 | 3.02× | ok |
| tramp3d-v4 | 1.903 | 14.484 | 7.61× | 11.838 | 6.22× | ok |

### Frontend only: -fsyntax-only

| Program | cppgm++ (s) | GCC (s) | Speed vs GCC ↑ | Clang (s) | Speed vs Clang ↑ | Status |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| Adobe-C++/functionobjects | 0.058 | 0.121 | 2.09× | 0.137 | 2.36× | ok |
| Adobe-C++/loop_unroll | 0.036 | 0.089 | 2.47× | 0.096 | 2.67× | ok |
| Adobe-C++/simple_types_constant_folding | 0.035 | 0.090 | 2.57× | 0.107 | 3.06× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.034 | 0.083 | 2.44× | 0.090 | 2.65× | ok |
| Adobe-C++/stepanov_abstraction | 0.032 | 0.065 | 2.03× | 0.080 | 2.50× | ok |
| Adobe-C++/stepanov_vector | 0.056 | 0.115 | 2.05× | 0.169 | 3.02× | ok |
| Bullet | 4.318 | 9.866 | 2.28× | 12.914 | 2.99× | ok |
| CLAMR | 2.125 | 4.428 | 2.08× | 5.354 | 2.52× | ok |
| CoyoteBench/almabench | 0.026 | 0.056 | 2.15× | 0.070 | 2.69× | ok |
| CoyoteBench/fftbench | 0.097 | 0.199 | 2.05× | 0.253 | 2.61× | ok |
| CoyoteBench/huffbench | 0.026 | 0.063 | 2.42× | 0.070 | 2.69× | ok |
| CoyoteBench/lpbench | 0.030 | 0.058 | 1.93× | 0.064 | 2.13× | ok |
| DOE/HACCKernels | 0.107 | 0.213 | 1.99× | 0.302 | 2.82× | ok |
| DOE/HPCCG | 0.584 | 1.398 | 2.39× | 1.608 | 2.75× | ok |
| DOE/PENNANT | 1.295 | 2.558 | 1.98× | 3.258 | 2.52× | ok |
| DOE/miniFE | 0.542 | 1.283 | 2.37× | 1.594 | 2.94× | ok |
| Misc-C++-EH/spirit | 0.281 | 0.814 | 2.90× | 1.273 | 4.53× | ok |
| Misc-C++-Large/ray | 0.095 | 0.210 | 2.21× | 0.285 | 3.00× | ok |
| Misc-C++-Large/sphereflake | 0.085 | 0.182 | 2.14× | 0.250 | 2.94× | ok |
| Misc-C++/bigfib | 0.097 | 0.250 | 2.58× | 0.298 | 3.07× | ok |
| Misc-C++/oopack_v1p8 | 0.028 | 0.063 | 2.25× | 0.076 | 2.71× | ok |
| Misc-C++/stepanov_container | 0.146 | 0.287 | 1.97× | 0.351 | 2.40× | ok |
| Misc-C++/stepanov_v1p2 | 0.030 | 0.060 | 2.00× | 0.067 | 2.23× | ok |
| PAQ8p | 0.043 | 0.111 | 2.58× | 0.120 | 2.79× | ok |
| Prolangs-C++/life | 0.003 | 0.010 | — | 0.022 | — | ok |
| Prolangs-C++/ocean | 0.009 | 0.022 | — | 0.031 | — | ok |
| Prolangs-C++/primes | 0.006 | 0.016 | — | 0.024 | — | ok |
| Shootout-C++-EH/except | 0.062 | 0.139 | 2.24× | 0.172 | 2.77× | ok |
| Shootout-C++/ackermann | 0.065 | 0.145 | 2.23× | 0.183 | 2.82× | ok |
| Shootout-C++/ary3 | 0.073 | 0.152 | 2.08× | 0.198 | 2.71× | ok |
| Shootout-C++/fibo | 0.065 | 0.141 | 2.17× | 0.179 | 2.75× | ok |
| Shootout-C++/hash | 0.092 | 0.195 | 2.12× | 0.243 | 2.64× | ok |
| Shootout-C++/hash2 | 0.087 | 0.201 | 2.31× | 0.239 | 2.75× | ok |
| Shootout-C++/heapsort | 0.083 | 0.166 | 2.00× | 0.198 | 2.39× | ok |
| Shootout-C++/lists | 0.076 | 0.158 | 2.08× | 0.205 | 2.70× | ok |
| Shootout-C++/lists1 | 0.081 | 0.168 | 2.07× | 0.210 | 2.59× | ok |
| Shootout-C++/matrix | 0.065 | 0.149 | 2.29× | 0.177 | 2.72× | ok |
| Shootout-C++/methcall | 0.061 | 0.146 | 2.39× | 0.172 | 2.82× | ok |
| Shootout-C++/moments | 0.089 | 0.187 | 2.10× | 0.216 | 2.43× | ok |
| Shootout-C++/random | 0.099 | 0.170 | 1.72× | 0.234 | 2.36× | ok |
| Shootout-C++/sieve | 0.080 | 0.196 | 2.45× | 0.201 | 2.51× | ok |
| SmallPT | 0.025 | 0.055 | 2.20× | 0.068 | 2.72× | ok |
| Stanford/Bubblesort | 0.009 | 0.023 | — | 0.028 | — | ok |
| Stanford/FloatMM | 0.008 | 0.022 | — | 0.030 | — | ok |
| Stanford/Puzzle | 0.010 | 0.025 | 2.50× | 0.029 | 2.90× | ok |
| Stanford/Treesort | 0.009 | 0.024 | — | 0.029 | — | ok |
| bg/binarytrees/2 | 0.061 | 0.140 | 2.30× | 0.180 | 2.95× | ok |
| bg/fannkuchredux/3 | 0.028 | 0.056 | 2.00× | 0.070 | 2.50× | ok |
| bg/fasta/1 | 0.049 | 0.087 | 1.78× | 0.109 | 2.22× | ok |
| bg/fasta/2 | 0.048 | 0.084 | 1.75× | 0.107 | 2.23× | ok |
| bg/fasta/3 | 0.072 | 0.157 | 2.18× | 0.196 | 2.72× | ok |
| bg/fasta/4 | 0.066 | 0.137 | 2.08× | 0.154 | 2.33× | ok |
| bg/fasta/6 | 0.115 | 0.282 | 2.45× | 0.320 | 2.78× | ok |
| bg/mandelbrot/0 | 0.125 | 0.269 | 2.15× | 0.329 | 2.63× | ok |
| bg/mandelbrot/2 | 0.088 | 0.179 | 2.03× | 0.206 | 2.34× | ok |
| bg/mandelbrot/3 | 0.083 | 0.175 | 2.11× | 0.217 | 2.61× | ok |
| bg/mandelbrot/5 | 0.040 | 0.079 | 1.97× | 0.099 | 2.48× | ok |
| bg/mandelbrot/7 | 0.042 | 0.080 | 1.90× | 0.107 | 2.55× | ok |
| bg/mandelbrot/9 | 0.038 | 0.075 | 1.97× | 0.097 | 2.55× | ok |
| bg/nbody/1 | 0.026 | 0.056 | 2.15× | 0.072 | 2.77× | ok |
| bg/nbody/2 | 0.025 | 0.054 | 2.16× | 0.065 | 2.60× | ok |
| bg/nbody/3 | 0.026 | 0.055 | 2.12× | 0.066 | 2.54× | ok |
| bg/nbody/6 | 0.025 | 0.058 | 2.32× | 0.070 | 2.80× | ok |
| bg/pidigits/4 | 0.112 | 0.240 | 2.14× | 0.300 | 2.68× | ok |
| bg/pidigits/5 | 0.069 | 0.143 | 2.07× | 0.201 | 2.91× | ok |
| bg/revcomp/1 | 0.073 | 0.152 | 2.08× | 0.201 | 2.75× | ok |
| bg/revcomp/5 | 0.065 | 0.153 | 2.35× | 0.181 | 2.78× | ok |
| bg/spectralnorm/1 | 0.096 | 0.211 | 2.20× | 0.270 | 2.81× | ok |
| box2d | 1.278 | 2.790 | 2.18× | 3.894 | 3.05× | ok |
| fmt | 0.392 | 0.999 | 2.55× | 1.293 | 3.30× | ok |
| hexxagon | 0.238 | 0.551 | 2.32× | 0.665 | 2.79× | ok |
| lambda | 0.040 | 0.140 | 3.50× | 0.152 | 3.80× | ok |
| minisat | 0.049 | 0.120 | 2.45× | 0.132 | 2.69× | ok |
| nlohmann-json | 0.302 | 0.725 | 2.40× | 1.152 | 3.81× | ok |
| pugixml | 0.180 | 0.482 | 2.68× | 0.626 | 3.48× | ok |
| pugixml-tests | 3.098 | 6.728 | 2.17× | 8.031 | 2.59× | ok |
| rapidjson | 0.092 | 0.220 | 2.39× | 0.335 | 3.64× | ok |
| re2 | 2.663 | 6.015 | 2.26× | 4.869 | 1.83× | ok |
| re2-test-re2 | 3.872 | 8.671 | 2.24× | 11.860 | 3.06× | ok |
| re2-test-search | 4.157 | 8.493 | 2.04× | 10.665 | 2.57× | ok |
| tramp3d-v4 | 0.412 | 2.110 | 5.12× | 1.266 | 3.07× | ok |


## Toolchains

- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)
- **valgrind**: valgrind-3.26.0

[Compiler build and codegen results](self-build.md) have their own measured revision.
