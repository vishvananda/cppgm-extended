# Public benchmarks: AMD EPYC

Measured compiler: [`08bba949b`](https://github.com/vishvananda/cppgm-extended/commit/08bba949b4a0c3f2a728bf52c6e7089a38b44797) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/41766426226) · [Raw program results](results.json) · [Raw rebuild results](selfhost.json)

81 of 81 programs built with cppgm++ and g++ and agreed. Each number is the reference over cppgm++, so above 1× means cppgm++ is smaller or faster.

CPU: AMD EPYC 9V45 96-Core Processor.

| Measure | vs g++ | Programs | vs clang++ | Programs |
| --- | ---: | ---: | ---: | ---: |
| Frontend CPU, `-fsyntax-only` | 2.01× | 79 | 2.49× | 79 |
| Compile CPU, `-O0` | 2.78× | 79 | 2.68× | 79 |
| Compile CPU, `-O3` | 4.08× | 79 | 3.60× | 79 |
| Compile peak RSS, `-O0` | 1.48× | 81 | 2.26× | 81 |
| Compile peak RSS, `-O3` | 1.75× | 81 | 2.36× | 81 |
| Executable size, `-O0` | 0.93× | 81 | 0.88× | 81 |
| Executable size, `-O3` | 0.93× | 81 | 0.88× | 81 |
| Run CPU, `-O0` | 1.03× | 81 | 1.01× | 81 |
| Run CPU, `-O3` | 0.99× | 80 | 0.99× | 80 |
| Run instructions, `-O3` | 0.95× | 43 | - | 0 |
| Self compile CPU | 4.46× | - | - | - |
| Self-built compiler CPU | 0.98× | - | - | - |

O3 execution uses paired ABBA/BAAB blocks. GCC A/A CPU ratios ranged from 0.857 to 1.480; raw observations and block ratios are retained in the artifact.

Self-built / GCC-built compiler CPU: paired ratios 1.024–1.026; A/A calibration 0.980. A/A variation limits conclusions about small changes.

## Slowest generated code, by run-time instructions

| Program | vs g++ |
| --- | ---: |
| bg/nbody/2 | 0.63× |
| Stanford/Bubblesort | 0.72× |
| PAQ8p | 0.74× |
| tramp3d-v4 | 0.75× |
| bg/spectralnorm/1 | 0.75× |
| bg/nbody/1 | 0.83× |
| bg/nbody/3 | 0.83× |
| bg/mandelbrot/7 | 0.84× |
| bg/mandelbrot/9 | 0.84× |
| Misc-C++/oopack_v1p8 | 0.86× |

## Compile CPU times

Times are CPU seconds, excluding linking. Speed is reference time divided by cppgm++ time; higher is better. Every program is listed. Missing times show a dash. Ratios are omitted for failed programs or when either time is below 10 ms.

Compilation is measured once per compiler and level; the ABBA/BAAB repetitions apply to generated-program execution and self-built compiler speed.

### O0 compilation

| Program | cppgm++ (s) | GCC (s) | Speed vs GCC ↑ | Clang (s) | Speed vs Clang ↑ | Status |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| Adobe-C++/functionobjects | 0.086 | 0.262 | 3.05× | 0.204 | 2.37× | ok |
| Adobe-C++/loop_unroll | 0.049 | 0.556 | 11.35× | 0.165 | 3.37× | ok |
| Adobe-C++/simple_types_constant_folding | 0.063 | 0.640 | 10.16× | 0.197 | 3.13× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.053 | 0.468 | 8.83× | 0.216 | 4.08× | ok |
| Adobe-C++/stepanov_abstraction | 0.048 | 0.166 | 3.46× | 0.122 | 2.54× | ok |
| Adobe-C++/stepanov_vector | 0.081 | 0.270 | 3.33× | 0.196 | 2.42× | ok |
| Bullet | 5.210 | 17.316 | 3.32× | 15.822 | 3.04× | ok |
| CLAMR | 2.759 | 9.025 | 3.27× | 7.102 | 2.57× | ok |
| CoyoteBench/almabench | 0.032 | 0.079 | 2.47× | 0.082 | 2.56× | ok |
| CoyoteBench/fftbench | 0.114 | 0.250 | 2.19× | 0.288 | 2.53× | ok |
| CoyoteBench/huffbench | 0.034 | 0.071 | 2.09× | 0.073 | 2.15× | ok |
| CoyoteBench/lpbench | 0.035 | 0.076 | 2.17× | 0.081 | 2.31× | ok |
| DOE/HACCKernels | 0.136 | 0.281 | 2.07× | 0.305 | 2.24× | ok |
| DOE/HPCCG | 0.792 | 1.555 | 1.96× | 2.245 | 2.83× | ok |
| DOE/PENNANT | 1.638 | 3.698 | 2.26× | 4.005 | 2.45× | ok |
| DOE/miniFE | 0.694 | 2.020 | 2.91× | 1.809 | 2.61× | ok |
| Misc-C++-EH/spirit | 0.481 | 2.319 | 4.82× | 1.817 | 3.78× | ok |
| Misc-C++-Large/ray | 0.107 | 0.271 | 2.53× | 0.325 | 3.04× | ok |
| Misc-C++-Large/sphereflake | 0.102 | 0.227 | 2.23× | 0.258 | 2.53× | ok |
| Misc-C++/bigfib | 0.138 | 0.401 | 2.91× | 0.383 | 2.78× | ok |
| Misc-C++/oopack_v1p8 | 0.036 | 0.094 | 2.61× | 0.101 | 2.81× | ok |
| Misc-C++/stepanov_container | 0.182 | 0.545 | 2.99× | 0.548 | 3.01× | ok |
| Misc-C++/stepanov_v1p2 | 0.034 | 0.120 | 3.53× | 0.088 | 2.59× | ok |
| PAQ8p | 0.071 | 0.430 | 6.06× | 0.202 | 2.85× | ok |
| Prolangs-C++/life | 0.004 | 0.031 | — | 0.028 | — | ok |
| Prolangs-C++/ocean | 0.011 | 0.064 | 5.82× | 0.048 | 4.36× | ok |
| Prolangs-C++/primes | 0.007 | 0.025 | — | 0.030 | — | ok |
| Shootout-C++-EH/except | 0.082 | 0.176 | 2.15× | 0.215 | 2.62× | ok |
| Shootout-C++/ackermann | 0.090 | 0.155 | 1.72× | 0.210 | 2.33× | ok |
| Shootout-C++/ary3 | 0.093 | 0.197 | 2.12× | 0.268 | 2.88× | ok |
| Shootout-C++/fibo | 0.091 | 0.154 | 1.69× | 0.228 | 2.51× | ok |
| Shootout-C++/hash | 0.130 | 0.292 | 2.25× | 0.295 | 2.27× | ok |
| Shootout-C++/hash2 | 0.121 | 0.276 | 2.28× | 0.292 | 2.41× | ok |
| Shootout-C++/heapsort | 0.096 | 0.181 | 1.89× | 0.240 | 2.50× | ok |
| Shootout-C++/lists | 0.094 | 0.207 | 2.20× | 0.235 | 2.50× | ok |
| Shootout-C++/lists1 | 0.101 | 0.217 | 2.15× | 0.270 | 2.67× | ok |
| Shootout-C++/matrix | 0.084 | 0.158 | 1.88× | 0.196 | 2.33× | ok |
| Shootout-C++/methcall | 0.107 | 0.161 | 1.50× | 0.201 | 1.88× | ok |
| Shootout-C++/moments | 0.118 | 0.266 | 2.25× | 0.268 | 2.27× | ok |
| Shootout-C++/random | 0.102 | 0.185 | 1.81× | 0.252 | 2.47× | ok |
| Shootout-C++/sieve | 0.103 | 0.240 | 2.33× | 0.301 | 2.92× | ok |
| SmallPT | 0.034 | 0.088 | 2.59× | 0.085 | 2.50× | ok |
| Stanford/Bubblesort | 0.011 | 0.030 | 2.73× | 0.036 | 3.27× | ok |
| Stanford/FloatMM | 0.011 | 0.031 | 2.82× | 0.037 | 3.36× | ok |
| Stanford/Puzzle | 0.012 | 0.039 | 3.25× | 0.037 | 3.08× | ok |
| Stanford/Treesort | 0.012 | 0.033 | 2.75× | 0.038 | 3.17× | ok |
| bg/binarytrees/2 | 0.081 | 0.162 | 2.00× | 0.223 | 2.75× | ok |
| bg/fannkuchredux/3 | 0.038 | 0.075 | 1.97× | 0.083 | 2.18× | ok |
| bg/fasta/1 | 0.061 | 0.126 | 2.07× | 0.132 | 2.16× | ok |
| bg/fasta/2 | 0.058 | 0.127 | 2.19× | 0.133 | 2.29× | ok |
| bg/fasta/3 | 0.093 | 0.184 | 1.98× | 0.223 | 2.40× | ok |
| bg/fasta/4 | 0.086 | 0.147 | 1.71× | 0.191 | 2.22× | ok |
| bg/fasta/6 | 0.163 | 0.407 | 2.50× | 0.369 | 2.26× | ok |
| bg/mandelbrot/0 | 0.155 | 0.449 | 2.90× | 0.407 | 2.63× | ok |
| bg/mandelbrot/2 | 0.099 | 0.206 | 2.08× | 0.295 | 2.98× | ok |
| bg/mandelbrot/3 | 0.101 | 0.208 | 2.06× | 0.252 | 2.50× | ok |
| bg/mandelbrot/5 | 0.048 | 0.109 | 2.27× | 0.116 | 2.42× | ok |
| bg/mandelbrot/7 | 0.048 | 0.119 | 2.48× | 0.123 | 2.56× | ok |
| bg/mandelbrot/9 | 0.046 | 0.134 | 2.91× | 0.117 | 2.54× | ok |
| bg/nbody/1 | 0.031 | 0.104 | 3.35× | 0.084 | 2.71× | ok |
| bg/nbody/2 | 0.029 | 0.067 | 2.31× | 0.075 | 2.59× | ok |
| bg/nbody/3 | 0.029 | 0.066 | 2.28× | 0.074 | 2.55× | ok |
| bg/nbody/6 | 0.030 | 0.095 | 3.17× | 0.089 | 2.97× | ok |
| bg/pidigits/4 | 0.136 | 0.291 | 2.14× | 0.351 | 2.58× | ok |
| bg/pidigits/5 | 0.086 | 0.163 | 1.90× | 0.206 | 2.40× | ok |
| bg/revcomp/1 | 0.087 | 0.201 | 2.31× | 0.229 | 2.63× | ok |
| bg/revcomp/5 | 0.093 | 0.208 | 2.24× | 0.250 | 2.69× | ok |
| bg/spectralnorm/1 | 0.125 | 0.260 | 2.08× | 0.318 | 2.54× | ok |
| box2d | 1.729 | 4.150 | 2.40× | 4.707 | 2.72× | ok |
| fmt | 0.561 | 2.290 | 4.08× | 1.815 | 3.24× | ok |
| hexxagon | 0.312 | 0.667 | 2.14× | 0.815 | 2.61× | ok |
| lambda | 0.058 | 0.317 | 5.47× | 0.235 | 4.05× | ok |
| minisat | 0.064 | 0.332 | 5.19× | 0.200 | 3.12× | ok |
| nlohmann-json | 0.468 | 1.724 | 3.68× | 1.746 | 3.73× | ok |
| pugixml | 0.328 | 1.360 | 4.15× | 0.789 | 2.41× | ok |
| pugixml-tests | 3.910 | 14.218 | 3.64× | 10.491 | 2.68× | ok |
| rapidjson | 0.124 | 0.405 | 3.27× | 0.371 | 2.99× | ok |
| re2 | 3.471 | 9.896 | 2.85× | 6.319 | 1.82× | ok |
| re2-test-re2 | 5.077 | 14.432 | 2.84× | 15.054 | 2.97× | ok |
| re2-test-search | 5.758 | 13.653 | 2.37× | 12.427 | 2.16× | ok |
| tramp3d-v4 | 0.720 | 8.921 | 12.39× | 2.381 | 3.31× | ok |

### O3 compilation

| Program | cppgm++ (s) | GCC (s) | Speed vs GCC ↑ | Clang (s) | Speed vs Clang ↑ | Status |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| Adobe-C++/functionobjects | 0.110 | 0.791 | 7.19× | 0.443 | 4.03× | ok |
| Adobe-C++/loop_unroll | 0.239 | 4.860 | 20.33× | 4.291 | 17.95× | ok |
| Adobe-C++/simple_types_constant_folding | 0.140 | 2.739 | 19.56× | 2.135 | 15.25× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.117 | 2.115 | 18.08× | 1.692 | 14.46× | ok |
| Adobe-C++/stepanov_abstraction | 0.072 | 0.932 | 12.94× | 0.478 | 6.64× | ok |
| Adobe-C++/stepanov_vector | 0.115 | 1.208 | 10.50× | 0.565 | 4.91× | ok |
| Bullet | 6.428 | 36.767 | 5.72× | 26.412 | 4.11× | ok |
| CLAMR | 4.503 | 23.016 | 5.11× | 16.290 | 3.62× | ok |
| CoyoteBench/almabench | 0.037 | 0.156 | 4.22× | 0.114 | 3.08× | ok |
| CoyoteBench/fftbench | 0.129 | 0.404 | 3.13× | 0.410 | 3.18× | ok |
| CoyoteBench/huffbench | 0.036 | 0.212 | 5.89× | 0.125 | 3.47× | ok |
| CoyoteBench/lpbench | 0.038 | 0.174 | 4.58× | 0.192 | 5.05× | ok |
| DOE/HACCKernels | 0.127 | 0.397 | 3.13× | 0.378 | 2.98× | ok |
| DOE/HPCCG | 0.814 | 2.241 | 2.75× | 2.624 | 3.22× | ok |
| DOE/PENNANT | 1.864 | 5.888 | 3.16× | 5.885 | 3.16× | ok |
| DOE/miniFE | 0.889 | 3.747 | 4.21× | 3.610 | 4.06× | ok |
| Misc-C++-EH/spirit | 0.674 | 3.341 | 4.96× | 3.513 | 5.21× | ok |
| Misc-C++-Large/ray | 0.131 | 0.444 | 3.39× | 0.394 | 3.01× | ok |
| Misc-C++-Large/sphereflake | 0.111 | 0.722 | 6.50× | 0.328 | 2.95× | ok |
| Misc-C++/bigfib | 0.163 | 0.570 | 3.50× | 0.549 | 3.37× | ok |
| Misc-C++/oopack_v1p8 | 0.043 | 0.187 | 4.35× | 0.150 | 3.49× | ok |
| Misc-C++/stepanov_container | 0.250 | 0.864 | 3.46× | 0.942 | 3.77× | ok |
| Misc-C++/stepanov_v1p2 | 0.040 | 0.224 | 5.60× | 0.187 | 4.67× | ok |
| PAQ8p | 0.267 | 3.636 | 13.62× | 1.530 | 5.73× | ok |
| Prolangs-C++/life | 0.008 | 0.100 | — | 0.064 | — | ok |
| Prolangs-C++/ocean | 0.018 | 0.194 | 10.78× | 0.111 | 6.17× | ok |
| Prolangs-C++/primes | 0.008 | 0.035 | — | 0.036 | — | ok |
| Shootout-C++-EH/except | 0.101 | 0.214 | 2.12× | 0.205 | 2.03× | ok |
| Shootout-C++/ackermann | 0.091 | 0.197 | 2.16× | 0.203 | 2.23× | ok |
| Shootout-C++/ary3 | 0.107 | 0.246 | 2.30× | 0.258 | 2.41× | ok |
| Shootout-C++/fibo | 0.087 | 0.195 | 2.24× | 0.224 | 2.57× | ok |
| Shootout-C++/hash | 0.135 | 0.417 | 3.09× | 0.377 | 2.79× | ok |
| Shootout-C++/hash2 | 0.137 | 0.424 | 3.09× | 0.456 | 3.33× | ok |
| Shootout-C++/heapsort | 0.104 | 0.222 | 2.13× | 0.254 | 2.44× | ok |
| Shootout-C++/lists | 0.097 | 0.255 | 2.63× | 0.278 | 2.87× | ok |
| Shootout-C++/lists1 | 0.110 | 0.266 | 2.42× | 0.301 | 2.74× | ok |
| Shootout-C++/matrix | 0.085 | 0.203 | 2.39× | 0.305 | 3.59× | ok |
| Shootout-C++/methcall | 0.082 | 0.189 | 2.30× | 0.216 | 2.63× | ok |
| Shootout-C++/moments | 0.127 | 0.324 | 2.55× | 0.328 | 2.58× | ok |
| Shootout-C++/random | 0.092 | 0.245 | 2.66× | 0.254 | 2.76× | ok |
| Shootout-C++/sieve | 0.114 | 0.247 | 2.17× | 0.295 | 2.59× | ok |
| SmallPT | 0.038 | 0.283 | 7.45× | 0.152 | 4.00× | ok |
| Stanford/Bubblesort | 0.013 | 0.057 | 4.38× | 0.052 | 4.00× | ok |
| Stanford/FloatMM | 0.014 | 0.064 | 4.57× | 0.069 | 4.93× | ok |
| Stanford/Puzzle | 0.017 | 0.125 | 7.35× | 0.114 | 6.71× | ok |
| Stanford/Treesort | 0.015 | 0.085 | 5.67× | 0.059 | 3.93× | ok |
| bg/binarytrees/2 | 0.083 | 0.410 | 4.94× | 0.222 | 2.67× | ok |
| bg/fannkuchredux/3 | 0.044 | 0.118 | 2.68× | 0.119 | 2.70× | ok |
| bg/fasta/1 | 0.066 | 0.155 | 2.35× | 0.175 | 2.65× | ok |
| bg/fasta/2 | 0.064 | 0.171 | 2.67× | 0.184 | 2.88× | ok |
| bg/fasta/3 | 0.100 | 0.221 | 2.21× | 0.282 | 2.82× | ok |
| bg/fasta/4 | 0.093 | 0.187 | 2.01× | 0.264 | 2.84× | ok |
| bg/fasta/6 | 0.155 | 0.507 | 3.27× | 0.490 | 3.16× | ok |
| bg/mandelbrot/0 | 0.198 | 0.598 | 3.02× | 0.677 | 3.42× | ok |
| bg/mandelbrot/2 | 0.107 | 0.280 | 2.62× | 0.307 | 2.87× | ok |
| bg/mandelbrot/3 | 0.114 | 0.225 | 1.97× | 0.264 | 2.32× | ok |
| bg/mandelbrot/5 | 0.052 | 0.155 | 2.98× | 0.140 | 2.69× | ok |
| bg/mandelbrot/7 | 0.057 | 0.178 | 3.12× | 0.174 | 3.05× | ok |
| bg/mandelbrot/9 | 0.054 | 0.207 | 3.83× | 0.150 | 2.78× | ok |
| bg/nbody/1 | 0.051 | 0.145 | 2.84× | 0.149 | 2.92× | ok |
| bg/nbody/2 | 0.032 | 0.123 | 3.84× | 0.112 | 3.50× | ok |
| bg/nbody/3 | 0.023 | 0.119 | 5.17× | 0.102 | 4.43× | ok |
| bg/nbody/6 | 0.039 | 0.154 | 3.95× | 0.152 | 3.90× | ok |
| bg/pidigits/4 | 0.150 | 0.327 | 2.18× | 0.393 | 2.62× | ok |
| bg/pidigits/5 | 0.085 | 0.198 | 2.33× | 0.228 | 2.68× | ok |
| bg/revcomp/1 | 0.107 | 0.228 | 2.13× | 0.317 | 2.96× | ok |
| bg/revcomp/5 | 0.091 | 0.264 | 2.90× | 0.284 | 3.12× | ok |
| bg/spectralnorm/1 | 0.140 | 0.318 | 2.27× | 0.387 | 2.76× | ok |
| box2d | 1.929 | 7.356 | 3.81× | 6.645 | 3.44× | ok |
| fmt | 1.027 | 5.145 | 5.01× | 4.199 | 4.09× | ok |
| hexxagon | 0.333 | 0.930 | 2.79× | 1.065 | 3.20× | ok |
| lambda | 0.099 | 1.012 | 10.22× | 0.558 | 5.64× | ok |
| minisat | 0.141 | 0.902 | 6.40× | 0.611 | 4.33× | ok |
| nlohmann-json | 0.675 | 3.330 | 4.93× | 2.674 | 3.96× | ok |
| pugixml | 0.675 | 5.270 | 7.81× | 3.465 | 5.13× | ok |
| pugixml-tests | 5.604 | 32.772 | 5.85× | 22.130 | 3.95× | ok |
| rapidjson | 0.222 | 1.145 | 5.16× | 0.661 | 2.98× | ok |
| re2 | 3.505 | 16.878 | 4.82× | 9.293 | 2.65× | ok |
| re2-test-re2 | 6.087 | 25.054 | 4.12× | 21.939 | 3.60× | ok |
| re2-test-search | 6.667 | 24.494 | 3.67× | 19.441 | 2.92× | ok |
| tramp3d-v4 | 2.467 | 22.694 | 9.20× | 12.776 | 5.18× | ok |

### Frontend only: -fsyntax-only

| Program | cppgm++ (s) | GCC (s) | Speed vs GCC ↑ | Clang (s) | Speed vs Clang ↑ | Status |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| Adobe-C++/functionobjects | 0.071 | 0.127 | 1.79× | 0.173 | 2.44× | ok |
| Adobe-C++/loop_unroll | 0.038 | 0.091 | 2.39× | 0.099 | 2.61× | ok |
| Adobe-C++/simple_types_constant_folding | 0.043 | 0.093 | 2.16× | 0.102 | 2.37× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.038 | 0.087 | 2.29× | 0.119 | 3.13× | ok |
| Adobe-C++/stepanov_abstraction | 0.039 | 0.071 | 1.82× | 0.094 | 2.41× | ok |
| Adobe-C++/stepanov_vector | 0.069 | 0.125 | 1.81× | 0.190 | 2.75× | ok |
| Bullet | 4.919 | 10.849 | 2.21× | 13.195 | 2.68× | ok |
| CLAMR | 2.519 | 4.898 | 1.94× | 5.841 | 2.32× | ok |
| CoyoteBench/almabench | 0.032 | 0.065 | 2.03× | 0.073 | 2.28× | ok |
| CoyoteBench/fftbench | 0.114 | 0.221 | 1.94× | 0.261 | 2.29× | ok |
| CoyoteBench/huffbench | 0.033 | 0.061 | 1.85× | 0.069 | 2.09× | ok |
| CoyoteBench/lpbench | 0.033 | 0.063 | 1.91× | 0.080 | 2.42× | ok |
| DOE/HACCKernels | 0.133 | 0.255 | 1.92× | 0.283 | 2.13× | ok |
| DOE/HPCCG | 0.769 | 1.364 | 1.77× | 2.037 | 2.65× | ok |
| DOE/PENNANT | 1.610 | 2.792 | 1.73× | 3.634 | 2.26× | ok |
| DOE/miniFE | 0.648 | 1.354 | 2.09× | 1.599 | 2.47× | ok |
| Misc-C++-EH/spirit | 0.398 | 0.878 | 2.21× | 1.022 | 2.57× | ok |
| Misc-C++-Large/ray | 0.102 | 0.221 | 2.17× | 0.303 | 2.97× | ok |
| Misc-C++-Large/sphereflake | 0.100 | 0.208 | 2.08× | 0.241 | 2.41× | ok |
| Misc-C++/bigfib | 0.128 | 0.278 | 2.17× | 0.342 | 2.67× | ok |
| Misc-C++/oopack_v1p8 | 0.033 | 0.065 | 1.97× | 0.086 | 2.61× | ok |
| Misc-C++/stepanov_container | 0.168 | 0.311 | 1.85× | 0.462 | 2.75× | ok |
| Misc-C++/stepanov_v1p2 | 0.033 | 0.066 | 2.00× | 0.073 | 2.21× | ok |
| PAQ8p | 0.049 | 0.120 | 2.45× | 0.117 | 2.39× | ok |
| Prolangs-C++/life | 0.003 | 0.010 | — | 0.021 | — | ok |
| Prolangs-C++/ocean | 0.010 | 0.025 | 2.50× | 0.033 | 3.30× | ok |
| Prolangs-C++/primes | 0.006 | 0.017 | — | 0.027 | — | ok |
| Shootout-C++-EH/except | 0.084 | 0.181 | 2.15× | 0.186 | 2.21× | ok |
| Shootout-C++/ackermann | 0.095 | 0.157 | 1.65× | 0.211 | 2.22× | ok |
| Shootout-C++/ary3 | 0.090 | 0.178 | 1.98× | 0.243 | 2.70× | ok |
| Shootout-C++/fibo | 0.092 | 0.160 | 1.74× | 0.221 | 2.40× | ok |
| Shootout-C++/hash | 0.117 | 0.224 | 1.91× | 0.271 | 2.32× | ok |
| Shootout-C++/hash2 | 0.113 | 0.205 | 1.81× | 0.260 | 2.30× | ok |
| Shootout-C++/heapsort | 0.099 | 0.185 | 1.87× | 0.228 | 2.30× | ok |
| Shootout-C++/lists | 0.091 | 0.173 | 1.90× | 0.211 | 2.32× | ok |
| Shootout-C++/lists1 | 0.096 | 0.188 | 1.96× | 0.242 | 2.52× | ok |
| Shootout-C++/matrix | 0.084 | 0.152 | 1.81× | 0.185 | 2.20× | ok |
| Shootout-C++/methcall | 0.085 | 0.162 | 1.91× | 0.193 | 2.27× | ok |
| Shootout-C++/moments | 0.114 | 0.212 | 1.86× | 0.243 | 2.13× | ok |
| Shootout-C++/random | 0.101 | 0.186 | 1.84× | 0.242 | 2.40× | ok |
| Shootout-C++/sieve | 0.096 | 0.220 | 2.29× | 0.244 | 2.54× | ok |
| SmallPT | 0.030 | 0.060 | 2.00× | 0.078 | 2.60× | ok |
| Stanford/Bubblesort | 0.010 | 0.026 | 2.60× | 0.034 | 3.40× | ok |
| Stanford/FloatMM | 0.011 | 0.026 | 2.36× | 0.034 | 3.09× | ok |
| Stanford/Puzzle | 0.011 | 0.028 | 2.55× | 0.035 | 3.18× | ok |
| Stanford/Treesort | 0.011 | 0.026 | 2.36× | 0.035 | 3.18× | ok |
| bg/binarytrees/2 | 0.081 | 0.160 | 1.98× | 0.205 | 2.53× | ok |
| bg/fannkuchredux/3 | 0.053 | 0.065 | 1.23× | 0.078 | 1.47× | ok |
| bg/fasta/1 | 0.057 | 0.100 | 1.75× | 0.116 | 2.04× | ok |
| bg/fasta/2 | 0.055 | 0.101 | 1.84× | 0.121 | 2.20× | ok |
| bg/fasta/3 | 0.091 | 0.169 | 1.86× | 0.217 | 2.38× | ok |
| bg/fasta/4 | 0.083 | 0.144 | 1.73× | 0.167 | 2.01× | ok |
| bg/fasta/6 | 0.145 | 0.289 | 1.99× | 0.329 | 2.27× | ok |
| bg/mandelbrot/0 | 0.149 | 0.310 | 2.08× | 0.376 | 2.52× | ok |
| bg/mandelbrot/2 | 0.101 | 0.174 | 1.72× | 0.278 | 2.75× | ok |
| bg/mandelbrot/3 | 0.100 | 0.190 | 1.90× | 0.261 | 2.61× | ok |
| bg/mandelbrot/5 | 0.046 | 0.090 | 1.96× | 0.103 | 2.24× | ok |
| bg/mandelbrot/7 | 0.047 | 0.088 | 1.87× | 0.110 | 2.34× | ok |
| bg/mandelbrot/9 | 0.043 | 0.080 | 1.86× | 0.103 | 2.40× | ok |
| bg/nbody/1 | 0.032 | 0.068 | 2.12× | 0.074 | 2.31× | ok |
| bg/nbody/2 | 0.029 | 0.054 | 1.86× | 0.069 | 2.38× | ok |
| bg/nbody/3 | 0.028 | 0.054 | 1.93× | 0.071 | 2.54× | ok |
| bg/nbody/6 | 0.029 | 0.059 | 2.03× | 0.076 | 2.62× | ok |
| bg/pidigits/4 | 0.123 | 0.257 | 2.09× | 0.329 | 2.67× | ok |
| bg/pidigits/5 | 0.081 | 0.162 | 2.00× | 0.196 | 2.42× | ok |
| bg/revcomp/1 | 0.084 | 0.162 | 1.93× | 0.232 | 2.76× | ok |
| bg/revcomp/5 | 0.084 | 0.155 | 1.85× | 0.224 | 2.67× | ok |
| bg/spectralnorm/1 | 0.116 | 0.223 | 1.92× | 0.291 | 2.51× | ok |
| box2d | 1.604 | 2.887 | 1.80× | 4.167 | 2.60× | ok |
| fmt | 0.492 | 0.972 | 1.98× | 1.475 | 3.00× | ok |
| hexxagon | 0.300 | 0.578 | 1.93× | 0.809 | 2.70× | ok |
| lambda | 0.046 | 0.122 | 2.65× | 0.167 | 3.63× | ok |
| minisat | 0.053 | 0.140 | 2.64× | 0.140 | 2.64× | ok |
| nlohmann-json | 0.394 | 0.796 | 2.02× | 1.201 | 3.05× | ok |
| pugixml | 0.226 | 0.510 | 2.26× | 0.585 | 2.59× | ok |
| pugixml-tests | 3.362 | 7.202 | 2.14× | 8.374 | 2.49× | ok |
| rapidjson | 0.107 | 0.215 | 2.01× | 0.281 | 2.63× | ok |
| re2 | 3.192 | 6.204 | 1.94× | 5.249 | 1.64× | ok |
| re2-test-re2 | 4.520 | 9.187 | 2.03× | 13.074 | 2.89× | ok |
| re2-test-search | 4.880 | 8.612 | 1.76× | 10.965 | 2.25× | ok |
| tramp3d-v4 | 0.505 | 2.355 | 4.66× | 1.391 | 2.75× | ok |

### Building the compiler

| Compiler doing the rebuild | CPU seconds |
| --- | ---: |
| GCC | 315.270 |
| GCC-built cppgm++ | 70.660 |
| Self-built cppgm++ | 72.760 |

These are the initial bootstrap builds. The GCC rebuild is measured once.

#### Paired self-built compiler measurements

| Block | GCC-built cppgm++ (s) | Self-built cppgm++ (s) | Self-built speed ↑ |
| ---: | ---: | ---: | ---: |
| 1 | 69.010 | 70.692 | 0.98× |
| 2 | 70.079 | 71.883 | 0.97× |

Each block averages its two observations per compiler. The badge uses the median block ratio; calibration and every observation are retained in the raw results.

## Toolchains

- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)
- **valgrind**: valgrind-3.26.0
