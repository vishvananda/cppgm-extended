# Public benchmarks: Intel (GitHub)

Measured compiler: [`1ddd7507e`](https://github.com/vishvananda/cppgm-extended/commit/1ddd7507ee861649d43d1e72dcde7bb17a85a691) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/42988175094) · [Raw program results](results.json)

81 of 81 programs built with cppgm++ and g++ and agreed. Each number is the reference over cppgm++, so above 1× means cppgm++ is smaller or faster.

CPU: Intel(R) Xeon(R) Platinum 8370C CPU @ 2.80GHz.

| Measure | vs g++ | Programs | vs clang++ | Programs |
| --- | ---: | ---: | ---: | ---: |
| Frontend CPU, `-fsyntax-only` | 2.11× | 79 | 2.76× | 79 |
| Compile CPU, `-O0` | 2.84× | 79 | 2.96× | 79 |
| Compile CPU, `-O3` | 4.25× | 81 | 3.90× | 81 |
| Compile peak RSS, `-O0` | 1.46× | 81 | 2.23× | 81 |
| Compile peak RSS, `-O3` | 1.72× | 81 | 2.34× | 81 |
| Executable size, `-O0` | 0.93× | 81 | 0.87× | 81 |
| Executable size, `-O3` | 0.93× | 81 | 0.89× | 81 |
| Run CPU, `-O0` | 1.25× | 81 | 1.19× | 81 |
| Run CPU, `-O3` | 0.98× | 81 | 0.96× | 80 |
| Run instructions, `-O3` | 0.93× | 43 | - | 0 |

O3 execution uses paired ABBA/BAAB blocks. GCC A/A CPU ratios ranged from 0.874 to 1.034; raw observations and block ratios are retained in the artifact.

## Slowest generated code, by run-time instructions

| Program | vs g++ |
| --- | ---: |
| tramp3d-v4 | 0.33× |
| bg/nbody/2 | 0.64× |
| bg/mandelbrot/0 | 0.65× |
| PAQ8p | 0.74× |
| bg/spectralnorm/1 | 0.75× |
| bg/mandelbrot/7 | 0.84× |
| bg/mandelbrot/9 | 0.84× |
| bg/nbody/1 | 0.84× |
| bg/nbody/3 | 0.85× |
| Misc-C++/oopack_v1p8 | 0.86× |

## Compile CPU times

Times are CPU seconds, excluding linking. Speed is reference time divided by cppgm++ time; higher is better. Every program is listed. Missing times show a dash. Ratios are omitted for failed programs or when either time is below 10 ms.

Compilation is measured once per compiler and level; the ABBA/BAAB repetitions apply to generated-program execution and self-built compiler speed.

### O0 compilation

| Program | cppgm++ (s) | GCC (s) | Speed vs GCC ↑ | Clang (s) | Speed vs Clang ↑ | Status |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| Adobe-C++/functionobjects | 0.116 | 0.380 | 3.28× | 0.292 | 2.52× | ok |
| Adobe-C++/loop_unroll | 0.084 | 0.802 | 9.55× | 0.304 | 3.62× | ok |
| Adobe-C++/simple_types_constant_folding | 0.094 | 0.935 | 9.95× | 0.370 | 3.94× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.078 | 0.670 | 8.59× | 0.295 | 3.78× | ok |
| Adobe-C++/stepanov_abstraction | 0.061 | 0.291 | 4.77× | 0.193 | 3.16× | ok |
| Adobe-C++/stepanov_vector | 0.119 | 0.393 | 3.30× | 0.393 | 3.30× | ok |
| Bullet | 8.666 | 26.727 | 3.08× | 24.858 | 2.87× | ok |
| CLAMR | 3.960 | 13.932 | 3.52× | 12.140 | 3.07× | ok |
| CoyoteBench/almabench | 0.048 | 0.110 | 2.29× | 0.119 | 2.48× | ok |
| CoyoteBench/fftbench | 0.194 | 0.391 | 2.02× | 0.598 | 3.08× | ok |
| CoyoteBench/huffbench | 0.051 | 0.103 | 2.02× | 0.145 | 2.84× | ok |
| CoyoteBench/lpbench | 0.049 | 0.117 | 2.39× | 0.119 | 2.43× | ok |
| DOE/HACCKernels | 0.183 | 0.402 | 2.20× | 0.520 | 2.84× | ok |
| DOE/HPCCG | 1.097 | 2.424 | 2.21× | 3.219 | 2.93× | ok |
| DOE/PENNANT | 2.353 | 6.190 | 2.63× | 6.962 | 2.96× | ok |
| DOE/miniFE | 1.090 | 3.097 | 2.84× | 3.655 | 3.35× | ok |
| Misc-C++-EH/spirit | 0.660 | 3.643 | 5.52× | 2.622 | 3.97× | ok |
| Misc-C++-Large/ray | 0.189 | 0.422 | 2.23× | 0.487 | 2.58× | ok |
| Misc-C++-Large/sphereflake | 0.164 | 0.353 | 2.15× | 0.418 | 2.55× | ok |
| Misc-C++/bigfib | 0.240 | 0.591 | 2.46× | 0.621 | 2.59× | ok |
| Misc-C++/oopack_v1p8 | 0.053 | 0.138 | 2.60× | 0.149 | 2.81× | ok |
| Misc-C++/stepanov_container | 0.254 | 0.873 | 3.44× | 0.893 | 3.52× | ok |
| Misc-C++/stepanov_v1p2 | 0.050 | 0.163 | 3.26× | 0.139 | 2.78× | ok |
| PAQ8p | 0.105 | 0.700 | 6.67× | 0.369 | 3.51× | ok |
| Prolangs-C++/life | 0.005 | 0.045 | — | 0.039 | — | ok |
| Prolangs-C++/ocean | 0.016 | 0.091 | 5.69× | 0.066 | 4.12× | ok |
| Prolangs-C++/primes | 0.008 | 0.033 | — | 0.031 | — | ok |
| Shootout-C++-EH/except | 0.128 | 0.241 | 1.88× | 0.341 | 2.66× | ok |
| Shootout-C++/ackermann | 0.121 | 0.247 | 2.04× | 0.326 | 2.69× | ok |
| Shootout-C++/ary3 | 0.139 | 0.308 | 2.22× | 0.454 | 3.27× | ok |
| Shootout-C++/fibo | 0.107 | 0.222 | 2.07× | 0.318 | 2.97× | ok |
| Shootout-C++/hash | 0.175 | 0.409 | 2.34× | 0.494 | 2.82× | ok |
| Shootout-C++/hash2 | 0.175 | 0.428 | 2.45× | 0.480 | 2.74× | ok |
| Shootout-C++/heapsort | 0.147 | 0.294 | 2.00× | 0.376 | 2.56× | ok |
| Shootout-C++/lists | 0.141 | 0.344 | 2.44× | 0.368 | 2.61× | ok |
| Shootout-C++/lists1 | 0.161 | 0.360 | 2.24× | 0.429 | 2.66× | ok |
| Shootout-C++/matrix | 0.123 | 0.243 | 1.98× | 0.338 | 2.75× | ok |
| Shootout-C++/methcall | 0.120 | 0.231 | 1.93× | 0.319 | 2.66× | ok |
| Shootout-C++/moments | 0.174 | 0.435 | 2.50× | 0.426 | 2.45× | ok |
| Shootout-C++/random | 0.151 | 0.274 | 1.81× | 0.412 | 2.73× | ok |
| Shootout-C++/sieve | 0.159 | 0.353 | 2.22× | 0.416 | 2.62× | ok |
| SmallPT | 0.046 | 0.138 | 3.00× | 0.124 | 2.70× | ok |
| Stanford/Bubblesort | 0.015 | 0.040 | 2.67× | 0.045 | 3.00× | ok |
| Stanford/FloatMM | 0.015 | 0.041 | 2.73× | 0.045 | 3.00× | ok |
| Stanford/Puzzle | 0.016 | 0.049 | 3.06× | 0.047 | 2.94× | ok |
| Stanford/Treesort | 0.015 | 0.042 | 2.80× | 0.046 | 3.07× | ok |
| bg/binarytrees/2 | 0.121 | 0.261 | 2.16× | 0.363 | 3.00× | ok |
| bg/fannkuchredux/3 | 0.062 | 0.107 | 1.73× | 0.145 | 2.34× | ok |
| bg/fasta/1 | 0.098 | 0.208 | 2.12× | 0.240 | 2.45× | ok |
| bg/fasta/2 | 0.092 | 0.201 | 2.18× | 0.209 | 2.27× | ok |
| bg/fasta/3 | 0.143 | 0.266 | 1.86× | 0.380 | 2.66× | ok |
| bg/fasta/4 | 0.138 | 0.263 | 1.91× | 0.350 | 2.54× | ok |
| bg/fasta/6 | 0.230 | 0.598 | 2.60× | 0.657 | 2.86× | ok |
| bg/mandelbrot/0 | 0.268 | 0.672 | 2.51× | 0.679 | 2.53× | ok |
| bg/mandelbrot/2 | 0.147 | 0.346 | 2.35× | 0.391 | 2.66× | ok |
| bg/mandelbrot/3 | 0.167 | 0.293 | 1.75× | 0.449 | 2.69× | ok |
| bg/mandelbrot/5 | 0.066 | 0.159 | 2.41× | 0.205 | 3.11× | ok |
| bg/mandelbrot/7 | 0.074 | 0.180 | 2.43× | 0.230 | 3.11× | ok |
| bg/mandelbrot/9 | 0.079 | 0.180 | 2.28× | 0.197 | 2.49× | ok |
| bg/nbody/1 | 0.049 | 0.158 | 3.22× | 0.134 | 2.73× | ok |
| bg/nbody/2 | 0.049 | 0.100 | 2.04× | 0.121 | 2.47× | ok |
| bg/nbody/3 | 0.048 | 0.100 | 2.08× | 0.121 | 2.52× | ok |
| bg/nbody/6 | 0.049 | 0.157 | 3.20× | 0.174 | 3.55× | ok |
| bg/pidigits/4 | 0.209 | 0.513 | 2.45× | 0.685 | 3.28× | ok |
| bg/pidigits/5 | 0.130 | 0.288 | 2.22× | 0.404 | 3.11× | ok |
| bg/revcomp/1 | 0.145 | 0.331 | 2.28× | 0.449 | 3.10× | ok |
| bg/revcomp/5 | 0.138 | 0.318 | 2.30× | 0.434 | 3.14× | ok |
| bg/spectralnorm/1 | 0.198 | 0.422 | 2.13× | 0.608 | 3.07× | ok |
| box2d | 2.559 | 6.733 | 2.63× | 7.509 | 2.93× | ok |
| fmt | 0.849 | 3.536 | 4.16× | 3.325 | 3.92× | ok |
| hexxagon | 0.452 | 1.017 | 2.25× | 1.362 | 3.01× | ok |
| lambda | 0.088 | 0.458 | 5.20× | 0.345 | 3.92× | ok |
| minisat | 0.106 | 0.453 | 4.27× | 0.358 | 3.38× | ok |
| nlohmann-json | 0.674 | 2.886 | 4.28× | 2.887 | 4.28× | ok |
| pugixml | 0.424 | 1.986 | 4.68× | 1.629 | 3.84× | ok |
| pugixml-tests | 6.091 | 22.403 | 3.68× | 19.340 | 3.18× | ok |
| rapidjson | 0.198 | 0.708 | 3.58× | 0.599 | 3.03× | ok |
| re2 | 4.376 | 15.942 | 3.64× | 10.382 | 2.37× | ok |
| re2-test-re2 | 7.840 | 22.726 | 2.90× | 24.439 | 3.12× | ok |
| re2-test-search | 7.742 | 21.605 | 2.79× | 23.655 | 3.06× | ok |
| tramp3d-v4 | 1.049 | 12.393 | 11.81× | 4.054 | 3.86× | ok |

### O3 compilation

| Program | cppgm++ (s) | GCC (s) | Speed vs GCC ↑ | Clang (s) | Speed vs Clang ↑ | Status |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| Adobe-C++/functionobjects | 0.164 | 1.267 | 7.73× | 0.725 | 4.42× | ok |
| Adobe-C++/loop_unroll | 0.397 | 8.299 | 20.90× | 6.519 | 16.42× | ok |
| Adobe-C++/simple_types_constant_folding | 0.269 | 4.291 | 15.95× | 3.444 | 12.80× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.215 | 3.264 | 15.18× | 3.127 | 14.54× | ok |
| Adobe-C++/stepanov_abstraction | 0.119 | 1.451 | 12.19× | 0.726 | 6.10× | ok |
| Adobe-C++/stepanov_vector | 0.190 | 1.948 | 10.25× | 0.823 | 4.33× | ok |
| Bullet | 9.653 | 54.461 | 5.64× | 43.850 | 4.54× | ok |
| CLAMR | 6.244 | 34.769 | 5.57× | 29.718 | 4.76× | ok |
| CoyoteBench/almabench | 0.052 | 0.233 | 4.48× | 0.176 | 3.38× | ok |
| CoyoteBench/fftbench | 0.208 | 0.692 | 3.33× | 0.787 | 3.78× | ok |
| CoyoteBench/huffbench | 0.060 | 0.335 | 5.58× | 0.218 | 3.63× | ok |
| CoyoteBench/lpbench | 0.054 | 0.247 | 4.57× | 0.281 | 5.20× | ok |
| DOE/HACCKernels | 0.196 | 0.554 | 2.83× | 0.610 | 3.11× | ok |
| DOE/HPCCG | 1.168 | 3.452 | 2.96× | 3.607 | 3.09× | ok |
| DOE/PENNANT | 2.772 | 10.364 | 3.74× | 8.753 | 3.16× | ok |
| DOE/miniFE | 1.357 | 6.385 | 4.71× | 4.946 | 3.64× | ok |
| Misc-C++-EH/spirit | 1.091 | 5.467 | 5.01× | 4.998 | 4.58× | ok |
| Misc-C++-Large/ray | 0.190 | 0.738 | 3.88× | 0.642 | 3.38× | ok |
| Misc-C++-Large/sphereflake | 0.180 | 1.143 | 6.35× | 0.615 | 3.42× | ok |
| Misc-C++/bigfib | 0.244 | 0.852 | 3.49× | 0.820 | 3.36× | ok |
| Misc-C++/oopack_v1p8 | 0.061 | 0.275 | 4.51× | 0.251 | 4.11× | ok |
| Misc-C++/stepanov_container | 0.352 | 1.241 | 3.53× | 1.463 | 4.16× | ok |
| Misc-C++/stepanov_v1p2 | 0.060 | 0.287 | 4.78× | 0.277 | 4.62× | ok |
| PAQ8p | 0.364 | 6.189 | 17.00× | 2.591 | 7.12× | ok |
| Prolangs-C++/life | 0.010 | 0.148 | 14.80× | 0.093 | 9.30× | ok |
| Prolangs-C++/ocean | 0.027 | 0.306 | 11.33× | 0.158 | 5.85× | ok |
| Prolangs-C++/primes | 0.011 | 0.050 | 4.55× | 0.044 | 4.00× | ok |
| Shootout-C++-EH/except | 0.140 | 0.288 | 2.06× | 0.357 | 2.55× | ok |
| Shootout-C++/ackermann | 0.125 | 0.309 | 2.47× | 0.340 | 2.72× | ok |
| Shootout-C++/ary3 | 0.201 | 0.355 | 1.77× | 0.449 | 2.23× | ok |
| Shootout-C++/fibo | 0.120 | 0.259 | 2.16× | 0.320 | 2.67× | ok |
| Shootout-C++/hash | 0.192 | 0.568 | 2.96× | 0.624 | 3.25× | ok |
| Shootout-C++/hash2 | 0.197 | 0.619 | 3.14× | 0.721 | 3.66× | ok |
| Shootout-C++/heapsort | 0.149 | 0.364 | 2.44× | 0.399 | 2.68× | ok |
| Shootout-C++/lists | 0.145 | 0.383 | 2.64× | 0.514 | 3.54× | ok |
| Shootout-C++/lists1 | 0.172 | 0.426 | 2.48× | 0.500 | 2.91× | ok |
| Shootout-C++/matrix | 0.126 | 0.323 | 2.56× | 0.483 | 3.83× | ok |
| Shootout-C++/methcall | 0.128 | 0.276 | 2.16× | 0.335 | 2.62× | ok |
| Shootout-C++/moments | 0.181 | 0.487 | 2.69× | 0.497 | 2.75× | ok |
| Shootout-C++/random | 0.156 | 0.315 | 2.02× | 0.426 | 2.73× | ok |
| Shootout-C++/sieve | 0.156 | 0.383 | 2.46× | 0.492 | 3.15× | ok |
| SmallPT | 0.060 | 0.417 | 6.95× | 0.236 | 3.93× | ok |
| Stanford/Bubblesort | 0.018 | 0.078 | 4.33× | 0.068 | 3.78× | ok |
| Stanford/FloatMM | 0.025 | 0.092 | 3.68× | 0.096 | 3.84× | ok |
| Stanford/Puzzle | 0.024 | 0.180 | 7.50× | 0.188 | 7.83× | ok |
| Stanford/Treesort | 0.020 | 0.122 | 6.10× | 0.073 | 3.65× | ok |
| bg/binarytrees/2 | 0.129 | 0.680 | 5.27× | 0.381 | 2.95× | ok |
| bg/fannkuchredux/3 | 0.068 | 0.179 | 2.63× | 0.187 | 2.75× | ok |
| bg/fasta/1 | 0.104 | 0.262 | 2.52× | 0.294 | 2.83× | ok |
| bg/fasta/2 | 0.105 | 0.261 | 2.49× | 0.272 | 2.59× | ok |
| bg/fasta/3 | 0.150 | 0.331 | 2.21× | 0.461 | 3.07× | ok |
| bg/fasta/4 | 0.140 | 0.287 | 2.05× | 0.345 | 2.46× | ok |
| bg/fasta/6 | 0.251 | 0.695 | 2.77× | 0.860 | 3.43× | ok |
| bg/mandelbrot/0 | 0.268 | 0.985 | 3.68× | 0.970 | 3.62× | ok |
| bg/mandelbrot/2 | 0.158 | 0.420 | 2.66× | 0.485 | 3.07× | ok |
| bg/mandelbrot/3 | 0.172 | 0.334 | 1.94× | 0.490 | 2.85× | ok |
| bg/mandelbrot/5 | 0.077 | 0.219 | 2.84× | 0.267 | 3.47× | ok |
| bg/mandelbrot/7 | 0.083 | 0.287 | 3.46× | 0.262 | 3.16× | ok |
| bg/mandelbrot/9 | 0.090 | 0.288 | 3.20× | 0.261 | 2.90× | ok |
| bg/nbody/1 | 0.068 | 0.250 | 3.68× | 0.281 | 4.13× | ok |
| bg/nbody/2 | 0.052 | 0.175 | 3.37× | 0.194 | 3.73× | ok |
| bg/nbody/3 | 0.052 | 0.198 | 3.81× | 0.182 | 3.50× | ok |
| bg/nbody/6 | 0.065 | 0.246 | 3.78× | 0.289 | 4.45× | ok |
| bg/pidigits/4 | 0.221 | 0.570 | 2.58× | 0.692 | 3.13× | ok |
| bg/pidigits/5 | 0.143 | 0.341 | 2.38× | 0.400 | 2.80× | ok |
| bg/revcomp/1 | 0.154 | 0.374 | 2.43× | 0.478 | 3.10× | ok |
| bg/revcomp/5 | 0.138 | 0.340 | 2.46× | 0.480 | 3.48× | ok |
| bg/spectralnorm/1 | 0.213 | 0.555 | 2.61× | 0.669 | 3.14× | ok |
| box2d | 2.855 | 11.887 | 4.16× | 10.719 | 3.75× | ok |
| fmt | 1.385 | 8.003 | 5.78× | 6.708 | 4.84× | ok |
| hexxagon | 0.497 | 1.657 | 3.33× | 1.697 | 3.41× | ok |
| lambda | 0.156 | 1.711 | 10.97× | 0.844 | 5.41× | ok |
| minisat | 0.194 | 1.549 | 7.98× | 1.005 | 5.18× | ok |
| nlohmann-json | 1.084 | 5.418 | 5.00× | 4.734 | 4.37× | ok |
| pugixml | 0.908 | 8.278 | 9.12× | 5.920 | 6.52× | ok |
| pugixml-tests | 8.190 | 53.869 | 6.58× | 36.879 | 4.50× | ok |
| rapidjson | 0.290 | 1.879 | 6.48× | 1.117 | 3.85× | ok |
| re2 | 4.143 | 27.493 | 6.64× | 14.819 | 3.58× | ok |
| re2-test-re2 | 9.281 | 40.080 | 4.32× | 34.688 | 3.74× | ok |
| re2-test-search | 9.091 | 35.634 | 3.92× | 27.829 | 3.06× | ok |
| tramp3d-v4 | 2.498 | 28.401 | 11.37× | 19.777 | 7.92× | ok |

### Frontend only: -fsyntax-only

| Program | cppgm++ (s) | GCC (s) | Speed vs GCC ↑ | Clang (s) | Speed vs Clang ↑ | Status |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| Adobe-C++/functionobjects | 0.109 | 0.219 | 2.01× | 0.249 | 2.28× | ok |
| Adobe-C++/loop_unroll | 0.060 | 0.161 | 2.68× | 0.175 | 2.92× | ok |
| Adobe-C++/simple_types_constant_folding | 0.066 | 0.171 | 2.59× | 0.187 | 2.83× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.059 | 0.154 | 2.61× | 0.161 | 2.73× | ok |
| Adobe-C++/stepanov_abstraction | 0.052 | 0.103 | 1.98× | 0.150 | 2.88× | ok |
| Adobe-C++/stepanov_vector | 0.105 | 0.191 | 1.82× | 0.309 | 2.94× | ok |
| Bullet | 8.006 | 16.186 | 2.02× | 21.186 | 2.65× | ok |
| CLAMR | 3.482 | 7.766 | 2.23× | 9.679 | 2.78× | ok |
| CoyoteBench/almabench | 0.047 | 0.090 | 1.91× | 0.113 | 2.40× | ok |
| CoyoteBench/fftbench | 0.191 | 0.349 | 1.83× | 0.561 | 2.94× | ok |
| CoyoteBench/huffbench | 0.051 | 0.089 | 1.75× | 0.134 | 2.63× | ok |
| CoyoteBench/lpbench | 0.048 | 0.100 | 2.08× | 0.117 | 2.44× | ok |
| DOE/HACCKernels | 0.178 | 0.361 | 2.03× | 0.507 | 2.85× | ok |
| DOE/HPCCG | 1.072 | 2.135 | 1.99× | 2.933 | 2.74× | ok |
| DOE/PENNANT | 2.370 | 4.463 | 1.88× | 5.939 | 2.51× | ok |
| DOE/miniFE | 1.021 | 1.970 | 1.93× | 3.070 | 3.01× | ok |
| Misc-C++-EH/spirit | 0.486 | 1.341 | 2.76× | 1.674 | 3.44× | ok |
| Misc-C++-Large/ray | 0.183 | 0.325 | 1.78× | 0.444 | 2.43× | ok |
| Misc-C++-Large/sphereflake | 0.160 | 0.318 | 1.99× | 0.393 | 2.46× | ok |
| Misc-C++/bigfib | 0.189 | 0.409 | 2.16× | 0.542 | 2.87× | ok |
| Misc-C++/oopack_v1p8 | 0.051 | 0.100 | 1.96× | 0.134 | 2.63× | ok |
| Misc-C++/stepanov_container | 0.233 | 0.541 | 2.32× | 0.718 | 3.08× | ok |
| Misc-C++/stepanov_v1p2 | 0.046 | 0.092 | 2.00× | 0.116 | 2.52× | ok |
| PAQ8p | 0.073 | 0.197 | 2.70× | 0.219 | 3.00× | ok |
| Prolangs-C++/life | 0.004 | 0.014 | — | 0.026 | — | ok |
| Prolangs-C++/ocean | 0.013 | 0.033 | 2.54× | 0.044 | 3.38× | ok |
| Prolangs-C++/primes | 0.007 | 0.023 | — | 0.031 | — | ok |
| Shootout-C++-EH/except | 0.127 | 0.240 | 1.89× | 0.324 | 2.55× | ok |
| Shootout-C++/ackermann | 0.119 | 0.254 | 2.13× | 0.317 | 2.66× | ok |
| Shootout-C++/ary3 | 0.133 | 0.278 | 2.09× | 0.426 | 3.20× | ok |
| Shootout-C++/fibo | 0.104 | 0.227 | 2.18× | 0.330 | 3.17× | ok |
| Shootout-C++/hash | 0.168 | 0.315 | 1.88× | 0.442 | 2.63× | ok |
| Shootout-C++/hash2 | 0.166 | 0.317 | 1.91× | 0.424 | 2.55× | ok |
| Shootout-C++/heapsort | 0.144 | 0.307 | 2.13× | 0.366 | 2.54× | ok |
| Shootout-C++/lists | 0.144 | 0.268 | 1.86× | 0.336 | 2.33× | ok |
| Shootout-C++/lists1 | 0.156 | 0.300 | 1.92× | 0.384 | 2.46× | ok |
| Shootout-C++/matrix | 0.121 | 0.241 | 1.99× | 0.326 | 2.69× | ok |
| Shootout-C++/methcall | 0.117 | 0.232 | 1.98× | 0.305 | 2.61× | ok |
| Shootout-C++/moments | 0.159 | 0.345 | 2.17× | 0.386 | 2.43× | ok |
| Shootout-C++/random | 0.154 | 0.278 | 1.81× | 0.411 | 2.67× | ok |
| Shootout-C++/sieve | 0.150 | 0.293 | 1.95× | 0.385 | 2.57× | ok |
| SmallPT | 0.044 | 0.092 | 2.09× | 0.113 | 2.57× | ok |
| Stanford/Bubblesort | 0.014 | 0.035 | 2.50× | 0.043 | 3.07× | ok |
| Stanford/FloatMM | 0.014 | 0.036 | 2.57× | 0.042 | 3.00× | ok |
| Stanford/Puzzle | 0.015 | 0.034 | 2.27× | 0.043 | 2.87× | ok |
| Stanford/Treesort | 0.014 | 0.036 | 2.57× | 0.042 | 3.00× | ok |
| bg/binarytrees/2 | 0.120 | 0.259 | 2.16× | 0.343 | 2.86× | ok |
| bg/fannkuchredux/3 | 0.061 | 0.095 | 1.56× | 0.138 | 2.26× | ok |
| bg/fasta/1 | 0.096 | 0.167 | 1.74× | 0.222 | 2.31× | ok |
| bg/fasta/2 | 0.088 | 0.164 | 1.86× | 0.191 | 2.17× | ok |
| bg/fasta/3 | 0.140 | 0.252 | 1.80× | 0.361 | 2.58× | ok |
| bg/fasta/4 | 0.131 | 0.250 | 1.91× | 0.323 | 2.47× | ok |
| bg/fasta/6 | 0.215 | 0.487 | 2.27× | 0.589 | 2.74× | ok |
| bg/mandelbrot/0 | 0.246 | 0.478 | 1.94× | 0.633 | 2.57× | ok |
| bg/mandelbrot/2 | 0.145 | 0.307 | 2.12× | 0.388 | 2.68× | ok |
| bg/mandelbrot/3 | 0.166 | 0.291 | 1.75× | 0.438 | 2.64× | ok |
| bg/mandelbrot/5 | 0.063 | 0.133 | 2.11× | 0.177 | 2.81× | ok |
| bg/mandelbrot/7 | 0.070 | 0.132 | 1.89× | 0.201 | 2.87× | ok |
| bg/mandelbrot/9 | 0.074 | 0.135 | 1.82× | 0.177 | 2.39× | ok |
| bg/nbody/1 | 0.047 | 0.095 | 2.02× | 0.113 | 2.40× | ok |
| bg/nbody/2 | 0.048 | 0.083 | 1.73× | 0.113 | 2.35× | ok |
| bg/nbody/3 | 0.048 | 0.082 | 1.71× | 0.113 | 2.35× | ok |
| bg/nbody/6 | 0.046 | 0.094 | 2.04× | 0.146 | 3.17× | ok |
| bg/pidigits/4 | 0.206 | 0.462 | 2.24× | 0.612 | 2.97× | ok |
| bg/pidigits/5 | 0.134 | 0.285 | 2.13× | 0.389 | 2.90× | ok |
| bg/revcomp/1 | 0.144 | 0.273 | 1.90× | 0.411 | 2.85× | ok |
| bg/revcomp/5 | 0.132 | 0.266 | 2.02× | 0.386 | 2.92× | ok |
| bg/spectralnorm/1 | 0.198 | 0.393 | 1.98× | 0.560 | 2.83× | ok |
| box2d | 2.233 | 4.681 | 2.10× | 7.120 | 3.19× | ok |
| fmt | 0.751 | 1.690 | 2.25× | 2.646 | 3.52× | ok |
| hexxagon | 0.442 | 0.927 | 2.10× | 1.302 | 2.95× | ok |
| lambda | 0.070 | 0.195 | 2.79× | 0.249 | 3.56× | ok |
| minisat | 0.089 | 0.212 | 2.38× | 0.265 | 2.98× | ok |
| nlohmann-json | 0.565 | 1.359 | 2.41× | 2.214 | 3.92× | ok |
| pugixml | 0.321 | 0.827 | 2.58× | 1.203 | 3.75× | ok |
| pugixml-tests | 5.319 | 11.907 | 2.24× | 14.873 | 2.80× | ok |
| rapidjson | 0.166 | 0.390 | 2.35× | 0.469 | 2.83× | ok |
| re2 | 4.972 | 10.579 | 2.13× | 8.991 | 1.81× | ok |
| re2-test-re2 | 7.173 | 15.525 | 2.16× | 21.290 | 2.97× | ok |
| re2-test-search | 7.152 | 15.004 | 2.10× | 21.199 | 2.96× | ok |
| tramp3d-v4 | 0.640 | 3.407 | 5.32× | 2.367 | 3.70× | ok |


## Toolchains

- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)
- **valgrind**: valgrind-3.26.0

[Compiler build and codegen results](self-build.md) have their own measured revision.
