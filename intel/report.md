# Public benchmarks: Intel Xeon (dedicated)

Measured compiler: [`6c589867b`](https://github.com/vishvananda/cppgm-extended/commit/6c589867bdd5c99bc35b90267a46778f59e1127e) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/42675346808) · [Raw program results](results.json)

81 of 81 programs built with cppgm++ and g++ and agreed. Each number is the reference over cppgm++, so above 1× means cppgm++ is smaller or faster.

CPU: Intel(R) Xeon(R) CPU E5-2696 v4 @ 2.20GHz.

| Measure | vs g++ | Programs | vs clang++ | Programs |
| --- | ---: | ---: | ---: | ---: |
| Frontend CPU, `-fsyntax-only` | 2.32× | 80 | 3.64× | 80 |
| Compile CPU, `-O0` | 3.07× | 80 | 3.74× | 80 |
| Compile CPU, `-O3` | 4.41× | 81 | 4.51× | 81 |
| Compile peak RSS, `-O0` | 1.37× | 81 | 2.06× | 81 |
| Compile peak RSS, `-O3` | 1.54× | 81 | 2.02× | 81 |
| Executable size, `-O0` | 0.94× | 81 | 0.88× | 81 |
| Executable size, `-O3` | 0.93× | 81 | 0.88× | 81 |
| Run CPU, `-O0` | 1.01× | 81 | 0.99× | 81 |
| Run CPU, `-O3` | 0.98× | 81 | 0.96× | 81 |
| Run instructions, `-O3` | 0.92× | 43 | - | 0 |

O3 execution uses paired ABBA/BAAB blocks. GCC A/A CPU ratios ranged from 0.897 to 1.201; raw observations and block ratios are retained in the artifact.

## Slowest generated code, by run-time instructions

| Program | vs g++ |
| --- | ---: |
| tramp3d-v4 | 0.18× |
| bg/nbody/2 | 0.64× |
| Stanford/Bubblesort | 0.72× |
| PAQ8p | 0.74× |
| bg/spectralnorm/1 | 0.75× |
| bg/nbody/1 | 0.83× |
| bg/mandelbrot/7 | 0.84× |
| bg/mandelbrot/9 | 0.84× |
| bg/nbody/3 | 0.85× |
| Misc-C++/oopack_v1p8 | 0.86× |

## Compile CPU times

Times are CPU seconds, excluding linking. Speed is reference time divided by cppgm++ time; higher is better. Every program is listed. Missing times show a dash. Ratios are omitted for failed programs or when either time is below 10 ms.

Compilation is measured once per compiler and level; the ABBA/BAAB repetitions apply to generated-program execution and self-built compiler speed.

### O0 compilation

| Program | cppgm++ (s) | GCC (s) | Speed vs GCC ↑ | Clang (s) | Speed vs Clang ↑ | Status |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| Adobe-C++/functionobjects | 0.115 | 0.360 | 3.13× | 0.384 | 3.34× | ok |
| Adobe-C++/loop_unroll | 0.091 | 0.824 | 9.05× | 0.415 | 4.56× | ok |
| Adobe-C++/simple_types_constant_folding | 0.095 | 0.951 | 10.01× | 0.521 | 5.48× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.081 | 0.686 | 8.47× | 0.403 | 4.98× | ok |
| Adobe-C++/stepanov_abstraction | 0.058 | 0.270 | 4.66× | 0.244 | 4.21× | ok |
| Adobe-C++/stepanov_vector | 0.117 | 0.477 | 4.08× | 0.422 | 3.61× | ok |
| Bullet | 7.661 | 26.972 | 3.52× | 32.352 | 4.22× | ok |
| CLAMR | 3.851 | 13.847 | 3.60× | 15.558 | 4.04× | ok |
| CoyoteBench/almabench | 0.051 | 0.135 | 2.65× | 0.156 | 3.06× | ok |
| CoyoteBench/fftbench | 0.170 | 0.443 | 2.61× | 0.681 | 4.01× | ok |
| CoyoteBench/huffbench | 0.055 | 0.101 | 1.84× | 0.166 | 3.02× | ok |
| CoyoteBench/lpbench | 0.044 | 0.126 | 2.86× | 0.152 | 3.45× | ok |
| DOE/HACCKernels | 0.176 | 0.477 | 2.71× | 0.626 | 3.56× | ok |
| DOE/HPCCG | 1.106 | 2.531 | 2.29× | 3.883 | 3.51× | ok |
| DOE/PENNANT | 2.324 | 6.081 | 2.62× | 8.410 | 3.62× | ok |
| DOE/miniFE | 0.982 | 3.022 | 3.08× | 3.724 | 3.79× | ok |
| Misc-C++-EH/spirit | 0.664 | 4.052 | 6.10× | 3.232 | 4.87× | ok |
| Misc-C++-Large/ray | 0.195 | 0.415 | 2.13× | 0.595 | 3.05× | ok |
| Misc-C++-Large/sphereflake | 0.144 | 0.339 | 2.35× | 0.523 | 3.63× | ok |
| Misc-C++/bigfib | 0.182 | 0.575 | 3.16× | 0.741 | 4.07× | ok |
| Misc-C++/oopack_v1p8 | 0.058 | 0.155 | 2.67× | 0.181 | 3.12× | ok |
| Misc-C++/stepanov_container | 0.247 | 0.841 | 3.40× | 0.922 | 3.73× | ok |
| Misc-C++/stepanov_v1p2 | 0.049 | 0.179 | 3.65× | 0.190 | 3.88× | ok |
| PAQ8p | 0.096 | 0.650 | 6.77× | 0.403 | 4.20× | ok |
| Prolangs-C++/life | 0.007 | 0.051 | — | 0.047 | — | ok |
| Prolangs-C++/ocean | 0.020 | 0.103 | 5.15× | 0.084 | 4.20× | ok |
| Prolangs-C++/primes | 0.010 | 0.032 | 3.20× | 0.049 | 4.90× | ok |
| Shootout-C++-EH/except | 0.113 | 0.259 | 2.29× | 0.437 | 3.87× | ok |
| Shootout-C++/ackermann | 0.115 | 0.276 | 2.40× | 0.411 | 3.57× | ok |
| Shootout-C++/ary3 | 0.134 | 0.307 | 2.29× | 0.482 | 3.60× | ok |
| Shootout-C++/fibo | 0.114 | 0.250 | 2.19× | 0.407 | 3.57× | ok |
| Shootout-C++/hash | 0.172 | 0.460 | 2.67× | 0.592 | 3.44× | ok |
| Shootout-C++/hash2 | 0.166 | 0.471 | 2.84× | 0.613 | 3.69× | ok |
| Shootout-C++/heapsort | 0.166 | 0.290 | 1.75× | 0.509 | 3.07× | ok |
| Shootout-C++/lists | 0.129 | 0.332 | 2.57× | 0.493 | 3.82× | ok |
| Shootout-C++/lists1 | 0.144 | 0.362 | 2.51× | 0.532 | 3.69× | ok |
| Shootout-C++/matrix | 0.120 | 0.250 | 2.08× | 0.412 | 3.43× | ok |
| Shootout-C++/methcall | 0.116 | 0.308 | 2.66× | 0.417 | 3.59× | ok |
| Shootout-C++/moments | 0.160 | 0.460 | 2.88× | 0.541 | 3.38× | ok |
| Shootout-C++/random | 0.139 | 0.301 | 2.17× | 0.501 | 3.60× | ok |
| Shootout-C++/sieve | 0.148 | 0.386 | 2.61× | 0.516 | 3.49× | ok |
| SmallPT | 0.046 | 0.179 | 3.89× | 0.165 | 3.59× | ok |
| Stanford/Bubblesort | 0.015 | 0.050 | 3.33× | 0.062 | 4.13× | ok |
| Stanford/FloatMM | 0.014 | 0.050 | 3.57× | 0.059 | 4.21× | ok |
| Stanford/Puzzle | 0.016 | 0.052 | 3.25× | 0.061 | 3.81× | ok |
| Stanford/Treesort | 0.019 | 0.042 | 2.21× | 0.064 | 3.37× | ok |
| bg/binarytrees/2 | 0.114 | 0.282 | 2.47× | 0.450 | 3.95× | ok |
| bg/fannkuchredux/3 | 0.055 | 0.115 | 2.09× | 0.177 | 3.22× | ok |
| bg/fasta/1 | 0.085 | 0.199 | 2.34× | 0.280 | 3.29× | ok |
| bg/fasta/2 | 0.091 | 0.196 | 2.15× | 0.282 | 3.10× | ok |
| bg/fasta/3 | 0.131 | 0.294 | 2.24× | 0.486 | 3.71× | ok |
| bg/fasta/4 | 0.125 | 0.239 | 1.91× | 0.369 | 2.95× | ok |
| bg/fasta/6 | 0.217 | 0.729 | 3.36× | 0.858 | 3.95× | ok |
| bg/mandelbrot/0 | 0.249 | 0.721 | 2.90× | 0.878 | 3.53× | ok |
| bg/mandelbrot/2 | 0.142 | 0.348 | 2.45× | 0.541 | 3.81× | ok |
| bg/mandelbrot/3 | 0.148 | 0.328 | 2.22× | 0.539 | 3.64× | ok |
| bg/mandelbrot/5 | 0.072 | 0.173 | 2.40× | 0.247 | 3.43× | ok |
| bg/mandelbrot/7 | 0.093 | 0.199 | 2.14× | 0.257 | 2.76× | ok |
| bg/mandelbrot/9 | 0.070 | 0.220 | 3.14× | 0.241 | 3.44× | ok |
| bg/nbody/1 | 0.044 | 0.149 | 3.39× | 0.188 | 4.27× | ok |
| bg/nbody/2 | 0.040 | 0.100 | 2.50× | 0.156 | 3.90× | ok |
| bg/nbody/3 | 0.040 | 0.101 | 2.53× | 0.153 | 3.82× | ok |
| bg/nbody/6 | 0.045 | 0.155 | 3.44× | 0.189 | 4.20× | ok |
| bg/pidigits/4 | 0.194 | 0.483 | 2.49× | 0.774 | 3.99× | ok |
| bg/pidigits/5 | 0.121 | 0.270 | 2.23× | 0.482 | 3.98× | ok |
| bg/revcomp/1 | 0.136 | 0.395 | 2.90× | 0.472 | 3.47× | ok |
| bg/revcomp/5 | 0.118 | 0.316 | 2.68× | 0.436 | 3.69× | ok |
| bg/spectralnorm/1 | 0.180 | 0.427 | 2.37× | 0.608 | 3.38× | ok |
| box2d | 2.308 | 6.463 | 2.80× | 8.837 | 3.83× | ok |
| fmt | 0.819 | 3.555 | 4.34× | 3.425 | 4.18× | ok |
| hexxagon | 0.439 | 0.975 | 2.22× | 1.407 | 3.21× | ok |
| lambda | 0.091 | 0.449 | 4.93× | 0.372 | 4.09× | ok |
| minisat | 0.095 | 0.447 | 4.71× | 0.387 | 4.07× | ok |
| nlohmann-json | 0.603 | 2.823 | 4.68× | 2.974 | 4.93× | ok |
| pugixml | 0.400 | 1.927 | 4.82× | 1.749 | 4.37× | ok |
| pugixml-tests | 5.424 | 22.026 | 4.06× | 21.552 | 3.97× | ok |
| rapidjson | 0.176 | 0.685 | 3.89× | 0.630 | 3.58× | ok |
| re2 | 4.908 | 15.525 | 3.16× | 17.523 | 3.57× | ok |
| re2-test-re2 | 7.060 | 21.622 | 3.06× | 25.455 | 3.61× | ok |
| re2-test-search | 7.073 | 20.856 | 2.95× | 25.212 | 3.56× | ok |
| tramp3d-v4 | 1.494 | 11.499 | 7.70× | 6.283 | 4.21× | ok |

### O3 compilation

| Program | cppgm++ (s) | GCC (s) | Speed vs GCC ↑ | Clang (s) | Speed vs Clang ↑ | Status |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| Adobe-C++/functionobjects | 0.164 | 1.381 | 8.42× | 0.838 | 5.11× | ok |
| Adobe-C++/loop_unroll | 0.455 | 8.211 | 18.05× | 6.485 | 14.25× | ok |
| Adobe-C++/simple_types_constant_folding | 0.299 | 4.426 | 14.80× | 3.532 | 11.81× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.244 | 3.422 | 14.02× | 2.758 | 11.30× | ok |
| Adobe-C++/stepanov_abstraction | 0.104 | 1.558 | 14.98× | 0.762 | 7.33× | ok |
| Adobe-C++/stepanov_vector | 0.162 | 2.089 | 12.90× | 0.965 | 5.96× | ok |
| Bullet | 9.947 | 56.533 | 5.68× | 48.108 | 4.84× | ok |
| CLAMR | 6.535 | 37.252 | 5.70× | 31.414 | 4.81× | ok |
| CoyoteBench/almabench | 0.050 | 0.233 | 4.66× | 0.225 | 4.50× | ok |
| CoyoteBench/fftbench | 0.192 | 0.649 | 3.38× | 0.783 | 4.08× | ok |
| CoyoteBench/huffbench | 0.057 | 0.350 | 6.14× | 0.298 | 5.23× | ok |
| CoyoteBench/lpbench | 0.065 | 0.257 | 3.95× | 0.312 | 4.80× | ok |
| DOE/HACCKernels | 0.188 | 0.574 | 3.05× | 0.705 | 3.75× | ok |
| DOE/HPCCG | 1.105 | 3.593 | 3.25× | 4.446 | 4.02× | ok |
| DOE/PENNANT | 2.734 | 9.544 | 3.49× | 10.824 | 3.96× | ok |
| DOE/miniFE | 1.346 | 6.218 | 4.62× | 5.728 | 4.26× | ok |
| Misc-C++-EH/spirit | 1.108 | 5.412 | 4.88× | 5.346 | 4.82× | ok |
| Misc-C++-Large/ray | 0.182 | 0.696 | 3.82× | 0.660 | 3.63× | ok |
| Misc-C++-Large/sphereflake | 0.164 | 1.112 | 6.78× | 0.628 | 3.83× | ok |
| Misc-C++/bigfib | 0.226 | 0.820 | 3.63× | 0.913 | 4.04× | ok |
| Misc-C++/oopack_v1p8 | 0.067 | 0.263 | 3.93× | 0.267 | 3.99× | ok |
| Misc-C++/stepanov_container | 0.319 | 1.338 | 4.19× | 1.534 | 4.81× | ok |
| Misc-C++/stepanov_v1p2 | 0.057 | 0.332 | 5.82× | 0.296 | 5.19× | ok |
| PAQ8p | 0.405 | 5.627 | 13.89× | 2.814 | 6.95× | ok |
| Prolangs-C++/life | 0.011 | 0.154 | 14.00× | 0.093 | 8.45× | ok |
| Prolangs-C++/ocean | 0.027 | 0.300 | 11.11× | 0.209 | 7.74× | ok |
| Prolangs-C++/primes | 0.011 | 0.052 | 4.73× | 0.056 | 5.09× | ok |
| Shootout-C++-EH/except | 0.124 | 0.298 | 2.40× | 0.433 | 3.49× | ok |
| Shootout-C++/ackermann | 0.116 | 0.336 | 2.90× | 0.426 | 3.67× | ok |
| Shootout-C++/ary3 | 0.138 | 0.381 | 2.76× | 0.509 | 3.69× | ok |
| Shootout-C++/fibo | 0.116 | 0.304 | 2.62× | 0.406 | 3.50× | ok |
| Shootout-C++/hash | 0.200 | 0.622 | 3.11× | 0.781 | 3.91× | ok |
| Shootout-C++/hash2 | 0.194 | 0.667 | 3.44× | 0.858 | 4.42× | ok |
| Shootout-C++/heapsort | 0.136 | 0.352 | 2.59× | 0.576 | 4.24× | ok |
| Shootout-C++/lists | 0.141 | 0.442 | 3.13× | 0.527 | 3.74× | ok |
| Shootout-C++/lists1 | 0.163 | 0.440 | 2.70× | 0.583 | 3.58× | ok |
| Shootout-C++/matrix | 0.116 | 0.367 | 3.16× | 0.570 | 4.91× | ok |
| Shootout-C++/methcall | 0.121 | 0.303 | 2.50× | 0.439 | 3.63× | ok |
| Shootout-C++/moments | 0.178 | 0.504 | 2.83× | 0.629 | 3.53× | ok |
| Shootout-C++/random | 0.139 | 0.332 | 2.39× | 0.502 | 3.61× | ok |
| Shootout-C++/sieve | 0.152 | 0.426 | 2.80× | 0.563 | 3.70× | ok |
| SmallPT | 0.057 | 0.432 | 7.58× | 0.303 | 5.32× | ok |
| Stanford/Bubblesort | 0.018 | 0.082 | 4.56× | 0.084 | 4.67× | ok |
| Stanford/FloatMM | 0.018 | 0.094 | 5.22× | 0.108 | 6.00× | ok |
| Stanford/Puzzle | 0.023 | 0.180 | 7.83× | 0.187 | 8.13× | ok |
| Stanford/Treesort | 0.021 | 0.124 | 5.90× | 0.087 | 4.14× | ok |
| bg/binarytrees/2 | 0.118 | 0.640 | 5.42× | 0.431 | 3.65× | ok |
| bg/fannkuchredux/3 | 0.061 | 0.196 | 3.21× | 0.210 | 3.44× | ok |
| bg/fasta/1 | 0.098 | 0.255 | 2.60× | 0.330 | 3.37× | ok |
| bg/fasta/2 | 0.091 | 0.267 | 2.93× | 0.346 | 3.80× | ok |
| bg/fasta/3 | 0.154 | 0.347 | 2.25× | 0.500 | 3.25× | ok |
| bg/fasta/4 | 0.129 | 0.303 | 2.35× | 0.429 | 3.33× | ok |
| bg/fasta/6 | 0.234 | 0.730 | 3.12× | 0.942 | 4.03× | ok |
| bg/mandelbrot/0 | 0.286 | 0.993 | 3.47× | 1.181 | 4.13× | ok |
| bg/mandelbrot/2 | 0.151 | 0.427 | 2.83× | 0.594 | 3.93× | ok |
| bg/mandelbrot/3 | 0.152 | 0.375 | 2.47× | 0.588 | 3.87× | ok |
| bg/mandelbrot/5 | 0.077 | 0.224 | 2.91× | 0.298 | 3.87× | ok |
| bg/mandelbrot/7 | 0.094 | 0.307 | 3.27× | 0.324 | 3.45× | ok |
| bg/mandelbrot/9 | 0.102 | 0.280 | 2.75× | 0.312 | 3.06× | ok |
| bg/nbody/1 | 0.061 | 0.233 | 3.82× | 0.316 | 5.18× | ok |
| bg/nbody/2 | 0.047 | 0.195 | 4.15× | 0.214 | 4.55× | ok |
| bg/nbody/3 | 0.046 | 0.186 | 4.04× | 0.204 | 4.43× | ok |
| bg/nbody/6 | 0.060 | 0.241 | 4.02× | 0.317 | 5.28× | ok |
| bg/pidigits/4 | 0.223 | 0.545 | 2.44× | 0.848 | 3.80× | ok |
| bg/pidigits/5 | 0.123 | 0.322 | 2.62× | 0.472 | 3.84× | ok |
| bg/revcomp/1 | 0.141 | 0.393 | 2.79× | 0.536 | 3.80× | ok |
| bg/revcomp/5 | 0.125 | 0.351 | 2.81× | 0.503 | 4.02× | ok |
| bg/spectralnorm/1 | 0.186 | 0.546 | 2.94× | 0.701 | 3.77× | ok |
| box2d | 2.574 | 10.933 | 4.25× | 11.022 | 4.28× | ok |
| fmt | 1.610 | 7.894 | 4.90× | 6.543 | 4.06× | ok |
| hexxagon | 0.454 | 1.606 | 3.54× | 1.758 | 3.87× | ok |
| lambda | 0.157 | 1.642 | 10.46× | 0.843 | 5.37× | ok |
| minisat | 0.189 | 1.447 | 7.66× | 0.967 | 5.12× | ok |
| nlohmann-json | 1.210 | 5.053 | 4.18× | 4.517 | 3.73× | ok |
| pugixml | 1.089 | 8.101 | 7.44× | 5.506 | 5.06× | ok |
| pugixml-tests | 8.248 | 50.878 | 6.17× | 37.438 | 4.54× | ok |
| rapidjson | 0.317 | 1.731 | 5.46× | 1.272 | 4.01× | ok |
| re2 | 5.804 | 26.174 | 4.51× | 22.017 | 3.79× | ok |
| re2-test-re2 | 8.582 | 38.115 | 4.44× | 34.262 | 3.99× | ok |
| re2-test-search | 8.186 | 34.738 | 4.24× | 32.352 | 3.95× | ok |
| tramp3d-v4 | 4.253 | 32.465 | 7.63× | 26.998 | 6.35× | ok |

### Frontend only: -fsyntax-only

| Program | cppgm++ (s) | GCC (s) | Speed vs GCC ↑ | Clang (s) | Speed vs Clang ↑ | Status |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| Adobe-C++/functionobjects | 0.103 | 0.219 | 2.13× | 0.329 | 3.19× | ok |
| Adobe-C++/loop_unroll | 0.060 | 0.165 | 2.75× | 0.241 | 4.02× | ok |
| Adobe-C++/simple_types_constant_folding | 0.072 | 0.184 | 2.56× | 0.258 | 3.58× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.063 | 0.143 | 2.27× | 0.230 | 3.65× | ok |
| Adobe-C++/stepanov_abstraction | 0.047 | 0.109 | 2.32× | 0.193 | 4.11× | ok |
| Adobe-C++/stepanov_vector | 0.122 | 0.252 | 2.07× | 0.349 | 2.86× | ok |
| Bullet | 7.221 | 17.140 | 2.37× | 27.770 | 3.85× | ok |
| CLAMR | 3.486 | 8.063 | 2.31× | 12.570 | 3.61× | ok |
| CoyoteBench/almabench | 0.044 | 0.091 | 2.07× | 0.148 | 3.36× | ok |
| CoyoteBench/fftbench | 0.165 | 0.411 | 2.49× | 0.603 | 3.65× | ok |
| CoyoteBench/huffbench | 0.043 | 0.090 | 2.09× | 0.157 | 3.65× | ok |
| CoyoteBench/lpbench | 0.044 | 0.086 | 1.95× | 0.150 | 3.41× | ok |
| DOE/HACCKernels | 0.180 | 0.448 | 2.49× | 0.642 | 3.57× | ok |
| DOE/HPCCG | 0.999 | 2.281 | 2.28× | 3.743 | 3.75× | ok |
| DOE/PENNANT | 2.198 | 4.679 | 2.13× | 7.416 | 3.37× | ok |
| DOE/miniFE | 0.882 | 2.167 | 2.46× | 3.274 | 3.71× | ok |
| Misc-C++-EH/spirit | 0.473 | 1.429 | 3.02× | 2.196 | 4.64× | ok |
| Misc-C++-Large/ray | 0.191 | 0.339 | 1.77× | 0.539 | 2.82× | ok |
| Misc-C++-Large/sphereflake | 0.140 | 0.353 | 2.52× | 0.553 | 3.95× | ok |
| Misc-C++/bigfib | 0.165 | 0.404 | 2.45× | 0.593 | 3.59× | ok |
| Misc-C++/oopack_v1p8 | 0.044 | 0.125 | 2.84× | 0.156 | 3.55× | ok |
| Misc-C++/stepanov_container | 0.229 | 0.494 | 2.16× | 0.786 | 3.43× | ok |
| Misc-C++/stepanov_v1p2 | 0.043 | 0.105 | 2.44× | 0.176 | 4.09× | ok |
| PAQ8p | 0.066 | 0.183 | 2.77× | 0.263 | 3.98× | ok |
| Prolangs-C++/life | 0.004 | 0.018 | — | 0.034 | — | ok |
| Prolangs-C++/ocean | 0.013 | 0.041 | 3.15× | 0.060 | 4.62× | ok |
| Prolangs-C++/primes | 0.010 | 0.023 | 2.30× | 0.044 | 4.40× | ok |
| Shootout-C++-EH/except | 0.111 | 0.294 | 2.65× | 0.404 | 3.64× | ok |
| Shootout-C++/ackermann | 0.114 | 0.256 | 2.25× | 0.393 | 3.45× | ok |
| Shootout-C++/ary3 | 0.126 | 0.280 | 2.22× | 0.452 | 3.59× | ok |
| Shootout-C++/fibo | 0.115 | 0.249 | 2.17× | 0.395 | 3.43× | ok |
| Shootout-C++/hash | 0.160 | 0.341 | 2.13× | 0.558 | 3.49× | ok |
| Shootout-C++/hash2 | 0.159 | 0.376 | 2.36× | 0.558 | 3.51× | ok |
| Shootout-C++/heapsort | 0.162 | 0.307 | 1.90× | 0.473 | 2.92× | ok |
| Shootout-C++/lists | 0.131 | 0.284 | 2.17× | 0.425 | 3.24× | ok |
| Shootout-C++/lists1 | 0.139 | 0.305 | 2.19× | 0.496 | 3.57× | ok |
| Shootout-C++/matrix | 0.119 | 0.254 | 2.13× | 0.412 | 3.46× | ok |
| Shootout-C++/methcall | 0.116 | 0.253 | 2.18× | 0.416 | 3.59× | ok |
| Shootout-C++/moments | 0.153 | 0.380 | 2.48× | 0.536 | 3.50× | ok |
| Shootout-C++/random | 0.141 | 0.298 | 2.11× | 0.458 | 3.25× | ok |
| Shootout-C++/sieve | 0.140 | 0.317 | 2.26× | 0.494 | 3.53× | ok |
| SmallPT | 0.040 | 0.086 | 2.15× | 0.149 | 3.73× | ok |
| Stanford/Bubblesort | 0.014 | 0.036 | 2.57× | 0.052 | 3.71× | ok |
| Stanford/FloatMM | 0.013 | 0.037 | 2.85× | 0.057 | 4.38× | ok |
| Stanford/Puzzle | 0.013 | 0.040 | 3.08× | 0.055 | 4.23× | ok |
| Stanford/Treesort | 0.018 | 0.035 | 1.94× | 0.056 | 3.11× | ok |
| bg/binarytrees/2 | 0.112 | 0.294 | 2.62× | 0.399 | 3.56× | ok |
| bg/fannkuchredux/3 | 0.053 | 0.122 | 2.30× | 0.172 | 3.25× | ok |
| bg/fasta/1 | 0.082 | 0.161 | 1.96× | 0.266 | 3.24× | ok |
| bg/fasta/2 | 0.083 | 0.163 | 1.96× | 0.315 | 3.80× | ok |
| bg/fasta/3 | 0.127 | 0.278 | 2.19× | 0.496 | 3.91× | ok |
| bg/fasta/4 | 0.136 | 0.245 | 1.80× | 0.379 | 2.79× | ok |
| bg/fasta/6 | 0.198 | 0.450 | 2.27× | 0.757 | 3.82× | ok |
| bg/mandelbrot/0 | 0.216 | 0.494 | 2.29× | 0.774 | 3.58× | ok |
| bg/mandelbrot/2 | 0.149 | 0.313 | 2.10× | 0.488 | 3.28× | ok |
| bg/mandelbrot/3 | 0.142 | 0.318 | 2.24× | 0.532 | 3.75× | ok |
| bg/mandelbrot/5 | 0.085 | 0.144 | 1.69× | 0.227 | 2.67× | ok |
| bg/mandelbrot/7 | 0.069 | 0.149 | 2.16× | 0.234 | 3.39× | ok |
| bg/mandelbrot/9 | 0.065 | 0.131 | 2.02× | 0.216 | 3.32× | ok |
| bg/nbody/1 | 0.042 | 0.100 | 2.38× | 0.161 | 3.83× | ok |
| bg/nbody/2 | 0.039 | 0.084 | 2.15× | 0.148 | 3.79× | ok |
| bg/nbody/3 | 0.039 | 0.082 | 2.10× | 0.148 | 3.79× | ok |
| bg/nbody/6 | 0.042 | 0.092 | 2.19× | 0.183 | 4.36× | ok |
| bg/pidigits/4 | 0.185 | 0.445 | 2.41× | 0.697 | 3.77× | ok |
| bg/pidigits/5 | 0.119 | 0.285 | 2.39× | 0.410 | 3.45× | ok |
| bg/revcomp/1 | 0.126 | 0.291 | 2.31× | 0.516 | 4.10× | ok |
| bg/revcomp/5 | 0.113 | 0.276 | 2.44× | 0.396 | 3.50× | ok |
| bg/spectralnorm/1 | 0.167 | 0.404 | 2.42× | 0.637 | 3.81× | ok |
| box2d | 2.121 | 4.506 | 2.12× | 7.740 | 3.65× | ok |
| fmt | 0.678 | 1.661 | 2.45× | 2.841 | 4.19× | ok |
| hexxagon | 0.393 | 0.887 | 2.26× | 1.379 | 3.51× | ok |
| lambda | 0.074 | 0.203 | 2.74× | 0.305 | 4.12× | ok |
| minisat | 0.080 | 0.198 | 2.48× | 0.304 | 3.80× | ok |
| nlohmann-json | 0.512 | 1.326 | 2.59× | 2.445 | 4.78× | ok |
| pugixml | 0.309 | 0.790 | 2.56× | 1.302 | 4.21× | ok |
| pugixml-tests | 4.643 | 11.900 | 2.56× | 17.056 | 3.67× | ok |
| rapidjson | 0.139 | 0.388 | 2.79× | 0.491 | 3.53× | ok |
| re2 | 4.265 | 10.047 | 2.36× | 14.795 | 3.47× | ok |
| re2-test-re2 | 6.421 | 14.520 | 2.26× | 22.374 | 3.48× | ok |
| re2-test-search | 6.465 | 14.721 | 2.28× | 22.449 | 3.47× | ok |
| tramp3d-v4 | 0.868 | 3.120 | 3.59× | 3.664 | 4.22× | ok |


## Toolchains

- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)
- **valgrind**: valgrind-3.26.0

[Compiler build and codegen results](self-build.md) have their own measured revision.
