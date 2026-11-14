# Public benchmarks: Intel (GitHub)

Measured compiler: [`a649ddb21`](https://github.com/vishvananda/cppgm-extended/commit/a649ddb21dd810338ac87d93e7d2e713d0f98ab1) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/43609274612) · [Raw program results](results.json)

81 of 81 programs built with cppgm++ and g++ and agreed. Each number is the reference over cppgm++, so above 1× means cppgm++ is smaller or faster.

CPU: Intel(R) Xeon(R) 6973P-C.

| Measure | vs g++ | Programs | vs clang++ | Programs |
| --- | ---: | ---: | ---: | ---: |
| Frontend CPU, `-fsyntax-only` | 2.28× | 75 | 2.89× | 75 |
| Compile CPU, `-O0` | 3.19× | 74 | 3.15× | 74 |
| Compile CPU, `-O3` | 4.23× | 79 | 3.82× | 79 |
| Compile peak RSS, `-O0` | 1.44× | 81 | 2.19× | 81 |
| Compile peak RSS, `-O3` | 1.71× | 81 | 2.30× | 81 |
| Executable size, `-O0` | 0.93× | 81 | 0.87× | 81 |
| Executable size, `-O3` | 0.95× | 81 | 0.91× | 81 |
| Run CPU, `-O0` | 1.10× | 81 | 1.07× | 81 |
| Run CPU, `-O3` | 0.98× | 81 | 0.98× | 80 |
| Run instructions, `-O3` | 0.96× | 43 | - | 0 |

O3 execution uses paired ABBA/BAAB blocks. GCC A/A CPU ratios ranged from 0.928 to 1.064; raw observations and block ratios are retained in the artifact.

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
| Adobe-C++/functionobjects | 0.066 | 0.250 | 3.79× | 0.232 | 3.52× | ok |
| Adobe-C++/loop_unroll | 0.050 | 0.564 | 11.28× | 0.181 | 3.62× | ok |
| Adobe-C++/simple_types_constant_folding | 0.058 | 0.677 | 11.67× | 0.210 | 3.62× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.048 | 0.471 | 9.81× | 0.232 | 4.83× | ok |
| Adobe-C++/stepanov_abstraction | 0.036 | 0.173 | 4.81× | 0.106 | 2.94× | ok |
| Adobe-C++/stepanov_vector | 0.062 | 0.269 | 4.34× | 0.233 | 3.76× | ok |
| Bullet | 4.852 | 16.989 | 3.50× | 18.001 | 3.71× | ok |
| CLAMR | 2.439 | 9.321 | 3.82× | 8.319 | 3.41× | ok |
| CoyoteBench/almabench | 0.025 | 0.066 | 2.64× | 0.080 | 3.20× | ok |
| CoyoteBench/fftbench | 0.109 | 0.243 | 2.23× | 0.298 | 2.73× | ok |
| CoyoteBench/huffbench | 0.025 | 0.070 | 2.80× | 0.073 | 2.92× | ok |
| CoyoteBench/lpbench | 0.030 | 0.071 | 2.37× | 0.086 | 2.87× | ok |
| DOE/HACCKernels | 0.115 | 0.294 | 2.56× | 0.357 | 3.10× | ok |
| DOE/HPCCG | 0.675 | 1.508 | 2.23× | 1.911 | 2.83× | ok |
| DOE/PENNANT | 1.359 | 3.768 | 2.77× | 3.868 | 2.85× | ok |
| DOE/miniFE | 0.606 | 1.796 | 2.96× | 2.249 | 3.71× | ok |
| Misc-C++-EH/spirit | 0.369 | 2.817 | 7.63× | 1.603 | 4.34× | ok |
| Misc-C++-Large/ray | 0.094 | 0.278 | 2.96× | 0.264 | 2.81× | ok |
| Misc-C++-Large/sphereflake | 0.087 | 0.216 | 2.48× | 0.268 | 3.08× | ok |
| Misc-C++/bigfib | 0.108 | 0.334 | 3.09× | 0.397 | 3.68× | ok |
| Misc-C++/oopack_v1p8 | 0.031 | 0.089 | 2.87× | 0.091 | 2.94× | ok |
| Misc-C++/stepanov_container | 0.155 | 0.558 | 3.60× | 0.508 | 3.28× | ok |
| Misc-C++/stepanov_v1p2 | 0.027 | 0.117 | 4.33× | 0.086 | 3.19× | ok |
| PAQ8p | 0.064 | 0.433 | 6.77× | 0.204 | 3.19× | ok |
| Prolangs-C++/life | 0.004 | 0.030 | — | 0.029 | — | ok |
| Prolangs-C++/ocean | 0.009 | 0.058 | — | 0.045 | — | ok |
| Prolangs-C++/primes | 0.005 | 0.021 | — | 0.024 | — | ok |
| Shootout-C++-EH/except | 0.066 | 0.159 | 2.41× | 0.227 | 3.44× | ok |
| Shootout-C++/ackermann | 0.073 | 0.150 | 2.05× | 0.235 | 3.22× | ok |
| Shootout-C++/ary3 | 0.073 | 0.201 | 2.75× | 0.222 | 3.04× | ok |
| Shootout-C++/fibo | 0.073 | 0.159 | 2.18× | 0.253 | 3.47× | ok |
| Shootout-C++/hash | 0.111 | 0.301 | 2.71× | 0.313 | 2.82× | ok |
| Shootout-C++/hash2 | 0.093 | 0.262 | 2.82× | 0.297 | 3.19× | ok |
| Shootout-C++/heapsort | 0.097 | 0.179 | 1.85× | 0.220 | 2.27× | ok |
| Shootout-C++/lists | 0.077 | 0.196 | 2.55× | 0.215 | 2.79× | ok |
| Shootout-C++/lists1 | 0.085 | 0.257 | 3.02× | 0.248 | 2.92× | ok |
| Shootout-C++/matrix | 0.081 | 0.145 | 1.79× | 0.200 | 2.47× | ok |
| Shootout-C++/methcall | 0.064 | 0.166 | 2.59× | 0.187 | 2.92× | ok |
| Shootout-C++/moments | 0.093 | 0.236 | 2.54× | 0.311 | 3.34× | ok |
| Shootout-C++/random | 0.083 | 0.195 | 2.35× | 0.285 | 3.43× | ok |
| Shootout-C++/sieve | 0.078 | 0.232 | 2.97× | 0.270 | 3.46× | ok |
| SmallPT | 0.027 | 0.080 | 2.96× | 0.083 | 3.07× | ok |
| Stanford/Bubblesort | 0.008 | 0.025 | — | 0.033 | — | ok |
| Stanford/FloatMM | 0.009 | 0.026 | — | 0.030 | — | ok |
| Stanford/Puzzle | 0.009 | 0.032 | — | 0.034 | — | ok |
| Stanford/Treesort | 0.008 | 0.029 | — | 0.032 | — | ok |
| bg/binarytrees/2 | 0.064 | 0.159 | 2.48× | 0.219 | 3.42× | ok |
| bg/fannkuchredux/3 | 0.033 | 0.078 | 2.36× | 0.087 | 2.64× | ok |
| bg/fasta/1 | 0.048 | 0.129 | 2.69× | 0.138 | 2.88× | ok |
| bg/fasta/2 | 0.056 | 0.123 | 2.20× | 0.130 | 2.32× | ok |
| bg/fasta/3 | 0.084 | 0.174 | 2.07× | 0.250 | 2.98× | ok |
| bg/fasta/4 | 0.075 | 0.152 | 2.03× | 0.170 | 2.27× | ok |
| bg/fasta/6 | 0.123 | 0.380 | 3.09× | 0.386 | 3.14× | ok |
| bg/mandelbrot/0 | 0.153 | 0.425 | 2.78× | 0.406 | 2.65× | ok |
| bg/mandelbrot/2 | 0.090 | 0.245 | 2.72× | 0.272 | 3.02× | ok |
| bg/mandelbrot/3 | 0.088 | 0.205 | 2.33× | 0.254 | 2.89× | ok |
| bg/mandelbrot/5 | 0.050 | 0.103 | 2.06× | 0.116 | 2.32× | ok |
| bg/mandelbrot/7 | 0.046 | 0.126 | 2.74× | 0.117 | 2.54× | ok |
| bg/mandelbrot/9 | 0.038 | 0.113 | 2.97× | 0.142 | 3.74× | ok |
| bg/nbody/1 | 0.028 | 0.094 | 3.36× | 0.102 | 3.64× | ok |
| bg/nbody/2 | 0.025 | 0.059 | 2.36× | 0.073 | 2.92× | ok |
| bg/nbody/3 | 0.024 | 0.066 | 2.75× | 0.069 | 2.88× | ok |
| bg/nbody/6 | 0.030 | 0.091 | 3.03× | 0.099 | 3.30× | ok |
| bg/pidigits/4 | 0.117 | 0.269 | 2.30× | 0.340 | 2.91× | ok |
| bg/pidigits/5 | 0.073 | 0.159 | 2.18× | 0.200 | 2.74× | ok |
| bg/revcomp/1 | 0.077 | 0.237 | 3.08× | 0.244 | 3.17× | ok |
| bg/revcomp/5 | 0.075 | 0.205 | 2.73× | 0.208 | 2.77× | ok |
| bg/spectralnorm/1 | 0.106 | 0.274 | 2.58× | 0.397 | 3.75× | ok |
| box2d | 1.332 | 4.017 | 3.02× | 3.241 | 2.43× | ok |
| fmt | 0.543 | 2.440 | 4.49× | 1.718 | 3.16× | ok |
| hexxagon | 0.268 | 0.559 | 2.09× | 0.834 | 3.11× | ok |
| lambda | 0.047 | 0.292 | 6.21× | 0.221 | 4.70× | ok |
| minisat | 0.064 | 0.300 | 4.69× | 0.228 | 3.56× | ok |
| nlohmann-json | 0.403 | 1.655 | 4.11× | 1.604 | 3.98× | ok |
| pugixml | 0.265 | 1.261 | 4.76× | 0.953 | 3.60× | ok |
| pugixml-tests | 3.089 | 14.998 | 4.86× | 12.409 | 4.02× | ok |
| rapidjson | 0.110 | 0.476 | 4.33× | 0.366 | 3.33× | ok |
| re2 | 2.298 | 9.761 | 4.25× | 6.705 | 2.92× | ok |
| re2-test-re2 | 4.347 | 15.345 | 3.53× | 14.752 | 3.39× | ok |
| re2-test-search | 4.316 | 14.389 | 3.33× | 13.756 | 3.19× | ok |
| tramp3d-v4 | 0.748 | 8.397 | 11.23× | 2.819 | 3.77× | ok |

### O3 compilation

| Program | cppgm++ (s) | GCC (s) | Speed vs GCC ↑ | Clang (s) | Speed vs Clang ↑ | Status |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| Adobe-C++/functionobjects | 0.125 | 0.899 | 7.19× | 0.462 | 3.70× | ok |
| Adobe-C++/loop_unroll | 0.453 | 5.263 | 11.62× | 4.050 | 8.94× | ok |
| Adobe-C++/simple_types_constant_folding | 0.246 | 2.863 | 11.64× | 2.568 | 10.44× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.195 | 2.299 | 11.79× | 1.791 | 9.18× | ok |
| Adobe-C++/stepanov_abstraction | 0.085 | 1.040 | 12.24× | 0.426 | 5.01× | ok |
| Adobe-C++/stepanov_vector | 0.132 | 1.425 | 10.80× | 0.549 | 4.16× | ok |
| Bullet | 5.722 | 37.848 | 6.61× | 27.988 | 4.89× | ok |
| CLAMR | 4.273 | 22.836 | 5.34× | 18.543 | 4.34× | ok |
| CoyoteBench/almabench | 0.035 | 0.146 | 4.17× | 0.114 | 3.26× | ok |
| CoyoteBench/fftbench | 0.121 | 0.389 | 3.21× | 0.406 | 3.36× | ok |
| CoyoteBench/huffbench | 0.038 | 0.195 | 5.13× | 0.138 | 3.63× | ok |
| CoyoteBench/lpbench | 0.037 | 0.185 | 5.00× | 0.213 | 5.76× | ok |
| DOE/HACCKernels | 0.114 | 0.357 | 3.13× | 0.406 | 3.56× | ok |
| DOE/HPCCG | 0.708 | 2.149 | 3.04× | 2.127 | 3.00× | ok |
| DOE/PENNANT | 1.676 | 6.723 | 4.01× | 5.858 | 3.50× | ok |
| DOE/miniFE | 0.868 | 4.190 | 4.83× | 3.016 | 3.47× | ok |
| Misc-C++-EH/spirit | 0.761 | 3.268 | 4.29× | 3.429 | 4.51× | ok |
| Misc-C++-Large/ray | 0.103 | 0.475 | 4.61× | 0.361 | 3.50× | ok |
| Misc-C++-Large/sphereflake | 0.106 | 0.734 | 6.92× | 0.367 | 3.46× | ok |
| Misc-C++/bigfib | 0.137 | 0.467 | 3.41× | 0.588 | 4.29× | ok |
| Misc-C++/oopack_v1p8 | 0.039 | 0.177 | 4.54× | 0.170 | 4.36× | ok |
| Misc-C++/stepanov_container | 0.234 | 0.861 | 3.68× | 0.839 | 3.59× | ok |
| Misc-C++/stepanov_v1p2 | 0.039 | 0.194 | 4.97× | 0.184 | 4.72× | ok |
| PAQ8p | 0.320 | 3.681 | 11.50× | 1.511 | 4.72× | ok |
| Prolangs-C++/life | 0.008 | 0.107 | — | 0.066 | — | ok |
| Prolangs-C++/ocean | 0.019 | 0.186 | 9.79× | 0.107 | 5.63× | ok |
| Prolangs-C++/primes | 0.008 | 0.033 | — | 0.035 | — | ok |
| Shootout-C++-EH/except | 0.070 | 0.185 | 2.64× | 0.219 | 3.13× | ok |
| Shootout-C++/ackermann | 0.077 | 0.188 | 2.44× | 0.239 | 3.10× | ok |
| Shootout-C++/ary3 | 0.079 | 0.221 | 2.80× | 0.238 | 3.01× | ok |
| Shootout-C++/fibo | 0.072 | 0.203 | 2.82× | 0.250 | 3.47× | ok |
| Shootout-C++/hash | 0.125 | 0.402 | 3.22× | 0.405 | 3.24× | ok |
| Shootout-C++/hash2 | 0.125 | 0.387 | 3.10× | 0.407 | 3.26× | ok |
| Shootout-C++/heapsort | 0.100 | 0.233 | 2.33× | 0.237 | 2.37× | ok |
| Shootout-C++/lists | 0.078 | 0.224 | 2.87× | 0.242 | 3.10× | ok |
| Shootout-C++/lists1 | 0.094 | 0.295 | 3.14× | 0.287 | 3.05× | ok |
| Shootout-C++/matrix | 0.073 | 0.221 | 3.03× | 0.341 | 4.67× | ok |
| Shootout-C++/methcall | 0.070 | 0.174 | 2.49× | 0.262 | 3.74× | ok |
| Shootout-C++/moments | 0.105 | 0.299 | 2.85× | 0.308 | 2.93× | ok |
| Shootout-C++/random | 0.097 | 0.190 | 1.96× | 0.247 | 2.55× | ok |
| Shootout-C++/sieve | 0.087 | 0.250 | 2.87× | 0.286 | 3.29× | ok |
| SmallPT | 0.043 | 0.288 | 6.70× | 0.170 | 3.95× | ok |
| Stanford/Bubblesort | 0.010 | 0.050 | 5.00× | 0.050 | 5.00× | ok |
| Stanford/FloatMM | 0.013 | 0.067 | 5.15× | 0.067 | 5.15× | ok |
| Stanford/Puzzle | 0.018 | 0.131 | 7.28× | 0.111 | 6.17× | ok |
| Stanford/Treesort | 0.013 | 0.079 | 6.08× | 0.055 | 4.23× | ok |
| bg/binarytrees/2 | 0.083 | 0.429 | 5.17× | 0.241 | 2.90× | ok |
| bg/fannkuchredux/3 | 0.037 | 0.119 | 3.22× | 0.111 | 3.00× | ok |
| bg/fasta/1 | 0.061 | 0.154 | 2.52× | 0.177 | 2.90× | ok |
| bg/fasta/2 | 0.056 | 0.170 | 3.04× | 0.159 | 2.84× | ok |
| bg/fasta/3 | 0.085 | 0.217 | 2.55× | 0.292 | 3.44× | ok |
| bg/fasta/4 | 0.091 | 0.185 | 2.03× | 0.228 | 2.51× | ok |
| bg/fasta/6 | 0.153 | 0.441 | 2.88× | 0.508 | 3.32× | ok |
| bg/mandelbrot/0 | 0.166 | 0.687 | 4.14× | 0.637 | 3.84× | ok |
| bg/mandelbrot/2 | 0.098 | 0.255 | 2.60× | 0.307 | 3.13× | ok |
| bg/mandelbrot/3 | 0.090 | 0.242 | 2.69× | 0.286 | 3.18× | ok |
| bg/mandelbrot/5 | 0.053 | 0.140 | 2.64× | 0.145 | 2.74× | ok |
| bg/mandelbrot/7 | 0.056 | 0.183 | 3.27× | 0.153 | 2.73× | ok |
| bg/mandelbrot/9 | 0.050 | 0.187 | 3.74× | 0.177 | 3.54× | ok |
| bg/nbody/1 | 0.044 | 0.148 | 3.36× | 0.187 | 4.25× | ok |
| bg/nbody/2 | 0.030 | 0.120 | 4.00× | 0.140 | 4.67× | ok |
| bg/nbody/3 | 0.032 | 0.117 | 3.66× | 0.114 | 3.56× | ok |
| bg/nbody/6 | 0.040 | 0.162 | 4.05× | 0.203 | 5.08× | ok |
| bg/pidigits/4 | 0.127 | 0.342 | 2.69× | 0.487 | 3.83× | ok |
| bg/pidigits/5 | 0.091 | 0.189 | 2.08× | 0.242 | 2.66× | ok |
| bg/revcomp/1 | 0.083 | 0.250 | 3.01× | 0.270 | 3.25× | ok |
| bg/revcomp/5 | 0.093 | 0.222 | 2.39× | 0.238 | 2.56× | ok |
| bg/spectralnorm/1 | 0.114 | 0.319 | 2.80× | 0.401 | 3.52× | ok |
| box2d | 1.726 | 6.778 | 3.93× | 4.527 | 2.62× | ok |
| fmt | 0.958 | 5.338 | 5.57× | 3.590 | 3.75× | ok |
| hexxagon | 0.270 | 1.027 | 3.80× | 1.122 | 4.16× | ok |
| lambda | 0.114 | 1.099 | 9.64× | 0.556 | 4.88× | ok |
| minisat | 0.137 | 1.006 | 7.34× | 0.595 | 4.34× | ok |
| nlohmann-json | 0.781 | 3.228 | 4.13× | 2.804 | 3.59× | ok |
| pugixml | 0.648 | 5.086 | 7.85× | 3.929 | 6.06× | ok |
| pugixml-tests | 4.985 | 33.562 | 6.73× | 24.036 | 4.82× | ok |
| rapidjson | 0.227 | 1.106 | 4.87× | 0.698 | 3.07× | ok |
| re2 | 3.039 | 17.940 | 5.90× | 10.011 | 3.29× | ok |
| re2-test-re2 | 5.471 | 26.571 | 4.86× | 22.129 | 4.04× | ok |
| re2-test-search | 5.498 | 23.086 | 4.20× | 21.108 | 3.84× | ok |
| tramp3d-v4 | 2.518 | 17.552 | 6.97× | 14.459 | 5.74× | ok |

### Frontend only: -fsyntax-only

| Program | cppgm++ (s) | GCC (s) | Speed vs GCC ↑ | Clang (s) | Speed vs Clang ↑ | Status |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| Adobe-C++/functionobjects | 0.060 | 0.134 | 2.23× | 0.202 | 3.37× | ok |
| Adobe-C++/loop_unroll | 0.034 | 0.101 | 2.97× | 0.106 | 3.12× | ok |
| Adobe-C++/simple_types_constant_folding | 0.041 | 0.104 | 2.54× | 0.106 | 2.59× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.036 | 0.095 | 2.64× | 0.123 | 3.42× | ok |
| Adobe-C++/stepanov_abstraction | 0.032 | 0.069 | 2.16× | 0.082 | 2.56× | ok |
| Adobe-C++/stepanov_vector | 0.064 | 0.123 | 1.92× | 0.191 | 2.98× | ok |
| Bullet | 4.468 | 11.430 | 2.56× | 12.535 | 2.81× | ok |
| CLAMR | 2.304 | 5.337 | 2.32× | 7.235 | 3.14× | ok |
| CoyoteBench/almabench | 0.026 | 0.054 | 2.08× | 0.074 | 2.85× | ok |
| CoyoteBench/fftbench | 0.106 | 0.213 | 2.01× | 0.273 | 2.58× | ok |
| CoyoteBench/huffbench | 0.026 | 0.060 | 2.31× | 0.073 | 2.81× | ok |
| CoyoteBench/lpbench | 0.030 | 0.055 | 1.83× | 0.081 | 2.70× | ok |
| DOE/HACCKernels | 0.109 | 0.270 | 2.48× | 0.297 | 2.72× | ok |
| DOE/HPCCG | 0.674 | 1.328 | 1.97× | 1.715 | 2.54× | ok |
| DOE/PENNANT | 1.373 | 2.778 | 2.02× | 3.957 | 2.88× | ok |
| DOE/miniFE | 0.549 | 1.169 | 2.13× | 1.862 | 3.39× | ok |
| Misc-C++-EH/spirit | 0.322 | 0.948 | 2.94× | 1.174 | 3.65× | ok |
| Misc-C++-Large/ray | 0.096 | 0.230 | 2.40× | 0.261 | 2.72× | ok |
| Misc-C++-Large/sphereflake | 0.086 | 0.190 | 2.21× | 0.258 | 3.00× | ok |
| Misc-C++/bigfib | 0.098 | 0.245 | 2.50× | 0.362 | 3.69× | ok |
| Misc-C++/oopack_v1p8 | 0.029 | 0.063 | 2.17× | 0.085 | 2.93× | ok |
| Misc-C++/stepanov_container | 0.142 | 0.327 | 2.30× | 0.393 | 2.77× | ok |
| Misc-C++/stepanov_v1p2 | 0.026 | 0.061 | 2.35× | 0.074 | 2.85× | ok |
| PAQ8p | 0.041 | 0.113 | 2.76× | 0.116 | 2.83× | ok |
| Prolangs-C++/life | 0.003 | 0.010 | — | 0.023 | — | ok |
| Prolangs-C++/ocean | 0.008 | 0.019 | — | 0.032 | — | ok |
| Prolangs-C++/primes | 0.005 | 0.014 | — | 0.024 | — | ok |
| Shootout-C++-EH/except | 0.065 | 0.162 | 2.49× | 0.209 | 3.22× | ok |
| Shootout-C++/ackermann | 0.077 | 0.153 | 1.99× | 0.236 | 3.06× | ok |
| Shootout-C++/ary3 | 0.071 | 0.170 | 2.39× | 0.206 | 2.90× | ok |
| Shootout-C++/fibo | 0.075 | 0.162 | 2.16× | 0.218 | 2.91× | ok |
| Shootout-C++/hash | 0.102 | 0.233 | 2.28× | 0.279 | 2.74× | ok |
| Shootout-C++/hash2 | 0.089 | 0.194 | 2.18× | 0.243 | 2.73× | ok |
| Shootout-C++/heapsort | 0.097 | 0.179 | 1.85× | 0.210 | 2.16× | ok |
| Shootout-C++/lists | 0.071 | 0.155 | 2.18× | 0.196 | 2.76× | ok |
| Shootout-C++/lists1 | 0.083 | 0.206 | 2.48× | 0.225 | 2.71× | ok |
| Shootout-C++/matrix | 0.080 | 0.144 | 1.80× | 0.194 | 2.43× | ok |
| Shootout-C++/methcall | 0.065 | 0.166 | 2.55× | 0.179 | 2.75× | ok |
| Shootout-C++/moments | 0.089 | 0.182 | 2.04× | 0.217 | 2.44× | ok |
| Shootout-C++/random | 0.084 | 0.183 | 2.18× | 0.228 | 2.71× | ok |
| Shootout-C++/sieve | 0.076 | 0.209 | 2.75× | 0.244 | 3.21× | ok |
| SmallPT | 0.028 | 0.051 | 1.82× | 0.077 | 2.75× | ok |
| Stanford/Bubblesort | 0.008 | 0.022 | — | 0.034 | — | ok |
| Stanford/FloatMM | 0.010 | 0.021 | 2.10× | 0.030 | 3.00× | ok |
| Stanford/Puzzle | 0.009 | 0.022 | — | 0.031 | — | ok |
| Stanford/Treesort | 0.009 | 0.025 | — | 0.031 | — | ok |
| bg/binarytrees/2 | 0.066 | 0.156 | 2.36× | 0.206 | 3.12× | ok |
| bg/fannkuchredux/3 | 0.032 | 0.064 | 2.00× | 0.084 | 2.62× | ok |
| bg/fasta/1 | 0.047 | 0.097 | 2.06× | 0.124 | 2.64× | ok |
| bg/fasta/2 | 0.057 | 0.099 | 1.74× | 0.120 | 2.11× | ok |
| bg/fasta/3 | 0.079 | 0.166 | 2.10× | 0.242 | 3.06× | ok |
| bg/fasta/4 | 0.073 | 0.141 | 1.93× | 0.181 | 2.48× | ok |
| bg/fasta/6 | 0.110 | 0.281 | 2.55× | 0.344 | 3.13× | ok |
| bg/mandelbrot/0 | 0.134 | 0.303 | 2.26× | 0.354 | 2.64× | ok |
| bg/mandelbrot/2 | 0.086 | 0.219 | 2.55× | 0.247 | 2.87× | ok |
| bg/mandelbrot/3 | 0.089 | 0.196 | 2.20× | 0.247 | 2.78× | ok |
| bg/mandelbrot/5 | 0.046 | 0.097 | 2.11× | 0.107 | 2.33× | ok |
| bg/mandelbrot/7 | 0.043 | 0.089 | 2.07× | 0.105 | 2.44× | ok |
| bg/mandelbrot/9 | 0.037 | 0.082 | 2.22× | 0.119 | 3.22× | ok |
| bg/nbody/1 | 0.027 | 0.055 | 2.04× | 0.091 | 3.37× | ok |
| bg/nbody/2 | 0.026 | 0.050 | 1.92× | 0.068 | 2.62× | ok |
| bg/nbody/3 | 0.024 | 0.051 | 2.12× | 0.067 | 2.79× | ok |
| bg/nbody/6 | 0.030 | 0.054 | 1.80× | 0.084 | 2.80× | ok |
| bg/pidigits/4 | 0.112 | 0.238 | 2.12× | 0.334 | 2.98× | ok |
| bg/pidigits/5 | 0.073 | 0.153 | 2.10× | 0.187 | 2.56× | ok |
| bg/revcomp/1 | 0.074 | 0.201 | 2.72× | 0.206 | 2.78× | ok |
| bg/revcomp/5 | 0.075 | 0.169 | 2.25× | 0.187 | 2.49× | ok |
| bg/spectralnorm/1 | 0.098 | 0.256 | 2.61× | 0.327 | 3.34× | ok |
| box2d | 1.294 | 2.751 | 2.13× | 3.355 | 2.59× | ok |
| fmt | 0.408 | 1.006 | 2.47× | 1.514 | 3.71× | ok |
| hexxagon | 0.247 | 0.477 | 1.93× | 0.793 | 3.21× | ok |
| lambda | 0.038 | 0.118 | 3.11× | 0.163 | 4.29× | ok |
| minisat | 0.051 | 0.146 | 2.86× | 0.164 | 3.22× | ok |
| nlohmann-json | 0.303 | 0.778 | 2.57× | 1.266 | 4.18× | ok |
| pugixml | 0.217 | 0.458 | 2.11× | 0.695 | 3.20× | ok |
| pugixml-tests | 2.875 | 7.367 | 2.56× | 9.687 | 3.37× | ok |
| rapidjson | 0.085 | 0.228 | 2.68× | 0.273 | 3.21× | ok |
| re2 | 2.891 | 6.792 | 2.35× | 5.707 | 1.97× | ok |
| re2-test-re2 | 3.945 | 9.540 | 2.42× | 12.343 | 3.13× | ok |
| re2-test-search | 3.879 | 8.990 | 2.32× | 12.375 | 3.19× | ok |
| tramp3d-v4 | 0.489 | 2.372 | 4.85× | 1.538 | 3.15× | ok |


## Toolchains

- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)
- **valgrind**: valgrind-3.26.0

[Compiler build and codegen results](self-build.md) have their own measured revision.
