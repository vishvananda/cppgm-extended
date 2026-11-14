# Public benchmarks: Intel (GitHub)

Measured compiler: [`01bac6195`](https://github.com/vishvananda/cppgm-extended/commit/01bac61953a982df28933dfde0fa03eb65838ea4) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/43559814509) · [Raw program results](results.json)

81 of 81 programs built with cppgm++ and g++ and agreed. Each number is the reference over cppgm++, so above 1× means cppgm++ is smaller or faster.

CPU: Intel(R) Xeon(R) 6975P-C.

| Measure | vs g++ | Programs | vs clang++ | Programs |
| --- | ---: | ---: | ---: | ---: |
| Frontend CPU, `-fsyntax-only` | 2.28× | 74 | 2.94× | 74 |
| Compile CPU, `-O0` | 3.16× | 76 | 3.16× | 76 |
| Compile CPU, `-O3` | 4.29× | 79 | 3.84× | 79 |
| Compile peak RSS, `-O0` | 1.46× | 81 | 2.22× | 81 |
| Compile peak RSS, `-O3` | 1.73× | 81 | 2.33× | 81 |
| Executable size, `-O0` | 0.93× | 81 | 0.87× | 81 |
| Executable size, `-O3` | 0.96× | 81 | 0.92× | 81 |
| Run CPU, `-O0` | 1.11× | 81 | 1.07× | 81 |
| Run CPU, `-O3` | 0.98× | 81 | 0.97× | 80 |
| Run instructions, `-O3` | 0.96× | 43 | - | 0 |

O3 execution uses paired ABBA/BAAB blocks. GCC A/A CPU ratios ranged from 0.946 to 1.011; raw observations and block ratios are retained in the artifact.

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
| Adobe-C++/functionobjects | 0.065 | 0.258 | 3.97× | 0.219 | 3.37× | ok |
| Adobe-C++/loop_unroll | 0.050 | 0.576 | 11.52× | 0.173 | 3.46× | ok |
| Adobe-C++/simple_types_constant_folding | 0.054 | 0.672 | 12.44× | 0.208 | 3.85× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.046 | 0.479 | 10.41× | 0.237 | 5.15× | ok |
| Adobe-C++/stepanov_abstraction | 0.038 | 0.180 | 4.74× | 0.112 | 2.95× | ok |
| Adobe-C++/stepanov_vector | 0.075 | 0.275 | 3.67× | 0.198 | 2.64× | ok |
| Bullet | 5.102 | 16.501 | 3.23× | 17.241 | 3.38× | ok |
| CLAMR | 2.544 | 8.599 | 3.38× | 9.109 | 3.58× | ok |
| CoyoteBench/almabench | 0.026 | 0.068 | 2.62× | 0.078 | 3.00× | ok |
| CoyoteBench/fftbench | 0.111 | 0.256 | 2.31× | 0.336 | 3.03× | ok |
| CoyoteBench/huffbench | 0.025 | 0.065 | 2.60× | 0.075 | 3.00× | ok |
| CoyoteBench/lpbench | 0.030 | 0.078 | 2.60× | 0.091 | 3.03× | ok |
| DOE/HACCKernels | 0.105 | 0.298 | 2.84× | 0.316 | 3.01× | ok |
| DOE/HPCCG | 0.663 | 1.581 | 2.38× | 2.279 | 3.44× | ok |
| DOE/PENNANT | 1.396 | 3.804 | 2.72× | 3.973 | 2.85× | ok |
| DOE/miniFE | 0.662 | 1.895 | 2.86× | 1.897 | 2.87× | ok |
| Misc-C++-EH/spirit | 0.477 | 2.873 | 6.02× | 1.804 | 3.78× | ok |
| Misc-C++-Large/ray | 0.099 | 0.258 | 2.61× | 0.285 | 2.88× | ok |
| Misc-C++-Large/sphereflake | 0.084 | 0.203 | 2.42× | 0.280 | 3.33× | ok |
| Misc-C++/bigfib | 0.110 | 0.347 | 3.15× | 0.339 | 3.08× | ok |
| Misc-C++/oopack_v1p8 | 0.031 | 0.094 | 3.03× | 0.097 | 3.13× | ok |
| Misc-C++/stepanov_container | 0.160 | 0.594 | 3.71× | 0.492 | 3.08× | ok |
| Misc-C++/stepanov_v1p2 | 0.028 | 0.118 | 4.21× | 0.090 | 3.21× | ok |
| PAQ8p | 0.063 | 0.446 | 7.08× | 0.211 | 3.35× | ok |
| Prolangs-C++/life | 0.004 | 0.032 | — | 0.028 | — | ok |
| Prolangs-C++/ocean | 0.010 | 0.067 | 6.70× | 0.046 | 4.60× | ok |
| Prolangs-C++/primes | 0.006 | 0.023 | — | 0.026 | — | ok |
| Shootout-C++-EH/except | 0.073 | 0.166 | 2.27× | 0.238 | 3.26× | ok |
| Shootout-C++/ackermann | 0.066 | 0.146 | 2.21× | 0.198 | 3.00× | ok |
| Shootout-C++/ary3 | 0.084 | 0.208 | 2.48× | 0.278 | 3.31× | ok |
| Shootout-C++/fibo | 0.079 | 0.145 | 1.84× | 0.196 | 2.48× | ok |
| Shootout-C++/hash | 0.097 | 0.268 | 2.76× | 0.296 | 3.05× | ok |
| Shootout-C++/hash2 | 0.117 | 0.295 | 2.52× | 0.296 | 2.53× | ok |
| Shootout-C++/heapsort | 0.091 | 0.175 | 1.92× | 0.239 | 2.63× | ok |
| Shootout-C++/lists | 0.073 | 0.194 | 2.66× | 0.228 | 3.12× | ok |
| Shootout-C++/lists1 | 0.097 | 0.214 | 2.21× | 0.289 | 2.98× | ok |
| Shootout-C++/matrix | 0.067 | 0.147 | 2.19× | 0.250 | 3.73× | ok |
| Shootout-C++/methcall | 0.064 | 0.149 | 2.33× | 0.215 | 3.36× | ok |
| Shootout-C++/moments | 0.095 | 0.249 | 2.62× | 0.263 | 2.77× | ok |
| Shootout-C++/random | 0.081 | 0.180 | 2.22× | 0.239 | 2.95× | ok |
| Shootout-C++/sieve | 0.086 | 0.228 | 2.65× | 0.281 | 3.27× | ok |
| SmallPT | 0.028 | 0.085 | 3.04× | 0.089 | 3.18× | ok |
| Stanford/Bubblesort | 0.009 | 0.026 | — | 0.035 | — | ok |
| Stanford/FloatMM | 0.009 | 0.031 | — | 0.034 | — | ok |
| Stanford/Puzzle | 0.010 | 0.037 | 3.70× | 0.035 | 3.50× | ok |
| Stanford/Treesort | 0.009 | 0.028 | — | 0.033 | — | ok |
| bg/binarytrees/2 | 0.067 | 0.153 | 2.28× | 0.226 | 3.37× | ok |
| bg/fannkuchredux/3 | 0.032 | 0.071 | 2.22× | 0.085 | 2.66× | ok |
| bg/fasta/1 | 0.050 | 0.119 | 2.38× | 0.129 | 2.58× | ok |
| bg/fasta/2 | 0.052 | 0.129 | 2.48× | 0.132 | 2.54× | ok |
| bg/fasta/3 | 0.077 | 0.174 | 2.26× | 0.236 | 3.06× | ok |
| bg/fasta/4 | 0.069 | 0.148 | 2.14× | 0.183 | 2.65× | ok |
| bg/fasta/6 | 0.132 | 0.344 | 2.61× | 0.420 | 3.18× | ok |
| bg/mandelbrot/0 | 0.152 | 0.420 | 2.76× | 0.423 | 2.78× | ok |
| bg/mandelbrot/2 | 0.089 | 0.219 | 2.46× | 0.264 | 2.97× | ok |
| bg/mandelbrot/3 | 0.091 | 0.206 | 2.26× | 0.273 | 3.00× | ok |
| bg/mandelbrot/5 | 0.046 | 0.115 | 2.50× | 0.132 | 2.87× | ok |
| bg/mandelbrot/7 | 0.047 | 0.131 | 2.79× | 0.121 | 2.57× | ok |
| bg/mandelbrot/9 | 0.042 | 0.120 | 2.86× | 0.133 | 3.17× | ok |
| bg/nbody/1 | 0.028 | 0.097 | 3.46× | 0.093 | 3.32× | ok |
| bg/nbody/2 | 0.025 | 0.067 | 2.68× | 0.076 | 3.04× | ok |
| bg/nbody/3 | 0.027 | 0.070 | 2.59× | 0.074 | 2.74× | ok |
| bg/nbody/6 | 0.027 | 0.099 | 3.67× | 0.091 | 3.37× | ok |
| bg/pidigits/4 | 0.117 | 0.291 | 2.49× | 0.384 | 3.28× | ok |
| bg/pidigits/5 | 0.086 | 0.164 | 1.91× | 0.230 | 2.67× | ok |
| bg/revcomp/1 | 0.082 | 0.215 | 2.62× | 0.289 | 3.52× | ok |
| bg/revcomp/5 | 0.076 | 0.202 | 2.66× | 0.213 | 2.80× | ok |
| bg/spectralnorm/1 | 0.119 | 0.245 | 2.06× | 0.362 | 3.04× | ok |
| box2d | 1.537 | 4.246 | 2.76× | 3.719 | 2.42× | ok |
| fmt | 0.531 | 2.498 | 4.70× | 2.075 | 3.91× | ok |
| hexxagon | 0.261 | 0.546 | 2.09× | 0.862 | 3.30× | ok |
| lambda | 0.051 | 0.310 | 6.08× | 0.239 | 4.69× | ok |
| minisat | 0.061 | 0.305 | 5.00× | 0.218 | 3.57× | ok |
| nlohmann-json | 0.421 | 1.942 | 4.61× | 1.810 | 4.30× | ok |
| pugixml | 0.260 | 1.239 | 4.77× | 0.986 | 3.79× | ok |
| pugixml-tests | 3.569 | 15.084 | 4.23× | 12.798 | 3.59× | ok |
| rapidjson | 0.121 | 0.481 | 3.98× | 0.397 | 3.28× | ok |
| re2 | 2.765 | 10.296 | 3.72× | 6.708 | 2.43× | ok |
| re2-test-re2 | 4.645 | 15.629 | 3.36× | 14.986 | 3.23× | ok |
| re2-test-search | 4.671 | 14.393 | 3.08× | 14.940 | 3.20× | ok |
| tramp3d-v4 | 0.731 | 8.700 | 11.90× | 2.660 | 3.64× | ok |

### O3 compilation

| Program | cppgm++ (s) | GCC (s) | Speed vs GCC ↑ | Clang (s) | Speed vs Clang ↑ | Status |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| Adobe-C++/functionobjects | 0.125 | 0.908 | 7.26× | 0.525 | 4.20× | ok |
| Adobe-C++/loop_unroll | 0.401 | 5.350 | 13.34× | 4.204 | 10.48× | ok |
| Adobe-C++/simple_types_constant_folding | 0.232 | 2.856 | 12.31× | 2.247 | 9.69× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.214 | 2.330 | 10.89× | 1.861 | 8.70× | ok |
| Adobe-C++/stepanov_abstraction | 0.087 | 0.972 | 11.17× | 0.448 | 5.15× | ok |
| Adobe-C++/stepanov_vector | 0.122 | 1.452 | 11.90× | 0.522 | 4.28× | ok |
| Bullet | 6.291 | 39.224 | 6.23× | 26.511 | 4.21× | ok |
| CLAMR | 4.721 | 22.822 | 4.83× | 19.372 | 4.10× | ok |
| CoyoteBench/almabench | 0.032 | 0.151 | 4.72× | 0.114 | 3.56× | ok |
| CoyoteBench/fftbench | 0.135 | 0.413 | 3.06× | 0.498 | 3.69× | ok |
| CoyoteBench/huffbench | 0.040 | 0.200 | 5.00× | 0.122 | 3.05× | ok |
| CoyoteBench/lpbench | 0.039 | 0.193 | 4.95× | 0.207 | 5.31× | ok |
| DOE/HACCKernels | 0.116 | 0.411 | 3.54× | 0.382 | 3.29× | ok |
| DOE/HPCCG | 0.748 | 2.234 | 2.99× | 2.688 | 3.59× | ok |
| DOE/PENNANT | 1.686 | 6.270 | 3.72× | 5.523 | 3.28× | ok |
| DOE/miniFE | 0.901 | 3.962 | 4.40× | 3.107 | 3.45× | ok |
| Misc-C++-EH/spirit | 0.727 | 3.964 | 5.45× | 3.558 | 4.89× | ok |
| Misc-C++-Large/ray | 0.108 | 0.500 | 4.63× | 0.334 | 3.09× | ok |
| Misc-C++-Large/sphereflake | 0.109 | 0.709 | 6.50× | 0.393 | 3.61× | ok |
| Misc-C++/bigfib | 0.136 | 0.511 | 3.76× | 0.548 | 4.03× | ok |
| Misc-C++/oopack_v1p8 | 0.039 | 0.190 | 4.87× | 0.175 | 4.49× | ok |
| Misc-C++/stepanov_container | 0.238 | 0.963 | 4.05× | 0.870 | 3.66× | ok |
| Misc-C++/stepanov_v1p2 | 0.040 | 0.206 | 5.15× | 0.195 | 4.88× | ok |
| PAQ8p | 0.313 | 3.835 | 12.25× | 1.659 | 5.30× | ok |
| Prolangs-C++/life | 0.008 | 0.104 | — | 0.071 | — | ok |
| Prolangs-C++/ocean | 0.020 | 0.215 | 10.75× | 0.117 | 5.85× | ok |
| Prolangs-C++/primes | 0.007 | 0.036 | — | 0.033 | — | ok |
| Shootout-C++-EH/except | 0.074 | 0.197 | 2.66× | 0.252 | 3.41× | ok |
| Shootout-C++/ackermann | 0.068 | 0.184 | 2.71× | 0.205 | 3.01× | ok |
| Shootout-C++/ary3 | 0.093 | 0.252 | 2.71× | 0.275 | 2.96× | ok |
| Shootout-C++/fibo | 0.066 | 0.179 | 2.71× | 0.204 | 3.09× | ok |
| Shootout-C++/hash | 0.113 | 0.362 | 3.20× | 0.395 | 3.50× | ok |
| Shootout-C++/hash2 | 0.140 | 0.432 | 3.09× | 0.428 | 3.06× | ok |
| Shootout-C++/heapsort | 0.090 | 0.213 | 2.37× | 0.253 | 2.81× | ok |
| Shootout-C++/lists | 0.081 | 0.219 | 2.70× | 0.258 | 3.19× | ok |
| Shootout-C++/lists1 | 0.106 | 0.256 | 2.42× | 0.304 | 2.87× | ok |
| Shootout-C++/matrix | 0.082 | 0.197 | 2.40× | 0.297 | 3.62× | ok |
| Shootout-C++/methcall | 0.069 | 0.194 | 2.81× | 0.229 | 3.32× | ok |
| Shootout-C++/moments | 0.106 | 0.318 | 3.00× | 0.321 | 3.03× | ok |
| Shootout-C++/random | 0.084 | 0.199 | 2.37× | 0.246 | 2.93× | ok |
| Shootout-C++/sieve | 0.098 | 0.247 | 2.52× | 0.287 | 2.93× | ok |
| SmallPT | 0.040 | 0.288 | 7.20× | 0.166 | 4.15× | ok |
| Stanford/Bubblesort | 0.012 | 0.052 | 4.33× | 0.052 | 4.33× | ok |
| Stanford/FloatMM | 0.012 | 0.062 | 5.17× | 0.077 | 6.42× | ok |
| Stanford/Puzzle | 0.018 | 0.129 | 7.17× | 0.113 | 6.28× | ok |
| Stanford/Treesort | 0.014 | 0.084 | 6.00× | 0.054 | 3.86× | ok |
| bg/binarytrees/2 | 0.071 | 0.412 | 5.80× | 0.223 | 3.14× | ok |
| bg/fannkuchredux/3 | 0.036 | 0.119 | 3.31× | 0.109 | 3.03× | ok |
| bg/fasta/1 | 0.056 | 0.157 | 2.80× | 0.164 | 2.93× | ok |
| bg/fasta/2 | 0.056 | 0.168 | 3.00× | 0.174 | 3.11× | ok |
| bg/fasta/3 | 0.088 | 0.214 | 2.43× | 0.244 | 2.77× | ok |
| bg/fasta/4 | 0.079 | 0.183 | 2.32× | 0.235 | 2.97× | ok |
| bg/fasta/6 | 0.169 | 0.437 | 2.59× | 0.509 | 3.01× | ok |
| bg/mandelbrot/0 | 0.173 | 0.646 | 3.73× | 0.681 | 3.94× | ok |
| bg/mandelbrot/2 | 0.099 | 0.263 | 2.66× | 0.317 | 3.20× | ok |
| bg/mandelbrot/3 | 0.092 | 0.223 | 2.42× | 0.294 | 3.20× | ok |
| bg/mandelbrot/5 | 0.049 | 0.147 | 3.00× | 0.147 | 3.00× | ok |
| bg/mandelbrot/7 | 0.058 | 0.184 | 3.17× | 0.165 | 2.84× | ok |
| bg/mandelbrot/9 | 0.052 | 0.188 | 3.62× | 0.160 | 3.08× | ok |
| bg/nbody/1 | 0.042 | 0.153 | 3.64× | 0.165 | 3.93× | ok |
| bg/nbody/2 | 0.030 | 0.126 | 4.20× | 0.126 | 4.20× | ok |
| bg/nbody/3 | 0.031 | 0.126 | 4.06× | 0.104 | 3.35× | ok |
| bg/nbody/6 | 0.039 | 0.160 | 4.10× | 0.191 | 4.90× | ok |
| bg/pidigits/4 | 0.121 | 0.331 | 2.74× | 0.458 | 3.79× | ok |
| bg/pidigits/5 | 0.081 | 0.197 | 2.43× | 0.287 | 3.54× | ok |
| bg/revcomp/1 | 0.088 | 0.265 | 3.01× | 0.323 | 3.67× | ok |
| bg/revcomp/5 | 0.086 | 0.215 | 2.50× | 0.244 | 2.84× | ok |
| bg/spectralnorm/1 | 0.127 | 0.340 | 2.68× | 0.425 | 3.35× | ok |
| box2d | 1.818 | 6.898 | 3.79× | 5.617 | 3.09× | ok |
| fmt | 1.006 | 5.545 | 5.51× | 3.760 | 3.74× | ok |
| hexxagon | 0.289 | 1.061 | 3.67× | 1.108 | 3.83× | ok |
| lambda | 0.120 | 1.144 | 9.53× | 0.578 | 4.82× | ok |
| minisat | 0.140 | 1.040 | 7.43× | 0.659 | 4.71× | ok |
| nlohmann-json | 0.767 | 3.546 | 4.62× | 3.131 | 4.08× | ok |
| pugixml | 0.683 | 5.655 | 8.28× | 3.847 | 5.63× | ok |
| pugixml-tests | 5.182 | 35.601 | 6.87× | 25.749 | 4.97× | ok |
| rapidjson | 0.219 | 1.175 | 5.37× | 0.760 | 3.47× | ok |
| re2 | 2.943 | 18.810 | 6.39× | 10.071 | 3.42× | ok |
| re2-test-re2 | 5.907 | 27.146 | 4.60× | 21.811 | 3.69× | ok |
| re2-test-search | 5.803 | 24.450 | 4.21× | 20.999 | 3.62× | ok |
| tramp3d-v4 | 2.404 | 17.944 | 7.46× | 14.376 | 5.98× | ok |

### Frontend only: -fsyntax-only

| Program | cppgm++ (s) | GCC (s) | Speed vs GCC ↑ | Clang (s) | Speed vs Clang ↑ | Status |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| Adobe-C++/functionobjects | 0.059 | 0.136 | 2.31× | 0.184 | 3.12× | ok |
| Adobe-C++/loop_unroll | 0.034 | 0.099 | 2.91× | 0.101 | 2.97× | ok |
| Adobe-C++/simple_types_constant_folding | 0.038 | 0.103 | 2.71× | 0.107 | 2.82× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.035 | 0.095 | 2.71× | 0.120 | 3.43× | ok |
| Adobe-C++/stepanov_abstraction | 0.032 | 0.068 | 2.12× | 0.087 | 2.72× | ok |
| Adobe-C++/stepanov_vector | 0.058 | 0.122 | 2.10× | 0.159 | 2.74× | ok |
| Bullet | 4.669 | 10.141 | 2.17× | 14.160 | 3.03× | ok |
| CLAMR | 2.189 | 4.510 | 2.06× | 7.251 | 3.31× | ok |
| CoyoteBench/almabench | 0.026 | 0.055 | 2.12× | 0.072 | 2.77× | ok |
| CoyoteBench/fftbench | 0.102 | 0.226 | 2.22× | 0.322 | 3.16× | ok |
| CoyoteBench/huffbench | 0.024 | 0.055 | 2.29× | 0.071 | 2.96× | ok |
| CoyoteBench/lpbench | 0.029 | 0.064 | 2.21× | 0.082 | 2.83× | ok |
| DOE/HACCKernels | 0.102 | 0.263 | 2.58× | 0.296 | 2.90× | ok |
| DOE/HPCCG | 0.691 | 1.408 | 2.04× | 2.128 | 3.08× | ok |
| DOE/PENNANT | 1.382 | 2.865 | 2.07× | 3.801 | 2.75× | ok |
| DOE/miniFE | 0.595 | 1.203 | 2.02× | 1.619 | 2.72× | ok |
| Misc-C++-EH/spirit | 0.332 | 0.978 | 2.95× | 1.136 | 3.42× | ok |
| Misc-C++-Large/ray | 0.095 | 0.214 | 2.25× | 0.261 | 2.75× | ok |
| Misc-C++-Large/sphereflake | 0.081 | 0.199 | 2.46× | 0.261 | 3.22× | ok |
| Misc-C++/bigfib | 0.098 | 0.230 | 2.35× | 0.294 | 3.00× | ok |
| Misc-C++/oopack_v1p8 | 0.029 | 0.066 | 2.28× | 0.085 | 2.93× | ok |
| Misc-C++/stepanov_container | 0.142 | 0.347 | 2.44× | 0.413 | 2.91× | ok |
| Misc-C++/stepanov_v1p2 | 0.026 | 0.063 | 2.42× | 0.076 | 2.92× | ok |
| PAQ8p | 0.042 | 0.116 | 2.76× | 0.121 | 2.88× | ok |
| Prolangs-C++/life | 0.003 | 0.010 | — | 0.021 | — | ok |
| Prolangs-C++/ocean | 0.008 | 0.022 | — | 0.031 | — | ok |
| Prolangs-C++/primes | 0.006 | 0.015 | — | 0.022 | — | ok |
| Shootout-C++-EH/except | 0.071 | 0.156 | 2.20× | 0.224 | 3.15× | ok |
| Shootout-C++/ackermann | 0.065 | 0.148 | 2.28× | 0.189 | 2.91× | ok |
| Shootout-C++/ary3 | 0.081 | 0.190 | 2.35× | 0.256 | 3.16× | ok |
| Shootout-C++/fibo | 0.078 | 0.147 | 1.88× | 0.187 | 2.40× | ok |
| Shootout-C++/hash | 0.090 | 0.202 | 2.24× | 0.264 | 2.93× | ok |
| Shootout-C++/hash2 | 0.107 | 0.219 | 2.05× | 0.261 | 2.44× | ok |
| Shootout-C++/heapsort | 0.089 | 0.175 | 1.97× | 0.230 | 2.58× | ok |
| Shootout-C++/lists | 0.069 | 0.161 | 2.33× | 0.208 | 3.01× | ok |
| Shootout-C++/lists1 | 0.091 | 0.176 | 1.93× | 0.283 | 3.11× | ok |
| Shootout-C++/matrix | 0.066 | 0.145 | 2.20× | 0.237 | 3.59× | ok |
| Shootout-C++/methcall | 0.063 | 0.146 | 2.32× | 0.205 | 3.25× | ok |
| Shootout-C++/moments | 0.090 | 0.196 | 2.18× | 0.239 | 2.66× | ok |
| Shootout-C++/random | 0.081 | 0.180 | 2.22× | 0.229 | 2.83× | ok |
| Shootout-C++/sieve | 0.082 | 0.184 | 2.24× | 0.269 | 3.28× | ok |
| SmallPT | 0.026 | 0.055 | 2.12× | 0.079 | 3.04× | ok |
| Stanford/Bubblesort | 0.009 | 0.022 | — | 0.033 | — | ok |
| Stanford/FloatMM | 0.009 | 0.024 | — | 0.031 | — | ok |
| Stanford/Puzzle | 0.009 | 0.026 | — | 0.032 | — | ok |
| Stanford/Treesort | 0.009 | 0.022 | — | 0.031 | — | ok |
| bg/binarytrees/2 | 0.067 | 0.151 | 2.25× | 0.208 | 3.10× | ok |
| bg/fannkuchredux/3 | 0.031 | 0.060 | 1.94× | 0.078 | 2.52× | ok |
| bg/fasta/1 | 0.047 | 0.094 | 2.00× | 0.117 | 2.49× | ok |
| bg/fasta/2 | 0.048 | 0.101 | 2.10× | 0.119 | 2.48× | ok |
| bg/fasta/3 | 0.075 | 0.164 | 2.19× | 0.207 | 2.76× | ok |
| bg/fasta/4 | 0.068 | 0.143 | 2.10× | 0.170 | 2.50× | ok |
| bg/fasta/6 | 0.123 | 0.246 | 2.00× | 0.368 | 2.99× | ok |
| bg/mandelbrot/0 | 0.143 | 0.286 | 2.00× | 0.374 | 2.62× | ok |
| bg/mandelbrot/2 | 0.083 | 0.194 | 2.34× | 0.256 | 3.08× | ok |
| bg/mandelbrot/3 | 0.088 | 0.201 | 2.28× | 0.268 | 3.05× | ok |
| bg/mandelbrot/5 | 0.042 | 0.090 | 2.14× | 0.107 | 2.55× | ok |
| bg/mandelbrot/7 | 0.043 | 0.096 | 2.23× | 0.117 | 2.72× | ok |
| bg/mandelbrot/9 | 0.039 | 0.086 | 2.21× | 0.118 | 3.03× | ok |
| bg/nbody/1 | 0.026 | 0.057 | 2.19× | 0.078 | 3.00× | ok |
| bg/nbody/2 | 0.025 | 0.055 | 2.20× | 0.072 | 2.88× | ok |
| bg/nbody/3 | 0.028 | 0.052 | 1.86× | 0.069 | 2.46× | ok |
| bg/nbody/6 | 0.026 | 0.057 | 2.19× | 0.076 | 2.92× | ok |
| bg/pidigits/4 | 0.114 | 0.261 | 2.29× | 0.314 | 2.75× | ok |
| bg/pidigits/5 | 0.080 | 0.158 | 1.97× | 0.205 | 2.56× | ok |
| bg/revcomp/1 | 0.077 | 0.172 | 2.23× | 0.234 | 3.04× | ok |
| bg/revcomp/5 | 0.074 | 0.180 | 2.43× | 0.191 | 2.58× | ok |
| bg/spectralnorm/1 | 0.103 | 0.223 | 2.17× | 0.344 | 3.34× | ok |
| box2d | 1.333 | 3.017 | 2.26× | 4.064 | 3.05× | ok |
| fmt | 0.417 | 1.086 | 2.60× | 1.612 | 3.87× | ok |
| hexxagon | 0.255 | 0.489 | 1.92× | 0.752 | 2.95× | ok |
| lambda | 0.040 | 0.125 | 3.12× | 0.170 | 4.25× | ok |
| minisat | 0.050 | 0.134 | 2.68× | 0.157 | 3.14× | ok |
| nlohmann-json | 0.335 | 0.865 | 2.58× | 1.399 | 4.18× | ok |
| pugixml | 0.210 | 0.483 | 2.30× | 0.729 | 3.47× | ok |
| pugixml-tests | 3.009 | 7.614 | 2.53× | 9.530 | 3.17× | ok |
| rapidjson | 0.100 | 0.253 | 2.53× | 0.294 | 2.94× | ok |
| re2 | 2.843 | 6.213 | 2.19× | 5.685 | 2.00× | ok |
| re2-test-re2 | 4.152 | 10.345 | 2.49× | 12.934 | 3.12× | ok |
| re2-test-search | 4.229 | 9.700 | 2.29× | 13.022 | 3.08× | ok |
| tramp3d-v4 | 0.540 | 2.306 | 4.27× | 1.552 | 2.87× | ok |


## Toolchains

- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)
- **valgrind**: valgrind-3.26.0

[Compiler build and codegen results](self-build.md) have their own measured revision.
