# Public benchmarks: AMD EPYC

Measured compiler: [`25f26d178`](https://github.com/vishvananda/cppgm-extended/commit/25f26d17880cb90b049d47dab65249c2b4e8f582) · [CI run](https://github.com/vishvananda/cppgm-extended/actions/runs/42299582390) · [Raw program results](results.json) · [Raw rebuild results](selfhost.json)

81 of 81 programs built with cppgm++ and g++ and agreed. Each number is the reference over cppgm++, so above 1× means cppgm++ is smaller or faster.

CPU: AMD EPYC 9V45 96-Core Processor.

| Measure | vs g++ | Programs | vs clang++ | Programs |
| --- | ---: | ---: | ---: | ---: |
| Frontend CPU, `-fsyntax-only` | 2.02× | 79 | 2.48× | 79 |
| Compile CPU, `-O0` | 2.77× | 79 | 2.67× | 79 |
| Compile CPU, `-O3` | 4.09× | 79 | 3.57× | 79 |
| Compile peak RSS, `-O0` | 1.46× | 81 | 2.23× | 81 |
| Compile peak RSS, `-O3` | 1.74× | 81 | 2.34× | 81 |
| Executable size, `-O0` | 0.93× | 81 | 0.88× | 81 |
| Executable size, `-O3` | 0.93× | 81 | 0.88× | 81 |
| Run CPU, `-O0` | 1.02× | 81 | 1.01× | 81 |
| Run CPU, `-O3` | 0.99× | 80 | 1.00× | 80 |
| Run instructions, `-O3` | 0.95× | 43 | - | 0 |
| Self compile CPU | 4.24× | - | - | - |
| Self-built compiler CPU | 1.00× | - | - | - |

O3 execution uses paired ABBA/BAAB blocks. GCC A/A CPU ratios ranged from 0.899 to 1.141; raw observations and block ratios are retained in the artifact.

Self-built / GCC-built compiler CPU: paired ratios 0.988–1.010; A/A calibration 0.996. A/A variation limits conclusions about small changes.

## Slowest generated code, by run-time instructions

| Program | vs g++ |
| --- | ---: |
| bg/nbody/2 | 0.63× |
| Stanford/Bubblesort | 0.72× |
| PAQ8p | 0.74× |
| tramp3d-v4 | 0.75× |
| bg/spectralnorm/1 | 0.75× |
| bg/nbody/1 | 0.83× |
| bg/mandelbrot/7 | 0.84× |
| bg/mandelbrot/9 | 0.84× |
| bg/nbody/3 | 0.85× |
| Misc-C++/oopack_v1p8 | 0.86× |

## Compile CPU times

Times are CPU seconds, excluding linking. Speed is reference time divided by cppgm++ time; higher is better. Every program is listed. Missing times show a dash. Ratios are omitted for failed programs or when either time is below 10 ms.

Compilation is measured once per compiler and level; the ABBA/BAAB repetitions apply to generated-program execution and self-built compiler speed.

### O0 compilation

| Program | cppgm++ (s) | GCC (s) | Speed vs GCC ↑ | Clang (s) | Speed vs Clang ↑ | Status |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| Adobe-C++/functionobjects | 0.074 | 0.235 | 3.18× | 0.169 | 2.28× | ok |
| Adobe-C++/loop_unroll | 0.053 | 0.507 | 9.57× | 0.169 | 3.19× | ok |
| Adobe-C++/simple_types_constant_folding | 0.061 | 0.585 | 9.59× | 0.197 | 3.23× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.049 | 0.438 | 8.94× | 0.166 | 3.39× | ok |
| Adobe-C++/stepanov_abstraction | 0.043 | 0.166 | 3.86× | 0.123 | 2.86× | ok |
| Adobe-C++/stepanov_vector | 0.082 | 0.267 | 3.26× | 0.185 | 2.26× | ok |
| Bullet | 5.176 | 17.351 | 3.35× | 14.709 | 2.84× | ok |
| CLAMR | 2.795 | 8.760 | 3.13× | 6.968 | 2.49× | ok |
| CoyoteBench/almabench | 0.034 | 0.077 | 2.26× | 0.075 | 2.21× | ok |
| CoyoteBench/fftbench | 0.117 | 0.238 | 2.03× | 0.290 | 2.48× | ok |
| CoyoteBench/huffbench | 0.033 | 0.066 | 2.00× | 0.075 | 2.27× | ok |
| CoyoteBench/lpbench | 0.032 | 0.074 | 2.31× | 0.073 | 2.28× | ok |
| DOE/HACCKernels | 0.116 | 0.251 | 2.16× | 0.292 | 2.52× | ok |
| DOE/HPCCG | 0.760 | 1.534 | 2.02× | 2.168 | 2.85× | ok |
| DOE/PENNANT | 1.618 | 3.948 | 2.44× | 4.055 | 2.51× | ok |
| DOE/miniFE | 0.690 | 1.996 | 2.89× | 2.084 | 3.02× | ok |
| Misc-C++-EH/spirit | 0.541 | 2.489 | 4.60× | 1.828 | 3.38× | ok |
| Misc-C++-Large/ray | 0.120 | 0.273 | 2.28× | 0.286 | 2.38× | ok |
| Misc-C++-Large/sphereflake | 0.108 | 0.233 | 2.16× | 0.289 | 2.68× | ok |
| Misc-C++/bigfib | 0.129 | 0.357 | 2.77× | 0.362 | 2.81× | ok |
| Misc-C++/oopack_v1p8 | 0.032 | 0.094 | 2.94× | 0.085 | 2.66× | ok |
| Misc-C++/stepanov_container | 0.182 | 0.520 | 2.86× | 0.538 | 2.96× | ok |
| Misc-C++/stepanov_v1p2 | 0.036 | 0.112 | 3.11× | 0.095 | 2.64× | ok |
| PAQ8p | 0.069 | 0.432 | 6.26× | 0.227 | 3.29× | ok |
| Prolangs-C++/life | 0.005 | 0.033 | — | 0.030 | — | ok |
| Prolangs-C++/ocean | 0.013 | 0.064 | 4.92× | 0.047 | 3.62× | ok |
| Prolangs-C++/primes | 0.007 | 0.026 | — | 0.030 | — | ok |
| Shootout-C++-EH/except | 0.085 | 0.188 | 2.21× | 0.218 | 2.56× | ok |
| Shootout-C++/ackermann | 0.083 | 0.203 | 2.45× | 0.214 | 2.58× | ok |
| Shootout-C++/ary3 | 0.104 | 0.221 | 2.12× | 0.290 | 2.79× | ok |
| Shootout-C++/fibo | 0.084 | 0.158 | 1.88× | 0.192 | 2.29× | ok |
| Shootout-C++/hash | 0.114 | 0.258 | 2.26× | 0.302 | 2.65× | ok |
| Shootout-C++/hash2 | 0.121 | 0.309 | 2.55× | 0.289 | 2.39× | ok |
| Shootout-C++/heapsort | 0.095 | 0.179 | 1.88× | 0.248 | 2.61× | ok |
| Shootout-C++/lists | 0.099 | 0.202 | 2.04× | 0.257 | 2.60× | ok |
| Shootout-C++/lists1 | 0.107 | 0.222 | 2.07× | 0.253 | 2.36× | ok |
| Shootout-C++/matrix | 0.081 | 0.159 | 1.96× | 0.242 | 2.99× | ok |
| Shootout-C++/methcall | 0.082 | 0.161 | 1.96× | 0.195 | 2.38× | ok |
| Shootout-C++/moments | 0.109 | 0.259 | 2.38× | 0.266 | 2.44× | ok |
| Shootout-C++/random | 0.089 | 0.186 | 2.09× | 0.235 | 2.64× | ok |
| Shootout-C++/sieve | 0.102 | 0.235 | 2.30× | 0.250 | 2.45× | ok |
| SmallPT | 0.031 | 0.090 | 2.90× | 0.089 | 2.87× | ok |
| Stanford/Bubblesort | 0.010 | 0.030 | 3.00× | 0.055 | 5.50× | ok |
| Stanford/FloatMM | 0.011 | 0.031 | 2.82× | 0.035 | 3.18× | ok |
| Stanford/Puzzle | 0.012 | 0.037 | 3.08× | 0.038 | 3.17× | ok |
| Stanford/Treesort | 0.011 | 0.060 | 5.45× | 0.038 | 3.45× | ok |
| bg/binarytrees/2 | 0.074 | 0.165 | 2.23× | 0.190 | 2.57× | ok |
| bg/fannkuchredux/3 | 0.042 | 0.071 | 1.69× | 0.087 | 2.07× | ok |
| bg/fasta/1 | 0.058 | 0.122 | 2.10× | 0.127 | 2.19× | ok |
| bg/fasta/2 | 0.059 | 0.116 | 1.97× | 0.134 | 2.27× | ok |
| bg/fasta/3 | 0.093 | 0.204 | 2.19× | 0.217 | 2.33× | ok |
| bg/fasta/4 | 0.088 | 0.149 | 1.69× | 0.180 | 2.05× | ok |
| bg/fasta/6 | 0.156 | 0.382 | 2.45× | 0.466 | 2.99× | ok |
| bg/mandelbrot/0 | 0.214 | 0.450 | 2.10× | 0.411 | 1.92× | ok |
| bg/mandelbrot/2 | 0.101 | 0.216 | 2.14× | 0.259 | 2.56× | ok |
| bg/mandelbrot/3 | 0.106 | 0.197 | 1.86× | 0.253 | 2.39× | ok |
| bg/mandelbrot/5 | 0.052 | 0.109 | 2.10× | 0.113 | 2.17× | ok |
| bg/mandelbrot/7 | 0.049 | 0.122 | 2.49× | 0.117 | 2.39× | ok |
| bg/mandelbrot/9 | 0.047 | 0.111 | 2.36× | 0.116 | 2.47× | ok |
| bg/nbody/1 | 0.030 | 0.101 | 3.37× | 0.093 | 3.10× | ok |
| bg/nbody/2 | 0.028 | 0.074 | 2.64× | 0.075 | 2.68× | ok |
| bg/nbody/3 | 0.029 | 0.071 | 2.45× | 0.074 | 2.55× | ok |
| bg/nbody/6 | 0.031 | 0.098 | 3.16× | 0.087 | 2.81× | ok |
| bg/pidigits/4 | 0.137 | 0.284 | 2.07× | 0.365 | 2.66× | ok |
| bg/pidigits/5 | 0.098 | 0.173 | 1.77× | 0.222 | 2.27× | ok |
| bg/revcomp/1 | 0.097 | 0.215 | 2.22× | 0.235 | 2.42× | ok |
| bg/revcomp/5 | 0.097 | 0.188 | 1.94× | 0.257 | 2.65× | ok |
| bg/spectralnorm/1 | 0.128 | 0.254 | 1.98× | 0.328 | 2.56× | ok |
| box2d | 1.636 | 4.283 | 2.62× | 4.978 | 3.04× | ok |
| fmt | 0.551 | 2.281 | 4.14× | 1.753 | 3.18× | ok |
| hexxagon | 0.303 | 0.690 | 2.28× | 0.898 | 2.96× | ok |
| lambda | 0.066 | 0.298 | 4.52× | 0.235 | 3.56× | ok |
| minisat | 0.071 | 0.290 | 4.08× | 0.214 | 3.01× | ok |
| nlohmann-json | 0.448 | 1.976 | 4.41× | 1.638 | 3.66× | ok |
| pugixml | 0.298 | 1.350 | 4.53× | 0.846 | 2.84× | ok |
| pugixml-tests | 4.328 | 15.181 | 3.51× | 12.292 | 2.84× | ok |
| rapidjson | 0.136 | 0.409 | 3.01× | 0.360 | 2.65× | ok |
| re2 | 3.875 | 10.330 | 2.67× | 6.776 | 1.75× | ok |
| re2-test-re2 | 5.539 | 14.907 | 2.69× | 15.473 | 2.79× | ok |
| re2-test-search | 5.880 | 14.983 | 2.55× | 13.621 | 2.32× | ok |
| tramp3d-v4 | 1.006 | 8.149 | 8.10× | 2.589 | 2.57× | ok |

### O3 compilation

| Program | cppgm++ (s) | GCC (s) | Speed vs GCC ↑ | Clang (s) | Speed vs Clang ↑ | Status |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| Adobe-C++/functionobjects | 0.113 | 0.777 | 6.88× | 0.449 | 3.97× | ok |
| Adobe-C++/loop_unroll | 0.220 | 4.971 | 22.60× | 4.367 | 19.85× | ok |
| Adobe-C++/simple_types_constant_folding | 0.145 | 2.727 | 18.81× | 2.084 | 14.37× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.121 | 2.090 | 17.27× | 1.693 | 13.99× | ok |
| Adobe-C++/stepanov_abstraction | 0.069 | 0.904 | 13.10× | 0.419 | 6.07× | ok |
| Adobe-C++/stepanov_vector | 0.112 | 1.186 | 10.59× | 0.554 | 4.95× | ok |
| Bullet | 6.348 | 35.539 | 5.60× | 26.202 | 4.13× | ok |
| CLAMR | 3.983 | 22.263 | 5.59× | 15.950 | 4.00× | ok |
| CoyoteBench/almabench | 0.036 | 0.166 | 4.61× | 0.107 | 2.97× | ok |
| CoyoteBench/fftbench | 0.127 | 0.387 | 3.05× | 0.389 | 3.06× | ok |
| CoyoteBench/huffbench | 0.037 | 0.222 | 6.00× | 0.121 | 3.27× | ok |
| CoyoteBench/lpbench | 0.039 | 0.163 | 4.18× | 0.170 | 4.36× | ok |
| DOE/HACCKernels | 0.121 | 0.347 | 2.87× | 0.349 | 2.88× | ok |
| DOE/HPCCG | 0.796 | 2.137 | 2.68× | 2.502 | 3.14× | ok |
| DOE/PENNANT | 1.876 | 5.693 | 3.03× | 5.665 | 3.02× | ok |
| DOE/miniFE | 0.926 | 3.807 | 4.11× | 3.056 | 3.30× | ok |
| Misc-C++-EH/spirit | 0.714 | 3.453 | 4.84× | 3.566 | 4.99× | ok |
| Misc-C++-Large/ray | 0.125 | 0.442 | 3.54× | 0.322 | 2.58× | ok |
| Misc-C++-Large/sphereflake | 0.115 | 0.712 | 6.19× | 0.330 | 2.87× | ok |
| Misc-C++/bigfib | 0.159 | 0.515 | 3.24× | 0.500 | 3.14× | ok |
| Misc-C++/oopack_v1p8 | 0.037 | 0.183 | 4.95× | 0.154 | 4.16× | ok |
| Misc-C++/stepanov_container | 0.238 | 0.791 | 3.32× | 0.918 | 3.86× | ok |
| Misc-C++/stepanov_v1p2 | 0.040 | 0.197 | 4.92× | 0.171 | 4.28× | ok |
| PAQ8p | 0.254 | 3.643 | 14.34× | 1.625 | 6.40× | ok |
| Prolangs-C++/life | 0.008 | 0.102 | — | 0.064 | — | ok |
| Prolangs-C++/ocean | 0.020 | 0.202 | 10.10× | 0.109 | 5.45× | ok |
| Prolangs-C++/primes | 0.008 | 0.036 | — | 0.038 | — | ok |
| Shootout-C++-EH/except | 0.085 | 0.216 | 2.54× | 0.221 | 2.60× | ok |
| Shootout-C++/ackermann | 0.085 | 0.212 | 2.49× | 0.227 | 2.67× | ok |
| Shootout-C++/ary3 | 0.105 | 0.261 | 2.49× | 0.300 | 2.86× | ok |
| Shootout-C++/fibo | 0.078 | 0.207 | 2.65× | 0.201 | 2.58× | ok |
| Shootout-C++/hash | 0.121 | 0.357 | 2.95× | 0.380 | 3.14× | ok |
| Shootout-C++/hash2 | 0.130 | 0.417 | 3.21× | 0.414 | 3.18× | ok |
| Shootout-C++/heapsort | 0.096 | 0.216 | 2.25× | 0.260 | 2.71× | ok |
| Shootout-C++/lists | 0.101 | 0.241 | 2.39× | 0.263 | 2.60× | ok |
| Shootout-C++/lists1 | 0.121 | 0.273 | 2.26× | 0.307 | 2.54× | ok |
| Shootout-C++/matrix | 0.085 | 0.204 | 2.40× | 0.292 | 3.44× | ok |
| Shootout-C++/methcall | 0.080 | 0.177 | 2.21× | 0.214 | 2.67× | ok |
| Shootout-C++/moments | 0.118 | 0.348 | 2.95× | 0.326 | 2.76× | ok |
| Shootout-C++/random | 0.102 | 0.216 | 2.12× | 0.246 | 2.41× | ok |
| Shootout-C++/sieve | 0.105 | 0.283 | 2.70× | 0.274 | 2.61× | ok |
| SmallPT | 0.037 | 0.273 | 7.38× | 0.146 | 3.95× | ok |
| Stanford/Bubblesort | 0.013 | 0.057 | 4.38× | 0.051 | 3.92× | ok |
| Stanford/FloatMM | 0.014 | 0.061 | 4.36× | 0.067 | 4.79× | ok |
| Stanford/Puzzle | 0.017 | 0.118 | 6.94× | 0.157 | 9.24× | ok |
| Stanford/Treesort | 0.015 | 0.082 | 5.47× | 0.055 | 3.67× | ok |
| bg/binarytrees/2 | 0.077 | 0.392 | 5.09× | 0.217 | 2.82× | ok |
| bg/fannkuchredux/3 | 0.045 | 0.146 | 3.24× | 0.109 | 2.42× | ok |
| bg/fasta/1 | 0.063 | 0.157 | 2.49× | 0.156 | 2.48× | ok |
| bg/fasta/2 | 0.066 | 0.153 | 2.32× | 0.185 | 2.80× | ok |
| bg/fasta/3 | 0.096 | 0.245 | 2.55× | 0.286 | 2.98× | ok |
| bg/fasta/4 | 0.090 | 0.215 | 2.39× | 0.229 | 2.54× | ok |
| bg/fasta/6 | 0.153 | 0.440 | 2.88× | 0.468 | 3.06× | ok |
| bg/mandelbrot/0 | 0.189 | 0.573 | 3.03× | 0.610 | 3.23× | ok |
| bg/mandelbrot/2 | 0.104 | 0.267 | 2.57× | 0.331 | 3.18× | ok |
| bg/mandelbrot/3 | 0.108 | 0.240 | 2.22× | 0.285 | 2.64× | ok |
| bg/mandelbrot/5 | 0.055 | 0.144 | 2.62× | 0.138 | 2.51× | ok |
| bg/mandelbrot/7 | 0.056 | 0.198 | 3.54× | 0.164 | 2.93× | ok |
| bg/mandelbrot/9 | 0.055 | 0.163 | 2.96× | 0.153 | 2.78× | ok |
| bg/nbody/1 | 0.039 | 0.145 | 3.72× | 0.157 | 4.03× | ok |
| bg/nbody/2 | 0.033 | 0.120 | 3.64× | 0.111 | 3.36× | ok |
| bg/nbody/3 | 0.033 | 0.118 | 3.58× | 0.104 | 3.15× | ok |
| bg/nbody/6 | 0.039 | 0.150 | 3.85× | 0.156 | 4.00× | ok |
| bg/pidigits/4 | 0.147 | 0.315 | 2.14× | 0.432 | 2.94× | ok |
| bg/pidigits/5 | 0.096 | 0.205 | 2.14× | 0.243 | 2.53× | ok |
| bg/revcomp/1 | 0.101 | 0.232 | 2.30× | 0.275 | 2.72× | ok |
| bg/revcomp/5 | 0.099 | 0.248 | 2.51× | 0.278 | 2.81× | ok |
| bg/spectralnorm/1 | 0.135 | 0.339 | 2.51× | 0.373 | 2.76× | ok |
| box2d | 1.996 | 7.225 | 3.62× | 6.814 | 3.41× | ok |
| fmt | 1.006 | 4.930 | 4.90× | 3.953 | 3.93× | ok |
| hexxagon | 0.346 | 1.126 | 3.25× | 1.109 | 3.21× | ok |
| lambda | 0.112 | 1.134 | 10.12× | 0.565 | 5.04× | ok |
| minisat | 0.125 | 0.991 | 7.93× | 0.601 | 4.81× | ok |
| nlohmann-json | 0.730 | 3.411 | 4.67× | 2.764 | 3.79× | ok |
| pugixml | 0.707 | 5.484 | 7.76× | 3.831 | 5.42× | ok |
| pugixml-tests | 5.969 | 33.523 | 5.62× | 24.732 | 4.14× | ok |
| rapidjson | 0.236 | 1.181 | 5.00× | 0.727 | 3.08× | ok |
| re2 | 3.874 | 18.114 | 4.68× | 10.417 | 2.69× | ok |
| re2-test-re2 | 6.718 | 28.322 | 4.22× | 20.753 | 3.09× | ok |
| re2-test-search | 6.470 | 23.873 | 3.69× | 18.984 | 2.93× | ok |
| tramp3d-v4 | 2.726 | 23.408 | 8.59× | 13.514 | 4.96× | ok |

### Frontend only: -fsyntax-only

| Program | cppgm++ (s) | GCC (s) | Speed vs GCC ↑ | Clang (s) | Speed vs Clang ↑ | Status |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| Adobe-C++/functionobjects | 0.070 | 0.130 | 1.86× | 0.154 | 2.20× | ok |
| Adobe-C++/loop_unroll | 0.039 | 0.094 | 2.41× | 0.099 | 2.54× | ok |
| Adobe-C++/simple_types_constant_folding | 0.044 | 0.099 | 2.25× | 0.102 | 2.32× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.038 | 0.093 | 2.45× | 0.093 | 2.45× | ok |
| Adobe-C++/stepanov_abstraction | 0.036 | 0.067 | 1.86× | 0.091 | 2.53× | ok |
| Adobe-C++/stepanov_vector | 0.071 | 0.121 | 1.70× | 0.154 | 2.17× | ok |
| Bullet | 4.617 | 11.070 | 2.40× | 12.652 | 2.74× | ok |
| CLAMR | 2.441 | 5.044 | 2.07× | 5.572 | 2.28× | ok |
| CoyoteBench/almabench | 0.054 | 0.064 | 1.19× | 0.069 | 1.28× | ok |
| CoyoteBench/fftbench | 0.116 | 0.214 | 1.84× | 0.272 | 2.34× | ok |
| CoyoteBench/huffbench | 0.031 | 0.055 | 1.77× | 0.071 | 2.29× | ok |
| CoyoteBench/lpbench | 0.032 | 0.060 | 1.88× | 0.067 | 2.09× | ok |
| DOE/HACCKernels | 0.112 | 0.224 | 2.00× | 0.260 | 2.32× | ok |
| DOE/HPCCG | 0.732 | 1.325 | 1.81× | 1.949 | 2.66× | ok |
| DOE/PENNANT | 1.544 | 2.992 | 1.94× | 3.818 | 2.47× | ok |
| DOE/miniFE | 0.638 | 1.289 | 2.02× | 1.777 | 2.79× | ok |
| Misc-C++-EH/spirit | 0.373 | 0.894 | 2.40× | 1.030 | 2.76× | ok |
| Misc-C++-Large/ray | 0.115 | 0.229 | 1.99× | 0.260 | 2.26× | ok |
| Misc-C++-Large/sphereflake | 0.109 | 0.208 | 1.91× | 0.257 | 2.36× | ok |
| Misc-C++/bigfib | 0.119 | 0.240 | 2.02× | 0.312 | 2.62× | ok |
| Misc-C++/oopack_v1p8 | 0.033 | 0.071 | 2.15× | 0.075 | 2.27× | ok |
| Misc-C++/stepanov_container | 0.168 | 0.310 | 1.85× | 0.443 | 2.64× | ok |
| Misc-C++/stepanov_v1p2 | 0.032 | 0.064 | 2.00× | 0.075 | 2.34× | ok |
| PAQ8p | 0.049 | 0.122 | 2.49× | 0.131 | 2.67× | ok |
| Prolangs-C++/life | 0.004 | 0.011 | — | 0.023 | — | ok |
| Prolangs-C++/ocean | 0.011 | 0.025 | 2.27× | 0.033 | 3.00× | ok |
| Prolangs-C++/primes | 0.006 | 0.017 | — | 0.027 | — | ok |
| Shootout-C++-EH/except | 0.084 | 0.216 | 2.57× | 0.205 | 2.44× | ok |
| Shootout-C++/ackermann | 0.084 | 0.177 | 2.11× | 0.205 | 2.44× | ok |
| Shootout-C++/ary3 | 0.102 | 0.213 | 2.09× | 0.265 | 2.60× | ok |
| Shootout-C++/fibo | 0.084 | 0.160 | 1.90× | 0.181 | 2.15× | ok |
| Shootout-C++/hash | 0.106 | 0.202 | 1.91× | 0.267 | 2.52× | ok |
| Shootout-C++/hash2 | 0.114 | 0.232 | 2.04× | 0.260 | 2.28× | ok |
| Shootout-C++/heapsort | 0.096 | 0.182 | 1.90× | 0.226 | 2.35× | ok |
| Shootout-C++/lists | 0.093 | 0.171 | 1.84× | 0.258 | 2.77× | ok |
| Shootout-C++/lists1 | 0.098 | 0.181 | 1.85× | 0.261 | 2.66× | ok |
| Shootout-C++/matrix | 0.081 | 0.157 | 1.94× | 0.204 | 2.52× | ok |
| Shootout-C++/methcall | 0.079 | 0.161 | 2.04× | 0.181 | 2.29× | ok |
| Shootout-C++/moments | 0.105 | 0.207 | 1.97× | 0.245 | 2.33× | ok |
| Shootout-C++/random | 0.085 | 0.189 | 2.22× | 0.231 | 2.72× | ok |
| Shootout-C++/sieve | 0.098 | 0.192 | 1.96× | 0.229 | 2.34× | ok |
| SmallPT | 0.029 | 0.059 | 2.03× | 0.077 | 2.66× | ok |
| Stanford/Bubblesort | 0.010 | 0.025 | 2.50× | 0.042 | 4.20× | ok |
| Stanford/FloatMM | 0.011 | 0.025 | 2.27× | 0.033 | 3.00× | ok |
| Stanford/Puzzle | 0.011 | 0.026 | 2.36× | 0.068 | 6.18× | ok |
| Stanford/Treesort | 0.011 | 0.046 | 4.18× | 0.034 | 3.09× | ok |
| bg/binarytrees/2 | 0.075 | 0.161 | 2.15× | 0.185 | 2.47× | ok |
| bg/fannkuchredux/3 | 0.040 | 0.060 | 1.50× | 0.084 | 2.10× | ok |
| bg/fasta/1 | 0.056 | 0.097 | 1.73× | 0.113 | 2.02× | ok |
| bg/fasta/2 | 0.061 | 0.091 | 1.49× | 0.138 | 2.26× | ok |
| bg/fasta/3 | 0.091 | 0.168 | 1.85× | 0.204 | 2.24× | ok |
| bg/fasta/4 | 0.084 | 0.146 | 1.74× | 0.173 | 2.06× | ok |
| bg/fasta/6 | 0.137 | 0.274 | 2.00× | 0.371 | 2.71× | ok |
| bg/mandelbrot/0 | 0.167 | 0.296 | 1.77× | 0.362 | 2.17× | ok |
| bg/mandelbrot/2 | 0.095 | 0.206 | 2.17× | 0.230 | 2.42× | ok |
| bg/mandelbrot/3 | 0.104 | 0.191 | 1.84× | 0.227 | 2.18× | ok |
| bg/mandelbrot/5 | 0.053 | 0.089 | 1.68× | 0.106 | 2.00× | ok |
| bg/mandelbrot/7 | 0.046 | 0.089 | 1.93× | 0.109 | 2.37× | ok |
| bg/mandelbrot/9 | 0.044 | 0.079 | 1.80× | 0.101 | 2.30× | ok |
| bg/nbody/1 | 0.029 | 0.058 | 2.00× | 0.080 | 2.76× | ok |
| bg/nbody/2 | 0.027 | 0.060 | 2.22× | 0.067 | 2.48× | ok |
| bg/nbody/3 | 0.028 | 0.058 | 2.07× | 0.070 | 2.50× | ok |
| bg/nbody/6 | 0.028 | 0.060 | 2.14× | 0.073 | 2.61× | ok |
| bg/pidigits/4 | 0.126 | 0.253 | 2.01× | 0.351 | 2.79× | ok |
| bg/pidigits/5 | 0.095 | 0.168 | 1.77× | 0.207 | 2.18× | ok |
| bg/revcomp/1 | 0.090 | 0.169 | 1.88× | 0.216 | 2.40× | ok |
| bg/revcomp/5 | 0.093 | 0.160 | 1.72× | 0.239 | 2.57× | ok |
| bg/spectralnorm/1 | 0.120 | 0.229 | 1.91× | 0.311 | 2.59× | ok |
| box2d | 1.551 | 3.086 | 1.99× | 4.009 | 2.58× | ok |
| fmt | 0.485 | 1.073 | 2.21× | 1.491 | 3.07× | ok |
| hexxagon | 0.296 | 0.600 | 2.03× | 0.802 | 2.71× | ok |
| lambda | 0.048 | 0.134 | 2.79× | 0.171 | 3.56× | ok |
| minisat | 0.063 | 0.127 | 2.02× | 0.160 | 2.54× | ok |
| nlohmann-json | 0.351 | 0.910 | 2.59× | 1.299 | 3.70× | ok |
| pugixml | 0.243 | 0.499 | 2.05× | 0.628 | 2.58× | ok |
| pugixml-tests | 3.718 | 7.366 | 1.98× | 8.945 | 2.41× | ok |
| rapidjson | 0.118 | 0.222 | 1.88× | 0.281 | 2.38× | ok |
| re2 | 3.576 | 6.682 | 1.87× | 5.702 | 1.59× | ok |
| re2-test-re2 | 5.096 | 9.805 | 1.92× | 13.643 | 2.68× | ok |
| re2-test-search | 5.202 | 10.141 | 1.95× | 9.941 | 1.91× | ok |
| tramp3d-v4 | 0.804 | 2.409 | 3.00× | 1.570 | 1.95× | ok |

### Building the compiler

| Compiler doing the rebuild | CPU seconds |
| --- | ---: |
| GCC | 319.870 |
| GCC-built cppgm++ | 75.510 |
| Self-built cppgm++ | 78.350 |

These are the initial bootstrap builds. The GCC rebuild is measured once.

#### Paired self-built compiler measurements

| Block | GCC-built cppgm++ (s) | Self-built cppgm++ (s) | Self-built speed ↑ |
| ---: | ---: | ---: | ---: |
| 1 | 73.895 | 72.976 | 1.01× |
| 2 | 73.409 | 74.129 | 0.99× |

Each block averages its two observations per compiler. The badge uses the median block ratio; calibration and every observation are retained in the raw results.

## Toolchains

- **gcc**: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- **clang**: Ubuntu clang version 21.1.8 (6ubuntu1)
- **valgrind**: valgrind-3.26.0
