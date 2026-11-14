# Public benchmarks: AMD EPYC

Measured compiler: [`9ed2f1eea`](https://github.com/vishvananda/cppgm-extended/commit/9ed2f1eea641727dd56efede0b4ca09c045e9f7f) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/43601267078) · [Raw program results](results.json)

81 of 81 programs built with cppgm++ and g++ and agreed. Each number is the reference over cppgm++, so above 1× means cppgm++ is smaller or faster.

CPU: AMD EPYC 7763 64-Core Processor.

| Measure | vs g++ | Programs | vs clang++ | Programs |
| --- | ---: | ---: | ---: | ---: |
| Frontend CPU, `-fsyntax-only` | 2.29× | 79 | 3.36× | 79 |
| Compile CPU, `-O0` | 2.99× | 79 | 3.57× | 79 |
| Compile CPU, `-O3` | 4.34× | 81 | 4.35× | 81 |
| Compile peak RSS, `-O0` | 1.51× | 81 | 2.44× | 81 |
| Compile peak RSS, `-O3` | 1.77× | 81 | 2.53× | 81 |
| Executable size, `-O0` | 0.93× | 81 | 0.87× | 81 |
| Executable size, `-O3` | 0.95× | 81 | 0.91× | 81 |
| Run CPU, `-O0` | 1.18× | 81 | 1.24× | 81 |
| Run CPU, `-O3` | 0.98× | 81 | 0.98× | 80 |
| Run instructions, `-O3` | 0.96× | 43 | - | 0 |

O3 execution uses paired ABBA/BAAB blocks. GCC A/A CPU ratios ranged from 0.981 to 1.023; raw observations and block ratios are retained in the artifact.

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
| Adobe-C++/functionobjects | 0.113 | 0.414 | 3.66× | 0.359 | 3.18× | ok |
| Adobe-C++/loop_unroll | 0.086 | 0.838 | 9.74× | 0.367 | 4.27× | ok |
| Adobe-C++/simple_types_constant_folding | 0.118 | 0.966 | 8.19× | 0.472 | 4.00× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.083 | 0.695 | 8.37× | 0.425 | 5.12× | ok |
| Adobe-C++/stepanov_abstraction | 0.079 | 0.266 | 3.37× | 0.292 | 3.70× | ok |
| Adobe-C++/stepanov_vector | 0.111 | 0.427 | 3.85× | 0.378 | 3.41× | ok |
| Bullet | 7.766 | 27.608 | 3.55× | 28.647 | 3.69× | ok |
| CLAMR | 4.408 | 14.426 | 3.27× | 14.159 | 3.21× | ok |
| CoyoteBench/almabench | 0.046 | 0.110 | 2.39× | 0.148 | 3.22× | ok |
| CoyoteBench/fftbench | 0.163 | 0.383 | 2.35× | 0.564 | 3.46× | ok |
| CoyoteBench/huffbench | 0.060 | 0.114 | 1.90× | 0.184 | 3.07× | ok |
| CoyoteBench/lpbench | 0.044 | 0.110 | 2.50× | 0.135 | 3.07× | ok |
| DOE/HACCKernels | 0.173 | 0.404 | 2.34× | 0.584 | 3.38× | ok |
| DOE/HPCCG | 1.082 | 2.842 | 2.63× | 3.545 | 3.28× | ok |
| DOE/PENNANT | 2.216 | 5.809 | 2.62× | 7.511 | 3.39× | ok |
| DOE/miniFE | 1.094 | 2.975 | 2.72× | 4.328 | 3.96× | ok |
| Misc-C++-EH/spirit | 0.804 | 3.948 | 4.91× | 3.683 | 4.58× | ok |
| Misc-C++-Large/ray | 0.181 | 0.463 | 2.56× | 0.578 | 3.19× | ok |
| Misc-C++-Large/sphereflake | 0.177 | 0.324 | 1.83× | 0.560 | 3.16× | ok |
| Misc-C++/bigfib | 0.175 | 0.534 | 3.05× | 0.624 | 3.57× | ok |
| Misc-C++/oopack_v1p8 | 0.048 | 0.131 | 2.73× | 0.155 | 3.23× | ok |
| Misc-C++/stepanov_container | 0.264 | 0.868 | 3.29× | 1.012 | 3.83× | ok |
| Misc-C++/stepanov_v1p2 | 0.050 | 0.183 | 3.66× | 0.237 | 4.74× | ok |
| PAQ8p | 0.111 | 0.764 | 6.88× | 0.444 | 4.00× | ok |
| Prolangs-C++/life | 0.006 | 0.048 | — | 0.037 | — | ok |
| Prolangs-C++/ocean | 0.018 | 0.090 | 5.00× | 0.079 | 4.39× | ok |
| Prolangs-C++/primes | 0.009 | 0.037 | — | 0.041 | — | ok |
| Shootout-C++-EH/except | 0.124 | 0.267 | 2.15× | 0.439 | 3.54× | ok |
| Shootout-C++/ackermann | 0.109 | 0.266 | 2.44× | 0.377 | 3.46× | ok |
| Shootout-C++/ary3 | 0.131 | 0.300 | 2.29× | 0.434 | 3.31× | ok |
| Shootout-C++/fibo | 0.114 | 0.237 | 2.08× | 0.505 | 4.43× | ok |
| Shootout-C++/hash | 0.163 | 0.413 | 2.53× | 0.552 | 3.39× | ok |
| Shootout-C++/hash2 | 0.165 | 0.543 | 3.29× | 0.542 | 3.28× | ok |
| Shootout-C++/heapsort | 0.134 | 0.285 | 2.13× | 0.431 | 3.22× | ok |
| Shootout-C++/lists | 0.136 | 0.324 | 2.38× | 0.476 | 3.50× | ok |
| Shootout-C++/lists1 | 0.138 | 0.371 | 2.69× | 0.471 | 3.41× | ok |
| Shootout-C++/matrix | 0.114 | 0.247 | 2.17× | 0.387 | 3.39× | ok |
| Shootout-C++/methcall | 0.112 | 0.250 | 2.23× | 0.349 | 3.12× | ok |
| Shootout-C++/moments | 0.148 | 0.364 | 2.46× | 0.559 | 3.78× | ok |
| Shootout-C++/random | 0.137 | 0.275 | 2.01× | 0.480 | 3.50× | ok |
| Shootout-C++/sieve | 0.138 | 0.372 | 2.70× | 0.493 | 3.57× | ok |
| SmallPT | 0.049 | 0.138 | 2.82× | 0.153 | 3.12× | ok |
| Stanford/Bubblesort | 0.014 | 0.043 | 3.07× | 0.050 | 3.57× | ok |
| Stanford/FloatMM | 0.014 | 0.043 | 3.07× | 0.051 | 3.64× | ok |
| Stanford/Puzzle | 0.016 | 0.055 | 3.44× | 0.051 | 3.19× | ok |
| Stanford/Treesort | 0.015 | 0.043 | 2.87× | 0.055 | 3.67× | ok |
| bg/binarytrees/2 | 0.107 | 0.251 | 2.35× | 0.374 | 3.50× | ok |
| bg/fannkuchredux/3 | 0.052 | 0.108 | 2.08× | 0.151 | 2.90× | ok |
| bg/fasta/1 | 0.081 | 0.191 | 2.36× | 0.248 | 3.06× | ok |
| bg/fasta/2 | 0.083 | 0.191 | 2.30× | 0.254 | 3.06× | ok |
| bg/fasta/3 | 0.127 | 0.288 | 2.27× | 0.435 | 3.43× | ok |
| bg/fasta/4 | 0.121 | 0.238 | 1.97× | 0.485 | 4.01× | ok |
| bg/fasta/6 | 0.222 | 0.587 | 2.64× | 0.955 | 4.30× | ok |
| bg/mandelbrot/0 | 0.233 | 0.666 | 2.86× | 0.771 | 3.31× | ok |
| bg/mandelbrot/2 | 0.141 | 0.385 | 2.73× | 0.576 | 4.09× | ok |
| bg/mandelbrot/3 | 0.149 | 0.315 | 2.11× | 0.499 | 3.35× | ok |
| bg/mandelbrot/5 | 0.068 | 0.167 | 2.46× | 0.211 | 3.10× | ok |
| bg/mandelbrot/7 | 0.071 | 0.189 | 2.66× | 0.239 | 3.37× | ok |
| bg/mandelbrot/9 | 0.071 | 0.224 | 3.15× | 0.214 | 3.01× | ok |
| bg/nbody/1 | 0.045 | 0.162 | 3.60× | 0.159 | 3.53× | ok |
| bg/nbody/2 | 0.042 | 0.104 | 2.48× | 0.130 | 3.10× | ok |
| bg/nbody/3 | 0.046 | 0.104 | 2.26× | 0.133 | 2.89× | ok |
| bg/nbody/6 | 0.047 | 0.158 | 3.36× | 0.243 | 5.17× | ok |
| bg/pidigits/4 | 0.184 | 0.477 | 2.59× | 0.746 | 4.05× | ok |
| bg/pidigits/5 | 0.121 | 0.288 | 2.38× | 0.510 | 4.21× | ok |
| bg/revcomp/1 | 0.162 | 0.333 | 2.06× | 0.450 | 2.78× | ok |
| bg/revcomp/5 | 0.134 | 0.304 | 2.27× | 0.516 | 3.85× | ok |
| bg/spectralnorm/1 | 0.187 | 0.399 | 2.13× | 0.573 | 3.06× | ok |
| box2d | 2.572 | 6.832 | 2.66× | 10.742 | 4.18× | ok |
| fmt | 0.872 | 3.676 | 4.22× | 3.115 | 3.57× | ok |
| hexxagon | 0.391 | 1.068 | 2.73× | 1.661 | 4.25× | ok |
| lambda | 0.098 | 0.507 | 5.17× | 0.329 | 3.36× | ok |
| minisat | 0.106 | 0.411 | 3.88× | 0.477 | 4.50× | ok |
| nlohmann-json | 0.713 | 2.944 | 4.13× | 2.737 | 3.84× | ok |
| pugixml | 0.458 | 2.093 | 4.57× | 1.999 | 4.36× | ok |
| pugixml-tests | 6.085 | 23.232 | 3.82× | 22.386 | 3.68× | ok |
| rapidjson | 0.199 | 0.709 | 3.56× | 0.734 | 3.69× | ok |
| re2 | 5.406 | 16.217 | 3.00× | 12.465 | 2.31× | ok |
| re2-test-re2 | 7.651 | 24.548 | 3.21× | 29.553 | 3.86× | ok |
| re2-test-search | 7.286 | 23.328 | 3.20× | 27.314 | 3.75× | ok |
| tramp3d-v4 | 1.046 | 12.455 | 11.91× | 4.663 | 4.46× | ok |

### O3 compilation

| Program | cppgm++ (s) | GCC (s) | Speed vs GCC ↑ | Clang (s) | Speed vs Clang ↑ | Status |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| Adobe-C++/functionobjects | 0.166 | 1.346 | 8.11× | 1.055 | 6.36× | ok |
| Adobe-C++/loop_unroll | 0.575 | 8.241 | 14.33× | 7.359 | 12.80× | ok |
| Adobe-C++/simple_types_constant_folding | 0.362 | 4.221 | 11.66× | 3.407 | 9.41× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.306 | 3.287 | 10.74× | 2.979 | 9.74× | ok |
| Adobe-C++/stepanov_abstraction | 0.152 | 1.471 | 9.68× | 0.820 | 5.39× | ok |
| Adobe-C++/stepanov_vector | 0.186 | 2.116 | 11.38× | 1.130 | 6.08× | ok |
| Bullet | 9.920 | 57.058 | 5.75× | 44.330 | 4.47× | ok |
| CLAMR | 7.115 | 34.031 | 4.78× | 28.367 | 3.99× | ok |
| CoyoteBench/almabench | 0.053 | 0.237 | 4.47× | 0.241 | 4.55× | ok |
| CoyoteBench/fftbench | 0.186 | 0.611 | 3.28× | 0.730 | 3.92× | ok |
| CoyoteBench/huffbench | 0.056 | 0.380 | 6.79× | 0.222 | 3.96× | ok |
| CoyoteBench/lpbench | 0.055 | 0.261 | 4.75× | 0.295 | 5.36× | ok |
| DOE/HACCKernels | 0.182 | 0.559 | 3.07× | 0.664 | 3.65× | ok |
| DOE/HPCCG | 1.080 | 4.252 | 3.94× | 4.033 | 3.73× | ok |
| DOE/PENNANT | 2.620 | 8.983 | 3.43× | 10.408 | 3.97× | ok |
| DOE/miniFE | 1.381 | 5.832 | 4.22× | 5.449 | 3.95× | ok |
| Misc-C++-EH/spirit | 0.967 | 5.750 | 5.95× | 5.059 | 5.23× | ok |
| Misc-C++-Large/ray | 0.182 | 0.773 | 4.25× | 0.614 | 3.37× | ok |
| Misc-C++-Large/sphereflake | 0.175 | 1.154 | 6.59× | 0.675 | 3.86× | ok |
| Misc-C++/bigfib | 0.212 | 0.768 | 3.62× | 0.875 | 4.13× | ok |
| Misc-C++/oopack_v1p8 | 0.055 | 0.241 | 4.38× | 0.250 | 4.55× | ok |
| Misc-C++/stepanov_container | 0.344 | 1.263 | 3.67× | 1.712 | 4.98× | ok |
| Misc-C++/stepanov_v1p2 | 0.062 | 0.305 | 4.92× | 0.288 | 4.65× | ok |
| PAQ8p | 0.421 | 6.297 | 14.96× | 2.945 | 7.00× | ok |
| Prolangs-C++/life | 0.011 | 0.132 | 12.00× | 0.090 | 8.18× | ok |
| Prolangs-C++/ocean | 0.031 | 0.294 | 9.48× | 0.177 | 5.71× | ok |
| Prolangs-C++/primes | 0.011 | 0.052 | 4.73× | 0.052 | 4.73× | ok |
| Shootout-C++-EH/except | 0.122 | 0.314 | 2.57× | 0.462 | 3.79× | ok |
| Shootout-C++/ackermann | 0.117 | 0.331 | 2.83× | 0.382 | 3.26× | ok |
| Shootout-C++/ary3 | 0.127 | 0.382 | 3.01× | 0.635 | 5.00× | ok |
| Shootout-C++/fibo | 0.116 | 0.299 | 2.58× | 0.519 | 4.47× | ok |
| Shootout-C++/hash | 0.178 | 0.549 | 3.08× | 0.685 | 3.85× | ok |
| Shootout-C++/hash2 | 0.196 | 0.641 | 3.27× | 0.751 | 3.83× | ok |
| Shootout-C++/heapsort | 0.139 | 0.337 | 2.42× | 0.455 | 3.27× | ok |
| Shootout-C++/lists | 0.131 | 0.361 | 2.76× | 0.465 | 3.55× | ok |
| Shootout-C++/lists1 | 0.151 | 0.411 | 2.72× | 0.523 | 3.46× | ok |
| Shootout-C++/matrix | 0.125 | 0.424 | 3.39× | 0.545 | 4.36× | ok |
| Shootout-C++/methcall | 0.143 | 0.304 | 2.13× | 0.401 | 2.80× | ok |
| Shootout-C++/moments | 0.153 | 0.493 | 3.22× | 0.560 | 3.66× | ok |
| Shootout-C++/random | 0.137 | 0.305 | 2.23× | 0.472 | 3.45× | ok |
| Shootout-C++/sieve | 0.143 | 0.346 | 2.42× | 0.513 | 3.59× | ok |
| SmallPT | 0.066 | 0.465 | 7.05× | 0.252 | 3.82× | ok |
| Stanford/Bubblesort | 0.017 | 0.080 | 4.71× | 0.072 | 4.24× | ok |
| Stanford/FloatMM | 0.019 | 0.091 | 4.79× | 0.102 | 5.37× | ok |
| Stanford/Puzzle | 0.026 | 0.185 | 7.12× | 0.168 | 6.46× | ok |
| Stanford/Treesort | 0.021 | 0.123 | 5.86× | 0.087 | 4.14× | ok |
| bg/binarytrees/2 | 0.112 | 0.617 | 5.51× | 0.395 | 3.53× | ok |
| bg/fannkuchredux/3 | 0.057 | 0.171 | 3.00× | 0.188 | 3.30× | ok |
| bg/fasta/1 | 0.089 | 0.248 | 2.79× | 0.307 | 3.45× | ok |
| bg/fasta/2 | 0.090 | 0.246 | 2.73× | 0.302 | 3.36× | ok |
| bg/fasta/3 | 0.131 | 0.343 | 2.62× | 0.490 | 3.74× | ok |
| bg/fasta/4 | 0.139 | 0.289 | 2.08× | 0.430 | 3.09× | ok |
| bg/fasta/6 | 0.240 | 0.677 | 2.82× | 0.866 | 3.61× | ok |
| bg/mandelbrot/0 | 0.260 | 0.949 | 3.65× | 1.318 | 5.07× | ok |
| bg/mandelbrot/2 | 0.141 | 0.422 | 2.99× | 0.554 | 3.93× | ok |
| bg/mandelbrot/3 | 0.147 | 0.394 | 2.68× | 0.626 | 4.26× | ok |
| bg/mandelbrot/5 | 0.075 | 0.220 | 2.93× | 0.253 | 3.37× | ok |
| bg/mandelbrot/7 | 0.084 | 0.279 | 3.32× | 0.294 | 3.50× | ok |
| bg/mandelbrot/9 | 0.084 | 0.278 | 3.31× | 0.282 | 3.36× | ok |
| bg/nbody/1 | 0.061 | 0.239 | 3.92× | 0.275 | 4.51× | ok |
| bg/nbody/2 | 0.045 | 0.185 | 4.11× | 0.224 | 4.98× | ok |
| bg/nbody/3 | 0.049 | 0.222 | 4.53× | 0.202 | 4.12× | ok |
| bg/nbody/6 | 0.066 | 0.249 | 3.77× | 0.352 | 5.33× | ok |
| bg/pidigits/4 | 0.191 | 0.528 | 2.76× | 0.817 | 4.28× | ok |
| bg/pidigits/5 | 0.133 | 0.338 | 2.54× | 0.419 | 3.15× | ok |
| bg/revcomp/1 | 0.180 | 0.366 | 2.03× | 0.514 | 2.86× | ok |
| bg/revcomp/5 | 0.139 | 0.377 | 2.71× | 0.557 | 4.01× | ok |
| bg/spectralnorm/1 | 0.183 | 0.571 | 3.12× | 0.648 | 3.54× | ok |
| box2d | 3.025 | 11.863 | 3.92× | 12.389 | 4.10× | ok |
| fmt | 1.515 | 7.938 | 5.24× | 7.141 | 4.71× | ok |
| hexxagon | 0.423 | 1.725 | 4.08× | 1.973 | 4.66× | ok |
| lambda | 0.156 | 1.766 | 11.32× | 0.782 | 5.01× | ok |
| minisat | 0.193 | 1.602 | 8.30× | 1.062 | 5.50× | ok |
| nlohmann-json | 1.180 | 4.540 | 3.85× | 5.522 | 4.68× | ok |
| pugixml | 1.036 | 7.034 | 6.79× | 6.323 | 6.10× | ok |
| pugixml-tests | 8.591 | 53.181 | 6.19× | 43.730 | 5.09× | ok |
| rapidjson | 0.339 | 1.866 | 5.50× | 1.189 | 3.51× | ok |
| re2 | 4.938 | 27.522 | 5.57× | 16.730 | 3.39× | ok |
| re2-test-re2 | 9.693 | 44.059 | 4.55× | 40.343 | 4.16× | ok |
| re2-test-search | 10.213 | 38.696 | 3.79× | 25.814 | 2.53× | ok |
| tramp3d-v4 | 2.998 | 25.272 | 8.43× | 20.581 | 6.86× | ok |

### Frontend only: -fsyntax-only

| Program | cppgm++ (s) | GCC (s) | Speed vs GCC ↑ | Clang (s) | Speed vs Clang ↑ | Status |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| Adobe-C++/functionobjects | 0.106 | 0.239 | 2.25× | 0.312 | 2.94× | ok |
| Adobe-C++/loop_unroll | 0.059 | 0.185 | 3.14× | 0.211 | 3.58× | ok |
| Adobe-C++/simple_types_constant_folding | 0.076 | 0.180 | 2.37× | 0.242 | 3.18× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.062 | 0.159 | 2.56× | 0.204 | 3.29× | ok |
| Adobe-C++/stepanov_abstraction | 0.066 | 0.114 | 1.73× | 0.231 | 3.50× | ok |
| Adobe-C++/stepanov_vector | 0.115 | 0.207 | 1.80× | 0.308 | 2.68× | ok |
| Bullet | 6.966 | 17.742 | 2.55× | 23.813 | 3.42× | ok |
| CLAMR | 3.940 | 7.992 | 2.03× | 11.416 | 2.90× | ok |
| CoyoteBench/almabench | 0.045 | 0.122 | 2.71× | 0.141 | 3.13× | ok |
| CoyoteBench/fftbench | 0.161 | 0.345 | 2.14× | 0.525 | 3.26× | ok |
| CoyoteBench/huffbench | 0.052 | 0.100 | 1.92× | 0.175 | 3.37× | ok |
| CoyoteBench/lpbench | 0.044 | 0.092 | 2.09× | 0.131 | 2.98× | ok |
| DOE/HACCKernels | 0.172 | 0.371 | 2.16× | 0.541 | 3.15× | ok |
| DOE/HPCCG | 0.969 | 2.266 | 2.34× | 3.309 | 3.41× | ok |
| DOE/PENNANT | 2.416 | 4.536 | 1.88× | 7.010 | 2.90× | ok |
| DOE/miniFE | 0.985 | 2.000 | 2.03× | 3.931 | 3.99× | ok |
| Misc-C++-EH/spirit | 0.575 | 1.407 | 2.45× | 2.437 | 4.24× | ok |
| Misc-C++-Large/ray | 0.179 | 0.392 | 2.19× | 0.535 | 2.99× | ok |
| Misc-C++-Large/sphereflake | 0.176 | 0.291 | 1.65× | 0.544 | 3.09× | ok |
| Misc-C++/bigfib | 0.160 | 0.372 | 2.33× | 0.548 | 3.43× | ok |
| Misc-C++/oopack_v1p8 | 0.046 | 0.097 | 2.11× | 0.142 | 3.09× | ok |
| Misc-C++/stepanov_container | 0.241 | 0.531 | 2.20× | 0.857 | 3.56× | ok |
| Misc-C++/stepanov_v1p2 | 0.047 | 0.107 | 2.28× | 0.203 | 4.32× | ok |
| PAQ8p | 0.084 | 0.236 | 2.81× | 0.269 | 3.20× | ok |
| Prolangs-C++/life | 0.004 | 0.016 | — | 0.026 | — | ok |
| Prolangs-C++/ocean | 0.015 | 0.034 | 2.27× | 0.052 | 3.47× | ok |
| Prolangs-C++/primes | 0.009 | 0.027 | — | 0.038 | — | ok |
| Shootout-C++-EH/except | 0.120 | 0.271 | 2.26× | 0.430 | 3.58× | ok |
| Shootout-C++/ackermann | 0.111 | 0.276 | 2.49× | 0.364 | 3.28× | ok |
| Shootout-C++/ary3 | 0.127 | 0.280 | 2.20× | 0.398 | 3.13× | ok |
| Shootout-C++/fibo | 0.110 | 0.240 | 2.18× | 0.372 | 3.38× | ok |
| Shootout-C++/hash | 0.156 | 0.321 | 2.06× | 0.495 | 3.17× | ok |
| Shootout-C++/hash2 | 0.157 | 0.444 | 2.83× | 0.490 | 3.12× | ok |
| Shootout-C++/heapsort | 0.135 | 0.288 | 2.13× | 0.420 | 3.11× | ok |
| Shootout-C++/lists | 0.130 | 0.279 | 2.15× | 0.477 | 3.67× | ok |
| Shootout-C++/lists1 | 0.134 | 0.311 | 2.32× | 0.427 | 3.19× | ok |
| Shootout-C++/matrix | 0.114 | 0.245 | 2.15× | 0.368 | 3.23× | ok |
| Shootout-C++/methcall | 0.112 | 0.248 | 2.21× | 0.337 | 3.01× | ok |
| Shootout-C++/moments | 0.137 | 0.300 | 2.19× | 0.505 | 3.69× | ok |
| Shootout-C++/random | 0.137 | 0.276 | 2.01× | 0.470 | 3.43× | ok |
| Shootout-C++/sieve | 0.134 | 0.307 | 2.29× | 0.452 | 3.37× | ok |
| SmallPT | 0.047 | 0.094 | 2.00× | 0.138 | 2.94× | ok |
| Stanford/Bubblesort | 0.014 | 0.038 | 2.71× | 0.047 | 3.36× | ok |
| Stanford/FloatMM | 0.015 | 0.038 | 2.53× | 0.049 | 3.27× | ok |
| Stanford/Puzzle | 0.016 | 0.041 | 2.56× | 0.047 | 2.94× | ok |
| Stanford/Treesort | 0.015 | 0.036 | 2.40× | 0.048 | 3.20× | ok |
| bg/binarytrees/2 | 0.107 | 0.268 | 2.50× | 0.352 | 3.29× | ok |
| bg/fannkuchredux/3 | 0.051 | 0.097 | 1.90× | 0.143 | 2.80× | ok |
| bg/fasta/1 | 0.079 | 0.155 | 1.96× | 0.232 | 2.94× | ok |
| bg/fasta/2 | 0.079 | 0.155 | 1.96× | 0.238 | 3.01× | ok |
| bg/fasta/3 | 0.127 | 0.270 | 2.13× | 0.411 | 3.24× | ok |
| bg/fasta/4 | 0.121 | 0.231 | 1.91× | 0.452 | 3.74× | ok |
| bg/fasta/6 | 0.191 | 0.433 | 2.27× | 0.760 | 3.98× | ok |
| bg/mandelbrot/0 | 0.211 | 0.479 | 2.27× | 0.701 | 3.32× | ok |
| bg/mandelbrot/2 | 0.165 | 0.349 | 2.12× | 0.608 | 3.68× | ok |
| bg/mandelbrot/3 | 0.144 | 0.329 | 2.28× | 0.475 | 3.30× | ok |
| bg/mandelbrot/5 | 0.065 | 0.177 | 2.72× | 0.198 | 3.05× | ok |
| bg/mandelbrot/7 | 0.069 | 0.143 | 2.07× | 0.209 | 3.03× | ok |
| bg/mandelbrot/9 | 0.062 | 0.176 | 2.84× | 0.227 | 3.66× | ok |
| bg/nbody/1 | 0.043 | 0.098 | 2.28× | 0.135 | 3.14× | ok |
| bg/nbody/2 | 0.041 | 0.088 | 2.15× | 0.121 | 2.95× | ok |
| bg/nbody/3 | 0.044 | 0.087 | 1.98× | 0.125 | 2.84× | ok |
| bg/nbody/6 | 0.046 | 0.095 | 2.07× | 0.197 | 4.28× | ok |
| bg/pidigits/4 | 0.180 | 0.434 | 2.41× | 0.637 | 3.54× | ok |
| bg/pidigits/5 | 0.120 | 0.284 | 2.37× | 0.430 | 3.58× | ok |
| bg/revcomp/1 | 0.161 | 0.279 | 1.73× | 0.434 | 2.70× | ok |
| bg/revcomp/5 | 0.125 | 0.318 | 2.54× | 0.458 | 3.66× | ok |
| bg/spectralnorm/1 | 0.167 | 0.375 | 2.25× | 0.543 | 3.25× | ok |
| box2d | 2.428 | 4.981 | 2.05× | 10.209 | 4.20× | ok |
| fmt | 0.723 | 1.752 | 2.42× | 2.510 | 3.47× | ok |
| hexxagon | 0.383 | 0.983 | 2.57× | 1.439 | 3.76× | ok |
| lambda | 0.074 | 0.195 | 2.64× | 0.242 | 3.27× | ok |
| minisat | 0.090 | 0.199 | 2.21× | 0.360 | 4.00× | ok |
| nlohmann-json | 0.465 | 1.413 | 3.04× | 2.075 | 4.46× | ok |
| pugixml | 0.309 | 0.897 | 2.90× | 1.471 | 4.76× | ok |
| pugixml-tests | 5.143 | 12.549 | 2.44× | 19.735 | 3.84× | ok |
| rapidjson | 0.175 | 0.399 | 2.28× | 0.645 | 3.69× | ok |
| re2 | 5.075 | 11.265 | 2.22× | 10.830 | 2.13× | ok |
| re2-test-re2 | 7.122 | 16.385 | 2.30× | 26.194 | 3.68× | ok |
| re2-test-search | 6.492 | 16.192 | 2.49× | 27.067 | 4.17× | ok |
| tramp3d-v4 | 0.644 | 3.930 | 6.10× | 2.780 | 4.32× | ok |


## Toolchains

- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)
- **valgrind**: valgrind-3.26.0

[Compiler build and codegen results](self-build.md) have their own measured revision.
