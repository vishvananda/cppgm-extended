# Public benchmarks: AMD EPYC

Measured compiler: [`dc9084330`](https://github.com/vishvananda/cppgm-extended/commit/dc908433031b1e72d07206262cb240cdedc9d13b) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/43395345867) · [Raw program results](results.json)

81 of 81 programs built with cppgm++ and g++ and agreed. Each number is the reference over cppgm++, so above 1× means cppgm++ is smaller or faster.

CPU: AMD EPYC 7763 64-Core Processor.

| Measure | vs g++ | Programs | vs clang++ | Programs |
| --- | ---: | ---: | ---: | ---: |
| Frontend CPU, `-fsyntax-only` | 2.30× | 79 | 3.42× | 79 |
| Compile CPU, `-O0` | 3.08× | 79 | 3.65× | 79 |
| Compile CPU, `-O3` | 4.40× | 81 | 4.39× | 81 |
| Compile peak RSS, `-O0` | 1.49× | 81 | 2.39× | 81 |
| Compile peak RSS, `-O3` | 1.75× | 81 | 2.48× | 81 |
| Executable size, `-O0` | 0.93× | 81 | 0.87× | 81 |
| Executable size, `-O3` | 0.96× | 81 | 0.92× | 81 |
| Run CPU, `-O0` | 1.17× | 81 | 1.22× | 81 |
| Run CPU, `-O3` | 0.98× | 81 | 0.97× | 80 |
| Run instructions, `-O3` | 0.95× | 43 | - | 0 |

O3 execution uses paired ABBA/BAAB blocks. GCC A/A CPU ratios ranged from 0.948 to 1.218; raw observations and block ratios are retained in the artifact.

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
| Adobe-C++/functionobjects | 0.107 | 0.408 | 3.81× | 0.324 | 3.03× | ok |
| Adobe-C++/loop_unroll | 0.089 | 0.840 | 9.44× | 0.364 | 4.09× | ok |
| Adobe-C++/simple_types_constant_folding | 0.095 | 0.958 | 10.08× | 0.441 | 4.64× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.079 | 0.688 | 8.71× | 0.495 | 6.27× | ok |
| Adobe-C++/stepanov_abstraction | 0.059 | 0.273 | 4.63× | 0.208 | 3.53× | ok |
| Adobe-C++/stepanov_vector | 0.105 | 0.393 | 3.74× | 0.399 | 3.80× | ok |
| Bullet | 7.865 | 26.973 | 3.43× | 28.548 | 3.63× | ok |
| CLAMR | 3.620 | 13.210 | 3.65× | 14.665 | 4.05× | ok |
| CoyoteBench/almabench | 0.048 | 0.130 | 2.71× | 0.137 | 2.85× | ok |
| CoyoteBench/fftbench | 0.185 | 0.424 | 2.29× | 0.783 | 4.23× | ok |
| CoyoteBench/huffbench | 0.047 | 0.107 | 2.28× | 0.210 | 4.47× | ok |
| CoyoteBench/lpbench | 0.053 | 0.134 | 2.53× | 0.134 | 2.53× | ok |
| DOE/HACCKernels | 0.169 | 0.421 | 2.49× | 0.613 | 3.63× | ok |
| DOE/HPCCG | 0.986 | 2.485 | 2.52× | 4.731 | 4.80× | ok |
| DOE/PENNANT | 2.211 | 5.900 | 2.67× | 7.529 | 3.41× | ok |
| DOE/miniFE | 1.039 | 2.995 | 2.88× | 4.195 | 4.04× | ok |
| Misc-C++-EH/spirit | 0.614 | 3.976 | 6.48× | 3.759 | 6.12× | ok |
| Misc-C++-Large/ray | 0.184 | 0.472 | 2.57× | 0.645 | 3.51× | ok |
| Misc-C++-Large/sphereflake | 0.173 | 0.425 | 2.46× | 0.529 | 3.06× | ok |
| Misc-C++/bigfib | 0.175 | 0.577 | 3.30× | 0.645 | 3.69× | ok |
| Misc-C++/oopack_v1p8 | 0.051 | 0.143 | 2.80× | 0.188 | 3.69× | ok |
| Misc-C++/stepanov_container | 0.228 | 0.894 | 3.92× | 0.881 | 3.86× | ok |
| Misc-C++/stepanov_v1p2 | 0.047 | 0.166 | 3.53× | 0.171 | 3.64× | ok |
| PAQ8p | 0.098 | 0.661 | 6.74× | 0.455 | 4.64× | ok |
| Prolangs-C++/life | 0.005 | 0.046 | — | 0.042 | — | ok |
| Prolangs-C++/ocean | 0.019 | 0.098 | 5.16× | 0.088 | 4.63× | ok |
| Prolangs-C++/primes | 0.009 | 0.040 | — | 0.045 | — | ok |
| Shootout-C++-EH/except | 0.119 | 0.258 | 2.17× | 0.440 | 3.70× | ok |
| Shootout-C++/ackermann | 0.122 | 0.265 | 2.17× | 0.367 | 3.01× | ok |
| Shootout-C++/ary3 | 0.143 | 0.298 | 2.08× | 0.484 | 3.38× | ok |
| Shootout-C++/fibo | 0.108 | 0.240 | 2.22× | 0.376 | 3.48× | ok |
| Shootout-C++/hash | 0.148 | 0.412 | 2.78× | 0.556 | 3.76× | ok |
| Shootout-C++/hash2 | 0.177 | 0.464 | 2.62× | 0.547 | 3.09× | ok |
| Shootout-C++/heapsort | 0.127 | 0.294 | 2.31× | 0.445 | 3.50× | ok |
| Shootout-C++/lists | 0.124 | 0.312 | 2.52× | 0.440 | 3.55× | ok |
| Shootout-C++/lists1 | 0.137 | 0.348 | 2.54× | 0.466 | 3.40× | ok |
| Shootout-C++/matrix | 0.107 | 0.250 | 2.34× | 0.383 | 3.58× | ok |
| Shootout-C++/methcall | 0.104 | 0.231 | 2.22× | 0.372 | 3.58× | ok |
| Shootout-C++/moments | 0.196 | 0.386 | 1.97× | 0.474 | 2.42× | ok |
| Shootout-C++/random | 0.138 | 0.253 | 1.83× | 0.446 | 3.23× | ok |
| Shootout-C++/sieve | 0.135 | 0.337 | 2.50× | 0.491 | 3.64× | ok |
| SmallPT | 0.043 | 0.135 | 3.14× | 0.141 | 3.28× | ok |
| Stanford/Bubblesort | 0.014 | 0.041 | 2.93× | 0.048 | 3.43× | ok |
| Stanford/FloatMM | 0.014 | 0.042 | 3.00× | 0.050 | 3.57× | ok |
| Stanford/Puzzle | 0.015 | 0.052 | 3.47× | 0.052 | 3.47× | ok |
| Stanford/Treesort | 0.014 | 0.044 | 3.14× | 0.050 | 3.57× | ok |
| bg/binarytrees/2 | 0.110 | 0.242 | 2.20× | 0.392 | 3.56× | ok |
| bg/fannkuchredux/3 | 0.056 | 0.123 | 2.20× | 0.158 | 2.82× | ok |
| bg/fasta/1 | 0.092 | 0.199 | 2.16× | 0.243 | 2.64× | ok |
| bg/fasta/2 | 0.081 | 0.188 | 2.32× | 0.250 | 3.09× | ok |
| bg/fasta/3 | 0.127 | 0.281 | 2.21× | 0.430 | 3.39× | ok |
| bg/fasta/4 | 0.114 | 0.246 | 2.16× | 0.393 | 3.45× | ok |
| bg/fasta/6 | 0.206 | 0.588 | 2.85× | 0.952 | 4.62× | ok |
| bg/mandelbrot/0 | 0.237 | 0.775 | 3.27× | 0.887 | 3.74× | ok |
| bg/mandelbrot/2 | 0.149 | 0.340 | 2.28× | 0.484 | 3.25× | ok |
| bg/mandelbrot/3 | 0.152 | 0.353 | 2.32× | 0.515 | 3.39× | ok |
| bg/mandelbrot/5 | 0.072 | 0.174 | 2.42× | 0.220 | 3.06× | ok |
| bg/mandelbrot/7 | 0.073 | 0.196 | 2.68× | 0.301 | 4.12× | ok |
| bg/mandelbrot/9 | 0.070 | 0.178 | 2.54× | 0.219 | 3.13× | ok |
| bg/nbody/1 | 0.047 | 0.194 | 4.13× | 0.159 | 3.38× | ok |
| bg/nbody/2 | 0.043 | 0.106 | 2.47× | 0.135 | 3.14× | ok |
| bg/nbody/3 | 0.044 | 0.106 | 2.41× | 0.145 | 3.30× | ok |
| bg/nbody/6 | 0.044 | 0.194 | 4.41× | 0.168 | 3.82× | ok |
| bg/pidigits/4 | 0.187 | 0.470 | 2.51× | 0.728 | 3.89× | ok |
| bg/pidigits/5 | 0.128 | 0.266 | 2.08× | 0.464 | 3.62× | ok |
| bg/revcomp/1 | 0.131 | 0.347 | 2.65× | 0.544 | 4.15× | ok |
| bg/revcomp/5 | 0.138 | 0.315 | 2.28× | 0.548 | 3.97× | ok |
| bg/spectralnorm/1 | 0.210 | 0.526 | 2.50× | 0.730 | 3.48× | ok |
| box2d | 2.243 | 6.838 | 3.05× | 11.130 | 4.96× | ok |
| fmt | 0.975 | 3.735 | 3.83× | 3.972 | 4.07× | ok |
| hexxagon | 0.424 | 1.132 | 2.67× | 1.337 | 3.15× | ok |
| lambda | 0.098 | 0.451 | 4.60× | 0.390 | 3.98× | ok |
| minisat | 0.100 | 0.498 | 4.98× | 0.456 | 4.56× | ok |
| nlohmann-json | 0.583 | 2.915 | 5.00× | 3.397 | 5.83× | ok |
| pugixml | 0.475 | 2.105 | 4.43× | 1.721 | 3.62× | ok |
| pugixml-tests | 6.240 | 24.454 | 3.92× | 25.284 | 4.05× | ok |
| rapidjson | 0.205 | 0.707 | 3.45× | 0.731 | 3.57× | ok |
| re2 | 5.335 | 17.263 | 3.24× | 12.569 | 2.36× | ok |
| re2-test-re2 | 7.628 | 25.713 | 3.37× | 30.882 | 4.05× | ok |
| re2-test-search | 8.082 | 23.928 | 2.96× | 24.026 | 2.97× | ok |
| tramp3d-v4 | 1.110 | 12.289 | 11.07× | 5.002 | 4.51× | ok |

### O3 compilation

| Program | cppgm++ (s) | GCC (s) | Speed vs GCC ↑ | Clang (s) | Speed vs Clang ↑ | Status |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| Adobe-C++/functionobjects | 0.158 | 1.289 | 8.16× | 0.983 | 6.22× | ok |
| Adobe-C++/loop_unroll | 0.548 | 7.669 | 13.99× | 6.114 | 11.16× | ok |
| Adobe-C++/simple_types_constant_folding | 0.316 | 4.214 | 13.34× | 3.568 | 11.29× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.250 | 3.198 | 12.79× | 2.709 | 10.84× | ok |
| Adobe-C++/stepanov_abstraction | 0.119 | 1.491 | 12.53× | 0.881 | 7.40× | ok |
| Adobe-C++/stepanov_vector | 0.193 | 2.175 | 11.27× | 0.851 | 4.41× | ok |
| Bullet | 9.851 | 56.464 | 5.73× | 46.132 | 4.68× | ok |
| CLAMR | 6.181 | 35.622 | 5.76× | 29.489 | 4.77× | ok |
| CoyoteBench/almabench | 0.050 | 0.231 | 4.62× | 0.188 | 3.76× | ok |
| CoyoteBench/fftbench | 0.202 | 0.707 | 3.50× | 0.993 | 4.92× | ok |
| CoyoteBench/huffbench | 0.056 | 0.373 | 6.66× | 0.226 | 4.04× | ok |
| CoyoteBench/lpbench | 0.052 | 0.321 | 6.17× | 0.291 | 5.60× | ok |
| DOE/HACCKernels | 0.190 | 0.567 | 2.98× | 0.651 | 3.43× | ok |
| DOE/HPCCG | 1.335 | 3.753 | 2.81× | 4.015 | 3.01× | ok |
| DOE/PENNANT | 2.780 | 9.078 | 3.27× | 9.691 | 3.49× | ok |
| DOE/miniFE | 1.326 | 6.477 | 4.88× | 6.251 | 4.71× | ok |
| Misc-C++-EH/spirit | 1.215 | 5.765 | 4.74× | 5.042 | 4.15× | ok |
| Misc-C++-Large/ray | 0.211 | 0.865 | 4.10× | 0.751 | 3.56× | ok |
| Misc-C++-Large/sphereflake | 0.170 | 1.238 | 7.28× | 0.650 | 3.82× | ok |
| Misc-C++/bigfib | 0.216 | 0.799 | 3.70× | 1.031 | 4.77× | ok |
| Misc-C++/oopack_v1p8 | 0.064 | 0.295 | 4.61× | 0.290 | 4.53× | ok |
| Misc-C++/stepanov_container | 0.304 | 1.174 | 3.86× | 1.342 | 4.41× | ok |
| Misc-C++/stepanov_v1p2 | 0.059 | 0.310 | 5.25× | 0.346 | 5.86× | ok |
| PAQ8p | 0.384 | 5.546 | 14.44× | 2.732 | 7.11× | ok |
| Prolangs-C++/life | 0.012 | 0.159 | 13.25× | 0.099 | 8.25× | ok |
| Prolangs-C++/ocean | 0.032 | 0.305 | 9.53× | 0.201 | 6.28× | ok |
| Prolangs-C++/primes | 0.011 | 0.056 | 5.09× | 0.052 | 4.73× | ok |
| Shootout-C++-EH/except | 0.119 | 0.313 | 2.63× | 0.448 | 3.76× | ok |
| Shootout-C++/ackermann | 0.112 | 0.336 | 3.00× | 0.379 | 3.38× | ok |
| Shootout-C++/ary3 | 0.129 | 0.405 | 3.14× | 0.477 | 3.70× | ok |
| Shootout-C++/fibo | 0.147 | 0.307 | 2.09× | 0.364 | 2.48× | ok |
| Shootout-C++/hash | 0.168 | 0.545 | 3.24× | 0.704 | 4.19× | ok |
| Shootout-C++/hash2 | 0.178 | 0.644 | 3.62× | 0.762 | 4.28× | ok |
| Shootout-C++/heapsort | 0.144 | 0.366 | 2.54× | 0.444 | 3.08× | ok |
| Shootout-C++/lists | 0.133 | 0.343 | 2.58× | 0.564 | 4.24× | ok |
| Shootout-C++/lists1 | 0.151 | 0.406 | 2.69× | 0.525 | 3.48× | ok |
| Shootout-C++/matrix | 0.112 | 0.328 | 2.93× | 0.515 | 4.60× | ok |
| Shootout-C++/methcall | 0.107 | 0.289 | 2.70× | 0.377 | 3.52× | ok |
| Shootout-C++/moments | 0.162 | 0.479 | 2.96× | 0.559 | 3.45× | ok |
| Shootout-C++/random | 0.138 | 0.404 | 2.93× | 0.491 | 3.56× | ok |
| Shootout-C++/sieve | 0.148 | 0.337 | 2.28× | 0.610 | 4.12× | ok |
| SmallPT | 0.062 | 0.426 | 6.87× | 0.273 | 4.40× | ok |
| Stanford/Bubblesort | 0.017 | 0.078 | 4.59× | 0.071 | 4.18× | ok |
| Stanford/FloatMM | 0.018 | 0.087 | 4.83× | 0.098 | 5.44× | ok |
| Stanford/Puzzle | 0.024 | 0.179 | 7.46× | 0.171 | 7.12× | ok |
| Stanford/Treesort | 0.020 | 0.119 | 5.95× | 0.076 | 3.80× | ok |
| bg/binarytrees/2 | 0.115 | 0.610 | 5.30× | 0.404 | 3.51× | ok |
| bg/fannkuchredux/3 | 0.060 | 0.193 | 3.22× | 0.200 | 3.33× | ok |
| bg/fasta/1 | 0.091 | 0.241 | 2.65× | 0.295 | 3.24× | ok |
| bg/fasta/2 | 0.088 | 0.246 | 2.80× | 0.400 | 4.55× | ok |
| bg/fasta/3 | 0.130 | 0.342 | 2.63× | 0.564 | 4.34× | ok |
| bg/fasta/4 | 0.125 | 0.287 | 2.30× | 0.387 | 3.10× | ok |
| bg/fasta/6 | 0.235 | 0.774 | 3.29× | 1.081 | 4.60× | ok |
| bg/mandelbrot/0 | 0.259 | 0.971 | 3.75× | 1.142 | 4.41× | ok |
| bg/mandelbrot/2 | 0.195 | 0.446 | 2.29× | 0.531 | 2.72× | ok |
| bg/mandelbrot/3 | 0.153 | 0.372 | 2.43× | 0.548 | 3.58× | ok |
| bg/mandelbrot/5 | 0.075 | 0.233 | 3.11× | 0.263 | 3.51× | ok |
| bg/mandelbrot/7 | 0.084 | 0.303 | 3.61× | 0.310 | 3.69× | ok |
| bg/mandelbrot/9 | 0.083 | 0.276 | 3.33× | 0.276 | 3.33× | ok |
| bg/nbody/1 | 0.061 | 0.237 | 3.89× | 0.272 | 4.46× | ok |
| bg/nbody/2 | 0.063 | 0.207 | 3.29× | 0.203 | 3.22× | ok |
| bg/nbody/3 | 0.048 | 0.185 | 3.85× | 0.186 | 3.88× | ok |
| bg/nbody/6 | 0.057 | 0.237 | 4.16× | 0.283 | 4.96× | ok |
| bg/pidigits/4 | 0.211 | 0.590 | 2.80× | 0.788 | 3.73× | ok |
| bg/pidigits/5 | 0.134 | 0.318 | 2.37× | 0.610 | 4.55× | ok |
| bg/revcomp/1 | 0.140 | 0.389 | 2.78× | 0.631 | 4.51× | ok |
| bg/revcomp/5 | 0.144 | 0.381 | 2.65× | 0.600 | 4.17× | ok |
| bg/spectralnorm/1 | 0.187 | 0.604 | 3.23× | 0.660 | 3.53× | ok |
| box2d | 2.975 | 11.924 | 4.01× | 13.051 | 4.39× | ok |
| fmt | 1.445 | 8.332 | 5.77× | 7.283 | 5.04× | ok |
| hexxagon | 0.492 | 1.593 | 3.24× | 1.990 | 4.04× | ok |
| lambda | 0.157 | 1.514 | 9.64× | 0.793 | 5.05× | ok |
| minisat | 0.222 | 1.541 | 6.94× | 1.086 | 4.89× | ok |
| nlohmann-json | 1.245 | 4.581 | 3.68× | 5.157 | 4.14× | ok |
| pugixml | 0.992 | 8.320 | 8.39× | 5.847 | 5.89× | ok |
| pugixml-tests | 8.801 | 55.958 | 6.36× | 42.449 | 4.82× | ok |
| rapidjson | 0.342 | 2.017 | 5.90× | 1.243 | 3.63× | ok |
| re2 | 5.206 | 29.319 | 5.63× | 16.763 | 3.22× | ok |
| re2-test-re2 | 9.803 | 40.845 | 4.17× | 41.399 | 4.22× | ok |
| re2-test-search | 9.386 | 37.855 | 4.03× | 37.388 | 3.98× | ok |
| tramp3d-v4 | 3.092 | 28.727 | 9.29× | 20.957 | 6.78× | ok |

### Frontend only: -fsyntax-only

| Program | cppgm++ (s) | GCC (s) | Speed vs GCC ↑ | Clang (s) | Speed vs Clang ↑ | Status |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| Adobe-C++/functionobjects | 0.098 | 0.231 | 2.36× | 0.291 | 2.97× | ok |
| Adobe-C++/loop_unroll | 0.058 | 0.172 | 2.97× | 0.210 | 3.62× | ok |
| Adobe-C++/simple_types_constant_folding | 0.062 | 0.188 | 3.03× | 0.225 | 3.63× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.058 | 0.161 | 2.78× | 0.300 | 5.17× | ok |
| Adobe-C++/stepanov_abstraction | 0.049 | 0.115 | 2.35× | 0.161 | 3.29× | ok |
| Adobe-C++/stepanov_vector | 0.104 | 0.186 | 1.79× | 0.320 | 3.08× | ok |
| Bullet | 7.590 | 16.634 | 2.19× | 26.018 | 3.43× | ok |
| CLAMR | 3.208 | 7.516 | 2.34× | 12.855 | 4.01× | ok |
| CoyoteBench/almabench | 0.045 | 0.115 | 2.56× | 0.130 | 2.89× | ok |
| CoyoteBench/fftbench | 0.185 | 0.372 | 2.01× | 0.742 | 4.01× | ok |
| CoyoteBench/huffbench | 0.046 | 0.091 | 1.98× | 0.183 | 3.98× | ok |
| CoyoteBench/lpbench | 0.058 | 0.122 | 2.10× | 0.127 | 2.19× | ok |
| DOE/HACCKernels | 0.164 | 0.372 | 2.27× | 0.582 | 3.55× | ok |
| DOE/HPCCG | 0.977 | 2.158 | 2.21× | 4.448 | 4.55× | ok |
| DOE/PENNANT | 2.159 | 4.607 | 2.13× | 6.711 | 3.11× | ok |
| DOE/miniFE | 0.966 | 2.018 | 2.09× | 3.568 | 3.69× | ok |
| Misc-C++-EH/spirit | 0.542 | 1.428 | 2.63× | 2.479 | 4.57× | ok |
| Misc-C++-Large/ray | 0.185 | 0.416 | 2.25× | 0.600 | 3.24× | ok |
| Misc-C++-Large/sphereflake | 0.167 | 0.328 | 1.96× | 0.501 | 3.00× | ok |
| Misc-C++/bigfib | 0.159 | 0.397 | 2.50× | 0.570 | 3.58× | ok |
| Misc-C++/oopack_v1p8 | 0.050 | 0.116 | 2.32× | 0.175 | 3.50× | ok |
| Misc-C++/stepanov_container | 0.212 | 0.568 | 2.68× | 0.740 | 3.49× | ok |
| Misc-C++/stepanov_v1p2 | 0.046 | 0.093 | 2.02× | 0.145 | 3.15× | ok |
| PAQ8p | 0.067 | 0.184 | 2.75× | 0.238 | 3.55× | ok |
| Prolangs-C++/life | 0.004 | 0.015 | — | 0.029 | — | ok |
| Prolangs-C++/ocean | 0.015 | 0.039 | 2.60× | 0.058 | 3.87× | ok |
| Prolangs-C++/primes | 0.009 | 0.030 | — | 0.039 | — | ok |
| Shootout-C++-EH/except | 0.120 | 0.277 | 2.31× | 0.417 | 3.48× | ok |
| Shootout-C++/ackermann | 0.119 | 0.272 | 2.29× | 0.353 | 2.97× | ok |
| Shootout-C++/ary3 | 0.134 | 0.279 | 2.08× | 0.417 | 3.11× | ok |
| Shootout-C++/fibo | 0.106 | 0.251 | 2.37× | 0.372 | 3.51× | ok |
| Shootout-C++/hash | 0.142 | 0.322 | 2.27× | 0.500 | 3.52× | ok |
| Shootout-C++/hash2 | 0.160 | 0.349 | 2.18× | 0.473 | 2.96× | ok |
| Shootout-C++/heapsort | 0.133 | 0.296 | 2.23× | 0.420 | 3.16× | ok |
| Shootout-C++/lists | 0.120 | 0.261 | 2.18× | 0.406 | 3.38× | ok |
| Shootout-C++/lists1 | 0.133 | 0.291 | 2.19× | 0.426 | 3.20× | ok |
| Shootout-C++/matrix | 0.111 | 0.244 | 2.20× | 0.360 | 3.24× | ok |
| Shootout-C++/methcall | 0.106 | 0.230 | 2.17× | 0.429 | 4.05× | ok |
| Shootout-C++/moments | 0.190 | 0.312 | 1.64× | 0.447 | 2.35× | ok |
| Shootout-C++/random | 0.139 | 0.257 | 1.85× | 0.434 | 3.12× | ok |
| Shootout-C++/sieve | 0.128 | 0.274 | 2.14× | 0.452 | 3.53× | ok |
| SmallPT | 0.041 | 0.091 | 2.22× | 0.132 | 3.22× | ok |
| Stanford/Bubblesort | 0.014 | 0.037 | 2.64× | 0.046 | 3.29× | ok |
| Stanford/FloatMM | 0.014 | 0.036 | 2.57× | 0.049 | 3.50× | ok |
| Stanford/Puzzle | 0.015 | 0.039 | 2.60× | 0.047 | 3.13× | ok |
| Stanford/Treesort | 0.014 | 0.037 | 2.64× | 0.046 | 3.29× | ok |
| bg/binarytrees/2 | 0.107 | 0.242 | 2.26× | 0.368 | 3.44× | ok |
| bg/fannkuchredux/3 | 0.054 | 0.108 | 2.00× | 0.150 | 2.78× | ok |
| bg/fasta/1 | 0.087 | 0.161 | 1.85× | 0.222 | 2.55× | ok |
| bg/fasta/2 | 0.079 | 0.154 | 1.95× | 0.231 | 2.92× | ok |
| bg/fasta/3 | 0.124 | 0.267 | 2.15× | 0.408 | 3.29× | ok |
| bg/fasta/4 | 0.112 | 0.240 | 2.14× | 0.472 | 4.21× | ok |
| bg/fasta/6 | 0.197 | 0.443 | 2.25× | 0.725 | 3.68× | ok |
| bg/mandelbrot/0 | 0.263 | 0.481 | 1.83× | 0.795 | 3.02× | ok |
| bg/mandelbrot/2 | 0.124 | 0.303 | 2.44× | 0.489 | 3.94× | ok |
| bg/mandelbrot/3 | 0.159 | 0.321 | 2.02× | 0.502 | 3.16× | ok |
| bg/mandelbrot/5 | 0.067 | 0.143 | 2.13× | 0.207 | 3.09× | ok |
| bg/mandelbrot/7 | 0.070 | 0.152 | 2.17× | 0.210 | 3.00× | ok |
| bg/mandelbrot/9 | 0.065 | 0.130 | 2.00× | 0.215 | 3.31× | ok |
| bg/nbody/1 | 0.044 | 0.122 | 2.77× | 0.134 | 3.05× | ok |
| bg/nbody/2 | 0.043 | 0.089 | 2.07× | 0.127 | 2.95× | ok |
| bg/nbody/3 | 0.043 | 0.087 | 2.02× | 0.148 | 3.44× | ok |
| bg/nbody/6 | 0.041 | 0.095 | 2.32× | 0.142 | 3.46× | ok |
| bg/pidigits/4 | 0.185 | 0.440 | 2.38× | 0.798 | 4.31× | ok |
| bg/pidigits/5 | 0.123 | 0.287 | 2.33× | 0.399 | 3.24× | ok |
| bg/revcomp/1 | 0.128 | 0.286 | 2.23× | 0.428 | 3.34× | ok |
| bg/revcomp/5 | 0.132 | 0.256 | 1.94× | 0.445 | 3.37× | ok |
| bg/spectralnorm/1 | 0.199 | 0.392 | 1.97× | 0.765 | 3.84× | ok |
| box2d | 2.328 | 4.811 | 2.07× | 9.644 | 4.14× | ok |
| fmt | 0.736 | 1.756 | 2.39× | 3.214 | 4.37× | ok |
| hexxagon | 0.433 | 1.042 | 2.41× | 1.259 | 2.91× | ok |
| lambda | 0.077 | 0.200 | 2.60× | 0.288 | 3.74× | ok |
| minisat | 0.085 | 0.255 | 3.00× | 0.370 | 4.35× | ok |
| nlohmann-json | 0.484 | 1.451 | 3.00× | 2.595 | 5.36× | ok |
| pugixml | 0.378 | 0.978 | 2.59× | 1.287 | 3.40× | ok |
| pugixml-tests | 5.425 | 13.101 | 2.41× | 20.026 | 3.69× | ok |
| rapidjson | 0.177 | 0.394 | 2.23× | 0.602 | 3.40× | ok |
| re2 | 5.229 | 11.619 | 2.22× | 11.019 | 2.11× | ok |
| re2-test-re2 | 6.792 | 17.281 | 2.54× | 26.033 | 3.83× | ok |
| re2-test-search | 6.901 | 16.096 | 2.33× | 25.626 | 3.71× | ok |
| tramp3d-v4 | 0.674 | 3.905 | 5.79× | 2.963 | 4.40× | ok |


## Toolchains

- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)
- **valgrind**: valgrind-3.26.0

[Compiler build and codegen results](self-build.md) have their own measured revision.
