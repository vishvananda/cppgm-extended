# Public benchmarks: AMD EPYC

Measured compiler: [`b858d4082`](https://github.com/vishvananda/cppgm-extended/commit/b858d4082cddcb8e1e930d786f3f1139c6996e6d) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/43479737925) · [Raw program results](results.json)

81 of 81 programs built with cppgm++ and g++ and agreed. Each number is the reference over cppgm++, so above 1× means cppgm++ is smaller or faster.

CPU: AMD EPYC 9V45 96-Core Processor.

| Measure | vs g++ | Programs | vs clang++ | Programs |
| --- | ---: | ---: | ---: | ---: |
| Frontend CPU, `-fsyntax-only` | 2.19× | 78 | 2.70× | 78 |
| Compile CPU, `-O0` | 3.02× | 77 | 2.92× | 77 |
| Compile CPU, `-O3` | 4.32× | 79 | 3.78× | 79 |
| Compile peak RSS, `-O0` | 1.46× | 81 | 2.22× | 81 |
| Compile peak RSS, `-O3` | 1.73× | 81 | 2.33× | 81 |
| Executable size, `-O0` | 0.93× | 81 | 0.87× | 81 |
| Executable size, `-O3` | 0.96× | 81 | 0.92× | 81 |
| Run CPU, `-O0` | 1.11× | 81 | 1.09× | 81 |
| Run CPU, `-O3` | 0.98× | 80 | 0.99× | 80 |
| Run instructions, `-O3` | 0.95× | 43 | - | 0 |

O3 execution uses paired ABBA/BAAB blocks. GCC A/A CPU ratios ranged from 0.853 to 1.076; raw observations and block ratios are retained in the artifact.

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
| Adobe-C++/functionobjects | 0.067 | 0.237 | 3.54× | 0.184 | 2.75× | ok |
| Adobe-C++/loop_unroll | 0.049 | 0.496 | 10.12× | 0.163 | 3.33× | ok |
| Adobe-C++/simple_types_constant_folding | 0.062 | 0.588 | 9.48× | 0.186 | 3.00× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.045 | 0.420 | 9.33× | 0.215 | 4.78× | ok |
| Adobe-C++/stepanov_abstraction | 0.040 | 0.168 | 4.20× | 0.108 | 2.70× | ok |
| Adobe-C++/stepanov_vector | 0.070 | 0.263 | 3.76× | 0.192 | 2.74× | ok |
| Bullet | 4.849 | 17.218 | 3.55× | 14.989 | 3.09× | ok |
| CLAMR | 2.331 | 8.409 | 3.61× | 8.357 | 3.59× | ok |
| CoyoteBench/almabench | 0.030 | 0.069 | 2.30× | 0.083 | 2.77× | ok |
| CoyoteBench/fftbench | 0.102 | 0.241 | 2.36× | 0.323 | 3.17× | ok |
| CoyoteBench/huffbench | 0.030 | 0.067 | 2.23× | 0.077 | 2.57× | ok |
| CoyoteBench/lpbench | 0.031 | 0.076 | 2.45× | 0.075 | 2.42× | ok |
| DOE/HACCKernels | 0.108 | 0.253 | 2.34× | 0.304 | 2.81× | ok |
| DOE/HPCCG | 0.638 | 1.582 | 2.48× | 1.941 | 3.04× | ok |
| DOE/PENNANT | 1.505 | 3.949 | 2.62× | 3.888 | 2.58× | ok |
| DOE/miniFE | 0.606 | 2.044 | 3.37× | 1.847 | 3.05× | ok |
| Misc-C++-EH/spirit | 0.493 | 2.631 | 5.34× | 2.115 | 4.29× | ok |
| Misc-C++-Large/ray | 0.115 | 0.239 | 2.08× | 0.302 | 2.63× | ok |
| Misc-C++-Large/sphereflake | 0.098 | 0.205 | 2.09× | 0.244 | 2.49× | ok |
| Misc-C++/bigfib | 0.129 | 0.368 | 2.85× | 0.364 | 2.82× | ok |
| Misc-C++/oopack_v1p8 | 0.032 | 0.092 | 2.88× | 0.085 | 2.66× | ok |
| Misc-C++/stepanov_container | 0.151 | 0.529 | 3.50× | 0.447 | 2.96× | ok |
| Misc-C++/stepanov_v1p2 | 0.031 | 0.105 | 3.39× | 0.088 | 2.84× | ok |
| PAQ8p | 0.065 | 0.424 | 6.52× | 0.211 | 3.25× | ok |
| Prolangs-C++/life | 0.004 | 0.030 | — | 0.030 | — | ok |
| Prolangs-C++/ocean | 0.012 | 0.063 | 5.25× | 0.055 | 4.58× | ok |
| Prolangs-C++/primes | 0.006 | 0.025 | — | 0.032 | — | ok |
| Shootout-C++-EH/except | 0.076 | 0.173 | 2.28× | 0.224 | 2.95× | ok |
| Shootout-C++/ackermann | 0.073 | 0.152 | 2.08× | 0.197 | 2.70× | ok |
| Shootout-C++/ary3 | 0.085 | 0.191 | 2.25× | 0.230 | 2.71× | ok |
| Shootout-C++/fibo | 0.072 | 0.156 | 2.17× | 0.206 | 2.86× | ok |
| Shootout-C++/hash | 0.097 | 0.264 | 2.72× | 0.282 | 2.91× | ok |
| Shootout-C++/hash2 | 0.111 | 0.307 | 2.77× | 0.342 | 3.08× | ok |
| Shootout-C++/heapsort | 0.087 | 0.178 | 2.05× | 0.233 | 2.68× | ok |
| Shootout-C++/lists | 0.078 | 0.195 | 2.50× | 0.220 | 2.82× | ok |
| Shootout-C++/lists1 | 0.085 | 0.232 | 2.73× | 0.244 | 2.87× | ok |
| Shootout-C++/matrix | 0.073 | 0.146 | 2.00× | 0.195 | 2.67× | ok |
| Shootout-C++/methcall | 0.069 | 0.148 | 2.14× | 0.178 | 2.58× | ok |
| Shootout-C++/moments | 0.094 | 0.245 | 2.61× | 0.234 | 2.49× | ok |
| Shootout-C++/random | 0.082 | 0.177 | 2.16× | 0.224 | 2.73× | ok |
| Shootout-C++/sieve | 0.092 | 0.221 | 2.40× | 0.263 | 2.86× | ok |
| SmallPT | 0.030 | 0.084 | 2.80× | 0.080 | 2.67× | ok |
| Stanford/Bubblesort | 0.009 | 0.028 | — | 0.033 | — | ok |
| Stanford/FloatMM | 0.010 | 0.029 | 2.90× | 0.034 | 3.40× | ok |
| Stanford/Puzzle | 0.010 | 0.035 | 3.50× | 0.035 | 3.50× | ok |
| Stanford/Treesort | 0.009 | 0.031 | — | 0.036 | — | ok |
| bg/binarytrees/2 | 0.073 | 0.161 | 2.21× | 0.210 | 2.88× | ok |
| bg/fannkuchredux/3 | 0.036 | 0.076 | 2.11× | 0.084 | 2.33× | ok |
| bg/fasta/1 | 0.053 | 0.123 | 2.32× | 0.134 | 2.53× | ok |
| bg/fasta/2 | 0.056 | 0.120 | 2.14× | 0.141 | 2.52× | ok |
| bg/fasta/3 | 0.084 | 0.178 | 2.12× | 0.238 | 2.83× | ok |
| bg/fasta/4 | 0.076 | 0.162 | 2.13× | 0.198 | 2.61× | ok |
| bg/fasta/6 | 0.149 | 0.395 | 2.65× | 0.410 | 2.75× | ok |
| bg/mandelbrot/0 | 0.156 | 0.456 | 2.92× | 0.430 | 2.76× | ok |
| bg/mandelbrot/2 | 0.101 | 0.221 | 2.19× | 0.265 | 2.62× | ok |
| bg/mandelbrot/3 | 0.098 | 0.196 | 2.00× | 0.273 | 2.79× | ok |
| bg/mandelbrot/5 | 0.047 | 0.109 | 2.32× | 0.122 | 2.60× | ok |
| bg/mandelbrot/7 | 0.052 | 0.128 | 2.46× | 0.136 | 2.62× | ok |
| bg/mandelbrot/9 | 0.044 | 0.129 | 2.93× | 0.119 | 2.70× | ok |
| bg/nbody/1 | 0.030 | 0.108 | 3.60× | 0.089 | 2.97× | ok |
| bg/nbody/2 | 0.028 | 0.072 | 2.57× | 0.080 | 2.86× | ok |
| bg/nbody/3 | 0.029 | 0.072 | 2.48× | 0.078 | 2.69× | ok |
| bg/nbody/6 | 0.030 | 0.101 | 3.37× | 0.090 | 3.00× | ok |
| bg/pidigits/4 | 0.125 | 0.318 | 2.54× | 0.414 | 3.31× | ok |
| bg/pidigits/5 | 0.081 | 0.170 | 2.10× | 0.211 | 2.60× | ok |
| bg/revcomp/1 | 0.082 | 0.214 | 2.61× | 0.261 | 3.18× | ok |
| bg/revcomp/5 | 0.077 | 0.199 | 2.58× | 0.261 | 3.39× | ok |
| bg/spectralnorm/1 | 0.121 | 0.260 | 2.15× | 0.297 | 2.45× | ok |
| box2d | 1.640 | 4.554 | 2.78× | 4.629 | 2.82× | ok |
| fmt | 0.542 | 2.453 | 4.53× | 1.999 | 3.69× | ok |
| hexxagon | 0.271 | 0.667 | 2.46× | 0.877 | 3.24× | ok |
| lambda | 0.059 | 0.311 | 5.27× | 0.210 | 3.56× | ok |
| minisat | 0.067 | 0.306 | 4.57× | 0.229 | 3.42× | ok |
| nlohmann-json | 0.406 | 1.875 | 4.62× | 1.439 | 3.54× | ok |
| pugixml | 0.239 | 1.385 | 5.79× | 0.803 | 3.36× | ok |
| pugixml-tests | 3.752 | 14.562 | 3.88× | 12.930 | 3.45× | ok |
| rapidjson | 0.119 | 0.456 | 3.83× | 0.355 | 2.98× | ok |
| re2 | 3.244 | 9.661 | 2.98× | 6.928 | 2.14× | ok |
| re2-test-re2 | 4.780 | 14.727 | 3.08× | 14.557 | 3.05× | ok |
| re2-test-search | 4.812 | 13.848 | 2.88× | 12.627 | 2.62× | ok |
| tramp3d-v4 | 0.791 | 8.625 | 10.90× | 2.532 | 3.20× | ok |

### O3 compilation

| Program | cppgm++ (s) | GCC (s) | Speed vs GCC ↑ | Clang (s) | Speed vs Clang ↑ | Status |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| Adobe-C++/functionobjects | 0.105 | 0.789 | 7.51× | 0.444 | 4.23× | ok |
| Adobe-C++/loop_unroll | 0.281 | 4.989 | 17.75× | 3.862 | 13.74× | ok |
| Adobe-C++/simple_types_constant_folding | 0.153 | 2.857 | 18.67× | 2.067 | 13.51× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.140 | 2.104 | 15.03× | 1.742 | 12.44× | ok |
| Adobe-C++/stepanov_abstraction | 0.075 | 0.965 | 12.87× | 0.491 | 6.55× | ok |
| Adobe-C++/stepanov_vector | 0.113 | 1.349 | 11.94× | 0.481 | 4.26× | ok |
| Bullet | 6.198 | 36.413 | 5.87× | 25.464 | 4.11× | ok |
| CLAMR | 3.864 | 22.086 | 5.72× | 16.177 | 4.19× | ok |
| CoyoteBench/almabench | 0.037 | 0.165 | 4.46× | 0.113 | 3.05× | ok |
| CoyoteBench/fftbench | 0.123 | 0.388 | 3.15× | 0.377 | 3.07× | ok |
| CoyoteBench/huffbench | 0.038 | 0.213 | 5.61× | 0.141 | 3.71× | ok |
| CoyoteBench/lpbench | 0.035 | 0.186 | 5.31× | 0.171 | 4.89× | ok |
| DOE/HACCKernels | 0.117 | 0.343 | 2.93× | 0.356 | 3.04× | ok |
| DOE/HPCCG | 0.686 | 2.244 | 3.27× | 2.190 | 3.19× | ok |
| DOE/PENNANT | 1.797 | 6.406 | 3.56× | 5.522 | 3.07× | ok |
| DOE/miniFE | 0.821 | 4.041 | 4.92× | 3.082 | 3.75× | ok |
| Misc-C++-EH/spirit | 0.667 | 3.934 | 5.90× | 2.946 | 4.42× | ok |
| Misc-C++-Large/ray | 0.110 | 0.487 | 4.43× | 0.442 | 4.02× | ok |
| Misc-C++-Large/sphereflake | 0.103 | 0.777 | 7.54× | 0.324 | 3.15× | ok |
| Misc-C++/bigfib | 0.151 | 0.522 | 3.46× | 0.550 | 3.64× | ok |
| Misc-C++/oopack_v1p8 | 0.038 | 0.175 | 4.61× | 0.150 | 3.95× | ok |
| Misc-C++/stepanov_container | 0.204 | 0.827 | 4.05× | 0.782 | 3.83× | ok |
| Misc-C++/stepanov_v1p2 | 0.037 | 0.215 | 5.81× | 0.189 | 5.11× | ok |
| PAQ8p | 0.263 | 3.534 | 13.44× | 1.467 | 5.58× | ok |
| Prolangs-C++/life | 0.008 | 0.097 | — | 0.067 | — | ok |
| Prolangs-C++/ocean | 0.021 | 0.199 | 9.48× | 0.123 | 5.86× | ok |
| Prolangs-C++/primes | 0.008 | 0.037 | — | 0.040 | — | ok |
| Shootout-C++-EH/except | 0.077 | 0.197 | 2.56× | 0.235 | 3.05× | ok |
| Shootout-C++/ackermann | 0.073 | 0.190 | 2.60× | 0.235 | 3.22× | ok |
| Shootout-C++/ary3 | 0.088 | 0.239 | 2.72× | 0.248 | 2.82× | ok |
| Shootout-C++/fibo | 0.078 | 0.196 | 2.51× | 0.251 | 3.22× | ok |
| Shootout-C++/hash | 0.115 | 0.355 | 3.09× | 0.379 | 3.30× | ok |
| Shootout-C++/hash2 | 0.137 | 0.421 | 3.07× | 0.440 | 3.21× | ok |
| Shootout-C++/heapsort | 0.090 | 0.213 | 2.37× | 0.249 | 2.77× | ok |
| Shootout-C++/lists | 0.085 | 0.224 | 2.64× | 0.276 | 3.25× | ok |
| Shootout-C++/lists1 | 0.093 | 0.276 | 2.97× | 0.279 | 3.00× | ok |
| Shootout-C++/matrix | 0.073 | 0.203 | 2.78× | 0.278 | 3.81× | ok |
| Shootout-C++/methcall | 0.068 | 0.183 | 2.69× | 0.212 | 3.12× | ok |
| Shootout-C++/moments | 0.104 | 0.311 | 2.99× | 0.293 | 2.82× | ok |
| Shootout-C++/random | 0.087 | 0.193 | 2.22× | 0.236 | 2.71× | ok |
| Shootout-C++/sieve | 0.095 | 0.237 | 2.49× | 0.297 | 3.13× | ok |
| SmallPT | 0.039 | 0.255 | 6.54× | 0.154 | 3.95× | ok |
| Stanford/Bubblesort | 0.011 | 0.051 | 4.64× | 0.049 | 4.45× | ok |
| Stanford/FloatMM | 0.013 | 0.060 | 4.62× | 0.067 | 5.15× | ok |
| Stanford/Puzzle | 0.016 | 0.113 | 7.06× | 0.105 | 6.56× | ok |
| Stanford/Treesort | 0.015 | 0.079 | 5.27× | 0.054 | 3.60× | ok |
| bg/binarytrees/2 | 0.075 | 0.388 | 5.17× | 0.219 | 2.92× | ok |
| bg/fannkuchredux/3 | 0.041 | 0.127 | 3.10× | 0.109 | 2.66× | ok |
| bg/fasta/1 | 0.061 | 0.159 | 2.61× | 0.165 | 2.70× | ok |
| bg/fasta/2 | 0.061 | 0.158 | 2.59× | 0.180 | 2.95× | ok |
| bg/fasta/3 | 0.091 | 0.220 | 2.42× | 0.267 | 2.93× | ok |
| bg/fasta/4 | 0.079 | 0.193 | 2.44× | 0.249 | 3.15× | ok |
| bg/fasta/6 | 0.171 | 0.513 | 3.00× | 0.499 | 2.92× | ok |
| bg/mandelbrot/0 | 0.181 | 0.677 | 3.74× | 0.636 | 3.51× | ok |
| bg/mandelbrot/2 | 0.100 | 0.279 | 2.79× | 0.295 | 2.95× | ok |
| bg/mandelbrot/3 | 0.103 | 0.231 | 2.24× | 0.281 | 2.73× | ok |
| bg/mandelbrot/5 | 0.058 | 0.147 | 2.53× | 0.151 | 2.60× | ok |
| bg/mandelbrot/7 | 0.057 | 0.195 | 3.42× | 0.181 | 3.18× | ok |
| bg/mandelbrot/9 | 0.053 | 0.184 | 3.47× | 0.160 | 3.02× | ok |
| bg/nbody/1 | 0.039 | 0.178 | 4.56× | 0.162 | 4.15× | ok |
| bg/nbody/2 | 0.032 | 0.130 | 4.06× | 0.119 | 3.72× | ok |
| bg/nbody/3 | 0.032 | 0.122 | 3.81× | 0.114 | 3.56× | ok |
| bg/nbody/6 | 0.041 | 0.162 | 3.95× | 0.159 | 3.88× | ok |
| bg/pidigits/4 | 0.133 | 0.338 | 2.54× | 0.427 | 3.21× | ok |
| bg/pidigits/5 | 0.081 | 0.208 | 2.57× | 0.235 | 2.90× | ok |
| bg/revcomp/1 | 0.089 | 0.245 | 2.75× | 0.302 | 3.39× | ok |
| bg/revcomp/5 | 0.082 | 0.228 | 2.78× | 0.296 | 3.61× | ok |
| bg/spectralnorm/1 | 0.126 | 0.350 | 2.78× | 0.360 | 2.86× | ok |
| box2d | 1.892 | 7.684 | 4.06× | 6.213 | 3.28× | ok |
| fmt | 1.020 | 5.108 | 5.01× | 4.068 | 3.99× | ok |
| hexxagon | 0.326 | 1.030 | 3.16× | 1.164 | 3.57× | ok |
| lambda | 0.121 | 1.115 | 9.21× | 0.585 | 4.83× | ok |
| minisat | 0.142 | 1.019 | 7.18× | 0.695 | 4.89× | ok |
| nlohmann-json | 0.775 | 3.112 | 4.02× | 2.827 | 3.65× | ok |
| pugixml | 0.564 | 5.383 | 9.54× | 3.417 | 6.06× | ok |
| pugixml-tests | 5.239 | 33.528 | 6.40× | 23.744 | 4.53× | ok |
| rapidjson | 0.219 | 1.120 | 5.11× | 0.717 | 3.27× | ok |
| re2 | 3.371 | 17.091 | 5.07× | 9.851 | 2.92× | ok |
| re2-test-re2 | 5.766 | 26.745 | 4.64× | 21.421 | 3.72× | ok |
| re2-test-search | 5.758 | 23.011 | 4.00× | 17.881 | 3.11× | ok |
| tramp3d-v4 | 2.204 | 20.560 | 9.33× | 13.958 | 6.33× | ok |

### Frontend only: -fsyntax-only

| Program | cppgm++ (s) | GCC (s) | Speed vs GCC ↑ | Clang (s) | Speed vs Clang ↑ | Status |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| Adobe-C++/functionobjects | 0.061 | 0.124 | 2.03× | 0.152 | 2.49× | ok |
| Adobe-C++/loop_unroll | 0.037 | 0.089 | 2.41× | 0.096 | 2.59× | ok |
| Adobe-C++/simple_types_constant_folding | 0.045 | 0.096 | 2.13× | 0.100 | 2.22× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.038 | 0.088 | 2.32× | 0.121 | 3.18× | ok |
| Adobe-C++/stepanov_abstraction | 0.033 | 0.070 | 2.12× | 0.085 | 2.58× | ok |
| Adobe-C++/stepanov_vector | 0.071 | 0.117 | 1.65× | 0.161 | 2.27× | ok |
| Bullet | 4.472 | 10.196 | 2.28× | 13.510 | 3.02× | ok |
| CLAMR | 2.157 | 4.502 | 2.09× | 5.580 | 2.59× | ok |
| CoyoteBench/almabench | 0.030 | 0.057 | 1.90× | 0.078 | 2.60× | ok |
| CoyoteBench/fftbench | 0.101 | 0.221 | 2.19× | 0.303 | 3.00× | ok |
| CoyoteBench/huffbench | 0.030 | 0.060 | 2.00× | 0.071 | 2.37× | ok |
| CoyoteBench/lpbench | 0.031 | 0.063 | 2.03× | 0.067 | 2.16× | ok |
| DOE/HACCKernels | 0.117 | 0.224 | 1.91× | 0.282 | 2.41× | ok |
| DOE/HPCCG | 0.620 | 1.346 | 2.17× | 1.760 | 2.84× | ok |
| DOE/PENNANT | 1.481 | 3.198 | 2.16× | 3.613 | 2.44× | ok |
| DOE/miniFE | 0.549 | 1.263 | 2.30× | 1.641 | 2.99× | ok |
| Misc-C++-EH/spirit | 0.350 | 0.868 | 2.48× | 1.192 | 3.41× | ok |
| Misc-C++-Large/ray | 0.104 | 0.199 | 1.91× | 0.271 | 2.61× | ok |
| Misc-C++-Large/sphereflake | 0.093 | 0.191 | 2.05× | 0.261 | 2.81× | ok |
| Misc-C++/bigfib | 0.116 | 0.249 | 2.15× | 0.313 | 2.70× | ok |
| Misc-C++/oopack_v1p8 | 0.031 | 0.072 | 2.32× | 0.076 | 2.45× | ok |
| Misc-C++/stepanov_container | 0.145 | 0.296 | 2.04× | 0.364 | 2.51× | ok |
| Misc-C++/stepanov_v1p2 | 0.029 | 0.060 | 2.07× | 0.074 | 2.55× | ok |
| PAQ8p | 0.046 | 0.116 | 2.52× | 0.126 | 2.74× | ok |
| Prolangs-C++/life | 0.003 | 0.010 | — | 0.023 | — | ok |
| Prolangs-C++/ocean | 0.010 | 0.024 | 2.40× | 0.036 | 3.60× | ok |
| Prolangs-C++/primes | 0.006 | 0.017 | — | 0.027 | — | ok |
| Shootout-C++-EH/except | 0.077 | 0.173 | 2.25× | 0.214 | 2.78× | ok |
| Shootout-C++/ackermann | 0.076 | 0.154 | 2.03× | 0.198 | 2.61× | ok |
| Shootout-C++/ary3 | 0.081 | 0.175 | 2.16× | 0.213 | 2.63× | ok |
| Shootout-C++/fibo | 0.074 | 0.155 | 2.09× | 0.195 | 2.64× | ok |
| Shootout-C++/hash | 0.093 | 0.209 | 2.25× | 0.255 | 2.74× | ok |
| Shootout-C++/hash2 | 0.112 | 0.231 | 2.06× | 0.307 | 2.74× | ok |
| Shootout-C++/heapsort | 0.093 | 0.198 | 2.13× | 0.232 | 2.49× | ok |
| Shootout-C++/lists | 0.075 | 0.172 | 2.29× | 0.203 | 2.71× | ok |
| Shootout-C++/lists1 | 0.081 | 0.194 | 2.40× | 0.218 | 2.69× | ok |
| Shootout-C++/matrix | 0.070 | 0.144 | 2.06× | 0.188 | 2.69× | ok |
| Shootout-C++/methcall | 0.067 | 0.146 | 2.18× | 0.166 | 2.48× | ok |
| Shootout-C++/moments | 0.092 | 0.201 | 2.18× | 0.215 | 2.34× | ok |
| Shootout-C++/random | 0.084 | 0.183 | 2.18× | 0.212 | 2.52× | ok |
| Shootout-C++/sieve | 0.086 | 0.180 | 2.09× | 0.236 | 2.74× | ok |
| SmallPT | 0.029 | 0.056 | 1.93× | 0.073 | 2.52× | ok |
| Stanford/Bubblesort | 0.010 | 0.024 | 2.40× | 0.032 | 3.20× | ok |
| Stanford/FloatMM | 0.010 | 0.024 | 2.40× | 0.032 | 3.20× | ok |
| Stanford/Puzzle | 0.010 | 0.026 | 2.60× | 0.032 | 3.20× | ok |
| Stanford/Treesort | 0.009 | 0.024 | — | 0.033 | — | ok |
| bg/binarytrees/2 | 0.072 | 0.160 | 2.22× | 0.191 | 2.65× | ok |
| bg/fannkuchredux/3 | 0.035 | 0.065 | 1.86× | 0.079 | 2.26× | ok |
| bg/fasta/1 | 0.050 | 0.098 | 1.96× | 0.124 | 2.48× | ok |
| bg/fasta/2 | 0.051 | 0.094 | 1.84× | 0.127 | 2.49× | ok |
| bg/fasta/3 | 0.081 | 0.165 | 2.04× | 0.223 | 2.75× | ok |
| bg/fasta/4 | 0.077 | 0.150 | 1.95× | 0.181 | 2.35× | ok |
| bg/fasta/6 | 0.139 | 0.285 | 2.05× | 0.360 | 2.59× | ok |
| bg/mandelbrot/0 | 0.131 | 0.302 | 2.31× | 0.373 | 2.85× | ok |
| bg/mandelbrot/2 | 0.096 | 0.216 | 2.25× | 0.255 | 2.66× | ok |
| bg/mandelbrot/3 | 0.096 | 0.193 | 2.01× | 0.277 | 2.89× | ok |
| bg/mandelbrot/5 | 0.044 | 0.086 | 1.95× | 0.111 | 2.52× | ok |
| bg/mandelbrot/7 | 0.048 | 0.094 | 1.96× | 0.119 | 2.48× | ok |
| bg/mandelbrot/9 | 0.043 | 0.093 | 2.16× | 0.106 | 2.47× | ok |
| bg/nbody/1 | 0.029 | 0.068 | 2.34× | 0.076 | 2.62× | ok |
| bg/nbody/2 | 0.027 | 0.060 | 2.22× | 0.075 | 2.78× | ok |
| bg/nbody/3 | 0.028 | 0.059 | 2.11× | 0.071 | 2.54× | ok |
| bg/nbody/6 | 0.028 | 0.061 | 2.18× | 0.073 | 2.61× | ok |
| bg/pidigits/4 | 0.122 | 0.287 | 2.35× | 0.315 | 2.58× | ok |
| bg/pidigits/5 | 0.081 | 0.174 | 2.15× | 0.199 | 2.46× | ok |
| bg/revcomp/1 | 0.079 | 0.172 | 2.18× | 0.253 | 3.20× | ok |
| bg/revcomp/5 | 0.071 | 0.165 | 2.32× | 0.258 | 3.63× | ok |
| bg/spectralnorm/1 | 0.113 | 0.237 | 2.10× | 0.281 | 2.49× | ok |
| box2d | 1.494 | 3.040 | 2.03× | 3.738 | 2.50× | ok |
| fmt | 0.488 | 1.013 | 2.08× | 1.567 | 3.21× | ok |
| hexxagon | 0.296 | 0.601 | 2.03× | 0.911 | 3.08× | ok |
| lambda | 0.048 | 0.137 | 2.85× | 0.156 | 3.25× | ok |
| minisat | 0.058 | 0.146 | 2.52× | 0.163 | 2.81× | ok |
| nlohmann-json | 0.320 | 0.887 | 2.77× | 1.116 | 3.49× | ok |
| pugixml | 0.206 | 0.518 | 2.51× | 0.596 | 2.89× | ok |
| pugixml-tests | 3.288 | 7.219 | 2.20× | 9.862 | 3.00× | ok |
| rapidjson | 0.097 | 0.227 | 2.34× | 0.294 | 3.03× | ok |
| re2 | 3.109 | 6.308 | 2.03× | 5.954 | 1.92× | ok |
| re2-test-re2 | 4.499 | 9.512 | 2.11× | 12.926 | 2.87× | ok |
| re2-test-search | 4.008 | 8.851 | 2.21× | 11.674 | 2.91× | ok |
| tramp3d-v4 | 0.499 | 2.298 | 4.61× | 1.496 | 3.00× | ok |


## Toolchains

- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)
- **valgrind**: valgrind-3.26.0

[Compiler build and codegen results](self-build.md) have their own measured revision.
