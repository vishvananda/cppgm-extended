# Public benchmarks: AMD EPYC

Measured compiler: [`a00938892`](https://github.com/vishvananda/cppgm-extended/commit/a0093889216091a98b4ab4561769544b02092c58) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/43509588897) · [Raw program results](results.json)

81 of 81 programs built with cppgm++ and g++ and agreed. Each number is the reference over cppgm++, so above 1× means cppgm++ is smaller or faster.

CPU: AMD EPYC 9V74 80-Core Processor.

| Measure | vs g++ | Programs | vs clang++ | Programs |
| --- | ---: | ---: | ---: | ---: |
| Frontend CPU, `-fsyntax-only` | 2.24× | 79 | 2.98× | 79 |
| Compile CPU, `-O0` | 3.05× | 79 | 3.17× | 79 |
| Compile CPU, `-O3` | 4.22× | 79 | 3.88× | 79 |
| Compile peak RSS, `-O0` | 1.50× | 81 | 2.42× | 81 |
| Compile peak RSS, `-O3` | 1.76× | 81 | 2.51× | 81 |
| Executable size, `-O0` | 0.93× | 81 | 0.87× | 81 |
| Executable size, `-O3` | 0.96× | 81 | 0.92× | 81 |
| Run CPU, `-O0` | 1.22× | 81 | 1.24× | 81 |
| Run CPU, `-O3` | 0.99× | 81 | 0.97× | 80 |
| Run instructions, `-O3` | 0.95× | 43 | - | 0 |

O3 execution uses paired ABBA/BAAB blocks. GCC A/A CPU ratios ranged from 0.916 to 1.053; raw observations and block ratios are retained in the artifact.

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
| Adobe-C++/functionobjects | 0.086 | 0.297 | 3.45× | 0.240 | 2.79× | ok |
| Adobe-C++/loop_unroll | 0.059 | 0.614 | 10.41× | 0.226 | 3.83× | ok |
| Adobe-C++/simple_types_constant_folding | 0.083 | 0.722 | 8.70× | 0.299 | 3.60× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.062 | 0.506 | 8.16× | 0.242 | 3.90× | ok |
| Adobe-C++/stepanov_abstraction | 0.047 | 0.196 | 4.17× | 0.138 | 2.94× | ok |
| Adobe-C++/stepanov_vector | 0.091 | 0.337 | 3.70× | 0.270 | 2.97× | ok |
| Bullet | 5.635 | 21.400 | 3.80× | 20.124 | 3.57× | ok |
| CLAMR | 2.849 | 10.992 | 3.86× | 11.169 | 3.92× | ok |
| CoyoteBench/almabench | 0.038 | 0.098 | 2.58× | 0.101 | 2.66× | ok |
| CoyoteBench/fftbench | 0.127 | 0.297 | 2.34× | 0.369 | 2.91× | ok |
| CoyoteBench/huffbench | 0.034 | 0.083 | 2.44× | 0.120 | 3.53× | ok |
| CoyoteBench/lpbench | 0.039 | 0.095 | 2.44× | 0.118 | 3.03× | ok |
| DOE/HACCKernels | 0.134 | 0.305 | 2.28× | 0.386 | 2.88× | ok |
| DOE/HPCCG | 0.756 | 1.809 | 2.39× | 2.313 | 3.06× | ok |
| DOE/PENNANT | 1.835 | 4.762 | 2.60× | 5.257 | 2.86× | ok |
| DOE/miniFE | 0.810 | 2.452 | 3.03× | 2.932 | 3.62× | ok |
| Misc-C++-EH/spirit | 0.553 | 2.745 | 4.96× | 2.483 | 4.49× | ok |
| Misc-C++-Large/ray | 0.138 | 0.331 | 2.40× | 0.350 | 2.54× | ok |
| Misc-C++-Large/sphereflake | 0.106 | 0.260 | 2.45× | 0.302 | 2.85× | ok |
| Misc-C++/bigfib | 0.126 | 0.395 | 3.13× | 0.441 | 3.50× | ok |
| Misc-C++/oopack_v1p8 | 0.037 | 0.109 | 2.95× | 0.104 | 2.81× | ok |
| Misc-C++/stepanov_container | 0.174 | 0.677 | 3.89× | 0.786 | 4.52× | ok |
| Misc-C++/stepanov_v1p2 | 0.034 | 0.126 | 3.71× | 0.119 | 3.50× | ok |
| PAQ8p | 0.078 | 0.493 | 6.32× | 0.254 | 3.26× | ok |
| Prolangs-C++/life | 0.004 | 0.035 | — | 0.030 | — | ok |
| Prolangs-C++/ocean | 0.014 | 0.071 | 5.07× | 0.061 | 4.36× | ok |
| Prolangs-C++/primes | 0.007 | 0.028 | — | 0.034 | — | ok |
| Shootout-C++-EH/except | 0.086 | 0.199 | 2.31× | 0.306 | 3.56× | ok |
| Shootout-C++/ackermann | 0.087 | 0.191 | 2.20× | 0.316 | 3.63× | ok |
| Shootout-C++/ary3 | 0.095 | 0.220 | 2.32× | 0.288 | 3.03× | ok |
| Shootout-C++/fibo | 0.096 | 0.180 | 1.88× | 0.320 | 3.33× | ok |
| Shootout-C++/hash | 0.124 | 0.383 | 3.09× | 0.363 | 2.93× | ok |
| Shootout-C++/hash2 | 0.133 | 0.341 | 2.56× | 0.372 | 2.80× | ok |
| Shootout-C++/heapsort | 0.102 | 0.232 | 2.27× | 0.297 | 2.91× | ok |
| Shootout-C++/lists | 0.091 | 0.246 | 2.70× | 0.296 | 3.25× | ok |
| Shootout-C++/lists1 | 0.097 | 0.265 | 2.73× | 0.290 | 2.99× | ok |
| Shootout-C++/matrix | 0.084 | 0.181 | 2.15× | 0.242 | 2.88× | ok |
| Shootout-C++/methcall | 0.079 | 0.195 | 2.47× | 0.250 | 3.16× | ok |
| Shootout-C++/moments | 0.123 | 0.312 | 2.54× | 0.310 | 2.52× | ok |
| Shootout-C++/random | 0.100 | 0.207 | 2.07× | 0.290 | 2.90× | ok |
| Shootout-C++/sieve | 0.102 | 0.271 | 2.66× | 0.314 | 3.08× | ok |
| SmallPT | 0.035 | 0.098 | 2.80× | 0.108 | 3.09× | ok |
| Stanford/Bubblesort | 0.011 | 0.032 | 2.91× | 0.036 | 3.27× | ok |
| Stanford/FloatMM | 0.011 | 0.032 | 2.91× | 0.037 | 3.36× | ok |
| Stanford/Puzzle | 0.011 | 0.040 | 3.64× | 0.040 | 3.64× | ok |
| Stanford/Treesort | 0.011 | 0.035 | 3.18× | 0.039 | 3.55× | ok |
| bg/binarytrees/2 | 0.082 | 0.182 | 2.22× | 0.253 | 3.09× | ok |
| bg/fannkuchredux/3 | 0.039 | 0.085 | 2.18× | 0.102 | 2.62× | ok |
| bg/fasta/1 | 0.061 | 0.143 | 2.34× | 0.160 | 2.62× | ok |
| bg/fasta/2 | 0.063 | 0.147 | 2.33× | 0.169 | 2.68× | ok |
| bg/fasta/3 | 0.097 | 0.210 | 2.16× | 0.281 | 2.90× | ok |
| bg/fasta/4 | 0.087 | 0.192 | 2.21× | 0.219 | 2.52× | ok |
| bg/fasta/6 | 0.157 | 0.451 | 2.87× | 0.505 | 3.22× | ok |
| bg/mandelbrot/0 | 0.178 | 0.523 | 2.94× | 0.540 | 3.03× | ok |
| bg/mandelbrot/2 | 0.103 | 0.260 | 2.52× | 0.374 | 3.63× | ok |
| bg/mandelbrot/3 | 0.109 | 0.235 | 2.16× | 0.324 | 2.97× | ok |
| bg/mandelbrot/5 | 0.053 | 0.131 | 2.47× | 0.147 | 2.77× | ok |
| bg/mandelbrot/7 | 0.056 | 0.146 | 2.61× | 0.154 | 2.75× | ok |
| bg/mandelbrot/9 | 0.051 | 0.131 | 2.57× | 0.141 | 2.76× | ok |
| bg/nbody/1 | 0.034 | 0.120 | 3.53× | 0.104 | 3.06× | ok |
| bg/nbody/2 | 0.033 | 0.078 | 2.36× | 0.083 | 2.52× | ok |
| bg/nbody/3 | 0.032 | 0.075 | 2.34× | 0.093 | 2.91× | ok |
| bg/nbody/6 | 0.034 | 0.113 | 3.32× | 0.108 | 3.18× | ok |
| bg/pidigits/4 | 0.141 | 0.336 | 2.38× | 0.440 | 3.12× | ok |
| bg/pidigits/5 | 0.092 | 0.200 | 2.17× | 0.288 | 3.13× | ok |
| bg/revcomp/1 | 0.101 | 0.254 | 2.51× | 0.310 | 3.07× | ok |
| bg/revcomp/5 | 0.087 | 0.225 | 2.59× | 0.333 | 3.83× | ok |
| bg/spectralnorm/1 | 0.132 | 0.303 | 2.30× | 0.410 | 3.11× | ok |
| box2d | 1.763 | 4.839 | 2.74× | 6.012 | 3.41× | ok |
| fmt | 0.575 | 2.703 | 4.70× | 2.310 | 4.02× | ok |
| hexxagon | 0.332 | 0.714 | 2.15× | 0.901 | 2.71× | ok |
| lambda | 0.065 | 0.338 | 5.20× | 0.283 | 4.35× | ok |
| minisat | 0.079 | 0.312 | 3.95× | 0.281 | 3.56× | ok |
| nlohmann-json | 0.503 | 2.203 | 4.38× | 2.132 | 4.24× | ok |
| pugixml | 0.338 | 1.510 | 4.47× | 1.063 | 3.14× | ok |
| pugixml-tests | 4.477 | 17.141 | 3.83× | 14.125 | 3.16× | ok |
| rapidjson | 0.144 | 0.520 | 3.61× | 0.469 | 3.26× | ok |
| re2 | 3.620 | 11.839 | 3.27× | 8.060 | 2.23× | ok |
| re2-test-re2 | 5.632 | 17.385 | 3.09× | 19.242 | 3.42× | ok |
| re2-test-search | 5.778 | 16.572 | 2.87× | 13.856 | 2.40× | ok |
| tramp3d-v4 | 0.804 | 9.091 | 11.31× | 3.044 | 3.79× | ok |

### O3 compilation

| Program | cppgm++ (s) | GCC (s) | Speed vs GCC ↑ | Clang (s) | Speed vs Clang ↑ | Status |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| Adobe-C++/functionobjects | 0.137 | 0.991 | 7.23× | 0.579 | 4.23× | ok |
| Adobe-C++/loop_unroll | 0.380 | 6.266 | 16.49× | 4.984 | 13.12× | ok |
| Adobe-C++/simple_types_constant_folding | 0.237 | 3.013 | 12.71× | 2.578 | 10.88× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.197 | 2.513 | 12.76× | 2.154 | 10.93× | ok |
| Adobe-C++/stepanov_abstraction | 0.089 | 1.096 | 12.31× | 0.551 | 6.19× | ok |
| Adobe-C++/stepanov_vector | 0.140 | 1.435 | 10.25× | 0.775 | 5.54× | ok |
| Bullet | 7.177 | 42.902 | 5.98× | 29.813 | 4.15× | ok |
| CLAMR | 4.804 | 26.915 | 5.60× | 19.913 | 4.15× | ok |
| CoyoteBench/almabench | 0.041 | 0.204 | 4.98× | 0.126 | 3.07× | ok |
| CoyoteBench/fftbench | 0.155 | 0.465 | 3.00× | 0.503 | 3.25× | ok |
| CoyoteBench/huffbench | 0.047 | 0.233 | 4.96× | 0.145 | 3.09× | ok |
| CoyoteBench/lpbench | 0.046 | 0.221 | 4.80× | 0.250 | 5.43× | ok |
| DOE/HACCKernels | 0.145 | 0.429 | 2.96× | 0.444 | 3.06× | ok |
| DOE/HPCCG | 0.810 | 2.539 | 3.13× | 2.684 | 3.31× | ok |
| DOE/PENNANT | 2.117 | 6.826 | 3.22× | 6.639 | 3.14× | ok |
| DOE/miniFE | 1.042 | 4.441 | 4.26× | 4.021 | 3.86× | ok |
| Misc-C++-EH/spirit | 0.772 | 4.212 | 5.46× | 3.462 | 4.48× | ok |
| Misc-C++-Large/ray | 0.125 | 0.555 | 4.44× | 0.403 | 3.22× | ok |
| Misc-C++-Large/sphereflake | 0.123 | 0.810 | 6.59× | 0.411 | 3.34× | ok |
| Misc-C++/bigfib | 0.153 | 0.556 | 3.63× | 0.556 | 3.63× | ok |
| Misc-C++/oopack_v1p8 | 0.045 | 0.221 | 4.91× | 0.208 | 4.62× | ok |
| Misc-C++/stepanov_container | 0.315 | 0.958 | 3.04× | 1.077 | 3.42× | ok |
| Misc-C++/stepanov_v1p2 | 0.046 | 0.231 | 5.02× | 0.203 | 4.41× | ok |
| PAQ8p | 0.309 | 4.185 | 13.54× | 1.832 | 5.93× | ok |
| Prolangs-C++/life | 0.009 | 0.108 | — | 0.073 | — | ok |
| Prolangs-C++/ocean | 0.025 | 0.218 | 8.72× | 0.133 | 5.32× | ok |
| Prolangs-C++/primes | 0.009 | 0.040 | — | 0.043 | — | ok |
| Shootout-C++-EH/except | 0.092 | 0.234 | 2.54× | 0.338 | 3.67× | ok |
| Shootout-C++/ackermann | 0.086 | 0.238 | 2.77× | 0.265 | 3.08× | ok |
| Shootout-C++/ary3 | 0.102 | 0.268 | 2.63× | 0.399 | 3.91× | ok |
| Shootout-C++/fibo | 0.094 | 0.262 | 2.79× | 0.288 | 3.06× | ok |
| Shootout-C++/hash | 0.140 | 0.417 | 2.98× | 0.545 | 3.89× | ok |
| Shootout-C++/hash2 | 0.151 | 0.500 | 3.31× | 0.522 | 3.46× | ok |
| Shootout-C++/heapsort | 0.104 | 0.229 | 2.20× | 0.314 | 3.02× | ok |
| Shootout-C++/lists | 0.097 | 0.276 | 2.85× | 0.311 | 3.21× | ok |
| Shootout-C++/lists1 | 0.104 | 0.313 | 3.01× | 0.357 | 3.43× | ok |
| Shootout-C++/matrix | 0.090 | 0.230 | 2.56× | 0.374 | 4.16× | ok |
| Shootout-C++/methcall | 0.081 | 0.222 | 2.74× | 0.265 | 3.27× | ok |
| Shootout-C++/moments | 0.134 | 0.383 | 2.86× | 0.379 | 2.83× | ok |
| Shootout-C++/random | 0.104 | 0.241 | 2.32× | 0.306 | 2.94× | ok |
| Shootout-C++/sieve | 0.108 | 0.284 | 2.63× | 0.325 | 3.01× | ok |
| SmallPT | 0.048 | 0.327 | 6.81× | 0.178 | 3.71× | ok |
| Stanford/Bubblesort | 0.014 | 0.060 | 4.29× | 0.054 | 3.86× | ok |
| Stanford/FloatMM | 0.016 | 0.068 | 4.25× | 0.075 | 4.69× | ok |
| Stanford/Puzzle | 0.018 | 0.139 | 7.72× | 0.131 | 7.28× | ok |
| Stanford/Treesort | 0.016 | 0.090 | 5.62× | 0.062 | 3.88× | ok |
| bg/binarytrees/2 | 0.086 | 0.449 | 5.22× | 0.272 | 3.16× | ok |
| bg/fannkuchredux/3 | 0.044 | 0.134 | 3.05× | 0.132 | 3.00× | ok |
| bg/fasta/1 | 0.068 | 0.184 | 2.71× | 0.199 | 2.93× | ok |
| bg/fasta/2 | 0.071 | 0.185 | 2.61× | 0.206 | 2.90× | ok |
| bg/fasta/3 | 0.101 | 0.250 | 2.48× | 0.325 | 3.22× | ok |
| bg/fasta/4 | 0.090 | 0.222 | 2.47× | 0.261 | 2.90× | ok |
| bg/fasta/6 | 0.171 | 0.496 | 2.90× | 0.587 | 3.43× | ok |
| bg/mandelbrot/0 | 0.197 | 0.732 | 3.72× | 0.894 | 4.54× | ok |
| bg/mandelbrot/2 | 0.112 | 0.329 | 2.94× | 0.373 | 3.33× | ok |
| bg/mandelbrot/3 | 0.113 | 0.266 | 2.35× | 0.334 | 2.96× | ok |
| bg/mandelbrot/5 | 0.060 | 0.173 | 2.88× | 0.181 | 3.02× | ok |
| bg/mandelbrot/7 | 0.065 | 0.217 | 3.34× | 0.205 | 3.15× | ok |
| bg/mandelbrot/9 | 0.059 | 0.200 | 3.39× | 0.184 | 3.12× | ok |
| bg/nbody/1 | 0.046 | 0.180 | 3.91× | 0.187 | 4.07× | ok |
| bg/nbody/2 | 0.034 | 0.144 | 4.24× | 0.135 | 3.97× | ok |
| bg/nbody/3 | 0.040 | 0.134 | 3.35× | 0.127 | 3.17× | ok |
| bg/nbody/6 | 0.045 | 0.179 | 3.98× | 0.189 | 4.20× | ok |
| bg/pidigits/4 | 0.146 | 0.375 | 2.57× | 0.495 | 3.39× | ok |
| bg/pidigits/5 | 0.093 | 0.236 | 2.54× | 0.320 | 3.44× | ok |
| bg/revcomp/1 | 0.111 | 0.288 | 2.59× | 0.346 | 3.12× | ok |
| bg/revcomp/5 | 0.100 | 0.257 | 2.57× | 0.371 | 3.71× | ok |
| bg/spectralnorm/1 | 0.138 | 0.391 | 2.83× | 0.469 | 3.40× | ok |
| box2d | 2.205 | 8.195 | 3.72× | 8.499 | 3.85× | ok |
| fmt | 1.068 | 5.965 | 5.59× | 4.738 | 4.44× | ok |
| hexxagon | 0.358 | 1.165 | 3.25× | 1.159 | 3.24× | ok |
| lambda | 0.137 | 1.118 | 8.16× | 0.564 | 4.12× | ok |
| minisat | 0.156 | 1.141 | 7.31× | 0.672 | 4.31× | ok |
| nlohmann-json | 0.909 | 4.054 | 4.46× | 3.413 | 3.75× | ok |
| pugixml | 0.686 | 6.244 | 9.10× | 3.879 | 5.65× | ok |
| pugixml-tests | 6.107 | 38.124 | 6.24× | 28.431 | 4.66× | ok |
| rapidjson | 0.247 | 1.349 | 5.46× | 0.797 | 3.23× | ok |
| re2 | 3.648 | 19.641 | 5.38× | 11.486 | 3.15× | ok |
| re2-test-re2 | 6.713 | 30.215 | 4.50× | 27.629 | 4.12× | ok |
| re2-test-search | 6.442 | 26.570 | 4.12× | 19.918 | 3.09× | ok |
| tramp3d-v4 | 2.327 | 18.650 | 8.01× | 14.761 | 6.34× | ok |

### Frontend only: -fsyntax-only

| Program | cppgm++ (s) | GCC (s) | Speed vs GCC ↑ | Clang (s) | Speed vs Clang ↑ | Status |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| Adobe-C++/functionobjects | 0.079 | 0.163 | 2.06× | 0.197 | 2.49× | ok |
| Adobe-C++/loop_unroll | 0.042 | 0.128 | 3.05× | 0.133 | 3.17× | ok |
| Adobe-C++/simple_types_constant_folding | 0.054 | 0.121 | 2.24× | 0.149 | 2.76× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.047 | 0.111 | 2.36× | 0.129 | 2.74× | ok |
| Adobe-C++/stepanov_abstraction | 0.039 | 0.085 | 2.18× | 0.107 | 2.74× | ok |
| Adobe-C++/stepanov_vector | 0.076 | 0.163 | 2.14× | 0.227 | 2.99× | ok |
| Bullet | 5.159 | 13.293 | 2.58× | 17.024 | 3.30× | ok |
| CLAMR | 2.502 | 6.233 | 2.49× | 8.053 | 3.22× | ok |
| CoyoteBench/almabench | 0.039 | 0.086 | 2.21× | 0.102 | 2.62× | ok |
| CoyoteBench/fftbench | 0.125 | 0.268 | 2.14× | 0.349 | 2.79× | ok |
| CoyoteBench/huffbench | 0.034 | 0.071 | 2.09× | 0.108 | 3.18× | ok |
| CoyoteBench/lpbench | 0.040 | 0.082 | 2.05× | 0.119 | 2.97× | ok |
| DOE/HACCKernels | 0.131 | 0.323 | 2.47× | 0.363 | 2.77× | ok |
| DOE/HPCCG | 0.738 | 1.595 | 2.16× | 2.147 | 2.91× | ok |
| DOE/PENNANT | 1.672 | 3.581 | 2.14× | 4.451 | 2.66× | ok |
| DOE/miniFE | 0.774 | 1.627 | 2.10× | 1.981 | 2.56× | ok |
| Misc-C++-EH/spirit | 0.374 | 0.961 | 2.57× | 1.687 | 4.51× | ok |
| Misc-C++-Large/ray | 0.133 | 0.276 | 2.08× | 0.321 | 2.41× | ok |
| Misc-C++-Large/sphereflake | 0.104 | 0.234 | 2.25× | 0.287 | 2.76× | ok |
| Misc-C++/bigfib | 0.116 | 0.273 | 2.35× | 0.430 | 3.71× | ok |
| Misc-C++/oopack_v1p8 | 0.039 | 0.080 | 2.05× | 0.095 | 2.44× | ok |
| Misc-C++/stepanov_container | 0.159 | 0.387 | 2.43× | 0.681 | 4.28× | ok |
| Misc-C++/stepanov_v1p2 | 0.032 | 0.070 | 2.19× | 0.091 | 2.84× | ok |
| PAQ8p | 0.055 | 0.138 | 2.51× | 0.150 | 2.73× | ok |
| Prolangs-C++/life | 0.004 | 0.012 | — | 0.023 | — | ok |
| Prolangs-C++/ocean | 0.012 | 0.027 | 2.25× | 0.040 | 3.33× | ok |
| Prolangs-C++/primes | 0.007 | 0.020 | — | 0.031 | — | ok |
| Shootout-C++-EH/except | 0.085 | 0.197 | 2.32× | 0.298 | 3.51× | ok |
| Shootout-C++/ackermann | 0.086 | 0.203 | 2.36× | 0.312 | 3.63× | ok |
| Shootout-C++/ary3 | 0.091 | 0.204 | 2.24× | 0.269 | 2.96× | ok |
| Shootout-C++/fibo | 0.096 | 0.186 | 1.94× | 0.303 | 3.16× | ok |
| Shootout-C++/hash | 0.118 | 0.298 | 2.53× | 0.326 | 2.76× | ok |
| Shootout-C++/hash2 | 0.126 | 0.269 | 2.13× | 0.335 | 2.66× | ok |
| Shootout-C++/heapsort | 0.104 | 0.210 | 2.02× | 0.289 | 2.78× | ok |
| Shootout-C++/lists | 0.088 | 0.216 | 2.45× | 0.272 | 3.09× | ok |
| Shootout-C++/lists1 | 0.092 | 0.222 | 2.41× | 0.263 | 2.86× | ok |
| Shootout-C++/matrix | 0.082 | 0.177 | 2.16× | 0.230 | 2.80× | ok |
| Shootout-C++/methcall | 0.078 | 0.180 | 2.31× | 0.242 | 3.10× | ok |
| Shootout-C++/moments | 0.117 | 0.249 | 2.13× | 0.283 | 2.42× | ok |
| Shootout-C++/random | 0.101 | 0.204 | 2.02× | 0.279 | 2.76× | ok |
| Shootout-C++/sieve | 0.098 | 0.219 | 2.23× | 0.287 | 2.93× | ok |
| SmallPT | 0.034 | 0.066 | 1.94× | 0.100 | 2.94× | ok |
| Stanford/Bubblesort | 0.011 | 0.028 | 2.55× | 0.036 | 3.27× | ok |
| Stanford/FloatMM | 0.011 | 0.028 | 2.55× | 0.035 | 3.18× | ok |
| Stanford/Puzzle | 0.011 | 0.028 | 2.55× | 0.037 | 3.36× | ok |
| Stanford/Treesort | 0.011 | 0.029 | 2.64× | 0.037 | 3.36× | ok |
| bg/binarytrees/2 | 0.081 | 0.177 | 2.19× | 0.242 | 2.99× | ok |
| bg/fannkuchredux/3 | 0.039 | 0.073 | 1.87× | 0.097 | 2.49× | ok |
| bg/fasta/1 | 0.059 | 0.116 | 1.97× | 0.146 | 2.47× | ok |
| bg/fasta/2 | 0.061 | 0.112 | 1.84× | 0.154 | 2.52× | ok |
| bg/fasta/3 | 0.095 | 0.199 | 2.09× | 0.268 | 2.82× | ok |
| bg/fasta/4 | 0.087 | 0.172 | 1.98× | 0.215 | 2.47× | ok |
| bg/fasta/6 | 0.144 | 0.341 | 2.37× | 0.463 | 3.22× | ok |
| bg/mandelbrot/0 | 0.162 | 0.346 | 2.14× | 0.474 | 2.93× | ok |
| bg/mandelbrot/2 | 0.100 | 0.231 | 2.31× | 0.394 | 3.94× | ok |
| bg/mandelbrot/3 | 0.109 | 0.229 | 2.10× | 0.311 | 2.85× | ok |
| bg/mandelbrot/5 | 0.052 | 0.110 | 2.12× | 0.135 | 2.60× | ok |
| bg/mandelbrot/7 | 0.055 | 0.109 | 1.98× | 0.141 | 2.56× | ok |
| bg/mandelbrot/9 | 0.049 | 0.098 | 2.00× | 0.128 | 2.61× | ok |
| bg/nbody/1 | 0.032 | 0.071 | 2.22× | 0.088 | 2.75× | ok |
| bg/nbody/2 | 0.033 | 0.066 | 2.00× | 0.093 | 2.82× | ok |
| bg/nbody/3 | 0.032 | 0.061 | 1.91× | 0.092 | 2.88× | ok |
| bg/nbody/6 | 0.033 | 0.068 | 2.06× | 0.092 | 2.79× | ok |
| bg/pidigits/4 | 0.142 | 0.300 | 2.11× | 0.409 | 2.88× | ok |
| bg/pidigits/5 | 0.089 | 0.194 | 2.18× | 0.262 | 2.94× | ok |
| bg/revcomp/1 | 0.108 | 0.207 | 1.92× | 0.289 | 2.68× | ok |
| bg/revcomp/5 | 0.083 | 0.184 | 2.22× | 0.340 | 4.10× | ok |
| bg/spectralnorm/1 | 0.144 | 0.272 | 1.89× | 0.390 | 2.71× | ok |
| box2d | 1.680 | 3.823 | 2.28× | 5.418 | 3.23× | ok |
| fmt | 0.568 | 1.289 | 2.27× | 1.830 | 3.22× | ok |
| hexxagon | 0.306 | 0.680 | 2.22× | 0.846 | 2.76× | ok |
| lambda | 0.053 | 0.150 | 2.83× | 0.223 | 4.21× | ok |
| minisat | 0.069 | 0.145 | 2.10× | 0.217 | 3.14× | ok |
| nlohmann-json | 0.427 | 1.004 | 2.35× | 1.737 | 4.07× | ok |
| pugixml | 0.236 | 0.588 | 2.49× | 0.810 | 3.43× | ok |
| pugixml-tests | 3.802 | 9.008 | 2.37× | 12.335 | 3.24× | ok |
| rapidjson | 0.120 | 0.269 | 2.24× | 0.399 | 3.33× | ok |
| re2 | 3.526 | 7.829 | 2.22× | 6.888 | 1.95× | ok |
| re2-test-re2 | 5.102 | 11.273 | 2.21× | 16.120 | 3.16× | ok |
| re2-test-search | 4.928 | 10.493 | 2.13× | 15.824 | 3.21× | ok |
| tramp3d-v4 | 0.506 | 2.222 | 4.39× | 1.798 | 3.55× | ok |


## Toolchains

- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)
- **valgrind**: valgrind-3.26.0

[Compiler build and codegen results](self-build.md) have their own measured revision.
