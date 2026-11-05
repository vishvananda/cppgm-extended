# Public benchmarks: AMD EPYC

Measured compiler: [`b7c6b8e72`](https://github.com/vishvananda/cppgm-extended/commit/b7c6b8e72595a524e1b3f147212d69c02b257dd8) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/41932548048) · [Raw program results](results.json) · [Raw rebuild results](selfhost.json)

81 of 81 programs built with cppgm++ and g++ and agreed. Each number is the reference over cppgm++, so above 1× means cppgm++ is smaller or faster.

CPU: AMD EPYC 9V74 80-Core Processor.

| Measure | vs g++ | Programs | vs clang++ | Programs |
| --- | ---: | ---: | ---: | ---: |
| Frontend CPU, `-fsyntax-only` | 2.01× | 79 | 2.66× | 79 |
| Compile CPU, `-O0` | 2.72× | 79 | 2.90× | 79 |
| Compile CPU, `-O3` | 4.00× | 79 | 3.69× | 79 |
| Compile peak RSS, `-O0` | 1.47× | 81 | 2.24× | 81 |
| Compile peak RSS, `-O3` | 1.72× | 81 | 2.33× | 81 |
| Executable size, `-O0` | 0.93× | 81 | 0.88× | 81 |
| Executable size, `-O3` | 0.93× | 81 | 0.88× | 81 |
| Run CPU, `-O0` | 1.03× | 81 | 1.05× | 81 |
| Run CPU, `-O3` | 0.98× | 81 | 0.97× | 80 |
| Run instructions, `-O3` | 0.95× | 43 | - | 0 |
| Self compile CPU | 4.30× | - | - | - |
| Self-built compiler CPU | 0.99× | - | - | - |

O3 execution uses paired ABBA/BAAB blocks. GCC A/A CPU ratios ranged from 0.921 to 1.137; raw observations and block ratios are retained in the artifact.

Self-built / GCC-built compiler CPU: paired ratios 1.011–1.018; A/A calibration 1.004. A/A variation limits conclusions about small changes.

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
| Adobe-C++/functionobjects | 0.091 | 0.287 | 3.15× | 0.230 | 2.53× | ok |
| Adobe-C++/loop_unroll | 0.066 | 0.610 | 9.24× | 0.243 | 3.68× | ok |
| Adobe-C++/simple_types_constant_folding | 0.090 | 0.698 | 7.76× | 0.289 | 3.21× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.063 | 0.519 | 8.24× | 0.235 | 3.73× | ok |
| Adobe-C++/stepanov_abstraction | 0.057 | 0.212 | 3.72× | 0.183 | 3.21× | ok |
| Adobe-C++/stepanov_vector | 0.098 | 0.325 | 3.32× | 0.265 | 2.70× | ok |
| Bullet | 6.786 | 20.445 | 3.01× | 19.309 | 2.85× | ok |
| CLAMR | 3.193 | 9.852 | 3.09× | 9.193 | 2.88× | ok |
| CoyoteBench/almabench | 0.038 | 0.087 | 2.29× | 0.095 | 2.50× | ok |
| CoyoteBench/fftbench | 0.140 | 0.289 | 2.06× | 0.402 | 2.87× | ok |
| CoyoteBench/huffbench | 0.040 | 0.083 | 2.08× | 0.125 | 3.12× | ok |
| CoyoteBench/lpbench | 0.043 | 0.093 | 2.16× | 0.094 | 2.19× | ok |
| DOE/HACCKernels | 0.144 | 0.310 | 2.15× | 0.383 | 2.66× | ok |
| DOE/HPCCG | 0.838 | 1.848 | 2.21× | 2.383 | 2.84× | ok |
| DOE/PENNANT | 1.865 | 4.295 | 2.30× | 5.161 | 2.77× | ok |
| DOE/miniFE | 0.877 | 2.165 | 2.47× | 2.285 | 2.61× | ok |
| Misc-C++-EH/spirit | 0.489 | 3.024 | 6.18× | 2.046 | 4.18× | ok |
| Misc-C++-Large/ray | 0.132 | 0.336 | 2.55× | 0.446 | 3.38× | ok |
| Misc-C++-Large/sphereflake | 0.125 | 0.244 | 1.95× | 0.336 | 2.69× | ok |
| Misc-C++/bigfib | 0.164 | 0.488 | 2.98× | 0.567 | 3.46× | ok |
| Misc-C++/oopack_v1p8 | 0.044 | 0.111 | 2.52× | 0.117 | 2.66× | ok |
| Misc-C++/stepanov_container | 0.223 | 0.663 | 2.97× | 0.717 | 3.22× | ok |
| Misc-C++/stepanov_v1p2 | 0.041 | 0.137 | 3.34× | 0.120 | 2.93× | ok |
| PAQ8p | 0.088 | 0.522 | 5.93× | 0.303 | 3.44× | ok |
| Prolangs-C++/life | 0.004 | 0.039 | — | 0.036 | — | ok |
| Prolangs-C++/ocean | 0.015 | 0.080 | 5.33× | 0.058 | 3.87× | ok |
| Prolangs-C++/primes | 0.007 | 0.028 | — | 0.036 | — | ok |
| Shootout-C++-EH/except | 0.095 | 0.188 | 1.98× | 0.258 | 2.72× | ok |
| Shootout-C++/ackermann | 0.095 | 0.178 | 1.87× | 0.256 | 2.69× | ok |
| Shootout-C++/ary3 | 0.127 | 0.223 | 1.76× | 0.322 | 2.54× | ok |
| Shootout-C++/fibo | 0.095 | 0.178 | 1.87× | 0.265 | 2.79× | ok |
| Shootout-C++/hash | 0.142 | 0.331 | 2.33× | 0.390 | 2.75× | ok |
| Shootout-C++/hash2 | 0.147 | 0.355 | 2.41× | 0.404 | 2.75× | ok |
| Shootout-C++/heapsort | 0.126 | 0.222 | 1.76× | 0.309 | 2.45× | ok |
| Shootout-C++/lists | 0.108 | 0.239 | 2.21× | 0.306 | 2.83× | ok |
| Shootout-C++/lists1 | 0.115 | 0.255 | 2.22× | 0.340 | 2.96× | ok |
| Shootout-C++/matrix | 0.090 | 0.194 | 2.16× | 0.273 | 3.03× | ok |
| Shootout-C++/methcall | 0.095 | 0.189 | 1.99× | 0.248 | 2.61× | ok |
| Shootout-C++/moments | 0.133 | 0.299 | 2.25× | 0.332 | 2.50× | ok |
| Shootout-C++/random | 0.116 | 0.217 | 1.87× | 0.395 | 3.41× | ok |
| Shootout-C++/sieve | 0.118 | 0.305 | 2.58× | 0.326 | 2.76× | ok |
| SmallPT | 0.037 | 0.102 | 2.76× | 0.129 | 3.49× | ok |
| Stanford/Bubblesort | 0.012 | 0.033 | 2.75× | 0.037 | 3.08× | ok |
| Stanford/FloatMM | 0.012 | 0.033 | 2.75× | 0.039 | 3.25× | ok |
| Stanford/Puzzle | 0.013 | 0.041 | 3.15× | 0.040 | 3.08× | ok |
| Stanford/Treesort | 0.012 | 0.035 | 2.92× | 0.039 | 3.25× | ok |
| bg/binarytrees/2 | 0.093 | 0.179 | 1.92× | 0.265 | 2.85× | ok |
| bg/fannkuchredux/3 | 0.045 | 0.085 | 1.89× | 0.110 | 2.44× | ok |
| bg/fasta/1 | 0.071 | 0.138 | 1.94× | 0.163 | 2.30× | ok |
| bg/fasta/2 | 0.072 | 0.152 | 2.11× | 0.164 | 2.28× | ok |
| bg/fasta/3 | 0.111 | 0.222 | 2.00× | 0.288 | 2.59× | ok |
| bg/fasta/4 | 0.102 | 0.187 | 1.83× | 0.218 | 2.14× | ok |
| bg/fasta/6 | 0.192 | 0.446 | 2.32× | 0.609 | 3.17× | ok |
| bg/mandelbrot/0 | 0.218 | 0.509 | 2.33× | 0.577 | 2.65× | ok |
| bg/mandelbrot/2 | 0.142 | 0.264 | 1.86× | 0.314 | 2.21× | ok |
| bg/mandelbrot/3 | 0.129 | 0.224 | 1.74× | 0.347 | 2.69× | ok |
| bg/mandelbrot/5 | 0.059 | 0.130 | 2.20× | 0.148 | 2.51× | ok |
| bg/mandelbrot/7 | 0.058 | 0.136 | 2.34× | 0.162 | 2.79× | ok |
| bg/mandelbrot/9 | 0.073 | 0.136 | 1.86× | 0.154 | 2.11× | ok |
| bg/nbody/1 | 0.037 | 0.122 | 3.30× | 0.119 | 3.22× | ok |
| bg/nbody/2 | 0.036 | 0.080 | 2.22× | 0.087 | 2.42× | ok |
| bg/nbody/3 | 0.036 | 0.091 | 2.53× | 0.090 | 2.50× | ok |
| bg/nbody/6 | 0.039 | 0.112 | 2.87× | 0.106 | 2.72× | ok |
| bg/pidigits/4 | 0.166 | 0.362 | 2.18× | 0.526 | 3.17× | ok |
| bg/pidigits/5 | 0.107 | 0.196 | 1.83× | 0.336 | 3.14× | ok |
| bg/revcomp/1 | 0.111 | 0.253 | 2.28× | 0.392 | 3.53× | ok |
| bg/revcomp/5 | 0.097 | 0.224 | 2.31× | 0.271 | 2.79× | ok |
| bg/spectralnorm/1 | 0.145 | 0.355 | 2.45× | 0.422 | 2.91× | ok |
| box2d | 2.137 | 5.423 | 2.54× | 6.679 | 3.13× | ok |
| fmt | 0.671 | 2.745 | 4.09× | 2.626 | 3.91× | ok |
| hexxagon | 0.380 | 0.711 | 1.87× | 1.137 | 2.99× | ok |
| lambda | 0.068 | 0.375 | 5.51× | 0.254 | 3.74× | ok |
| minisat | 0.082 | 0.333 | 4.06× | 0.273 | 3.33× | ok |
| nlohmann-json | 0.521 | 1.996 | 3.83× | 2.137 | 4.10× | ok |
| pugixml | 0.309 | 1.525 | 4.94× | 1.005 | 3.25× | ok |
| pugixml-tests | 5.052 | 16.672 | 3.30× | 15.919 | 3.15× | ok |
| rapidjson | 0.152 | 0.543 | 3.57× | 0.473 | 3.11× | ok |
| re2 | 4.792 | 12.022 | 2.51× | 8.283 | 1.73× | ok |
| re2-test-re2 | 6.539 | 17.650 | 2.70× | 20.021 | 3.06× | ok |
| re2-test-search | 6.690 | 16.023 | 2.40× | 18.512 | 2.77× | ok |
| tramp3d-v4 | 0.840 | 9.974 | 11.87× | 3.106 | 3.70× | ok |

### O3 compilation

| Program | cppgm++ (s) | GCC (s) | Speed vs GCC ↑ | Clang (s) | Speed vs Clang ↑ | Status |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| Adobe-C++/functionobjects | 0.144 | 0.974 | 6.76× | 0.549 | 3.81× | ok |
| Adobe-C++/loop_unroll | 0.331 | 5.745 | 17.36× | 4.595 | 13.88× | ok |
| Adobe-C++/simple_types_constant_folding | 0.195 | 2.979 | 15.28× | 2.492 | 12.78× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.153 | 2.437 | 15.93× | 1.986 | 12.98× | ok |
| Adobe-C++/stepanov_abstraction | 0.093 | 1.355 | 14.57× | 0.561 | 6.03× | ok |
| Adobe-C++/stepanov_vector | 0.126 | 1.426 | 11.32× | 0.658 | 5.22× | ok |
| Bullet | 8.298 | 42.761 | 5.15× | 29.981 | 3.61× | ok |
| CLAMR | 4.985 | 26.394 | 5.29× | 20.456 | 4.10× | ok |
| CoyoteBench/almabench | 0.041 | 0.176 | 4.29× | 0.132 | 3.22× | ok |
| CoyoteBench/fftbench | 0.154 | 0.468 | 3.04× | 0.584 | 3.79× | ok |
| CoyoteBench/huffbench | 0.054 | 0.249 | 4.61× | 0.152 | 2.81× | ok |
| CoyoteBench/lpbench | 0.052 | 0.231 | 4.44× | 0.210 | 4.04× | ok |
| DOE/HACCKernels | 0.153 | 0.426 | 2.78× | 0.442 | 2.89× | ok |
| DOE/HPCCG | 0.892 | 2.544 | 2.85× | 2.727 | 3.06× | ok |
| DOE/PENNANT | 2.172 | 6.496 | 2.99× | 6.773 | 3.12× | ok |
| DOE/miniFE | 1.028 | 4.322 | 4.20× | 3.673 | 3.57× | ok |
| Misc-C++-EH/spirit | 0.797 | 4.376 | 5.49× | 4.119 | 5.17× | ok |
| Misc-C++-Large/ray | 0.168 | 0.511 | 3.04× | 0.489 | 2.91× | ok |
| Misc-C++-Large/sphereflake | 0.136 | 0.826 | 6.07× | 0.424 | 3.12× | ok |
| Misc-C++/bigfib | 0.201 | 0.628 | 3.12× | 0.742 | 3.69× | ok |
| Misc-C++/oopack_v1p8 | 0.047 | 0.202 | 4.30× | 0.192 | 4.09× | ok |
| Misc-C++/stepanov_container | 0.269 | 1.030 | 3.83× | 1.178 | 4.38× | ok |
| Misc-C++/stepanov_v1p2 | 0.048 | 0.290 | 6.04× | 0.262 | 5.46× | ok |
| PAQ8p | 0.307 | 4.405 | 14.35× | 1.892 | 6.16× | ok |
| Prolangs-C++/life | 0.009 | 0.112 | — | 0.082 | — | ok |
| Prolangs-C++/ocean | 0.023 | 0.230 | 10.00× | 0.135 | 5.87× | ok |
| Prolangs-C++/primes | 0.009 | 0.041 | — | 0.046 | — | ok |
| Shootout-C++-EH/except | 0.108 | 0.222 | 2.06× | 0.264 | 2.44× | ok |
| Shootout-C++/ackermann | 0.099 | 0.223 | 2.25× | 0.271 | 2.74× | ok |
| Shootout-C++/ary3 | 0.119 | 0.272 | 2.29× | 0.334 | 2.81× | ok |
| Shootout-C++/fibo | 0.101 | 0.223 | 2.21× | 0.278 | 2.75× | ok |
| Shootout-C++/hash | 0.157 | 0.449 | 2.86× | 0.504 | 3.21× | ok |
| Shootout-C++/hash2 | 0.165 | 0.508 | 3.08× | 0.512 | 3.10× | ok |
| Shootout-C++/heapsort | 0.118 | 0.265 | 2.25× | 0.339 | 2.87× | ok |
| Shootout-C++/lists | 0.117 | 0.278 | 2.38× | 0.312 | 2.67× | ok |
| Shootout-C++/lists1 | 0.123 | 0.294 | 2.39× | 0.384 | 3.12× | ok |
| Shootout-C++/matrix | 0.104 | 0.229 | 2.20× | 0.359 | 3.45× | ok |
| Shootout-C++/methcall | 0.094 | 0.227 | 2.41× | 0.266 | 2.83× | ok |
| Shootout-C++/moments | 0.142 | 0.368 | 2.59× | 0.396 | 2.79× | ok |
| Shootout-C++/random | 0.119 | 0.241 | 2.03× | 0.314 | 2.64× | ok |
| Shootout-C++/sieve | 0.124 | 0.299 | 2.41× | 0.359 | 2.90× | ok |
| SmallPT | 0.046 | 0.334 | 7.26× | 0.176 | 3.83× | ok |
| Stanford/Bubblesort | 0.014 | 0.060 | 4.29× | 0.054 | 3.86× | ok |
| Stanford/FloatMM | 0.015 | 0.069 | 4.60× | 0.077 | 5.13× | ok |
| Stanford/Puzzle | 0.019 | 0.135 | 7.11× | 0.130 | 6.84× | ok |
| Stanford/Treesort | 0.016 | 0.092 | 5.75× | 0.059 | 3.69× | ok |
| bg/binarytrees/2 | 0.095 | 0.448 | 4.72× | 0.281 | 2.96× | ok |
| bg/fannkuchredux/3 | 0.049 | 0.135 | 2.76× | 0.128 | 2.61× | ok |
| bg/fasta/1 | 0.073 | 0.213 | 2.92× | 0.203 | 2.78× | ok |
| bg/fasta/2 | 0.075 | 0.188 | 2.51× | 0.212 | 2.83× | ok |
| bg/fasta/3 | 0.116 | 0.267 | 2.30× | 0.332 | 2.86× | ok |
| bg/fasta/4 | 0.099 | 0.221 | 2.23× | 0.362 | 3.66× | ok |
| bg/fasta/6 | 0.184 | 0.538 | 2.92× | 0.657 | 3.57× | ok |
| bg/mandelbrot/0 | 0.255 | 0.713 | 2.80× | 0.772 | 3.03× | ok |
| bg/mandelbrot/2 | 0.134 | 0.338 | 2.52× | 0.352 | 2.63× | ok |
| bg/mandelbrot/3 | 0.131 | 0.279 | 2.13× | 0.372 | 2.84× | ok |
| bg/mandelbrot/5 | 0.062 | 0.171 | 2.76× | 0.182 | 2.94× | ok |
| bg/mandelbrot/7 | 0.065 | 0.205 | 3.15× | 0.206 | 3.17× | ok |
| bg/mandelbrot/9 | 0.063 | 0.208 | 3.30× | 0.189 | 3.00× | ok |
| bg/nbody/1 | 0.049 | 0.183 | 3.73× | 0.204 | 4.16× | ok |
| bg/nbody/2 | 0.040 | 0.147 | 3.67× | 0.145 | 3.62× | ok |
| bg/nbody/3 | 0.039 | 0.113 | 2.90× | 0.122 | 3.13× | ok |
| bg/nbody/6 | 0.054 | 0.185 | 3.43× | 0.194 | 3.59× | ok |
| bg/pidigits/4 | 0.166 | 0.406 | 2.45× | 0.654 | 3.94× | ok |
| bg/pidigits/5 | 0.106 | 0.251 | 2.37× | 0.378 | 3.57× | ok |
| bg/revcomp/1 | 0.117 | 0.283 | 2.42× | 0.414 | 3.54× | ok |
| bg/revcomp/5 | 0.101 | 0.260 | 2.57× | 0.308 | 3.05× | ok |
| bg/spectralnorm/1 | 0.167 | 0.416 | 2.49× | 0.437 | 2.62× | ok |
| box2d | 2.364 | 8.502 | 3.60× | 9.371 | 3.96× | ok |
| fmt | 1.202 | 6.146 | 5.11× | 5.115 | 4.26× | ok |
| hexxagon | 0.405 | 1.132 | 2.80× | 1.364 | 3.37× | ok |
| lambda | 0.120 | 1.175 | 9.79× | 0.580 | 4.83× | ok |
| minisat | 0.153 | 1.199 | 7.84× | 0.674 | 4.41× | ok |
| nlohmann-json | 0.923 | 4.109 | 4.45× | 3.658 | 3.96× | ok |
| pugixml | 0.727 | 6.235 | 8.58× | 3.577 | 4.92× | ok |
| pugixml-tests | 6.923 | 38.637 | 5.58× | 28.775 | 4.16× | ok |
| rapidjson | 0.269 | 1.211 | 4.50× | 0.854 | 3.17× | ok |
| re2 | 3.857 | 20.051 | 5.20× | 11.580 | 3.00× | ok |
| re2-test-re2 | 7.636 | 31.121 | 4.08× | 25.779 | 3.38× | ok |
| re2-test-search | 7.295 | 29.378 | 4.03× | 18.258 | 2.50× | ok |
| tramp3d-v4 | 2.878 | 24.774 | 8.61× | 14.964 | 5.20× | ok |

### Frontend only: -fsyntax-only

| Program | cppgm++ (s) | GCC (s) | Speed vs GCC ↑ | Clang (s) | Speed vs Clang ↑ | Status |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| Adobe-C++/functionobjects | 0.087 | 0.168 | 1.93× | 0.224 | 2.57× | ok |
| Adobe-C++/loop_unroll | 0.049 | 0.121 | 2.47× | 0.136 | 2.78× | ok |
| Adobe-C++/simple_types_constant_folding | 0.058 | 0.130 | 2.24× | 0.145 | 2.50× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.049 | 0.112 | 2.29× | 0.127 | 2.59× | ok |
| Adobe-C++/stepanov_abstraction | 0.049 | 0.087 | 1.78× | 0.136 | 2.78× | ok |
| Adobe-C++/stepanov_vector | 0.091 | 0.153 | 1.68× | 0.205 | 2.25× | ok |
| Bullet | 5.961 | 12.771 | 2.14× | 17.373 | 2.91× | ok |
| CLAMR | 2.791 | 5.843 | 2.09× | 7.714 | 2.76× | ok |
| CoyoteBench/almabench | 0.037 | 0.073 | 1.97× | 0.088 | 2.38× | ok |
| CoyoteBench/fftbench | 0.136 | 0.264 | 1.94× | 0.373 | 2.74× | ok |
| CoyoteBench/huffbench | 0.039 | 0.072 | 1.85× | 0.109 | 2.79× | ok |
| CoyoteBench/lpbench | 0.044 | 0.078 | 1.77× | 0.089 | 2.02× | ok |
| DOE/HACCKernels | 0.141 | 0.277 | 1.96× | 0.356 | 2.52× | ok |
| DOE/HPCCG | 0.811 | 1.607 | 1.98× | 2.197 | 2.71× | ok |
| DOE/PENNANT | 1.873 | 3.306 | 1.77× | 4.615 | 2.46× | ok |
| DOE/miniFE | 0.849 | 1.449 | 1.71× | 2.028 | 2.39× | ok |
| Misc-C++-EH/spirit | 0.420 | 1.288 | 3.07× | 1.356 | 3.23× | ok |
| Misc-C++-Large/ray | 0.129 | 0.288 | 2.23× | 0.387 | 3.00× | ok |
| Misc-C++-Large/sphereflake | 0.125 | 0.221 | 1.77× | 0.314 | 2.51× | ok |
| Misc-C++/bigfib | 0.151 | 0.343 | 2.27× | 0.465 | 3.08× | ok |
| Misc-C++/oopack_v1p8 | 0.040 | 0.081 | 2.02× | 0.103 | 2.57× | ok |
| Misc-C++/stepanov_container | 0.203 | 0.427 | 2.10× | 0.661 | 3.26× | ok |
| Misc-C++/stepanov_v1p2 | 0.038 | 0.077 | 2.03× | 0.099 | 2.61× | ok |
| PAQ8p | 0.065 | 0.151 | 2.32× | 0.178 | 2.74× | ok |
| Prolangs-C++/life | 0.004 | 0.014 | — | 0.026 | — | ok |
| Prolangs-C++/ocean | 0.013 | 0.031 | 2.38× | 0.039 | 3.00× | ok |
| Prolangs-C++/primes | 0.007 | 0.020 | — | 0.032 | — | ok |
| Shootout-C++-EH/except | 0.094 | 0.191 | 2.03× | 0.243 | 2.59× | ok |
| Shootout-C++/ackermann | 0.095 | 0.181 | 1.91× | 0.246 | 2.59× | ok |
| Shootout-C++/ary3 | 0.113 | 0.205 | 1.81× | 0.284 | 2.51× | ok |
| Shootout-C++/fibo | 0.092 | 0.181 | 1.97× | 0.279 | 3.03× | ok |
| Shootout-C++/hash | 0.140 | 0.257 | 1.84× | 0.362 | 2.59× | ok |
| Shootout-C++/hash2 | 0.139 | 0.267 | 1.92× | 0.357 | 2.57× | ok |
| Shootout-C++/heapsort | 0.126 | 0.229 | 1.82× | 0.291 | 2.31× | ok |
| Shootout-C++/lists | 0.103 | 0.199 | 1.93× | 0.277 | 2.69× | ok |
| Shootout-C++/lists1 | 0.111 | 0.211 | 1.90× | 0.306 | 2.76× | ok |
| Shootout-C++/matrix | 0.089 | 0.193 | 2.17× | 0.259 | 2.91× | ok |
| Shootout-C++/methcall | 0.092 | 0.193 | 2.10× | 0.238 | 2.59× | ok |
| Shootout-C++/moments | 0.126 | 0.241 | 1.91× | 0.298 | 2.37× | ok |
| Shootout-C++/random | 0.116 | 0.221 | 1.91× | 0.305 | 2.63× | ok |
| Shootout-C++/sieve | 0.114 | 0.289 | 2.54× | 0.304 | 2.67× | ok |
| SmallPT | 0.036 | 0.069 | 1.92× | 0.089 | 2.47× | ok |
| Stanford/Bubblesort | 0.012 | 0.029 | 2.42× | 0.035 | 2.92× | ok |
| Stanford/FloatMM | 0.012 | 0.028 | 2.33× | 0.036 | 3.00× | ok |
| Stanford/Puzzle | 0.012 | 0.030 | 2.50× | 0.037 | 3.08× | ok |
| Stanford/Treesort | 0.012 | 0.028 | 2.33× | 0.041 | 3.42× | ok |
| bg/binarytrees/2 | 0.092 | 0.176 | 1.91× | 0.250 | 2.72× | ok |
| bg/fannkuchredux/3 | 0.044 | 0.074 | 1.68× | 0.094 | 2.14× | ok |
| bg/fasta/1 | 0.067 | 0.112 | 1.67× | 0.149 | 2.22× | ok |
| bg/fasta/2 | 0.071 | 0.124 | 1.75× | 0.148 | 2.08× | ok |
| bg/fasta/3 | 0.108 | 0.223 | 2.06× | 0.275 | 2.55× | ok |
| bg/fasta/4 | 0.111 | 0.183 | 1.65× | 0.218 | 1.96× | ok |
| bg/fasta/6 | 0.185 | 0.330 | 1.78× | 0.548 | 2.96× | ok |
| bg/mandelbrot/0 | 0.205 | 0.350 | 1.71× | 0.548 | 2.67× | ok |
| bg/mandelbrot/2 | 0.137 | 0.249 | 1.82× | 0.291 | 2.12× | ok |
| bg/mandelbrot/3 | 0.126 | 0.222 | 1.76× | 0.330 | 2.62× | ok |
| bg/mandelbrot/5 | 0.058 | 0.107 | 1.84× | 0.136 | 2.34× | ok |
| bg/mandelbrot/7 | 0.054 | 0.101 | 1.87× | 0.145 | 2.69× | ok |
| bg/mandelbrot/9 | 0.073 | 0.099 | 1.36× | 0.139 | 1.90× | ok |
| bg/nbody/1 | 0.035 | 0.073 | 2.09× | 0.101 | 2.89× | ok |
| bg/nbody/2 | 0.034 | 0.068 | 2.00× | 0.096 | 2.82× | ok |
| bg/nbody/3 | 0.034 | 0.066 | 1.94× | 0.085 | 2.50× | ok |
| bg/nbody/6 | 0.036 | 0.068 | 1.89× | 0.091 | 2.53× | ok |
| bg/pidigits/4 | 0.165 | 0.328 | 1.99× | 0.516 | 3.13× | ok |
| bg/pidigits/5 | 0.107 | 0.191 | 1.79× | 0.272 | 2.54× | ok |
| bg/revcomp/1 | 0.106 | 0.204 | 1.92× | 0.333 | 3.14× | ok |
| bg/revcomp/5 | 0.093 | 0.183 | 1.97× | 0.244 | 2.62× | ok |
| bg/spectralnorm/1 | 0.156 | 0.282 | 1.81× | 0.395 | 2.53× | ok |
| box2d | 1.927 | 3.690 | 1.91× | 5.779 | 3.00× | ok |
| fmt | 0.589 | 1.241 | 2.11× | 1.782 | 3.03× | ok |
| hexxagon | 0.374 | 0.645 | 1.72× | 1.019 | 2.72× | ok |
| lambda | 0.054 | 0.172 | 3.19× | 0.187 | 3.46× | ok |
| minisat | 0.071 | 0.157 | 2.21× | 0.228 | 3.21× | ok |
| nlohmann-json | 0.425 | 0.921 | 2.17× | 1.665 | 3.92× | ok |
| pugixml | 0.247 | 0.620 | 2.51× | 0.740 | 3.00× | ok |
| pugixml-tests | 4.406 | 8.725 | 1.98× | 12.359 | 2.81× | ok |
| rapidjson | 0.128 | 0.291 | 2.27× | 0.392 | 3.06× | ok |
| re2 | 4.166 | 7.348 | 1.76× | 7.131 | 1.71× | ok |
| re2-test-re2 | 5.992 | 11.225 | 1.87× | 17.363 | 2.90× | ok |
| re2-test-search | 6.360 | 11.464 | 1.80× | 15.811 | 2.49× | ok |
| tramp3d-v4 | 0.834 | 3.034 | 3.64× | 1.831 | 2.20× | ok |

### Building the compiler

| Compiler doing the rebuild | CPU seconds |
| --- | ---: |
| GCC | 368.510 |
| GCC-built cppgm++ | 85.650 |
| Self-built cppgm++ | 85.180 |

These are the initial bootstrap builds. The GCC rebuild is measured once.

#### Paired self-built compiler measurements

| Block | GCC-built cppgm++ (s) | Self-built cppgm++ (s) | Self-built speed ↑ |
| ---: | ---: | ---: | ---: |
| 1 | 82.750 | 84.235 | 0.98× |
| 2 | 83.222 | 84.125 | 0.99× |

Each block averages its two observations per compiler. The badge uses the median block ratio; calibration and every observation are retained in the raw results.

## Toolchains

- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)
- **valgrind**: valgrind-3.26.0
