# Public benchmarks: Intel (GitHub)

Measured compiler: [`e2831a513`](https://github.com/vishvananda/cppgm-extended/commit/e2831a513b668c602faa8098e55eb22ee08a670f) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/43133898208) · [Raw program results](results.json)

81 of 81 programs built with cppgm++ and g++ and agreed. Each number is the reference over cppgm++, so above 1× means cppgm++ is smaller or faster.

CPU: INTEL(R) XEON(R) PLATINUM 8573C.

| Measure | vs g++ | Programs | vs clang++ | Programs |
| --- | ---: | ---: | ---: | ---: |
| Frontend CPU, `-fsyntax-only` | 2.29× | 79 | 2.87× | 79 |
| Compile CPU, `-O0` | 3.12× | 79 | 3.10× | 79 |
| Compile CPU, `-O3` | 4.45× | 80 | 3.84× | 80 |
| Compile peak RSS, `-O0` | 1.42× | 81 | 2.11× | 81 |
| Compile peak RSS, `-O3` | 1.68× | 81 | 2.20× | 81 |
| Executable size, `-O0` | 0.93× | 81 | 0.87× | 81 |
| Executable size, `-O3` | 0.96× | 81 | 0.92× | 81 |
| Run CPU, `-O0` | 1.14× | 81 | 1.06× | 81 |
| Run CPU, `-O3` | 0.98× | 81 | 0.97× | 81 |
| Run instructions, `-O3` | 0.95× | 43 | - | 0 |

O3 execution uses paired ABBA/BAAB blocks. GCC A/A CPU ratios ranged from 0.985 to 1.730; raw observations and block ratios are retained in the artifact.

## Slowest generated code, by run-time instructions

| Program | vs g++ |
| --- | ---: |
| tramp3d-v4 | 0.46× |
| bg/nbody/2 | 0.64× |
| PAQ8p | 0.67× |
| bg/mandelbrot/0 | 0.68× |
| bg/spectralnorm/1 | 0.75× |
| bg/nbody/1 | 0.83× |
| bg/mandelbrot/7 | 0.83× |
| bg/mandelbrot/9 | 0.84× |
| bg/mandelbrot/5 | 0.85× |
| bg/mandelbrot/2 | 0.86× |

## Compile CPU times

Times are CPU seconds, excluding linking. Speed is reference time divided by cppgm++ time; higher is better. Every program is listed. Missing times show a dash. Ratios are omitted for failed programs or when either time is below 10 ms.

Compilation is measured once per compiler and level; the ABBA/BAAB repetitions apply to generated-program execution and self-built compiler speed.

### O0 compilation

| Program | cppgm++ (s) | GCC (s) | Speed vs GCC ↑ | Clang (s) | Speed vs Clang ↑ | Status |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| Adobe-C++/functionobjects | 0.095 | 0.367 | 3.86× | 0.298 | 3.14× | ok |
| Adobe-C++/loop_unroll | 0.073 | 0.757 | 10.37× | 0.297 | 4.07× | ok |
| Adobe-C++/simple_types_constant_folding | 0.052 | 0.859 | 16.52× | 0.306 | 5.88× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.068 | 0.681 | 10.01× | 0.317 | 4.66× | ok |
| Adobe-C++/stepanov_abstraction | 0.059 | 0.255 | 4.32× | 0.177 | 3.00× | ok |
| Adobe-C++/stepanov_vector | 0.102 | 0.396 | 3.88× | 0.335 | 3.28× | ok |
| Bullet | 6.172 | 24.166 | 3.92× | 21.631 | 3.50× | ok |
| CLAMR | 3.360 | 12.101 | 3.60× | 12.421 | 3.70× | ok |
| CoyoteBench/almabench | 0.040 | 0.097 | 2.43× | 0.125 | 3.12× | ok |
| CoyoteBench/fftbench | 0.149 | 0.405 | 2.72× | 0.425 | 2.85× | ok |
| CoyoteBench/huffbench | 0.045 | 0.091 | 2.02× | 0.115 | 2.56× | ok |
| CoyoteBench/lpbench | 0.041 | 0.109 | 2.66× | 0.121 | 2.95× | ok |
| DOE/HACCKernels | 0.159 | 0.392 | 2.47× | 0.532 | 3.35× | ok |
| DOE/HPCCG | 0.900 | 2.231 | 2.48× | 3.222 | 3.58× | ok |
| DOE/PENNANT | 2.090 | 5.413 | 2.59× | 5.500 | 2.63× | ok |
| DOE/miniFE | 0.911 | 3.135 | 3.44× | 2.870 | 3.15× | ok |
| Misc-C++-EH/spirit | 0.600 | 3.340 | 5.57× | 2.493 | 4.15× | ok |
| Misc-C++-Large/ray | 0.143 | 0.389 | 2.72× | 0.457 | 3.20× | ok |
| Misc-C++-Large/sphereflake | 0.137 | 0.323 | 2.36× | 0.397 | 2.90× | ok |
| Misc-C++/bigfib | 0.156 | 0.527 | 3.38× | 0.473 | 3.03× | ok |
| Misc-C++/oopack_v1p8 | 0.056 | 0.115 | 2.05× | 0.129 | 2.30× | ok |
| Misc-C++/stepanov_container | 0.213 | 0.708 | 3.32× | 0.633 | 2.97× | ok |
| Misc-C++/stepanov_v1p2 | 0.042 | 0.155 | 3.69× | 0.121 | 2.88× | ok |
| PAQ8p | 0.089 | 0.598 | 6.72× | 0.318 | 3.57× | ok |
| Prolangs-C++/life | 0.005 | 0.044 | — | 0.036 | — | ok |
| Prolangs-C++/ocean | 0.014 | 0.088 | 6.29× | 0.062 | 4.43× | ok |
| Prolangs-C++/primes | 0.007 | 0.032 | — | 0.035 | — | ok |
| Shootout-C++-EH/except | 0.104 | 0.234 | 2.25× | 0.332 | 3.19× | ok |
| Shootout-C++/ackermann | 0.096 | 0.211 | 2.20× | 0.253 | 2.64× | ok |
| Shootout-C++/ary3 | 0.120 | 0.289 | 2.41× | 0.324 | 2.70× | ok |
| Shootout-C++/fibo | 0.100 | 0.197 | 1.97× | 0.325 | 3.25× | ok |
| Shootout-C++/hash | 0.163 | 0.405 | 2.48× | 0.476 | 2.92× | ok |
| Shootout-C++/hash2 | 0.164 | 0.435 | 2.65× | 0.464 | 2.83× | ok |
| Shootout-C++/heapsort | 0.125 | 0.257 | 2.06× | 0.337 | 2.70× | ok |
| Shootout-C++/lists | 0.117 | 0.264 | 2.26× | 0.304 | 2.60× | ok |
| Shootout-C++/lists1 | 0.127 | 0.310 | 2.44× | 0.354 | 2.79× | ok |
| Shootout-C++/matrix | 0.101 | 0.217 | 2.15× | 0.277 | 2.74× | ok |
| Shootout-C++/methcall | 0.098 | 0.221 | 2.26× | 0.273 | 2.79× | ok |
| Shootout-C++/moments | 0.135 | 0.353 | 2.61× | 0.432 | 3.20× | ok |
| Shootout-C++/random | 0.121 | 0.250 | 2.07× | 0.353 | 2.92× | ok |
| Shootout-C++/sieve | 0.126 | 0.290 | 2.30× | 0.368 | 2.92× | ok |
| SmallPT | 0.040 | 0.126 | 3.15× | 0.111 | 2.77× | ok |
| Stanford/Bubblesort | 0.012 | 0.037 | 3.08× | 0.041 | 3.42× | ok |
| Stanford/FloatMM | 0.012 | 0.038 | 3.17× | 0.042 | 3.50× | ok |
| Stanford/Puzzle | 0.013 | 0.048 | 3.69× | 0.045 | 3.46× | ok |
| Stanford/Treesort | 0.012 | 0.040 | 3.33× | 0.042 | 3.50× | ok |
| bg/binarytrees/2 | 0.098 | 0.213 | 2.17× | 0.290 | 2.96× | ok |
| bg/fannkuchredux/3 | 0.048 | 0.100 | 2.08× | 0.112 | 2.33× | ok |
| bg/fasta/1 | 0.078 | 0.169 | 2.17× | 0.180 | 2.31× | ok |
| bg/fasta/2 | 0.074 | 0.171 | 2.31× | 0.189 | 2.55× | ok |
| bg/fasta/3 | 0.120 | 0.270 | 2.25× | 0.328 | 2.73× | ok |
| bg/fasta/4 | 0.108 | 0.215 | 1.99× | 0.250 | 2.31× | ok |
| bg/fasta/6 | 0.196 | 0.585 | 2.98× | 0.585 | 2.98× | ok |
| bg/mandelbrot/0 | 0.221 | 0.622 | 2.81× | 0.624 | 2.82× | ok |
| bg/mandelbrot/2 | 0.122 | 0.304 | 2.49× | 0.337 | 2.76× | ok |
| bg/mandelbrot/3 | 0.134 | 0.294 | 2.19× | 0.369 | 2.75× | ok |
| bg/mandelbrot/5 | 0.060 | 0.153 | 2.55× | 0.164 | 2.73× | ok |
| bg/mandelbrot/7 | 0.065 | 0.168 | 2.58× | 0.127 | 1.95× | ok |
| bg/mandelbrot/9 | 0.061 | 0.155 | 2.54× | 0.193 | 3.16× | ok |
| bg/nbody/1 | 0.027 | 0.157 | 5.81× | 0.124 | 4.59× | ok |
| bg/nbody/2 | 0.038 | 0.095 | 2.50× | 0.108 | 2.84× | ok |
| bg/nbody/3 | 0.038 | 0.065 | 1.71× | 0.105 | 2.76× | ok |
| bg/nbody/6 | 0.039 | 0.146 | 3.74× | 0.121 | 3.10× | ok |
| bg/pidigits/4 | 0.176 | 0.389 | 2.21× | 0.585 | 3.32× | ok |
| bg/pidigits/5 | 0.105 | 0.166 | 1.58× | 0.330 | 3.14× | ok |
| bg/revcomp/1 | 0.119 | 0.307 | 2.58× | 0.341 | 2.87× | ok |
| bg/revcomp/5 | 0.111 | 0.303 | 2.73× | 0.362 | 3.26× | ok |
| bg/spectralnorm/1 | 0.166 | 0.349 | 2.10× | 0.490 | 2.95× | ok |
| box2d | 2.052 | 6.020 | 2.93× | 4.414 | 2.15× | ok |
| fmt | 0.714 | 3.395 | 4.75× | 2.777 | 3.89× | ok |
| hexxagon | 0.359 | 0.928 | 2.58× | 1.143 | 3.18× | ok |
| lambda | 0.072 | 0.409 | 5.68× | 0.310 | 4.31× | ok |
| minisat | 0.081 | 0.419 | 5.17× | 0.297 | 3.67× | ok |
| nlohmann-json | 0.639 | 2.625 | 4.11× | 2.377 | 3.72× | ok |
| pugixml | 0.348 | 1.818 | 5.22× | 1.350 | 3.88× | ok |
| pugixml-tests | 4.757 | 20.719 | 4.36× | 16.362 | 3.44× | ok |
| rapidjson | 0.168 | 0.642 | 3.82× | 0.497 | 2.96× | ok |
| re2 | 2.999 | 14.595 | 4.87× | 9.077 | 3.03× | ok |
| re2-test-re2 | 6.799 | 21.696 | 3.19× | 22.009 | 3.24× | ok |
| re2-test-search | 6.880 | 19.666 | 2.86× | 20.150 | 2.93× | ok |
| tramp3d-v4 | 0.987 | 11.976 | 12.13× | 3.495 | 3.54× | ok |

### O3 compilation

| Program | cppgm++ (s) | GCC (s) | Speed vs GCC ↑ | Clang (s) | Speed vs Clang ↑ | Status |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| Adobe-C++/functionobjects | 0.153 | 1.198 | 7.83× | 0.642 | 4.20× | ok |
| Adobe-C++/loop_unroll | 0.476 | 7.506 | 15.77× | 6.287 | 13.21× | ok |
| Adobe-C++/simple_types_constant_folding | 0.266 | 3.934 | 14.79× | 3.544 | 13.32× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.274 | 3.208 | 11.71× | 2.515 | 9.18× | ok |
| Adobe-C++/stepanov_abstraction | 0.115 | 1.385 | 12.04× | 0.627 | 5.45× | ok |
| Adobe-C++/stepanov_vector | 0.138 | 2.023 | 14.66× | 0.720 | 5.22× | ok |
| Bullet | 9.098 | 54.149 | 5.95× | 39.424 | 4.33× | ok |
| CLAMR | 6.511 | 32.416 | 4.98× | 24.946 | 3.83× | ok |
| CoyoteBench/almabench | 0.048 | 0.218 | 4.54× | 0.181 | 3.77× | ok |
| CoyoteBench/fftbench | 0.174 | 0.653 | 3.75× | 0.622 | 3.57× | ok |
| CoyoteBench/huffbench | 0.052 | 0.288 | 5.54× | 0.189 | 3.63× | ok |
| CoyoteBench/lpbench | 0.052 | 0.262 | 5.04× | 0.290 | 5.58× | ok |
| DOE/HACCKernels | 0.186 | 0.574 | 3.09× | 0.538 | 2.89× | ok |
| DOE/HPCCG | 0.970 | 3.176 | 3.27× | 3.735 | 3.85× | ok |
| DOE/PENNANT | 2.464 | 8.326 | 3.38× | 7.730 | 3.14× | ok |
| DOE/miniFE | 1.225 | 6.204 | 5.06× | 4.823 | 3.94× | ok |
| Misc-C++-EH/spirit | 1.011 | 4.756 | 4.70× | 4.190 | 4.14× | ok |
| Misc-C++-Large/ray | 0.162 | 0.707 | 4.36× | 0.552 | 3.41× | ok |
| Misc-C++-Large/sphereflake | 0.152 | 1.066 | 7.01× | 0.487 | 3.20× | ok |
| Misc-C++/bigfib | 0.195 | 0.764 | 3.92× | 0.623 | 3.19× | ok |
| Misc-C++/oopack_v1p8 | 0.051 | 0.231 | 4.53× | 0.211 | 4.14× | ok |
| Misc-C++/stepanov_container | 0.294 | 1.284 | 4.37× | 1.128 | 3.84× | ok |
| Misc-C++/stepanov_v1p2 | 0.050 | 0.278 | 5.56× | 0.241 | 4.82× | ok |
| PAQ8p | 0.406 | 5.396 | 13.29× | 2.375 | 5.85× | ok |
| Prolangs-C++/life | 0.010 | 0.142 | 14.20× | 0.088 | 8.80× | ok |
| Prolangs-C++/ocean | 0.028 | 0.287 | 10.25× | 0.163 | 5.82× | ok |
| Prolangs-C++/primes | 0.009 | 0.047 | — | 0.045 | — | ok |
| Shootout-C++-EH/except | 0.106 | 0.275 | 2.59× | 0.345 | 3.25× | ok |
| Shootout-C++/ackermann | 0.098 | 0.267 | 2.72× | 0.294 | 3.00× | ok |
| Shootout-C++/ary3 | 0.119 | 0.346 | 2.91× | 0.402 | 3.38× | ok |
| Shootout-C++/fibo | 0.104 | 0.280 | 2.69× | 0.335 | 3.22× | ok |
| Shootout-C++/hash | 0.189 | 0.541 | 2.86× | 0.550 | 2.91× | ok |
| Shootout-C++/hash2 | 0.187 | 0.612 | 3.27× | 0.578 | 3.09× | ok |
| Shootout-C++/heapsort | 0.135 | 0.345 | 2.56× | 0.346 | 2.56× | ok |
| Shootout-C++/lists | 0.117 | 0.323 | 2.76× | 0.425 | 3.63× | ok |
| Shootout-C++/lists1 | 0.153 | 0.358 | 2.34× | 0.410 | 2.68× | ok |
| Shootout-C++/matrix | 0.102 | 0.305 | 2.99× | 0.407 | 3.99× | ok |
| Shootout-C++/methcall | 0.100 | 0.296 | 2.96× | 0.303 | 3.03× | ok |
| Shootout-C++/moments | 0.150 | 0.438 | 2.92× | 0.407 | 2.71× | ok |
| Shootout-C++/random | 0.129 | 0.328 | 2.54× | 0.355 | 2.75× | ok |
| Shootout-C++/sieve | 0.136 | 0.342 | 2.51× | 0.403 | 2.96× | ok |
| SmallPT | 0.057 | 0.416 | 7.30× | 0.230 | 4.04× | ok |
| Stanford/Bubblesort | 0.015 | 0.073 | 4.87× | 0.063 | 4.20× | ok |
| Stanford/FloatMM | 0.016 | 0.083 | 5.19× | 0.089 | 5.56× | ok |
| Stanford/Puzzle | 0.022 | 0.170 | 7.73× | 0.154 | 7.00× | ok |
| Stanford/Treesort | 0.018 | 0.113 | 6.28× | 0.068 | 3.78× | ok |
| bg/binarytrees/2 | 0.103 | 0.560 | 5.44× | 0.313 | 3.04× | ok |
| bg/fannkuchredux/3 | 0.051 | 0.164 | 3.22× | 0.149 | 2.92× | ok |
| bg/fasta/1 | 0.081 | 0.222 | 2.74× | 0.229 | 2.83× | ok |
| bg/fasta/2 | 0.081 | 0.217 | 2.68× | 0.239 | 2.95× | ok |
| bg/fasta/3 | 0.128 | 0.334 | 2.61× | 0.453 | 3.54× | ok |
| bg/fasta/4 | 0.111 | 0.263 | 2.37× | 0.321 | 2.89× | ok |
| bg/fasta/6 | 0.219 | 0.639 | 2.92× | 0.730 | 3.33× | ok |
| bg/mandelbrot/0 | 0.247 | 0.897 | 3.63× | 0.878 | 3.55× | ok |
| bg/mandelbrot/2 | 0.131 | 0.384 | 2.93× | 0.418 | 3.19× | ok |
| bg/mandelbrot/3 | 0.149 | 0.336 | 2.26× | 0.285 | 1.91× | ok |
| bg/mandelbrot/5 | 0.072 | 0.207 | 2.88× | 0.148 | 2.06× | ok |
| bg/mandelbrot/7 | 0.056 | 0.278 | 4.96× | 0.170 | 3.04× | ok |
| bg/mandelbrot/9 | 0.074 | 0.216 | 2.92× | 0.214 | 2.89× | ok |
| bg/nbody/1 | 0.038 | 0.147 | 3.87× | 0.238 | 6.26× | ok |
| bg/nbody/2 | 0.043 | 0.179 | 4.16× | 0.166 | 3.86× | ok |
| bg/nbody/3 | 0.043 | 0.176 | 4.09× | 0.144 | 3.35× | ok |
| bg/nbody/6 | 0.060 | 0.252 | 4.20× | 0.227 | 3.78× | ok |
| bg/pidigits/4 | 0.180 | 0.503 | 2.79× | 0.653 | 3.63× | ok |
| bg/pidigits/5 | 0.108 | 0.198 | 1.83× | 0.328 | 3.04× | ok |
| bg/revcomp/1 | 0.127 | 0.349 | 2.75× | 0.395 | 3.11× | ok |
| bg/revcomp/5 | 0.122 | 0.347 | 2.84× | 0.413 | 3.39× | ok |
| bg/spectralnorm/1 | 0.186 | 0.481 | 2.59× | 0.571 | 3.07× | ok |
| box2d | 2.372 | 10.175 | 4.29× | 6.972 | 2.94× | ok |
| fmt | 1.362 | 7.599 | 5.58× | 5.723 | 4.20× | ok |
| hexxagon | 0.402 | 1.418 | 3.53× | 1.502 | 3.74× | ok |
| lambda | 0.160 | 1.588 | 9.93× | 0.776 | 4.85× | ok |
| minisat | 0.193 | 1.412 | 7.32× | 0.851 | 4.41× | ok |
| nlohmann-json | 1.067 | 4.844 | 4.54× | 3.825 | 3.58× | ok |
| pugixml | 0.967 | 7.408 | 7.66× | 5.358 | 5.54× | ok |
| pugixml-tests | 7.091 | 48.592 | 6.85× | 34.940 | 4.93× | ok |
| rapidjson | 0.301 | 1.625 | 5.40× | 1.032 | 3.43× | ok |
| re2 | 3.918 | 25.055 | 6.39× | 13.527 | 3.45× | ok |
| re2-test-re2 | 8.461 | 37.245 | 4.40× | 31.545 | 3.73× | ok |
| re2-test-search | 8.526 | 33.603 | 3.94× | 29.581 | 3.47× | ok |
| tramp3d-v4 | 3.119 | 27.040 | 8.67× | 18.971 | 6.08× | ok |

### Frontend only: -fsyntax-only

| Program | cppgm++ (s) | GCC (s) | Speed vs GCC ↑ | Clang (s) | Speed vs Clang ↑ | Status |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| Adobe-C++/functionobjects | 0.087 | 0.192 | 2.21× | 0.253 | 2.91× | ok |
| Adobe-C++/loop_unroll | 0.051 | 0.141 | 2.76× | 0.131 | 2.57× | ok |
| Adobe-C++/simple_types_constant_folding | 0.052 | 0.150 | 2.88× | 0.194 | 3.73× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.050 | 0.134 | 2.68× | 0.164 | 3.28× | ok |
| Adobe-C++/stepanov_abstraction | 0.049 | 0.108 | 2.20× | 0.141 | 2.88× | ok |
| Adobe-C++/stepanov_vector | 0.095 | 0.176 | 1.85× | 0.270 | 2.84× | ok |
| Bullet | 5.596 | 15.678 | 2.80× | 19.483 | 3.48× | ok |
| CLAMR | 2.949 | 7.586 | 2.57× | 10.101 | 3.43× | ok |
| CoyoteBench/almabench | 0.039 | 0.076 | 1.95× | 0.118 | 3.03× | ok |
| CoyoteBench/fftbench | 0.149 | 0.364 | 2.44× | 0.395 | 2.65× | ok |
| CoyoteBench/huffbench | 0.044 | 0.080 | 1.82× | 0.109 | 2.48× | ok |
| CoyoteBench/lpbench | 0.040 | 0.088 | 2.20× | 0.112 | 2.80× | ok |
| DOE/HACCKernels | 0.154 | 0.373 | 2.42× | 0.500 | 3.25× | ok |
| DOE/HPCCG | 0.868 | 1.902 | 2.19× | 2.750 | 3.17× | ok |
| DOE/PENNANT | 1.866 | 3.989 | 2.14× | 4.907 | 2.63× | ok |
| DOE/miniFE | 0.835 | 2.060 | 2.47× | 2.438 | 2.92× | ok |
| Misc-C++-EH/spirit | 0.410 | 1.175 | 2.87× | 1.740 | 4.24× | ok |
| Misc-C++-Large/ray | 0.135 | 0.319 | 2.36× | 0.421 | 3.12× | ok |
| Misc-C++-Large/sphereflake | 0.133 | 0.289 | 2.17× | 0.375 | 2.82× | ok |
| Misc-C++/bigfib | 0.144 | 0.375 | 2.60× | 0.403 | 2.80× | ok |
| Misc-C++/oopack_v1p8 | 0.042 | 0.080 | 1.90× | 0.110 | 2.62× | ok |
| Misc-C++/stepanov_container | 0.207 | 0.399 | 1.93× | 0.534 | 2.58× | ok |
| Misc-C++/stepanov_v1p2 | 0.039 | 0.083 | 2.13× | 0.100 | 2.56× | ok |
| PAQ8p | 0.061 | 0.157 | 2.57× | 0.181 | 2.97× | ok |
| Prolangs-C++/life | 0.004 | 0.013 | — | 0.025 | — | ok |
| Prolangs-C++/ocean | 0.012 | 0.030 | 2.50× | 0.041 | 3.42× | ok |
| Prolangs-C++/primes | 0.007 | 0.021 | — | 0.031 | — | ok |
| Shootout-C++-EH/except | 0.104 | 0.233 | 2.24× | 0.310 | 2.98× | ok |
| Shootout-C++/ackermann | 0.096 | 0.214 | 2.23× | 0.246 | 2.56× | ok |
| Shootout-C++/ary3 | 0.114 | 0.265 | 2.32× | 0.300 | 2.63× | ok |
| Shootout-C++/fibo | 0.099 | 0.202 | 2.04× | 0.309 | 3.12× | ok |
| Shootout-C++/hash | 0.139 | 0.304 | 2.19× | 0.384 | 2.76× | ok |
| Shootout-C++/hash2 | 0.154 | 0.311 | 2.02× | 0.439 | 2.85× | ok |
| Shootout-C++/heapsort | 0.126 | 0.258 | 2.05× | 0.309 | 2.45× | ok |
| Shootout-C++/lists | 0.112 | 0.220 | 1.96× | 0.277 | 2.47× | ok |
| Shootout-C++/lists1 | 0.134 | 0.257 | 1.92× | 0.322 | 2.40× | ok |
| Shootout-C++/matrix | 0.100 | 0.219 | 2.19× | 0.263 | 2.63× | ok |
| Shootout-C++/methcall | 0.097 | 0.221 | 2.28× | 0.274 | 2.82× | ok |
| Shootout-C++/moments | 0.130 | 0.278 | 2.14× | 0.310 | 2.38× | ok |
| Shootout-C++/random | 0.119 | 0.253 | 2.13× | 0.320 | 2.69× | ok |
| Shootout-C++/sieve | 0.121 | 0.237 | 1.96× | 0.335 | 2.77× | ok |
| SmallPT | 0.038 | 0.081 | 2.13× | 0.099 | 2.61× | ok |
| Stanford/Bubblesort | 0.012 | 0.031 | 2.58× | 0.038 | 3.17× | ok |
| Stanford/FloatMM | 0.012 | 0.031 | 2.58× | 0.039 | 3.25× | ok |
| Stanford/Puzzle | 0.012 | 0.033 | 2.75× | 0.039 | 3.25× | ok |
| Stanford/Treesort | 0.012 | 0.031 | 2.58× | 0.038 | 3.17× | ok |
| bg/binarytrees/2 | 0.097 | 0.211 | 2.18× | 0.275 | 2.84× | ok |
| bg/fannkuchredux/3 | 0.046 | 0.085 | 1.85× | 0.105 | 2.28× | ok |
| bg/fasta/1 | 0.076 | 0.135 | 1.78× | 0.164 | 2.16× | ok |
| bg/fasta/2 | 0.075 | 0.136 | 1.81× | 0.192 | 2.56× | ok |
| bg/fasta/3 | 0.118 | 0.257 | 2.18× | 0.299 | 2.53× | ok |
| bg/fasta/4 | 0.109 | 0.209 | 1.92× | 0.242 | 2.22× | ok |
| bg/fasta/6 | 0.173 | 0.415 | 2.40× | 0.506 | 2.92× | ok |
| bg/mandelbrot/0 | 0.193 | 0.418 | 2.17× | 0.539 | 2.79× | ok |
| bg/mandelbrot/2 | 0.117 | 0.274 | 2.34× | 0.313 | 2.68× | ok |
| bg/mandelbrot/3 | 0.131 | 0.291 | 2.22× | 0.342 | 2.61× | ok |
| bg/mandelbrot/5 | 0.058 | 0.119 | 2.05× | 0.151 | 2.60× | ok |
| bg/mandelbrot/7 | 0.060 | 0.135 | 2.25× | 0.154 | 2.57× | ok |
| bg/mandelbrot/9 | 0.058 | 0.113 | 1.95× | 0.147 | 2.53× | ok |
| bg/nbody/1 | 0.026 | 0.091 | 3.50× | 0.104 | 4.00× | ok |
| bg/nbody/2 | 0.038 | 0.078 | 2.05× | 0.097 | 2.55× | ok |
| bg/nbody/3 | 0.038 | 0.051 | 1.34× | 0.103 | 2.71× | ok |
| bg/nbody/6 | 0.037 | 0.085 | 2.30× | 0.101 | 2.73× | ok |
| bg/pidigits/4 | 0.165 | 0.319 | 1.93× | 0.532 | 3.22× | ok |
| bg/pidigits/5 | 0.103 | 0.250 | 2.43× | 0.346 | 3.36× | ok |
| bg/revcomp/1 | 0.113 | 0.253 | 2.24× | 0.310 | 2.74× | ok |
| bg/revcomp/5 | 0.110 | 0.245 | 2.23× | 0.329 | 2.99× | ok |
| bg/spectralnorm/1 | 0.145 | 0.360 | 2.48× | 0.458 | 3.16× | ok |
| box2d | 1.896 | 4.312 | 2.27× | 4.846 | 2.56× | ok |
| fmt | 0.624 | 1.538 | 2.46× | 2.168 | 3.47× | ok |
| hexxagon | 0.350 | 0.842 | 2.41× | 1.070 | 3.06× | ok |
| lambda | 0.058 | 0.166 | 2.86× | 0.219 | 3.78× | ok |
| minisat | 0.068 | 0.189 | 2.78× | 0.215 | 3.16× | ok |
| nlohmann-json | 0.478 | 1.196 | 2.50× | 1.747 | 3.65× | ok |
| pugixml | 0.290 | 0.719 | 2.48× | 0.969 | 3.34× | ok |
| pugixml-tests | 4.100 | 10.417 | 2.54× | 13.300 | 3.24× | ok |
| rapidjson | 0.133 | 0.334 | 2.51× | 0.389 | 2.92× | ok |
| re2 | 4.012 | 9.726 | 2.42× | 7.734 | 1.93× | ok |
| re2-test-re2 | 6.156 | 14.133 | 2.30× | 17.785 | 2.89× | ok |
| re2-test-search | 6.094 | 13.169 | 2.16× | 18.764 | 3.08× | ok |
| tramp3d-v4 | 0.614 | 3.291 | 5.36× | 2.051 | 3.34× | ok |


## Toolchains

- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)
- **valgrind**: valgrind-3.26.0

[Compiler build and codegen results](self-build.md) have their own measured revision.
