# Public benchmarks: Intel (GitHub)

Measured compiler: [`393d1b1d0`](https://github.com/vishvananda/cppgm-extended/commit/393d1b1d09089ffdade676f4e7cbad8759c15d2a) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/43357224748) · [Raw program results](results.json)

81 of 81 programs built with cppgm++ and g++ and agreed. Each number is the reference over cppgm++, so above 1× means cppgm++ is smaller or faster.

CPU: Intel(R) Xeon(R) 6973P-C.

| Measure | vs g++ | Programs | vs clang++ | Programs |
| --- | ---: | ---: | ---: | ---: |
| Frontend CPU, `-fsyntax-only` | 2.25× | 74 | 2.90× | 74 |
| Compile CPU, `-O0` | 3.21× | 76 | 3.15× | 76 |
| Compile CPU, `-O3` | 4.41× | 79 | 3.89× | 79 |
| Compile peak RSS, `-O0` | 1.45× | 81 | 2.20× | 81 |
| Compile peak RSS, `-O3` | 1.71× | 81 | 2.30× | 81 |
| Executable size, `-O0` | 0.93× | 81 | 0.87× | 81 |
| Executable size, `-O3` | 0.96× | 81 | 0.92× | 81 |
| Run CPU, `-O0` | 1.11× | 81 | 1.08× | 81 |
| Run CPU, `-O3` | 0.98× | 81 | 0.97× | 80 |
| Run instructions, `-O3` | 0.95× | 43 | - | 0 |

O3 execution uses paired ABBA/BAAB blocks. GCC A/A CPU ratios ranged from 0.907 to 1.014; raw observations and block ratios are retained in the artifact.

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
| Adobe-C++/functionobjects | 0.076 | 0.268 | 3.53× | 0.184 | 2.42× | ok |
| Adobe-C++/loop_unroll | 0.058 | 0.615 | 10.60× | 0.187 | 3.22× | ok |
| Adobe-C++/simple_types_constant_folding | 0.061 | 0.705 | 11.56× | 0.227 | 3.72× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.053 | 0.506 | 9.55× | 0.199 | 3.75× | ok |
| Adobe-C++/stepanov_abstraction | 0.036 | 0.185 | 5.14× | 0.115 | 3.19× | ok |
| Adobe-C++/stepanov_vector | 0.068 | 0.289 | 4.25× | 0.254 | 3.74× | ok |
| Bullet | 5.032 | 19.718 | 3.92× | 16.421 | 3.26× | ok |
| CLAMR | 2.513 | 10.424 | 4.15× | 9.456 | 3.76× | ok |
| CoyoteBench/almabench | 0.027 | 0.070 | 2.59× | 0.080 | 2.96× | ok |
| CoyoteBench/fftbench | 0.100 | 0.268 | 2.68× | 0.305 | 3.05× | ok |
| CoyoteBench/huffbench | 0.030 | 0.069 | 2.30× | 0.076 | 2.53× | ok |
| CoyoteBench/lpbench | 0.027 | 0.073 | 2.70× | 0.073 | 2.70× | ok |
| DOE/HACCKernels | 0.105 | 0.261 | 2.49× | 0.325 | 3.10× | ok |
| DOE/HPCCG | 0.632 | 1.627 | 2.57× | 2.355 | 3.73× | ok |
| DOE/PENNANT | 1.404 | 4.613 | 3.29× | 4.618 | 3.29× | ok |
| DOE/miniFE | 0.591 | 1.961 | 3.32× | 1.950 | 3.30× | ok |
| Misc-C++-EH/spirit | 0.465 | 2.900 | 6.24× | 2.042 | 4.39× | ok |
| Misc-C++-Large/ray | 0.100 | 0.256 | 2.56× | 0.292 | 2.92× | ok |
| Misc-C++-Large/sphereflake | 0.085 | 0.214 | 2.52× | 0.257 | 3.02× | ok |
| Misc-C++/bigfib | 0.117 | 0.366 | 3.13× | 0.455 | 3.89× | ok |
| Misc-C++/oopack_v1p8 | 0.031 | 0.088 | 2.84× | 0.087 | 2.81× | ok |
| Misc-C++/stepanov_container | 0.163 | 0.515 | 3.16× | 0.468 | 2.87× | ok |
| Misc-C++/stepanov_v1p2 | 0.032 | 0.108 | 3.38× | 0.090 | 2.81× | ok |
| PAQ8p | 0.064 | 0.442 | 6.91× | 0.218 | 3.41× | ok |
| Prolangs-C++/life | 0.004 | 0.031 | — | 0.025 | — | ok |
| Prolangs-C++/ocean | 0.010 | 0.067 | 6.70× | 0.049 | 4.90× | ok |
| Prolangs-C++/primes | 0.005 | 0.023 | — | 0.024 | — | ok |
| Shootout-C++-EH/except | 0.074 | 0.171 | 2.31× | 0.244 | 3.30× | ok |
| Shootout-C++/ackermann | 0.067 | 0.149 | 2.22× | 0.205 | 3.06× | ok |
| Shootout-C++/ary3 | 0.084 | 0.207 | 2.46× | 0.294 | 3.50× | ok |
| Shootout-C++/fibo | 0.067 | 0.150 | 2.24× | 0.196 | 2.93× | ok |
| Shootout-C++/hash | 0.106 | 0.291 | 2.75× | 0.308 | 2.91× | ok |
| Shootout-C++/hash2 | 0.115 | 0.310 | 2.70× | 0.301 | 2.62× | ok |
| Shootout-C++/heapsort | 0.096 | 0.187 | 1.95× | 0.261 | 2.72× | ok |
| Shootout-C++/lists | 0.075 | 0.199 | 2.65× | 0.228 | 3.04× | ok |
| Shootout-C++/lists1 | 0.101 | 0.231 | 2.29× | 0.267 | 2.64× | ok |
| Shootout-C++/matrix | 0.066 | 0.182 | 2.76× | 0.204 | 3.09× | ok |
| Shootout-C++/methcall | 0.069 | 0.153 | 2.22× | 0.200 | 2.90× | ok |
| Shootout-C++/moments | 0.109 | 0.257 | 2.36× | 0.259 | 2.38× | ok |
| Shootout-C++/random | 0.088 | 0.178 | 2.02× | 0.247 | 2.81× | ok |
| Shootout-C++/sieve | 0.086 | 0.238 | 2.77× | 0.263 | 3.06× | ok |
| SmallPT | 0.028 | 0.081 | 2.89× | 0.090 | 3.21× | ok |
| Stanford/Bubblesort | 0.009 | 0.027 | — | 0.029 | — | ok |
| Stanford/FloatMM | 0.009 | 0.026 | — | 0.031 | — | ok |
| Stanford/Puzzle | 0.010 | 0.035 | 3.50× | 0.034 | 3.40× | ok |
| Stanford/Treesort | 0.009 | 0.029 | — | 0.031 | — | ok |
| bg/binarytrees/2 | 0.069 | 0.160 | 2.32× | 0.211 | 3.06× | ok |
| bg/fannkuchredux/3 | 0.033 | 0.072 | 2.18× | 0.082 | 2.48× | ok |
| bg/fasta/1 | 0.054 | 0.121 | 2.24× | 0.142 | 2.63× | ok |
| bg/fasta/2 | 0.051 | 0.124 | 2.43× | 0.146 | 2.86× | ok |
| bg/fasta/3 | 0.080 | 0.193 | 2.41× | 0.236 | 2.95× | ok |
| bg/fasta/4 | 0.074 | 0.153 | 2.07× | 0.188 | 2.54× | ok |
| bg/fasta/6 | 0.155 | 0.399 | 2.57× | 0.466 | 3.01× | ok |
| bg/mandelbrot/0 | 0.145 | 0.481 | 3.32× | 0.489 | 3.37× | ok |
| bg/mandelbrot/2 | 0.079 | 0.213 | 2.70× | 0.291 | 3.68× | ok |
| bg/mandelbrot/3 | 0.092 | 0.216 | 2.35× | 0.288 | 3.13× | ok |
| bg/mandelbrot/5 | 0.047 | 0.105 | 2.23× | 0.137 | 2.91× | ok |
| bg/mandelbrot/7 | 0.046 | 0.124 | 2.70× | 0.135 | 2.93× | ok |
| bg/mandelbrot/9 | 0.043 | 0.113 | 2.63× | 0.115 | 2.67× | ok |
| bg/nbody/1 | 0.031 | 0.106 | 3.42× | 0.096 | 3.10× | ok |
| bg/nbody/2 | 0.027 | 0.065 | 2.41× | 0.075 | 2.78× | ok |
| bg/nbody/3 | 0.026 | 0.074 | 2.85× | 0.076 | 2.92× | ok |
| bg/nbody/6 | 0.029 | 0.101 | 3.48× | 0.091 | 3.14× | ok |
| bg/pidigits/4 | 0.126 | 0.310 | 2.46× | 0.364 | 2.89× | ok |
| bg/pidigits/5 | 0.078 | 0.170 | 2.18× | 0.227 | 2.91× | ok |
| bg/revcomp/1 | 0.085 | 0.223 | 2.62× | 0.276 | 3.25× | ok |
| bg/revcomp/5 | 0.081 | 0.201 | 2.48× | 0.267 | 3.30× | ok |
| bg/spectralnorm/1 | 0.116 | 0.250 | 2.16× | 0.333 | 2.87× | ok |
| box2d | 1.473 | 4.238 | 2.88× | 3.540 | 2.40× | ok |
| fmt | 0.497 | 2.556 | 5.14× | 1.837 | 3.70× | ok |
| hexxagon | 0.254 | 0.661 | 2.60× | 0.857 | 3.37× | ok |
| lambda | 0.052 | 0.318 | 6.12× | 0.226 | 4.35× | ok |
| minisat | 0.056 | 0.297 | 5.30× | 0.240 | 4.29× | ok |
| nlohmann-json | 0.436 | 2.045 | 4.69× | 1.809 | 4.15× | ok |
| pugixml | 0.293 | 1.329 | 4.54× | 0.982 | 3.35× | ok |
| pugixml-tests | 3.336 | 15.714 | 4.71× | 12.730 | 3.82× | ok |
| rapidjson | 0.119 | 0.496 | 4.17× | 0.399 | 3.35× | ok |
| re2 | 2.163 | 9.950 | 4.60× | 7.497 | 3.47× | ok |
| re2-test-re2 | 4.654 | 16.102 | 3.46× | 15.013 | 3.23× | ok |
| re2-test-search | 4.632 | 14.726 | 3.18× | 14.939 | 3.23× | ok |
| tramp3d-v4 | 0.723 | 8.377 | 11.59× | 2.626 | 3.63× | ok |

### O3 compilation

| Program | cppgm++ (s) | GCC (s) | Speed vs GCC ↑ | Clang (s) | Speed vs Clang ↑ | Status |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| Adobe-C++/functionobjects | 0.122 | 0.973 | 7.98× | 0.605 | 4.96× | ok |
| Adobe-C++/loop_unroll | 0.365 | 5.851 | 16.03× | 4.408 | 12.08× | ok |
| Adobe-C++/simple_types_constant_folding | 0.236 | 3.387 | 14.35× | 2.688 | 11.39× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.188 | 2.501 | 13.30× | 1.969 | 10.47× | ok |
| Adobe-C++/stepanov_abstraction | 0.083 | 1.120 | 13.49× | 0.485 | 5.84× | ok |
| Adobe-C++/stepanov_vector | 0.116 | 1.274 | 10.98× | 0.671 | 5.78× | ok |
| Bullet | 6.356 | 41.322 | 6.50× | 28.610 | 4.50× | ok |
| CLAMR | 4.730 | 25.994 | 5.50× | 19.761 | 4.18× | ok |
| CoyoteBench/almabench | 0.032 | 0.164 | 5.12× | 0.124 | 3.88× | ok |
| CoyoteBench/fftbench | 0.125 | 0.430 | 3.44× | 0.415 | 3.32× | ok |
| CoyoteBench/huffbench | 0.041 | 0.240 | 5.85× | 0.141 | 3.44× | ok |
| CoyoteBench/lpbench | 0.034 | 0.169 | 4.97× | 0.180 | 5.29× | ok |
| DOE/HACCKernels | 0.123 | 0.376 | 3.06× | 0.401 | 3.26× | ok |
| DOE/HPCCG | 0.775 | 2.414 | 3.11× | 2.725 | 3.52× | ok |
| DOE/PENNANT | 1.848 | 6.895 | 3.73× | 5.952 | 3.22× | ok |
| DOE/miniFE | 0.842 | 4.106 | 4.88× | 3.217 | 3.82× | ok |
| Misc-C++-EH/spirit | 0.681 | 3.925 | 5.76× | 3.589 | 5.27× | ok |
| Misc-C++-Large/ray | 0.109 | 0.473 | 4.34× | 0.355 | 3.26× | ok |
| Misc-C++-Large/sphereflake | 0.106 | 0.740 | 6.98× | 0.426 | 4.02× | ok |
| Misc-C++/bigfib | 0.150 | 0.537 | 3.58× | 0.477 | 3.18× | ok |
| Misc-C++/oopack_v1p8 | 0.038 | 0.178 | 4.68× | 0.165 | 4.34× | ok |
| Misc-C++/stepanov_container | 0.217 | 0.962 | 4.43× | 0.839 | 3.87× | ok |
| Misc-C++/stepanov_v1p2 | 0.041 | 0.201 | 4.90× | 0.187 | 4.56× | ok |
| PAQ8p | 0.306 | 3.775 | 12.34× | 1.588 | 5.19× | ok |
| Prolangs-C++/life | 0.008 | 0.101 | — | 0.065 | — | ok |
| Prolangs-C++/ocean | 0.021 | 0.217 | 10.33× | 0.127 | 6.05× | ok |
| Prolangs-C++/primes | 0.007 | 0.035 | — | 0.032 | — | ok |
| Shootout-C++-EH/except | 0.076 | 0.205 | 2.70× | 0.266 | 3.50× | ok |
| Shootout-C++/ackermann | 0.073 | 0.193 | 2.64× | 0.219 | 3.00× | ok |
| Shootout-C++/ary3 | 0.091 | 0.263 | 2.89× | 0.294 | 3.23× | ok |
| Shootout-C++/fibo | 0.071 | 0.197 | 2.77× | 0.226 | 3.18× | ok |
| Shootout-C++/hash | 0.123 | 0.396 | 3.22× | 0.425 | 3.46× | ok |
| Shootout-C++/hash2 | 0.155 | 0.452 | 2.92× | 0.436 | 2.81× | ok |
| Shootout-C++/heapsort | 0.091 | 0.226 | 2.48× | 0.278 | 3.05× | ok |
| Shootout-C++/lists | 0.085 | 0.234 | 2.75× | 0.268 | 3.15× | ok |
| Shootout-C++/lists1 | 0.113 | 0.275 | 2.43× | 0.290 | 2.57× | ok |
| Shootout-C++/matrix | 0.071 | 0.214 | 3.01× | 0.318 | 4.48× | ok |
| Shootout-C++/methcall | 0.072 | 0.197 | 2.74× | 0.217 | 3.01× | ok |
| Shootout-C++/moments | 0.108 | 0.318 | 2.94× | 0.320 | 2.96× | ok |
| Shootout-C++/random | 0.088 | 0.209 | 2.38× | 0.260 | 2.95× | ok |
| Shootout-C++/sieve | 0.099 | 0.257 | 2.60× | 0.301 | 3.04× | ok |
| SmallPT | 0.043 | 0.288 | 6.70× | 0.175 | 4.07× | ok |
| Stanford/Bubblesort | 0.010 | 0.052 | 5.20× | 0.044 | 4.40× | ok |
| Stanford/FloatMM | 0.012 | 0.063 | 5.25× | 0.072 | 6.00× | ok |
| Stanford/Puzzle | 0.017 | 0.128 | 7.53× | 0.118 | 6.94× | ok |
| Stanford/Treesort | 0.014 | 0.086 | 6.14× | 0.052 | 3.71× | ok |
| bg/binarytrees/2 | 0.075 | 0.437 | 5.83× | 0.236 | 3.15× | ok |
| bg/fannkuchredux/3 | 0.039 | 0.120 | 3.08× | 0.118 | 3.03× | ok |
| bg/fasta/1 | 0.063 | 0.164 | 2.60× | 0.186 | 2.95× | ok |
| bg/fasta/2 | 0.059 | 0.165 | 2.80× | 0.180 | 3.05× | ok |
| bg/fasta/3 | 0.103 | 0.240 | 2.33× | 0.273 | 2.65× | ok |
| bg/fasta/4 | 0.089 | 0.192 | 2.16× | 0.229 | 2.57× | ok |
| bg/fasta/6 | 0.151 | 0.474 | 3.14× | 0.550 | 3.64× | ok |
| bg/mandelbrot/0 | 0.184 | 0.627 | 3.41× | 0.639 | 3.47× | ok |
| bg/mandelbrot/2 | 0.091 | 0.312 | 3.43× | 0.311 | 3.42× | ok |
| bg/mandelbrot/3 | 0.104 | 0.237 | 2.28× | 0.303 | 2.91× | ok |
| bg/mandelbrot/5 | 0.059 | 0.142 | 2.41× | 0.159 | 2.69× | ok |
| bg/mandelbrot/7 | 0.055 | 0.214 | 3.89× | 0.177 | 3.22× | ok |
| bg/mandelbrot/9 | 0.056 | 0.185 | 3.30× | 0.164 | 2.93× | ok |
| bg/nbody/1 | 0.045 | 0.162 | 3.60× | 0.174 | 3.87× | ok |
| bg/nbody/2 | 0.033 | 0.144 | 4.36× | 0.125 | 3.79× | ok |
| bg/nbody/3 | 0.032 | 0.134 | 4.19× | 0.109 | 3.41× | ok |
| bg/nbody/6 | 0.043 | 0.166 | 3.86× | 0.177 | 4.12× | ok |
| bg/pidigits/4 | 0.126 | 0.357 | 2.83× | 0.417 | 3.31× | ok |
| bg/pidigits/5 | 0.080 | 0.205 | 2.56× | 0.251 | 3.14× | ok |
| bg/revcomp/1 | 0.093 | 0.265 | 2.85× | 0.333 | 3.58× | ok |
| bg/revcomp/5 | 0.091 | 0.261 | 2.87× | 0.312 | 3.43× | ok |
| bg/spectralnorm/1 | 0.121 | 0.356 | 2.94× | 0.425 | 3.51× | ok |
| box2d | 1.730 | 7.888 | 4.56× | 5.708 | 3.30× | ok |
| fmt | 1.025 | 5.292 | 5.16× | 3.853 | 3.76× | ok |
| hexxagon | 0.283 | 1.139 | 4.02× | 1.168 | 4.13× | ok |
| lambda | 0.125 | 1.212 | 9.70× | 0.604 | 4.83× | ok |
| minisat | 0.148 | 0.982 | 6.64× | 0.694 | 4.69× | ok |
| nlohmann-json | 0.851 | 3.714 | 4.36× | 3.061 | 3.60× | ok |
| pugixml | 0.743 | 5.536 | 7.45× | 3.828 | 5.15× | ok |
| pugixml-tests | 4.566 | 35.534 | 7.78× | 25.737 | 5.64× | ok |
| rapidjson | 0.220 | 1.204 | 5.47× | 0.781 | 3.55× | ok |
| re2 | 3.028 | 18.983 | 6.27× | 11.023 | 3.64× | ok |
| re2-test-re2 | 5.898 | 27.294 | 4.63× | 21.628 | 3.67× | ok |
| re2-test-search | 5.863 | 24.443 | 4.17× | 20.910 | 3.57× | ok |
| tramp3d-v4 | 2.442 | 24.352 | 9.97× | 14.247 | 5.83× | ok |

### Frontend only: -fsyntax-only

| Program | cppgm++ (s) | GCC (s) | Speed vs GCC ↑ | Clang (s) | Speed vs Clang ↑ | Status |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| Adobe-C++/functionobjects | 0.065 | 0.145 | 2.23× | 0.162 | 2.49× | ok |
| Adobe-C++/loop_unroll | 0.042 | 0.106 | 2.52× | 0.129 | 3.07× | ok |
| Adobe-C++/simple_types_constant_folding | 0.044 | 0.109 | 2.48× | 0.117 | 2.66× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.040 | 0.100 | 2.50× | 0.110 | 2.75× | ok |
| Adobe-C++/stepanov_abstraction | 0.031 | 0.072 | 2.32× | 0.091 | 2.94× | ok |
| Adobe-C++/stepanov_vector | 0.060 | 0.130 | 2.17× | 0.206 | 3.43× | ok |
| Bullet | 4.582 | 11.274 | 2.46× | 13.178 | 2.88× | ok |
| CLAMR | 2.318 | 5.052 | 2.18× | 7.655 | 3.30× | ok |
| CoyoteBench/almabench | 0.027 | 0.058 | 2.15× | 0.078 | 2.89× | ok |
| CoyoteBench/fftbench | 0.102 | 0.239 | 2.34× | 0.283 | 2.77× | ok |
| CoyoteBench/huffbench | 0.031 | 0.060 | 1.94× | 0.075 | 2.42× | ok |
| CoyoteBench/lpbench | 0.027 | 0.060 | 2.22× | 0.070 | 2.59× | ok |
| DOE/HACCKernels | 0.104 | 0.234 | 2.25× | 0.306 | 2.94× | ok |
| DOE/HPCCG | 0.613 | 1.427 | 2.33× | 2.128 | 3.47× | ok |
| DOE/PENNANT | 1.419 | 3.212 | 2.26× | 3.744 | 2.64× | ok |
| DOE/miniFE | 0.544 | 1.264 | 2.32× | 1.651 | 3.03× | ok |
| Misc-C++-EH/spirit | 0.337 | 1.009 | 2.99× | 1.319 | 3.91× | ok |
| Misc-C++-Large/ray | 0.100 | 0.208 | 2.08× | 0.270 | 2.70× | ok |
| Misc-C++-Large/sphereflake | 0.086 | 0.190 | 2.21× | 0.242 | 2.81× | ok |
| Misc-C++/bigfib | 0.109 | 0.245 | 2.25× | 0.308 | 2.83× | ok |
| Misc-C++/oopack_v1p8 | 0.029 | 0.061 | 2.10× | 0.082 | 2.83× | ok |
| Misc-C++/stepanov_container | 0.141 | 0.298 | 2.11× | 0.396 | 2.81× | ok |
| Misc-C++/stepanov_v1p2 | 0.031 | 0.057 | 1.84× | 0.078 | 2.52× | ok |
| PAQ8p | 0.044 | 0.114 | 2.59× | 0.125 | 2.84× | ok |
| Prolangs-C++/life | 0.003 | 0.009 | — | 0.020 | — | ok |
| Prolangs-C++/ocean | 0.009 | 0.023 | — | 0.035 | — | ok |
| Prolangs-C++/primes | 0.006 | 0.016 | — | 0.023 | — | ok |
| Shootout-C++-EH/except | 0.076 | 0.171 | 2.25× | 0.233 | 3.07× | ok |
| Shootout-C++/ackermann | 0.069 | 0.153 | 2.22× | 0.198 | 2.87× | ok |
| Shootout-C++/ary3 | 0.082 | 0.191 | 2.33× | 0.277 | 3.38× | ok |
| Shootout-C++/fibo | 0.069 | 0.154 | 2.23× | 0.193 | 2.80× | ok |
| Shootout-C++/hash | 0.101 | 0.223 | 2.21× | 0.277 | 2.74× | ok |
| Shootout-C++/hash2 | 0.113 | 0.235 | 2.08× | 0.269 | 2.38× | ok |
| Shootout-C++/heapsort | 0.101 | 0.190 | 1.88× | 0.255 | 2.52× | ok |
| Shootout-C++/lists | 0.075 | 0.169 | 2.25× | 0.214 | 2.85× | ok |
| Shootout-C++/lists1 | 0.103 | 0.185 | 1.80× | 0.303 | 2.94× | ok |
| Shootout-C++/matrix | 0.068 | 0.186 | 2.74× | 0.198 | 2.91× | ok |
| Shootout-C++/methcall | 0.071 | 0.151 | 2.13× | 0.229 | 3.23× | ok |
| Shootout-C++/moments | 0.107 | 0.204 | 1.91× | 0.238 | 2.22× | ok |
| Shootout-C++/random | 0.090 | 0.178 | 1.98× | 0.239 | 2.66× | ok |
| Shootout-C++/sieve | 0.085 | 0.205 | 2.41× | 0.243 | 2.86× | ok |
| SmallPT | 0.028 | 0.054 | 1.93× | 0.083 | 2.96× | ok |
| Stanford/Bubblesort | 0.009 | 0.023 | — | 0.031 | — | ok |
| Stanford/FloatMM | 0.009 | 0.022 | — | 0.032 | — | ok |
| Stanford/Puzzle | 0.009 | 0.024 | — | 0.033 | — | ok |
| Stanford/Treesort | 0.009 | 0.023 | — | 0.032 | — | ok |
| bg/binarytrees/2 | 0.070 | 0.159 | 2.27× | 0.204 | 2.91× | ok |
| bg/fannkuchredux/3 | 0.034 | 0.063 | 1.85× | 0.080 | 2.35× | ok |
| bg/fasta/1 | 0.054 | 0.098 | 1.81× | 0.133 | 2.46× | ok |
| bg/fasta/2 | 0.050 | 0.099 | 1.98× | 0.124 | 2.48× | ok |
| bg/fasta/3 | 0.080 | 0.183 | 2.29× | 0.220 | 2.75× | ok |
| bg/fasta/4 | 0.073 | 0.151 | 2.07× | 0.191 | 2.62× | ok |
| bg/fasta/6 | 0.143 | 0.281 | 1.97× | 0.376 | 2.63× | ok |
| bg/mandelbrot/0 | 0.133 | 0.311 | 2.34× | 0.424 | 3.19× | ok |
| bg/mandelbrot/2 | 0.078 | 0.192 | 2.46× | 0.278 | 3.56× | ok |
| bg/mandelbrot/3 | 0.093 | 0.211 | 2.27× | 0.269 | 2.89× | ok |
| bg/mandelbrot/5 | 0.044 | 0.086 | 1.95× | 0.122 | 2.77× | ok |
| bg/mandelbrot/7 | 0.045 | 0.091 | 2.02× | 0.122 | 2.71× | ok |
| bg/mandelbrot/9 | 0.042 | 0.080 | 1.90× | 0.126 | 3.00× | ok |
| bg/nbody/1 | 0.030 | 0.061 | 2.03× | 0.091 | 3.03× | ok |
| bg/nbody/2 | 0.027 | 0.054 | 2.00× | 0.081 | 3.00× | ok |
| bg/nbody/3 | 0.027 | 0.058 | 2.15× | 0.074 | 2.74× | ok |
| bg/nbody/6 | 0.029 | 0.060 | 2.07× | 0.080 | 2.76× | ok |
| bg/pidigits/4 | 0.112 | 0.280 | 2.50× | 0.342 | 3.05× | ok |
| bg/pidigits/5 | 0.084 | 0.167 | 1.99× | 0.218 | 2.60× | ok |
| bg/revcomp/1 | 0.083 | 0.179 | 2.16× | 0.239 | 2.88× | ok |
| bg/revcomp/5 | 0.079 | 0.167 | 2.11× | 0.235 | 2.97× | ok |
| bg/spectralnorm/1 | 0.108 | 0.269 | 2.49× | 0.360 | 3.33× | ok |
| box2d | 1.408 | 2.807 | 1.99× | 2.826 | 2.01× | ok |
| fmt | 0.445 | 1.127 | 2.53× | 1.577 | 3.54× | ok |
| hexxagon | 0.255 | 0.604 | 2.37× | 0.790 | 3.10× | ok |
| lambda | 0.042 | 0.129 | 3.07× | 0.166 | 3.95× | ok |
| minisat | 0.048 | 0.131 | 2.73× | 0.178 | 3.71× | ok |
| nlohmann-json | 0.359 | 0.886 | 2.47× | 1.389 | 3.87× | ok |
| pugixml | 0.213 | 0.496 | 2.33× | 0.723 | 3.39× | ok |
| pugixml-tests | 2.864 | 7.791 | 2.72× | 9.414 | 3.29× | ok |
| rapidjson | 0.096 | 0.264 | 2.75× | 0.315 | 3.28× | ok |
| re2 | 2.954 | 6.590 | 2.23× | 6.241 | 2.11× | ok |
| re2-test-re2 | 4.137 | 10.405 | 2.52× | 12.874 | 3.11× | ok |
| re2-test-search | 4.111 | 9.688 | 2.36× | 13.117 | 3.19× | ok |
| tramp3d-v4 | 0.479 | 2.275 | 4.75× | 1.526 | 3.19× | ok |


## Toolchains

- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)
- **valgrind**: valgrind-3.26.0

[Compiler build and codegen results](self-build.md) have their own measured revision.
