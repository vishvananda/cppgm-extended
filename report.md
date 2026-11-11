# Public benchmarks: AMD EPYC

Measured compiler: [`a0ff7d04a`](https://github.com/vishvananda/cppgm-extended/commit/a0ff7d04a7ed4d67461b0912fca4a4fa7e055d74) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/42974658099) · [Raw program results](results.json)

81 of 81 programs built with cppgm++ and g++ and agreed. Each number is the reference over cppgm++, so above 1× means cppgm++ is smaller or faster.

CPU: AMD EPYC 7763 64-Core Processor.

| Measure | vs g++ | Programs | vs clang++ | Programs |
| --- | ---: | ---: | ---: | ---: |
| Frontend CPU, `-fsyntax-only` | 2.06× | 80 | 3.03× | 80 |
| Compile CPU, `-O0` | 2.73× | 79 | 3.20× | 79 |
| Compile CPU, `-O3` | 4.10× | 81 | 4.10× | 81 |
| Compile peak RSS, `-O0` | 1.50× | 81 | 2.41× | 81 |
| Compile peak RSS, `-O3` | 1.78× | 81 | 2.52× | 81 |
| Executable size, `-O0` | 0.93× | 81 | 0.87× | 81 |
| Executable size, `-O3` | 0.93× | 81 | 0.89× | 81 |
| Run CPU, `-O0` | 1.19× | 81 | 1.21× | 81 |
| Run CPU, `-O3` | 0.96× | 81 | 0.95× | 80 |
| Run instructions, `-O3` | 0.93× | 43 | - | 0 |

O3 execution uses paired ABBA/BAAB blocks. GCC A/A CPU ratios ranged from 0.967 to 1.016; raw observations and block ratios are retained in the artifact.

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
| Adobe-C++/functionobjects | 0.116 | 0.407 | 3.51× | 0.369 | 3.18× | ok |
| Adobe-C++/loop_unroll | 0.099 | 0.838 | 8.46× | 0.361 | 3.65× | ok |
| Adobe-C++/simple_types_constant_folding | 0.125 | 0.954 | 7.63× | 0.449 | 3.59× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.085 | 0.699 | 8.22× | 0.365 | 4.29× | ok |
| Adobe-C++/stepanov_abstraction | 0.064 | 0.260 | 4.06× | 0.205 | 3.20× | ok |
| Adobe-C++/stepanov_vector | 0.145 | 0.424 | 2.92× | 0.474 | 3.27× | ok |
| Bullet | 8.188 | 26.583 | 3.25× | 27.860 | 3.40× | ok |
| CLAMR | 3.908 | 12.937 | 3.31× | 13.684 | 3.50× | ok |
| CoyoteBench/almabench | 0.049 | 0.126 | 2.57× | 0.156 | 3.18× | ok |
| CoyoteBench/fftbench | 0.175 | 0.369 | 2.11× | 0.752 | 4.30× | ok |
| CoyoteBench/huffbench | 0.062 | 0.114 | 1.84× | 0.207 | 3.34× | ok |
| CoyoteBench/lpbench | 0.054 | 0.111 | 2.06× | 0.134 | 2.48× | ok |
| DOE/HACCKernels | 0.185 | 0.392 | 2.12× | 0.547 | 2.96× | ok |
| DOE/HPCCG | 1.092 | 2.533 | 2.32× | 3.351 | 3.07× | ok |
| DOE/PENNANT | 2.475 | 6.130 | 2.48× | 7.264 | 2.93× | ok |
| DOE/miniFE | 1.191 | 3.348 | 2.81× | 3.379 | 2.84× | ok |
| Misc-C++-EH/spirit | 0.826 | 3.917 | 4.74× | 3.595 | 4.35× | ok |
| Misc-C++-Large/ray | 0.170 | 0.464 | 2.73× | 0.741 | 4.36× | ok |
| Misc-C++-Large/sphereflake | 0.169 | 0.320 | 1.89× | 0.618 | 3.66× | ok |
| Misc-C++/bigfib | 0.188 | 0.533 | 2.84× | 0.619 | 3.29× | ok |
| Misc-C++/oopack_v1p8 | 0.051 | 0.131 | 2.57× | 0.197 | 3.86× | ok |
| Misc-C++/stepanov_container | 0.320 | 0.832 | 2.60× | 1.152 | 3.60× | ok |
| Misc-C++/stepanov_v1p2 | 0.050 | 0.179 | 3.58× | 0.159 | 3.18× | ok |
| PAQ8p | 0.132 | 0.713 | 5.40× | 0.506 | 3.83× | ok |
| Prolangs-C++/life | 0.006 | 0.046 | — | 0.040 | — | ok |
| Prolangs-C++/ocean | 0.018 | 0.093 | 5.17× | 0.075 | 4.17× | ok |
| Prolangs-C++/primes | 0.009 | 0.035 | — | 0.039 | — | ok |
| Shootout-C++-EH/except | 0.133 | 0.259 | 1.95× | 0.424 | 3.19× | ok |
| Shootout-C++/ackermann | 0.122 | 0.306 | 2.51× | 0.351 | 2.88× | ok |
| Shootout-C++/ary3 | 0.155 | 0.286 | 1.85× | 0.439 | 2.83× | ok |
| Shootout-C++/fibo | 0.121 | 0.225 | 1.86× | 0.342 | 2.83× | ok |
| Shootout-C++/hash | 0.181 | 0.389 | 2.15× | 0.738 | 4.08× | ok |
| Shootout-C++/hash2 | 0.171 | 0.418 | 2.44× | 0.522 | 3.05× | ok |
| Shootout-C++/heapsort | 0.149 | 0.365 | 2.45× | 0.427 | 2.87× | ok |
| Shootout-C++/lists | 0.134 | 0.319 | 2.38× | 0.433 | 3.23× | ok |
| Shootout-C++/lists1 | 0.152 | 0.330 | 2.17× | 0.441 | 2.90× | ok |
| Shootout-C++/matrix | 0.116 | 0.229 | 1.97× | 0.361 | 3.11× | ok |
| Shootout-C++/methcall | 0.117 | 0.237 | 2.03× | 0.482 | 4.12× | ok |
| Shootout-C++/moments | 0.167 | 0.375 | 2.25× | 0.462 | 2.77× | ok |
| Shootout-C++/random | 0.148 | 0.262 | 1.77× | 0.422 | 2.85× | ok |
| Shootout-C++/sieve | 0.158 | 0.376 | 2.38× | 0.471 | 2.98× | ok |
| SmallPT | 0.053 | 0.130 | 2.45× | 0.156 | 2.94× | ok |
| Stanford/Bubblesort | 0.015 | 0.039 | 2.60× | 0.046 | 3.07× | ok |
| Stanford/FloatMM | 0.015 | 0.041 | 2.73× | 0.046 | 3.07× | ok |
| Stanford/Puzzle | 0.017 | 0.053 | 3.12× | 0.050 | 2.94× | ok |
| Stanford/Treesort | 0.015 | 0.043 | 2.87× | 0.049 | 3.27× | ok |
| bg/binarytrees/2 | 0.122 | 0.234 | 1.92× | 0.372 | 3.05× | ok |
| bg/fannkuchredux/3 | 0.059 | 0.110 | 1.86× | 0.149 | 2.53× | ok |
| bg/fasta/1 | 0.090 | 0.189 | 2.10× | 0.230 | 2.56× | ok |
| bg/fasta/2 | 0.092 | 0.186 | 2.02× | 0.241 | 2.62× | ok |
| bg/fasta/3 | 0.142 | 0.275 | 1.94× | 0.402 | 2.83× | ok |
| bg/fasta/4 | 0.166 | 0.229 | 1.38× | 0.405 | 2.44× | ok |
| bg/fasta/6 | 0.227 | 0.670 | 2.95× | 0.727 | 3.20× | ok |
| bg/mandelbrot/0 | 0.249 | 0.757 | 3.04× | 0.766 | 3.08× | ok |
| bg/mandelbrot/2 | 0.149 | 0.366 | 2.46× | 0.494 | 3.32× | ok |
| bg/mandelbrot/3 | 0.158 | 0.323 | 2.04× | 0.463 | 2.93× | ok |
| bg/mandelbrot/5 | 0.078 | 0.192 | 2.46× | 0.217 | 2.78× | ok |
| bg/mandelbrot/7 | 0.090 | 0.221 | 2.46× | 0.238 | 2.64× | ok |
| bg/mandelbrot/9 | 0.073 | 0.176 | 2.41× | 0.199 | 2.73× | ok |
| bg/nbody/1 | 0.055 | 0.146 | 2.65× | 0.155 | 2.82× | ok |
| bg/nbody/2 | 0.058 | 0.109 | 1.88× | 0.150 | 2.59× | ok |
| bg/nbody/3 | 0.059 | 0.103 | 1.75× | 0.159 | 2.69× | ok |
| bg/nbody/6 | 0.049 | 0.142 | 2.90× | 0.185 | 3.78× | ok |
| bg/pidigits/4 | 0.224 | 0.478 | 2.13× | 0.854 | 3.81× | ok |
| bg/pidigits/5 | 0.144 | 0.272 | 1.89× | 0.396 | 2.75× | ok |
| bg/revcomp/1 | 0.146 | 0.328 | 2.25× | 0.507 | 3.47× | ok |
| bg/revcomp/5 | 0.157 | 0.377 | 2.40× | 0.400 | 2.55× | ok |
| bg/spectralnorm/1 | 0.215 | 0.375 | 1.74× | 0.562 | 2.61× | ok |
| box2d | 2.818 | 6.995 | 2.48× | 10.892 | 3.87× | ok |
| fmt | 0.967 | 3.481 | 3.60× | 3.550 | 3.67× | ok |
| hexxagon | 0.426 | 1.054 | 2.47× | 1.288 | 3.02× | ok |
| lambda | 0.083 | 0.446 | 5.37× | 0.329 | 3.96× | ok |
| minisat | 0.108 | 0.401 | 3.71× | 0.403 | 3.73× | ok |
| nlohmann-json | 0.680 | 2.888 | 4.25× | 2.646 | 3.89× | ok |
| pugixml | 0.426 | 2.170 | 5.09× | 1.953 | 4.58× | ok |
| pugixml-tests | 6.355 | 22.517 | 3.54× | 24.798 | 3.90× | ok |
| rapidjson | 0.214 | 0.723 | 3.38× | 0.752 | 3.51× | ok |
| re2 | 6.696 | 16.254 | 2.43× | 11.954 | 1.79× | ok |
| re2-test-re2 | 8.976 | 24.243 | 2.70× | 28.552 | 3.18× | ok |
| re2-test-search | 8.195 | 23.048 | 2.81× | 23.275 | 2.84× | ok |
| tramp3d-v4 | 1.061 | 12.656 | 11.93× | 4.599 | 4.33× | ok |

### O3 compilation

| Program | cppgm++ (s) | GCC (s) | Speed vs GCC ↑ | Clang (s) | Speed vs Clang ↑ | Status |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| Adobe-C++/functionobjects | 0.175 | 1.410 | 8.06× | 0.992 | 5.67× | ok |
| Adobe-C++/loop_unroll | 0.395 | 7.874 | 19.93× | 6.281 | 15.90× | ok |
| Adobe-C++/simple_types_constant_folding | 0.276 | 3.782 | 13.70× | 3.313 | 12.00× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.217 | 3.215 | 14.82× | 2.864 | 13.20× | ok |
| Adobe-C++/stepanov_abstraction | 0.139 | 1.433 | 10.31× | 0.895 | 6.44× | ok |
| Adobe-C++/stepanov_vector | 0.166 | 2.173 | 13.09× | 0.899 | 5.42× | ok |
| Bullet | 10.363 | 54.500 | 5.26× | 42.001 | 4.05× | ok |
| CLAMR | 7.057 | 34.521 | 4.89× | 26.692 | 3.78× | ok |
| CoyoteBench/almabench | 0.052 | 0.238 | 4.58× | 0.237 | 4.56× | ok |
| CoyoteBench/fftbench | 0.189 | 0.595 | 3.15× | 0.967 | 5.12× | ok |
| CoyoteBench/huffbench | 0.072 | 0.366 | 5.08× | 0.206 | 2.86× | ok |
| CoyoteBench/lpbench | 0.057 | 0.310 | 5.44× | 0.277 | 4.86× | ok |
| DOE/HACCKernels | 0.197 | 0.540 | 2.74× | 0.638 | 3.24× | ok |
| DOE/HPCCG | 1.147 | 3.503 | 3.05× | 3.805 | 3.32× | ok |
| DOE/PENNANT | 2.763 | 8.556 | 3.10× | 11.374 | 4.12× | ok |
| DOE/miniFE | 1.422 | 5.913 | 4.16× | 5.444 | 3.83× | ok |
| Misc-C++-EH/spirit | 0.971 | 5.602 | 5.77× | 5.274 | 5.43× | ok |
| Misc-C++-Large/ray | 0.199 | 0.673 | 3.38× | 0.606 | 3.05× | ok |
| Misc-C++-Large/sphereflake | 0.205 | 1.129 | 5.51× | 0.636 | 3.10× | ok |
| Misc-C++/bigfib | 0.222 | 0.755 | 3.40× | 0.797 | 3.59× | ok |
| Misc-C++/oopack_v1p8 | 0.058 | 0.241 | 4.16× | 0.306 | 5.28× | ok |
| Misc-C++/stepanov_container | 0.357 | 1.242 | 3.48× | 1.738 | 4.87× | ok |
| Misc-C++/stepanov_v1p2 | 0.061 | 0.284 | 4.66× | 0.281 | 4.61× | ok |
| PAQ8p | 0.402 | 5.811 | 14.46× | 2.866 | 7.13× | ok |
| Prolangs-C++/life | 0.011 | 0.148 | 13.45× | 0.103 | 9.36× | ok |
| Prolangs-C++/ocean | 0.030 | 0.282 | 9.40× | 0.173 | 5.77× | ok |
| Prolangs-C++/primes | 0.011 | 0.052 | 4.73× | 0.051 | 4.64× | ok |
| Shootout-C++-EH/except | 0.136 | 0.304 | 2.24× | 0.451 | 3.32× | ok |
| Shootout-C++/ackermann | 0.126 | 0.350 | 2.78× | 0.365 | 2.90× | ok |
| Shootout-C++/ary3 | 0.150 | 0.393 | 2.62× | 0.443 | 2.95× | ok |
| Shootout-C++/fibo | 0.113 | 0.288 | 2.55× | 0.353 | 3.12× | ok |
| Shootout-C++/hash | 0.194 | 0.514 | 2.65× | 0.915 | 4.72× | ok |
| Shootout-C++/hash2 | 0.194 | 0.603 | 3.11× | 0.699 | 3.60× | ok |
| Shootout-C++/heapsort | 0.153 | 0.325 | 2.12× | 0.448 | 2.93× | ok |
| Shootout-C++/lists | 0.153 | 0.360 | 2.35× | 0.441 | 2.88× | ok |
| Shootout-C++/lists1 | 0.158 | 0.390 | 2.47× | 0.495 | 3.13× | ok |
| Shootout-C++/matrix | 0.122 | 0.306 | 2.51× | 0.490 | 4.02× | ok |
| Shootout-C++/methcall | 0.155 | 0.274 | 1.77× | 0.403 | 2.60× | ok |
| Shootout-C++/moments | 0.177 | 0.489 | 2.76× | 0.794 | 4.49× | ok |
| Shootout-C++/random | 0.143 | 0.300 | 2.10× | 0.445 | 3.11× | ok |
| Shootout-C++/sieve | 0.163 | 0.395 | 2.42× | 0.524 | 3.21× | ok |
| SmallPT | 0.061 | 0.437 | 7.16× | 0.261 | 4.28× | ok |
| Stanford/Bubblesort | 0.018 | 0.078 | 4.33× | 0.069 | 3.83× | ok |
| Stanford/FloatMM | 0.020 | 0.087 | 4.35× | 0.099 | 4.95× | ok |
| Stanford/Puzzle | 0.025 | 0.177 | 7.08× | 0.170 | 6.80× | ok |
| Stanford/Treesort | 0.021 | 0.119 | 5.67× | 0.076 | 3.62× | ok |
| bg/binarytrees/2 | 0.128 | 0.596 | 4.66× | 0.388 | 3.03× | ok |
| bg/fannkuchredux/3 | 0.066 | 0.175 | 2.65× | 0.189 | 2.86× | ok |
| bg/fasta/1 | 0.097 | 0.233 | 2.40× | 0.301 | 3.10× | ok |
| bg/fasta/2 | 0.099 | 0.239 | 2.41× | 0.297 | 3.00× | ok |
| bg/fasta/3 | 0.150 | 0.333 | 2.22× | 0.452 | 3.01× | ok |
| bg/fasta/4 | 0.138 | 0.274 | 1.99× | 0.379 | 2.75× | ok |
| bg/fasta/6 | 0.240 | 0.662 | 2.76× | 0.816 | 3.40× | ok |
| bg/mandelbrot/0 | 0.280 | 1.132 | 4.04× | 1.128 | 4.03× | ok |
| bg/mandelbrot/2 | 0.159 | 0.457 | 2.87× | 0.582 | 3.66× | ok |
| bg/mandelbrot/3 | 0.161 | 0.365 | 2.27× | 0.552 | 3.43× | ok |
| bg/mandelbrot/5 | 0.094 | 0.255 | 2.71× | 0.255 | 2.71× | ok |
| bg/mandelbrot/7 | 0.093 | 0.287 | 3.09× | 0.304 | 3.27× | ok |
| bg/mandelbrot/9 | 0.083 | 0.259 | 3.12× | 0.260 | 3.13× | ok |
| bg/nbody/1 | 0.069 | 0.222 | 3.22× | 0.266 | 3.86× | ok |
| bg/nbody/2 | 0.050 | 0.194 | 3.88× | 0.187 | 3.74× | ok |
| bg/nbody/3 | 0.062 | 0.178 | 2.87× | 0.192 | 3.10× | ok |
| bg/nbody/6 | 0.062 | 0.223 | 3.60× | 0.280 | 4.52× | ok |
| bg/pidigits/4 | 0.222 | 0.513 | 2.31× | 0.727 | 3.27× | ok |
| bg/pidigits/5 | 0.160 | 0.307 | 1.92× | 0.573 | 3.58× | ok |
| bg/revcomp/1 | 0.153 | 0.367 | 2.40× | 0.585 | 3.82× | ok |
| bg/revcomp/5 | 0.152 | 0.386 | 2.54× | 0.444 | 2.92× | ok |
| bg/spectralnorm/1 | 0.226 | 0.503 | 2.23× | 0.719 | 3.18× | ok |
| box2d | 3.164 | 11.268 | 3.56× | 12.698 | 4.01× | ok |
| fmt | 1.419 | 8.335 | 5.87× | 6.448 | 4.54× | ok |
| hexxagon | 0.451 | 1.683 | 3.73× | 1.650 | 3.66× | ok |
| lambda | 0.146 | 1.708 | 11.70× | 0.754 | 5.16× | ok |
| minisat | 0.196 | 1.534 | 7.83× | 0.941 | 4.80× | ok |
| nlohmann-json | 1.092 | 5.221 | 4.78× | 5.231 | 4.79× | ok |
| pugixml | 0.992 | 8.378 | 8.45× | 6.019 | 6.07× | ok |
| pugixml-tests | 8.684 | 51.887 | 5.98× | 42.203 | 4.86× | ok |
| rapidjson | 0.314 | 1.765 | 5.62× | 1.148 | 3.66× | ok |
| re2 | 4.961 | 26.645 | 5.37× | 16.629 | 3.35× | ok |
| re2-test-re2 | 10.166 | 39.533 | 3.89× | 39.749 | 3.91× | ok |
| re2-test-search | 10.472 | 37.586 | 3.59× | 28.336 | 2.71× | ok |
| tramp3d-v4 | 2.436 | 23.233 | 9.54× | 20.216 | 8.30× | ok |

### Frontend only: -fsyntax-only

| Program | cppgm++ (s) | GCC (s) | Speed vs GCC ↑ | Clang (s) | Speed vs Clang ↑ | Status |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| Adobe-C++/functionobjects | 0.108 | 0.232 | 2.15× | 0.299 | 2.77× | ok |
| Adobe-C++/loop_unroll | 0.069 | 0.175 | 2.54× | 0.206 | 2.99× | ok |
| Adobe-C++/simple_types_constant_folding | 0.082 | 0.183 | 2.23× | 0.227 | 2.77× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.063 | 0.166 | 2.63× | 0.199 | 3.16× | ok |
| Adobe-C++/stepanov_abstraction | 0.054 | 0.109 | 2.02× | 0.160 | 2.96× | ok |
| Adobe-C++/stepanov_vector | 0.122 | 0.203 | 1.66× | 0.415 | 3.40× | ok |
| Bullet | 7.877 | 16.551 | 2.10× | 24.995 | 3.17× | ok |
| CLAMR | 3.486 | 7.525 | 2.16× | 11.826 | 3.39× | ok |
| CoyoteBench/almabench | 0.050 | 0.108 | 2.16× | 0.145 | 2.90× | ok |
| CoyoteBench/fftbench | 0.173 | 0.332 | 1.92× | 0.687 | 3.97× | ok |
| CoyoteBench/huffbench | 0.059 | 0.101 | 1.71× | 0.183 | 3.10× | ok |
| CoyoteBench/lpbench | 0.053 | 0.121 | 2.28× | 0.127 | 2.40× | ok |
| DOE/HACCKernels | 0.180 | 0.352 | 1.96× | 0.526 | 2.92× | ok |
| DOE/HPCCG | 1.081 | 2.147 | 1.99× | 3.147 | 2.91× | ok |
| DOE/PENNANT | 2.547 | 4.520 | 1.77× | 6.585 | 2.59× | ok |
| DOE/miniFE | 1.087 | 1.945 | 1.79× | 2.955 | 2.72× | ok |
| Misc-C++-EH/spirit | 0.605 | 1.532 | 2.53× | 2.304 | 3.81× | ok |
| Misc-C++-Large/ray | 0.165 | 0.389 | 2.36× | 0.577 | 3.50× | ok |
| Misc-C++-Large/sphereflake | 0.158 | 0.286 | 1.81× | 0.553 | 3.50× | ok |
| Misc-C++/bigfib | 0.175 | 0.363 | 2.07× | 0.544 | 3.11× | ok |
| Misc-C++/oopack_v1p8 | 0.049 | 0.097 | 1.98× | 0.174 | 3.55× | ok |
| Misc-C++/stepanov_container | 0.291 | 0.562 | 1.93× | 1.013 | 3.48× | ok |
| Misc-C++/stepanov_v1p2 | 0.048 | 0.096 | 2.00× | 0.131 | 2.73× | ok |
| PAQ8p | 0.075 | 0.207 | 2.76× | 0.327 | 4.36× | ok |
| Prolangs-C++/life | 0.005 | 0.015 | — | 0.028 | — | ok |
| Prolangs-C++/ocean | 0.016 | 0.036 | 2.25× | 0.051 | 3.19× | ok |
| Prolangs-C++/primes | 0.010 | 0.026 | 2.60× | 0.036 | 3.60× | ok |
| Shootout-C++-EH/except | 0.132 | 0.260 | 1.97× | 0.402 | 3.05× | ok |
| Shootout-C++/ackermann | 0.120 | 0.315 | 2.62× | 0.337 | 2.81× | ok |
| Shootout-C++/ary3 | 0.151 | 0.265 | 1.75× | 0.419 | 2.77× | ok |
| Shootout-C++/fibo | 0.122 | 0.230 | 1.89× | 0.341 | 2.80× | ok |
| Shootout-C++/hash | 0.172 | 0.304 | 1.77× | 0.675 | 3.92× | ok |
| Shootout-C++/hash2 | 0.165 | 0.318 | 1.93× | 0.467 | 2.83× | ok |
| Shootout-C++/heapsort | 0.153 | 0.368 | 2.41× | 0.402 | 2.63× | ok |
| Shootout-C++/lists | 0.130 | 0.272 | 2.09× | 0.395 | 3.04× | ok |
| Shootout-C++/lists1 | 0.149 | 0.276 | 1.85× | 0.392 | 2.63× | ok |
| Shootout-C++/matrix | 0.116 | 0.231 | 1.99× | 0.349 | 3.01× | ok |
| Shootout-C++/methcall | 0.117 | 0.235 | 2.01× | 0.454 | 3.88× | ok |
| Shootout-C++/moments | 0.159 | 0.303 | 1.91× | 0.427 | 2.69× | ok |
| Shootout-C++/random | 0.149 | 0.269 | 1.81× | 0.400 | 2.68× | ok |
| Shootout-C++/sieve | 0.150 | 0.312 | 2.08× | 0.441 | 2.94× | ok |
| SmallPT | 0.051 | 0.087 | 1.71× | 0.140 | 2.75× | ok |
| Stanford/Bubblesort | 0.015 | 0.036 | 2.40× | 0.044 | 2.93× | ok |
| Stanford/FloatMM | 0.015 | 0.036 | 2.40× | 0.045 | 3.00× | ok |
| Stanford/Puzzle | 0.016 | 0.039 | 2.44× | 0.046 | 2.88× | ok |
| Stanford/Treesort | 0.015 | 0.036 | 2.40× | 0.046 | 3.07× | ok |
| bg/binarytrees/2 | 0.121 | 0.234 | 1.93× | 0.358 | 2.96× | ok |
| bg/fannkuchredux/3 | 0.059 | 0.097 | 1.64× | 0.144 | 2.44× | ok |
| bg/fasta/1 | 0.088 | 0.155 | 1.76× | 0.216 | 2.45× | ok |
| bg/fasta/2 | 0.088 | 0.153 | 1.74× | 0.222 | 2.52× | ok |
| bg/fasta/3 | 0.139 | 0.262 | 1.88× | 0.377 | 2.71× | ok |
| bg/fasta/4 | 0.152 | 0.224 | 1.47× | 0.459 | 3.02× | ok |
| bg/fasta/6 | 0.213 | 0.492 | 2.31× | 0.633 | 2.97× | ok |
| bg/mandelbrot/0 | 0.236 | 0.496 | 2.10× | 0.759 | 3.22× | ok |
| bg/mandelbrot/2 | 0.144 | 0.339 | 2.35× | 0.436 | 3.03× | ok |
| bg/mandelbrot/3 | 0.155 | 0.317 | 2.05× | 0.446 | 2.88× | ok |
| bg/mandelbrot/5 | 0.075 | 0.137 | 1.83× | 0.197 | 2.63× | ok |
| bg/mandelbrot/7 | 0.080 | 0.152 | 1.90× | 0.210 | 2.62× | ok |
| bg/mandelbrot/9 | 0.068 | 0.131 | 1.93× | 0.181 | 2.66× | ok |
| bg/nbody/1 | 0.048 | 0.088 | 1.83× | 0.157 | 3.27× | ok |
| bg/nbody/2 | 0.049 | 0.092 | 1.88× | 0.143 | 2.92× | ok |
| bg/nbody/3 | 0.054 | 0.086 | 1.59× | 0.135 | 2.50× | ok |
| bg/nbody/6 | 0.048 | 0.088 | 1.83× | 0.154 | 3.21× | ok |
| bg/pidigits/4 | 0.217 | 0.424 | 1.95× | 0.630 | 2.90× | ok |
| bg/pidigits/5 | 0.139 | 0.256 | 1.84× | 0.408 | 2.94× | ok |
| bg/revcomp/1 | 0.140 | 0.273 | 1.95× | 0.509 | 3.64× | ok |
| bg/revcomp/5 | 0.137 | 0.332 | 2.42× | 0.365 | 2.66× | ok |
| bg/spectralnorm/1 | 0.210 | 0.411 | 1.96× | 0.546 | 2.60× | ok |
| box2d | 2.488 | 4.869 | 1.96× | 9.810 | 3.94× | ok |
| fmt | 0.797 | 1.551 | 1.95× | 2.878 | 3.61× | ok |
| hexxagon | 0.417 | 0.854 | 2.05× | 1.224 | 2.94× | ok |
| lambda | 0.069 | 0.194 | 2.81× | 0.243 | 3.52× | ok |
| minisat | 0.092 | 0.190 | 2.07× | 0.265 | 2.88× | ok |
| nlohmann-json | 0.678 | 1.507 | 2.22× | 2.040 | 3.01× | ok |
| pugixml | 0.348 | 0.934 | 2.68× | 1.446 | 4.16× | ok |
| pugixml-tests | 5.283 | 12.332 | 2.33× | 19.731 | 3.73× | ok |
| rapidjson | 0.182 | 0.390 | 2.14× | 0.552 | 3.03× | ok |
| re2 | 5.688 | 10.796 | 1.90× | 10.291 | 1.81× | ok |
| re2-test-re2 | 8.138 | 16.329 | 2.01× | 25.094 | 3.08× | ok |
| re2-test-search | 7.654 | 15.308 | 2.00× | 25.038 | 3.27× | ok |
| tramp3d-v4 | 0.659 | 2.913 | 4.42× | 2.736 | 4.15× | ok |


## Toolchains

- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)
- **valgrind**: valgrind-3.26.0

[Compiler build and codegen results](self-build.md) have their own measured revision.
