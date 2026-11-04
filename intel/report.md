# Public benchmarks: Intel Xeon

Measured compiler: [`08bba949b`](https://github.com/vishvananda/cppgm-extended/commit/08bba949b4a0c3f2a728bf52c6e7089a38b44797) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/41781359245) · [Raw program results](results.json) · [Raw rebuild results](selfhost.json)

81 of 81 programs built with cppgm++ and g++ and agreed. Each number is the reference over cppgm++, so above 1× means cppgm++ is smaller or faster.

CPU: Intel(R) Xeon(R) CPU E5-2696 v4 @ 2.20GHz.

| Measure | vs g++ | Programs | vs clang++ | Programs |
| --- | ---: | ---: | ---: | ---: |
| Frontend CPU, `-fsyntax-only` | 2.36× | 79 | 3.85× | 79 |
| Compile CPU, `-O0` | 3.03× | 79 | 3.90× | 79 |
| Compile CPU, `-O3` | 4.48× | 81 | 4.58× | 81 |
| Compile peak RSS, `-O0` | 1.38× | 81 | 2.11× | 81 |
| Compile peak RSS, `-O3` | 1.55× | 81 | 2.04× | 81 |
| Executable size, `-O0` | 0.93× | 81 | 0.88× | 81 |
| Executable size, `-O3` | 0.93× | 81 | 0.88× | 81 |
| Run CPU, `-O0` | 1.00× | 81 | 0.98× | 81 |
| Run CPU, `-O3` | 0.99× | 81 | 0.98× | 81 |
| Run instructions, `-O3` | 0.95× | 43 | - | 0 |
| Self compile CPU | 4.86× | - | - | - |
| Self-built compiler CPU | 1.00× | - | - | - |

O3 execution uses paired ABBA/BAAB blocks. GCC A/A CPU ratios ranged from 0.816 to 1.501; raw observations and block ratios are retained in the artifact.

Self-built / GCC-built compiler CPU: paired ratios 0.988–1.013; A/A calibration 1.032. A/A variation limits conclusions about small changes.

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
| Adobe-C++/functionobjects | 0.116 | 0.379 | 3.27× | 0.389 | 3.35× | ok |
| Adobe-C++/loop_unroll | 0.093 | 0.806 | 8.67× | 0.422 | 4.54× | ok |
| Adobe-C++/simple_types_constant_folding | 0.111 | 1.092 | 9.84× | 0.709 | 6.39× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.079 | 0.687 | 8.70× | 0.443 | 5.61× | ok |
| Adobe-C++/stepanov_abstraction | 0.061 | 0.298 | 4.89× | 0.297 | 4.87× | ok |
| Adobe-C++/stepanov_vector | 0.111 | 0.442 | 3.98× | 0.447 | 4.03× | ok |
| Bullet | 7.595 | 29.998 | 3.95× | 34.690 | 4.57× | ok |
| CLAMR | 4.073 | 14.612 | 3.59× | 15.964 | 3.92× | ok |
| CoyoteBench/almabench | 0.045 | 0.111 | 2.47× | 0.167 | 3.71× | ok |
| CoyoteBench/fftbench | 0.180 | 0.410 | 2.28× | 0.667 | 3.71× | ok |
| CoyoteBench/huffbench | 0.045 | 0.126 | 2.80× | 0.167 | 3.71× | ok |
| CoyoteBench/lpbench | 0.055 | 0.105 | 1.91× | 0.232 | 4.22× | ok |
| DOE/HACCKernels | 0.219 | 0.422 | 1.93× | 0.720 | 3.29× | ok |
| DOE/HPCCG | 1.170 | 2.717 | 2.32× | 4.130 | 3.53× | ok |
| DOE/PENNANT | 2.321 | 6.159 | 2.65× | 10.568 | 4.55× | ok |
| DOE/miniFE | 0.999 | 3.319 | 3.32× | 4.901 | 4.91× | ok |
| Misc-C++-EH/spirit | 0.707 | 5.353 | 7.57× | 4.943 | 6.99× | ok |
| Misc-C++-Large/ray | 0.163 | 0.413 | 2.53× | 0.896 | 5.50× | ok |
| Misc-C++-Large/sphereflake | 0.219 | 0.377 | 1.72× | 0.537 | 2.45× | ok |
| Misc-C++/bigfib | 0.231 | 0.604 | 2.61× | 0.728 | 3.15× | ok |
| Misc-C++/oopack_v1p8 | 0.049 | 0.129 | 2.63× | 0.306 | 6.24× | ok |
| Misc-C++/stepanov_container | 0.333 | 0.845 | 2.54× | 0.951 | 2.86× | ok |
| Misc-C++/stepanov_v1p2 | 0.047 | 0.192 | 4.09× | 0.234 | 4.98× | ok |
| PAQ8p | 0.097 | 1.070 | 11.03× | 0.417 | 4.30× | ok |
| Prolangs-C++/life | 0.008 | 0.073 | — | 0.047 | — | ok |
| Prolangs-C++/ocean | 0.023 | 0.112 | 4.87× | 0.081 | 3.52× | ok |
| Prolangs-C++/primes | 0.009 | 0.036 | — | 0.066 | — | ok |
| Shootout-C++-EH/except | 0.113 | 0.261 | 2.31× | 0.427 | 3.78× | ok |
| Shootout-C++/ackermann | 0.118 | 0.254 | 2.15× | 0.418 | 3.54× | ok |
| Shootout-C++/ary3 | 0.130 | 0.328 | 2.52× | 0.518 | 3.98× | ok |
| Shootout-C++/fibo | 0.122 | 0.259 | 2.12× | 0.444 | 3.64× | ok |
| Shootout-C++/hash | 0.168 | 0.451 | 2.68× | 0.680 | 4.05× | ok |
| Shootout-C++/hash2 | 0.192 | 0.465 | 2.42× | 0.617 | 3.21× | ok |
| Shootout-C++/heapsort | 0.134 | 0.315 | 2.35× | 0.574 | 4.28× | ok |
| Shootout-C++/lists | 0.157 | 0.347 | 2.21× | 0.497 | 3.17× | ok |
| Shootout-C++/lists1 | 0.147 | 0.381 | 2.59× | 0.525 | 3.57× | ok |
| Shootout-C++/matrix | 0.118 | 0.264 | 2.24× | 0.508 | 4.31× | ok |
| Shootout-C++/methcall | 0.112 | 0.263 | 2.35× | 0.413 | 3.69× | ok |
| Shootout-C++/moments | 0.170 | 0.420 | 2.47× | 0.558 | 3.28× | ok |
| Shootout-C++/random | 0.139 | 0.371 | 2.67× | 0.497 | 3.58× | ok |
| Shootout-C++/sieve | 0.148 | 0.389 | 2.63× | 0.569 | 3.84× | ok |
| SmallPT | 0.043 | 0.131 | 3.05× | 0.172 | 4.00× | ok |
| Stanford/Bubblesort | 0.014 | 0.048 | 3.43× | 0.062 | 4.43× | ok |
| Stanford/FloatMM | 0.014 | 0.056 | 4.00× | 0.057 | 4.07× | ok |
| Stanford/Puzzle | 0.015 | 0.051 | 3.40× | 0.063 | 4.20× | ok |
| Stanford/Treesort | 0.017 | 0.042 | 2.47× | 0.062 | 3.65× | ok |
| bg/binarytrees/2 | 0.115 | 0.265 | 2.30× | 0.408 | 3.55× | ok |
| bg/fannkuchredux/3 | 0.054 | 0.114 | 2.11× | 0.176 | 3.26× | ok |
| bg/fasta/1 | 0.087 | 0.196 | 2.25× | 0.273 | 3.14× | ok |
| bg/fasta/2 | 0.090 | 0.205 | 2.28× | 0.269 | 2.99× | ok |
| bg/fasta/3 | 0.173 | 0.307 | 1.77× | 0.493 | 2.85× | ok |
| bg/fasta/4 | 0.128 | 0.274 | 2.14× | 0.376 | 2.94× | ok |
| bg/fasta/6 | 0.264 | 0.648 | 2.45× | 0.840 | 3.18× | ok |
| bg/mandelbrot/0 | 0.237 | 0.731 | 3.08× | 0.900 | 3.80× | ok |
| bg/mandelbrot/2 | 0.174 | 0.417 | 2.40× | 0.579 | 3.33× | ok |
| bg/mandelbrot/3 | 0.148 | 0.323 | 2.18× | 0.528 | 3.57× | ok |
| bg/mandelbrot/5 | 0.090 | 0.175 | 1.94× | 0.250 | 2.78× | ok |
| bg/mandelbrot/7 | 0.076 | 0.206 | 2.71× | 0.253 | 3.33× | ok |
| bg/mandelbrot/9 | 0.081 | 0.180 | 2.22× | 0.237 | 2.93× | ok |
| bg/nbody/1 | 0.045 | 0.157 | 3.49× | 0.189 | 4.20× | ok |
| bg/nbody/2 | 0.044 | 0.100 | 2.27× | 0.153 | 3.48× | ok |
| bg/nbody/3 | 0.041 | 0.119 | 2.90× | 0.151 | 3.68× | ok |
| bg/nbody/6 | 0.044 | 0.149 | 3.39× | 0.176 | 4.00× | ok |
| bg/pidigits/4 | 0.185 | 0.493 | 2.66× | 0.758 | 4.10× | ok |
| bg/pidigits/5 | 0.146 | 0.273 | 1.87× | 0.422 | 2.89× | ok |
| bg/revcomp/1 | 0.133 | 0.336 | 2.53× | 0.469 | 3.53× | ok |
| bg/revcomp/5 | 0.119 | 0.308 | 2.59× | 0.747 | 6.28× | ok |
| bg/spectralnorm/1 | 0.305 | 0.403 | 1.32× | 0.655 | 2.15× | ok |
| box2d | 2.612 | 7.720 | 2.96× | 9.175 | 3.51× | ok |
| fmt | 0.789 | 3.452 | 4.38× | 3.411 | 4.32× | ok |
| hexxagon | 0.496 | 0.970 | 1.96× | 1.506 | 3.04× | ok |
| lambda | 0.097 | 0.434 | 4.47× | 0.557 | 5.74× | ok |
| minisat | 0.091 | 0.462 | 5.08× | 0.610 | 6.70× | ok |
| nlohmann-json | 0.578 | 3.259 | 5.64× | 2.963 | 5.13× | ok |
| pugixml | 0.415 | 2.536 | 6.11× | 2.483 | 5.98× | ok |
| pugixml-tests | 5.399 | 27.151 | 5.03× | 24.499 | 4.54× | ok |
| rapidjson | 0.196 | 0.882 | 4.50× | 0.630 | 3.21× | ok |
| re2 | 6.205 | 17.784 | 2.87× | 17.922 | 2.89× | ok |
| re2-test-re2 | 7.050 | 22.001 | 3.12× | 28.002 | 3.97× | ok |
| re2-test-search | 6.968 | 21.534 | 3.09× | 27.141 | 3.90× | ok |
| tramp3d-v4 | 1.640 | 11.624 | 7.09× | 13.324 | 8.12× | ok |

### O3 compilation

| Program | cppgm++ (s) | GCC (s) | Speed vs GCC ↑ | Clang (s) | Speed vs Clang ↑ | Status |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| Adobe-C++/functionobjects | 0.196 | 1.409 | 7.19× | 0.955 | 4.87× | ok |
| Adobe-C++/loop_unroll | 0.686 | 9.138 | 13.32× | 7.162 | 10.44× | ok |
| Adobe-C++/simple_types_constant_folding | 0.448 | 5.154 | 11.50× | 3.836 | 8.56× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.260 | 3.613 | 13.90× | 2.869 | 11.03× | ok |
| Adobe-C++/stepanov_abstraction | 0.183 | 1.635 | 8.93× | 0.807 | 4.41× | ok |
| Adobe-C++/stepanov_vector | 0.160 | 3.360 | 21.00× | 1.092 | 6.83× | ok |
| Bullet | 9.907 | 58.870 | 5.94× | 54.697 | 5.52× | ok |
| CLAMR | 6.452 | 41.351 | 6.41× | 31.856 | 4.94× | ok |
| CoyoteBench/almabench | 0.048 | 0.238 | 4.96× | 0.268 | 5.58× | ok |
| CoyoteBench/fftbench | 0.190 | 0.666 | 3.51× | 0.920 | 4.84× | ok |
| CoyoteBench/huffbench | 0.058 | 0.452 | 7.79× | 0.238 | 4.10× | ok |
| CoyoteBench/lpbench | 0.064 | 0.268 | 4.19× | 0.310 | 4.84× | ok |
| DOE/HACCKernels | 0.184 | 0.619 | 3.36× | 1.049 | 5.70× | ok |
| DOE/HPCCG | 1.414 | 3.886 | 2.75× | 5.960 | 4.21× | ok |
| DOE/PENNANT | 2.767 | 10.566 | 3.82× | 11.395 | 4.12× | ok |
| DOE/miniFE | 1.436 | 8.163 | 5.68× | 6.295 | 4.38× | ok |
| Misc-C++-EH/spirit | 1.247 | 5.809 | 4.66× | 5.380 | 4.31× | ok |
| Misc-C++-Large/ray | 0.184 | 0.742 | 4.03× | 0.757 | 4.11× | ok |
| Misc-C++-Large/sphereflake | 0.165 | 1.170 | 7.09× | 0.659 | 3.99× | ok |
| Misc-C++/bigfib | 0.234 | 0.900 | 3.85× | 0.998 | 4.26× | ok |
| Misc-C++/oopack_v1p8 | 0.055 | 0.285 | 5.18× | 0.304 | 5.53× | ok |
| Misc-C++/stepanov_container | 0.324 | 1.405 | 4.34× | 1.712 | 5.28× | ok |
| Misc-C++/stepanov_v1p2 | 0.056 | 0.321 | 5.73× | 0.590 | 10.54× | ok |
| PAQ8p | 0.398 | 6.550 | 16.46× | 2.620 | 6.58× | ok |
| Prolangs-C++/life | 0.016 | 0.152 | 9.50× | 0.105 | 6.56× | ok |
| Prolangs-C++/ocean | 0.027 | 0.302 | 11.19× | 0.183 | 6.78× | ok |
| Prolangs-C++/primes | 0.010 | 0.054 | 5.40× | 0.057 | 5.70× | ok |
| Shootout-C++-EH/except | 0.162 | 0.308 | 1.90× | 0.433 | 2.67× | ok |
| Shootout-C++/ackermann | 0.117 | 0.313 | 2.68× | 0.434 | 3.71× | ok |
| Shootout-C++/ary3 | 0.140 | 0.376 | 2.69× | 0.568 | 4.06× | ok |
| Shootout-C++/fibo | 0.117 | 0.311 | 2.66× | 0.412 | 3.52× | ok |
| Shootout-C++/hash | 0.195 | 0.632 | 3.24× | 0.804 | 4.12× | ok |
| Shootout-C++/hash2 | 0.207 | 0.734 | 3.55× | 0.899 | 4.34× | ok |
| Shootout-C++/heapsort | 0.140 | 0.351 | 2.51× | 0.540 | 3.86× | ok |
| Shootout-C++/lists | 0.134 | 0.421 | 3.14× | 0.612 | 4.57× | ok |
| Shootout-C++/lists1 | 0.188 | 0.431 | 2.29× | 0.727 | 3.87× | ok |
| Shootout-C++/matrix | 0.116 | 0.349 | 3.01× | 0.546 | 4.71× | ok |
| Shootout-C++/methcall | 0.132 | 0.313 | 2.37× | 0.440 | 3.33× | ok |
| Shootout-C++/moments | 0.182 | 0.518 | 2.85× | 0.666 | 3.66× | ok |
| Shootout-C++/random | 0.138 | 0.360 | 2.61× | 0.531 | 3.85× | ok |
| Shootout-C++/sieve | 0.154 | 0.473 | 3.07× | 0.614 | 3.99× | ok |
| SmallPT | 0.083 | 0.421 | 5.07× | 0.278 | 3.35× | ok |
| Stanford/Bubblesort | 0.020 | 0.082 | 4.10× | 0.082 | 4.10× | ok |
| Stanford/FloatMM | 0.018 | 0.094 | 5.22× | 0.103 | 5.72× | ok |
| Stanford/Puzzle | 0.023 | 0.176 | 7.65× | 0.177 | 7.70× | ok |
| Stanford/Treesort | 0.020 | 0.121 | 6.05× | 0.089 | 4.45× | ok |
| bg/binarytrees/2 | 0.115 | 0.646 | 5.62× | 0.440 | 3.83× | ok |
| bg/fannkuchredux/3 | 0.062 | 0.192 | 3.10× | 0.211 | 3.40× | ok |
| bg/fasta/1 | 0.090 | 0.258 | 2.87× | 0.342 | 3.80× | ok |
| bg/fasta/2 | 0.093 | 0.262 | 2.82× | 0.433 | 4.66× | ok |
| bg/fasta/3 | 0.140 | 0.417 | 2.98× | 0.528 | 3.77× | ok |
| bg/fasta/4 | 0.138 | 0.308 | 2.23× | 0.482 | 3.49× | ok |
| bg/fasta/6 | 0.241 | 0.781 | 3.24× | 0.965 | 4.00× | ok |
| bg/mandelbrot/0 | 0.318 | 1.074 | 3.38× | 1.265 | 3.98× | ok |
| bg/mandelbrot/2 | 0.148 | 0.479 | 3.24× | 0.628 | 4.24× | ok |
| bg/mandelbrot/3 | 0.148 | 0.378 | 2.55× | 0.739 | 4.99× | ok |
| bg/mandelbrot/5 | 0.077 | 0.231 | 3.00× | 0.276 | 3.58× | ok |
| bg/mandelbrot/7 | 0.084 | 0.312 | 3.71× | 0.320 | 3.81× | ok |
| bg/mandelbrot/9 | 0.081 | 0.276 | 3.41× | 0.337 | 4.16× | ok |
| bg/nbody/1 | 0.059 | 0.238 | 4.03× | 0.302 | 5.12× | ok |
| bg/nbody/2 | 0.047 | 0.184 | 3.91× | 0.216 | 4.60× | ok |
| bg/nbody/3 | 0.044 | 0.176 | 4.00× | 0.242 | 5.50× | ok |
| bg/nbody/6 | 0.057 | 0.247 | 4.33× | 0.285 | 5.00× | ok |
| bg/pidigits/4 | 0.189 | 0.554 | 2.93× | 1.143 | 6.05× | ok |
| bg/pidigits/5 | 0.121 | 0.322 | 2.66× | 0.480 | 3.97× | ok |
| bg/revcomp/1 | 0.144 | 0.409 | 2.84× | 0.519 | 3.60× | ok |
| bg/revcomp/5 | 0.131 | 0.377 | 2.88× | 0.678 | 5.18× | ok |
| bg/spectralnorm/1 | 0.321 | 0.527 | 1.64× | 0.813 | 2.53× | ok |
| box2d | 3.061 | 14.478 | 4.73× | 11.726 | 3.83× | ok |
| fmt | 1.615 | 12.263 | 7.59× | 6.836 | 4.23× | ok |
| hexxagon | 0.436 | 1.680 | 3.85× | 1.919 | 4.40× | ok |
| lambda | 0.241 | 2.847 | 11.81× | 0.884 | 3.67× | ok |
| minisat | 0.239 | 1.551 | 6.49× | 1.022 | 4.28× | ok |
| nlohmann-json | 1.239 | 8.025 | 6.48× | 4.659 | 3.76× | ok |
| pugixml | 1.066 | 11.568 | 10.85× | 5.738 | 5.38× | ok |
| pugixml-tests | 10.766 | 53.047 | 4.93× | 39.687 | 3.69× | ok |
| rapidjson | 0.319 | 2.151 | 6.74× | 1.277 | 4.00× | ok |
| re2 | 6.962 | 26.335 | 3.78× | 25.740 | 3.70× | ok |
| re2-test-re2 | 8.624 | 43.630 | 5.06× | 36.654 | 4.25× | ok |
| re2-test-search | 10.531 | 40.436 | 3.84× | 36.946 | 3.51× | ok |
| tramp3d-v4 | 6.029 | 39.029 | 6.47× | 45.379 | 7.53× | ok |

### Frontend only: -fsyntax-only

| Program | cppgm++ (s) | GCC (s) | Speed vs GCC ↑ | Clang (s) | Speed vs Clang ↑ | Status |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| Adobe-C++/functionobjects | 0.105 | 0.212 | 2.02× | 0.483 | 4.60× | ok |
| Adobe-C++/loop_unroll | 0.058 | 0.169 | 2.91× | 0.250 | 4.31× | ok |
| Adobe-C++/simple_types_constant_folding | 0.060 | 0.167 | 2.78× | 0.346 | 5.77× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.069 | 0.146 | 2.12× | 0.391 | 5.67× | ok |
| Adobe-C++/stepanov_abstraction | 0.051 | 0.115 | 2.25× | 0.196 | 3.84× | ok |
| Adobe-C++/stepanov_vector | 0.099 | 0.242 | 2.44× | 0.363 | 3.67× | ok |
| Bullet | 6.879 | 17.488 | 2.54× | 35.170 | 5.11× | ok |
| CLAMR | 3.415 | 8.313 | 2.43× | 13.276 | 3.89× | ok |
| CoyoteBench/almabench | 0.043 | 0.102 | 2.37× | 0.199 | 4.63× | ok |
| CoyoteBench/fftbench | 0.164 | 0.462 | 2.82× | 0.627 | 3.82× | ok |
| CoyoteBench/huffbench | 0.044 | 0.110 | 2.50× | 0.156 | 3.55× | ok |
| CoyoteBench/lpbench | 0.052 | 0.112 | 2.15× | 0.164 | 3.15× | ok |
| DOE/HACCKernels | 0.168 | 0.392 | 2.33× | 0.628 | 3.74× | ok |
| DOE/HPCCG | 1.066 | 2.385 | 2.24× | 3.802 | 3.57× | ok |
| DOE/PENNANT | 2.321 | 4.846 | 2.09× | 7.773 | 3.35× | ok |
| DOE/miniFE | 0.871 | 2.257 | 2.59× | 4.605 | 5.29× | ok |
| Misc-C++-EH/spirit | 0.494 | 1.630 | 3.30× | 2.872 | 5.81× | ok |
| Misc-C++-Large/ray | 0.152 | 0.440 | 2.89× | 0.691 | 4.55× | ok |
| Misc-C++-Large/sphereflake | 0.147 | 0.318 | 2.16× | 0.503 | 3.42× | ok |
| Misc-C++/bigfib | 0.169 | 0.420 | 2.49× | 0.633 | 3.75× | ok |
| Misc-C++/oopack_v1p8 | 0.044 | 0.095 | 2.16× | 0.274 | 6.23× | ok |
| Misc-C++/stepanov_container | 0.289 | 0.554 | 1.92× | 0.814 | 2.82× | ok |
| Misc-C++/stepanov_v1p2 | 0.043 | 0.100 | 2.33× | 0.176 | 4.09× | ok |
| PAQ8p | 0.075 | 0.257 | 3.43× | 0.266 | 3.55× | ok |
| Prolangs-C++/life | 0.006 | 0.016 | — | 0.033 | — | ok |
| Prolangs-C++/ocean | 0.014 | 0.035 | 2.50× | 0.056 | 4.00× | ok |
| Prolangs-C++/primes | 0.009 | 0.027 | — | 0.049 | — | ok |
| Shootout-C++-EH/except | 0.112 | 0.263 | 2.35× | 0.412 | 3.68× | ok |
| Shootout-C++/ackermann | 0.113 | 0.293 | 2.59× | 0.396 | 3.50× | ok |
| Shootout-C++/ary3 | 0.131 | 0.354 | 2.70× | 0.464 | 3.54× | ok |
| Shootout-C++/fibo | 0.116 | 0.260 | 2.24× | 0.409 | 3.53× | ok |
| Shootout-C++/hash | 0.159 | 0.356 | 2.24× | 0.541 | 3.40× | ok |
| Shootout-C++/hash2 | 0.166 | 0.367 | 2.21× | 0.549 | 3.31× | ok |
| Shootout-C++/heapsort | 0.137 | 0.304 | 2.22× | 0.498 | 3.64× | ok |
| Shootout-C++/lists | 0.126 | 0.296 | 2.35× | 0.436 | 3.46× | ok |
| Shootout-C++/lists1 | 0.142 | 0.311 | 2.19× | 0.490 | 3.45× | ok |
| Shootout-C++/matrix | 0.116 | 0.264 | 2.28× | 0.393 | 3.39× | ok |
| Shootout-C++/methcall | 0.114 | 0.259 | 2.27× | 0.394 | 3.46× | ok |
| Shootout-C++/moments | 0.149 | 0.330 | 2.21× | 0.557 | 3.74× | ok |
| Shootout-C++/random | 0.133 | 0.321 | 2.41× | 0.483 | 3.63× | ok |
| Shootout-C++/sieve | 0.131 | 0.328 | 2.50× | 0.491 | 3.75× | ok |
| SmallPT | 0.039 | 0.102 | 2.62× | 0.164 | 4.21× | ok |
| Stanford/Bubblesort | 0.013 | 0.038 | 2.92× | 0.060 | 4.62× | ok |
| Stanford/FloatMM | 0.013 | 0.039 | 3.00× | 0.058 | 4.46× | ok |
| Stanford/Puzzle | 0.014 | 0.038 | 2.71× | 0.055 | 3.93× | ok |
| Stanford/Treesort | 0.015 | 0.039 | 2.60× | 0.056 | 3.73× | ok |
| bg/binarytrees/2 | 0.112 | 0.256 | 2.29× | 0.415 | 3.71× | ok |
| bg/fannkuchredux/3 | 0.056 | 0.102 | 1.82× | 0.164 | 2.93× | ok |
| bg/fasta/1 | 0.082 | 0.162 | 1.98× | 0.260 | 3.17× | ok |
| bg/fasta/2 | 0.086 | 0.175 | 2.03× | 0.272 | 3.16× | ok |
| bg/fasta/3 | 0.166 | 0.285 | 1.72× | 0.535 | 3.22× | ok |
| bg/fasta/4 | 0.125 | 0.255 | 2.04× | 0.405 | 3.24× | ok |
| bg/fasta/6 | 0.206 | 0.468 | 2.27× | 0.796 | 3.86× | ok |
| bg/mandelbrot/0 | 0.211 | 0.507 | 2.40× | 0.781 | 3.70× | ok |
| bg/mandelbrot/2 | 0.139 | 0.340 | 2.45× | 0.509 | 3.66× | ok |
| bg/mandelbrot/3 | 0.216 | 0.326 | 1.51× | 0.535 | 2.48× | ok |
| bg/mandelbrot/5 | 0.067 | 0.147 | 2.19× | 0.230 | 3.43× | ok |
| bg/mandelbrot/7 | 0.070 | 0.150 | 2.14× | 0.241 | 3.44× | ok |
| bg/mandelbrot/9 | 0.081 | 0.139 | 1.72× | 0.223 | 2.75× | ok |
| bg/nbody/1 | 0.041 | 0.094 | 2.29× | 0.163 | 3.98× | ok |
| bg/nbody/2 | 0.063 | 0.083 | 1.32× | 0.148 | 2.35× | ok |
| bg/nbody/3 | 0.041 | 0.082 | 2.00× | 0.147 | 3.59× | ok |
| bg/nbody/6 | 0.041 | 0.087 | 2.12× | 0.149 | 3.63× | ok |
| bg/pidigits/4 | 0.179 | 0.468 | 2.61× | 0.693 | 3.87× | ok |
| bg/pidigits/5 | 0.118 | 0.261 | 2.21× | 0.432 | 3.66× | ok |
| bg/revcomp/1 | 0.125 | 0.277 | 2.22× | 0.425 | 3.40× | ok |
| bg/revcomp/5 | 0.113 | 0.258 | 2.28× | 0.767 | 6.79× | ok |
| bg/spectralnorm/1 | 0.168 | 0.379 | 2.26× | 0.801 | 4.77× | ok |
| box2d | 2.262 | 4.591 | 2.03× | 8.098 | 3.58× | ok |
| fmt | 0.739 | 1.682 | 2.28× | 3.121 | 4.22× | ok |
| hexxagon | 0.400 | 0.906 | 2.26× | 1.340 | 3.35× | ok |
| lambda | 0.087 | 0.196 | 2.25× | 0.458 | 5.26× | ok |
| minisat | 0.092 | 0.196 | 2.13× | 0.457 | 4.97× | ok |
| nlohmann-json | 0.512 | 1.618 | 3.16× | 2.272 | 4.44× | ok |
| pugixml | 0.320 | 1.098 | 3.43× | 2.232 | 6.98× | ok |
| pugixml-tests | 5.168 | 16.566 | 3.21× | 17.604 | 3.41× | ok |
| rapidjson | 0.241 | 0.607 | 2.52× | 0.501 | 2.08× | ok |
| re2 | 4.708 | 13.443 | 2.86× | 15.979 | 3.39× | ok |
| re2-test-re2 | 6.547 | 17.489 | 2.67× | 23.605 | 3.61× | ok |
| re2-test-search | 6.611 | 15.057 | 2.28× | 23.007 | 3.48× | ok |
| tramp3d-v4 | 0.899 | 3.260 | 3.63× | 7.542 | 8.39× | ok |

### Building the compiler

| Compiler doing the rebuild | CPU seconds |
| --- | ---: |
| GCC | 491.330 |
| GCC-built cppgm++ | 101.010 |
| Self-built cppgm++ | 102.860 |

These are the initial bootstrap builds. The GCC rebuild is measured once.

#### Paired self-built compiler measurements

| Block | GCC-built cppgm++ (s) | Self-built cppgm++ (s) | Self-built speed ↑ |
| ---: | ---: | ---: | ---: |
| 1 | 100.620 | 101.939 | 0.99× |
| 2 | 105.952 | 104.697 | 1.01× |

Each block averages its two observations per compiler. The badge uses the median block ratio; calibration and every observation are retained in the raw results.

## Toolchains

- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)
- **valgrind**: valgrind-3.26.0
