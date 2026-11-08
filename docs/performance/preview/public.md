> README preview snapshot: [Public programs CI](https://github.com/vishvananda/cppgm-extended/actions/runs/42476437424).

# Public benchmarks

The cppgm++ runtime result is our codegen benchmark: self-built versus GCC-built
compiler execution on identical compiler source inputs. Its paired measurements
appear under [paired self-built compiler measurements](#paired-self-built-compiler-measurements).

81 of 81 programs built with cppgm++ and g++ and agreed. Each number is the reference over cppgm++, so above 1× means cppgm++ is smaller or faster.

CPU: AMD EPYC 9V45 96-Core Processor.

| Measure | vs g++ | Programs | vs clang++ | Programs |
| --- | ---: | ---: | ---: | ---: |
| Frontend CPU, `-fsyntax-only` | 1.97× | 74 | 2.42× | 74 |
| Compile CPU, `-O0` | 2.75× | 76 | 2.64× | 76 |
| Compile CPU, `-O3` | 4.02× | 79 | 3.49× | 79 |
| Compile peak RSS, `-O0` | 1.48× | 81 | 2.26× | 81 |
| Compile peak RSS, `-O3` | 1.75× | 81 | 2.35× | 81 |
| Executable size, `-O0` | 0.93× | 81 | 0.88× | 81 |
| Executable size, `-O3` | 0.93× | 81 | 0.88× | 81 |
| Run CPU, `-O0` | 1.03× | 81 | 1.00× | 81 |
| Run CPU, `-O3` | 0.99× | 80 | 1.00× | 80 |
| Run instructions, `-O3` | 0.95× | 43 | - | 0 |
| Self compile CPU | 4.24× | - | - | - |
| Self-built compiler CPU | 0.97× | - | - | - |

O3 execution uses paired ABBA/BAAB blocks. GCC A/A CPU ratios ranged from 0.938 to 1.333; raw observations and block ratios are retained in the artifact.

Self-built / GCC-built compiler CPU: paired ratios 1.015–1.048; A/A calibration 1.004. A/A variation limits conclusions about small changes.

## Changes from the baseline

| Program | Measure | Baseline | Now | Change |
| --- | --- | ---: | ---: | ---: |
| tramp3d-v4 | compile_instructions | 11,449,094,053 | 20,058,504,850 | +75.20% |
| pugixml | compile_instructions | 2,919,890,092 | 4,126,240,753 | +41.31% |
| PAQ8p | compile_instructions | 1,139,925,750 | 1,496,039,340 | +31.24% |
| nlohmann-json | compile_instructions | 3,139,544,230 | 4,026,402,314 | +28.25% |
| rapidjson | compile_instructions | 994,061,605 | 1,226,879,198 | +23.42% |
| fmt | compile_instructions | 4,859,824,457 | 5,765,848,456 | +18.64% |
| Stanford/Bubblesort | run_instructions | 151,796,908 | 176,515,188 | +16.28% |
| Misc-C++-EH/spirit | compile_instructions | 3,068,114,435 | 3,527,695,668 | +14.98% |
| bg/fannkuchredux/3 | run_instructions | 1,050,803,386 | 1,172,769,753 | +11.61% |
| re2 | compile_instructions | 20,355,262,314 | 21,766,677,611 | +6.93% |
| Adobe-C++/stepanov_vector | compile_instructions | 590,222,434 | 627,147,834 | +6.26% |
| bg/spectralnorm/1 | compile_instructions | 695,497,664 | 738,697,702 | +6.21% |
| Misc-C++/oopack_v1p8 | compile_instructions | 208,607,906 | 218,037,140 | +4.52% |
| box2d | compile_instructions | 9,934,011,925 | 10,290,645,552 | +3.59% |
| DOE/PENNANT | compile_instructions | 9,849,122,669 | 10,133,677,527 | +2.89% |
| tramp3d-v4 | frontend_instructions | 2,713,398,166 | 2,765,055,144 | +1.90% |
| nlohmann-json | frontend_instructions | 1,698,414,513 | 1,724,985,926 | +1.56% |
| Misc-C++-EH/spirit | frontend_instructions | 1,545,878,671 | 1,569,863,571 | +1.55% |
| re2 | frontend_instructions | 16,566,292,691 | 16,821,700,739 | +1.54% |
| bg/revcomp/5 | run_instructions | 2,224,013,522 | 2,257,980,858 | +1.53% |
| fmt | frontend_instructions | 2,558,297,102 | 2,591,984,014 | +1.32% |
| pugixml | frontend_instructions | 1,234,242,416 | 1,249,991,580 | +1.28% |
| PAQ8p | frontend_instructions | 275,335,444 | 278,835,478 | +1.27% |
| DOE/PENNANT | frontend_instructions | 8,485,981,495 | 8,590,499,776 | +1.23% |
| rapidjson | frontend_instructions | 550,453,783 | 556,974,336 | +1.18% |
| Adobe-C++/stepanov_vector | frontend_instructions | 392,489,333 | 397,100,481 | +1.17% |
| bg/spectralnorm/1 | frontend_instructions | 671,115,838 | 677,979,668 | +1.02% |
| Misc-C++/oopack_v1p8 | frontend_instructions | 191,031,660 | 192,971,110 | +1.02% |
| box2d | frontend_instructions | 8,752,966,343 | 8,841,046,096 | +1.01% |
| pugixml | run_instructions | 4,754,044,832 | 4,608,064,592 | -3.07% |
| Shootout-C++/lists1 | run_instructions | 1,562,737,607 | 1,508,737,722 | -3.46% |
| Stanford/Treesort | run_instructions | 175,990,344 | 166,103,497 | -5.62% |
| Shootout-C++/heapsort | run_instructions | 3,757,120,356 | 3,363,932,275 | -10.47% |
| DOE/HPCCG | run_instructions | 5,039,328,454 | 4,466,803,365 | -11.36% |
| Prolangs-C++/primes | run_instructions | 797,255,660 | 704,402,345 | -11.65% |
| Shootout-C++/sieve | run_instructions | 4,452,150,713 | 3,867,272,089 | -13.14% |
| DOE/PENNANT | run_instructions | 4,092,626,352 | 3,339,319,451 | -18.41% |
| CoyoteBench/fftbench | run_instructions | 2,123,761,346 | 1,637,741,724 | -22.88% |
| bg/nbody/1 | run_instructions | 1,586,173,648 | 1,218,140,347 | -23.20% |
| Shootout-C++/ackermann | run_instructions | 5,607,887,291 | 4,295,817,072 | -23.40% |
| bg/nbody/6 | run_instructions | 1,606,173,648 | 1,208,140,347 | -24.78% |
| bg/mandelbrot/9 | run_instructions | 2,959,701,410 | 2,138,509,064 | -27.75% |
| bg/mandelbrot/7 | run_instructions | 2,981,714,397 | 2,144,023,309 | -28.09% |
| Misc-C++/bigfib | run_instructions | 1,234,835,296 | 883,110,829 | -28.48% |
| bg/mandelbrot/3 | run_instructions | 2,753,035,483 | 1,958,066,214 | -28.88% |
| bg/fasta/4 | run_instructions | 889,561,739 | 628,870,249 | -29.31% |
| Shootout-C++/moments | run_instructions | 316,927,036 | 211,925,884 | -33.13% |
| bg/fasta/3 | run_instructions | 1,053,846,469 | 698,206,336 | -33.75% |
| bg/nbody/2 | run_instructions | 2,926,173,281 | 1,910,139,952 | -34.72% |
| bg/fasta/1 | run_instructions | 2,551,027,529 | 1,587,734,155 | -37.76% |
| bg/nbody/3 | run_instructions | 2,140,173,231 | 1,302,139,935 | -39.16% |
| bg/fasta/2 | run_instructions | 2,551,027,475 | 1,547,736,033 | -39.33% |
| bg/mandelbrot/0 | run_instructions | 2,140,529,699 | 1,268,802,001 | -40.72% |
| bg/mandelbrot/5 | run_instructions | 3,737,802,094 | 2,197,870,988 | -41.20% |
| bg/spectralnorm/1 | run_instructions | 882,313,691 | 503,779,101 | -42.90% |
| Misc-C++/oopack_v1p8 | run_instructions | 1,258,433,238 | 712,925,959 | -43.35% |
| Stanford/Puzzle | run_instructions | 944,979,088 | 498,161,165 | -47.28% |
| PAQ8p | run_instructions | 800,503,982 | 405,298,802 | -49.37% |
| bg/mandelbrot/2 | run_instructions | 4,455,769,865 | 2,231,042,709 | -49.93% |
| Stanford/FloatMM | run_instructions | 4,003,542,037 | 1,110,762,328 | -72.26% |
| tramp3d-v4 | run_instructions | 4,156,173,902 | 894,952,917 | -78.47% |

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
| Adobe-C++/functionobjects | 0.065 | 0.213 | 3.28× | 0.182 | 2.80× | ok |
| Adobe-C++/loop_unroll | 0.046 | 0.449 | 9.76× | 0.144 | 3.13× | ok |
| Adobe-C++/simple_types_constant_folding | 0.054 | 0.541 | 10.02× | 0.181 | 3.35× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.044 | 0.384 | 8.73× | 0.147 | 3.34× | ok |
| Adobe-C++/stepanov_abstraction | 0.037 | 0.149 | 4.03× | 0.094 | 2.54× | ok |
| Adobe-C++/stepanov_vector | 0.066 | 0.229 | 3.47× | 0.194 | 2.94× | ok |
| Bullet | 4.717 | 15.018 | 3.18× | 13.706 | 2.91× | ok |
| CLAMR | 2.277 | 7.373 | 3.24× | 6.875 | 3.02× | ok |
| CoyoteBench/almabench | 0.041 | 0.064 | 1.56× | 0.070 | 1.71× | ok |
| CoyoteBench/fftbench | 0.103 | 0.214 | 2.08× | 0.258 | 2.50× | ok |
| CoyoteBench/huffbench | 0.027 | 0.061 | 2.26× | 0.065 | 2.41× | ok |
| CoyoteBench/lpbench | 0.028 | 0.066 | 2.36× | 0.072 | 2.57× | ok |
| DOE/HACCKernels | 0.116 | 0.249 | 2.15× | 0.307 | 2.65× | ok |
| DOE/HPCCG | 0.642 | 1.456 | 2.27× | 1.923 | 3.00× | ok |
| DOE/PENNANT | 1.403 | 3.664 | 2.61× | 3.492 | 2.49× | ok |
| DOE/miniFE | 0.638 | 1.694 | 2.66× | 1.698 | 2.66× | ok |
| Misc-C++-EH/spirit | 0.400 | 2.118 | 5.29× | 1.509 | 3.77× | ok |
| Misc-C++-Large/ray | 0.104 | 0.231 | 2.22× | 0.280 | 2.69× | ok |
| Misc-C++-Large/sphereflake | 0.093 | 0.204 | 2.19× | 0.198 | 2.13× | ok |
| Misc-C++/bigfib | 0.114 | 0.312 | 2.74× | 0.353 | 3.10× | ok |
| Misc-C++/oopack_v1p8 | 0.030 | 0.081 | 2.70× | 0.074 | 2.47× | ok |
| Misc-C++/stepanov_container | 0.149 | 0.463 | 3.11× | 0.483 | 3.24× | ok |
| Misc-C++/stepanov_v1p2 | 0.028 | 0.104 | 3.71× | 0.078 | 2.79× | ok |
| PAQ8p | 0.064 | 0.385 | 6.02× | 0.185 | 2.89× | ok |
| Prolangs-C++/life | 0.003 | 0.027 | — | 0.024 | — | ok |
| Prolangs-C++/ocean | 0.010 | 0.055 | 5.50× | 0.038 | 3.80× | ok |
| Prolangs-C++/primes | 0.006 | 0.021 | — | 0.024 | — | ok |
| Shootout-C++-EH/except | 0.073 | 0.133 | 1.82× | 0.197 | 2.70× | ok |
| Shootout-C++/ackermann | 0.071 | 0.130 | 1.83× | 0.174 | 2.45× | ok |
| Shootout-C++/ary3 | 0.077 | 0.177 | 2.30× | 0.235 | 3.05× | ok |
| Shootout-C++/fibo | 0.078 | 0.131 | 1.68× | 0.164 | 2.10× | ok |
| Shootout-C++/hash | 0.105 | 0.242 | 2.30× | 0.267 | 2.54× | ok |
| Shootout-C++/hash2 | 0.107 | 0.249 | 2.33× | 0.248 | 2.32× | ok |
| Shootout-C++/heapsort | 0.082 | 0.147 | 1.79× | 0.194 | 2.37× | ok |
| Shootout-C++/lists | 0.078 | 0.173 | 2.22× | 0.178 | 2.28× | ok |
| Shootout-C++/lists1 | 0.084 | 0.197 | 2.35× | 0.206 | 2.45× | ok |
| Shootout-C++/matrix | 0.066 | 0.130 | 1.97× | 0.166 | 2.52× | ok |
| Shootout-C++/methcall | 0.067 | 0.130 | 1.94× | 0.176 | 2.63× | ok |
| Shootout-C++/moments | 0.093 | 0.210 | 2.26× | 0.211 | 2.27× | ok |
| Shootout-C++/random | 0.083 | 0.156 | 1.88× | 0.190 | 2.29× | ok |
| Shootout-C++/sieve | 0.086 | 0.194 | 2.26× | 0.245 | 2.85× | ok |
| SmallPT | 0.027 | 0.068 | 2.52× | 0.068 | 2.52× | ok |
| Stanford/Bubblesort | 0.009 | 0.026 | — | 0.028 | — | ok |
| Stanford/FloatMM | 0.009 | 0.026 | — | 0.027 | — | ok |
| Stanford/Puzzle | 0.010 | 0.031 | 3.10× | 0.030 | 3.00× | ok |
| Stanford/Treesort | 0.009 | 0.028 | — | 0.028 | — | ok |
| bg/binarytrees/2 | 0.067 | 0.132 | 1.97× | 0.160 | 2.39× | ok |
| bg/fannkuchredux/3 | 0.034 | 0.064 | 1.88× | 0.071 | 2.09× | ok |
| bg/fasta/1 | 0.052 | 0.107 | 2.06× | 0.105 | 2.02× | ok |
| bg/fasta/2 | 0.054 | 0.107 | 1.98× | 0.118 | 2.19× | ok |
| bg/fasta/3 | 0.078 | 0.148 | 1.90× | 0.190 | 2.44× | ok |
| bg/fasta/4 | 0.074 | 0.131 | 1.77× | 0.150 | 2.03× | ok |
| bg/fasta/6 | 0.130 | 0.318 | 2.45× | 0.351 | 2.70× | ok |
| bg/mandelbrot/0 | 0.145 | 0.403 | 2.78× | 0.355 | 2.45× | ok |
| bg/mandelbrot/2 | 0.087 | 0.184 | 2.11× | 0.228 | 2.62× | ok |
| bg/mandelbrot/3 | 0.092 | 0.170 | 1.85× | 0.213 | 2.32× | ok |
| bg/mandelbrot/5 | 0.043 | 0.090 | 2.09× | 0.097 | 2.26× | ok |
| bg/mandelbrot/7 | 0.042 | 0.100 | 2.38× | 0.112 | 2.67× | ok |
| bg/mandelbrot/9 | 0.043 | 0.095 | 2.21× | 0.113 | 2.63× | ok |
| bg/nbody/1 | 0.029 | 0.085 | 2.93× | 0.072 | 2.48× | ok |
| bg/nbody/2 | 0.026 | 0.062 | 2.38× | 0.062 | 2.38× | ok |
| bg/nbody/3 | 0.026 | 0.066 | 2.54× | 0.064 | 2.46× | ok |
| bg/nbody/6 | 0.027 | 0.083 | 3.07× | 0.088 | 3.26× | ok |
| bg/pidigits/4 | 0.115 | 0.248 | 2.16× | 0.319 | 2.77× | ok |
| bg/pidigits/5 | 0.074 | 0.145 | 1.96× | 0.196 | 2.65× | ok |
| bg/revcomp/1 | 0.083 | 0.177 | 2.13× | 0.194 | 2.34× | ok |
| bg/revcomp/5 | 0.078 | 0.164 | 2.10× | 0.208 | 2.67× | ok |
| bg/spectralnorm/1 | 0.105 | 0.210 | 2.00× | 0.256 | 2.44× | ok |
| box2d | 1.471 | 3.599 | 2.45× | 3.754 | 2.55× | ok |
| fmt | 0.450 | 2.027 | 4.50× | 1.616 | 3.59× | ok |
| hexxagon | 0.259 | 0.511 | 1.97× | 0.667 | 2.58× | ok |
| lambda | 0.049 | 0.250 | 5.10× | 0.203 | 4.14× | ok |
| minisat | 0.058 | 0.268 | 4.62× | 0.209 | 3.60× | ok |
| nlohmann-json | 0.347 | 1.394 | 4.02× | 1.191 | 3.43× | ok |
| pugixml | 0.249 | 1.141 | 4.58× | 0.812 | 3.26× | ok |
| pugixml-tests | 3.430 | 12.295 | 3.58× | 10.535 | 3.07× | ok |
| rapidjson | 0.114 | 0.384 | 3.37× | 0.308 | 2.70× | ok |
| re2 | 2.985 | 8.624 | 2.89× | 5.428 | 1.82× | ok |
| re2-test-re2 | 4.572 | 12.250 | 2.68× | 12.622 | 2.76× | ok |
| re2-test-search | 4.535 | 11.883 | 2.62× | 7.513 | 1.66× | ok |
| tramp3d-v4 | 0.623 | 7.446 | 11.95× | 2.000 | 3.21× | ok |

### O3 compilation

| Program | cppgm++ (s) | GCC (s) | Speed vs GCC ↑ | Clang (s) | Speed vs Clang ↑ | Status |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| Adobe-C++/functionobjects | 0.089 | 0.769 | 8.64× | 0.407 | 4.57× | ok |
| Adobe-C++/loop_unroll | 0.211 | 4.335 | 20.55× | 3.881 | 18.39× | ok |
| Adobe-C++/simple_types_constant_folding | 0.129 | 2.438 | 18.90× | 1.898 | 14.71× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.111 | 1.912 | 17.23× | 1.547 | 13.94× | ok |
| Adobe-C++/stepanov_abstraction | 0.062 | 0.829 | 13.37× | 0.391 | 6.31× | ok |
| Adobe-C++/stepanov_vector | 0.104 | 1.092 | 10.50× | 0.443 | 4.26× | ok |
| Bullet | 6.308 | 31.560 | 5.00× | 23.592 | 3.74× | ok |
| CLAMR | 3.730 | 18.602 | 4.99× | 14.894 | 3.99× | ok |
| CoyoteBench/almabench | 0.030 | 0.135 | 4.50× | 0.099 | 3.30× | ok |
| CoyoteBench/fftbench | 0.113 | 0.347 | 3.07× | 0.382 | 3.38× | ok |
| CoyoteBench/huffbench | 0.036 | 0.174 | 4.83× | 0.115 | 3.19× | ok |
| CoyoteBench/lpbench | 0.038 | 0.154 | 4.05× | 0.171 | 4.50× | ok |
| DOE/HACCKernels | 0.124 | 0.345 | 2.78× | 0.362 | 2.92× | ok |
| DOE/HPCCG | 0.670 | 1.961 | 2.93× | 2.209 | 3.30× | ok |
| DOE/PENNANT | 1.594 | 5.115 | 3.21× | 4.770 | 2.99× | ok |
| DOE/miniFE | 0.804 | 3.530 | 4.39× | 3.010 | 3.74× | ok |
| Misc-C++-EH/spirit | 0.593 | 3.057 | 5.16× | 2.372 | 4.00× | ok |
| Misc-C++-Large/ray | 0.113 | 0.416 | 3.68× | 0.356 | 3.15× | ok |
| Misc-C++-Large/sphereflake | 0.103 | 0.624 | 6.06× | 0.307 | 2.98× | ok |
| Misc-C++/bigfib | 0.141 | 0.445 | 3.16× | 0.421 | 2.99× | ok |
| Misc-C++/oopack_v1p8 | 0.034 | 0.192 | 5.65× | 0.133 | 3.91× | ok |
| Misc-C++/stepanov_container | 0.200 | 0.727 | 3.63× | 0.758 | 3.79× | ok |
| Misc-C++/stepanov_v1p2 | 0.035 | 0.169 | 4.83× | 0.150 | 4.29× | ok |
| PAQ8p | 0.234 | 3.136 | 13.40× | 1.283 | 5.48× | ok |
| Prolangs-C++/life | 0.007 | 0.085 | — | 0.057 | — | ok |
| Prolangs-C++/ocean | 0.017 | 0.169 | 9.94× | 0.096 | 5.65× | ok |
| Prolangs-C++/primes | 0.007 | 0.031 | — | 0.031 | — | ok |
| Shootout-C++-EH/except | 0.076 | 0.158 | 2.08× | 0.221 | 2.91× | ok |
| Shootout-C++/ackermann | 0.073 | 0.165 | 2.26× | 0.177 | 2.42× | ok |
| Shootout-C++/ary3 | 0.091 | 0.207 | 2.27× | 0.232 | 2.55× | ok |
| Shootout-C++/fibo | 0.073 | 0.167 | 2.29× | 0.170 | 2.33× | ok |
| Shootout-C++/hash | 0.114 | 0.324 | 2.84× | 0.330 | 2.89× | ok |
| Shootout-C++/hash2 | 0.129 | 0.363 | 2.81× | 0.379 | 2.94× | ok |
| Shootout-C++/heapsort | 0.082 | 0.175 | 2.13× | 0.210 | 2.56× | ok |
| Shootout-C++/lists | 0.088 | 0.194 | 2.20× | 0.210 | 2.39× | ok |
| Shootout-C++/lists1 | 0.090 | 0.238 | 2.64× | 0.237 | 2.63× | ok |
| Shootout-C++/matrix | 0.070 | 0.175 | 2.50× | 0.240 | 3.43× | ok |
| Shootout-C++/methcall | 0.073 | 0.157 | 2.15× | 0.171 | 2.34× | ok |
| Shootout-C++/moments | 0.101 | 0.259 | 2.56× | 0.264 | 2.61× | ok |
| Shootout-C++/random | 0.085 | 0.175 | 2.06× | 0.204 | 2.40× | ok |
| Shootout-C++/sieve | 0.092 | 0.212 | 2.30× | 0.246 | 2.67× | ok |
| SmallPT | 0.035 | 0.237 | 6.77× | 0.134 | 3.83× | ok |
| Stanford/Bubblesort | 0.011 | 0.047 | 4.27× | 0.043 | 3.91× | ok |
| Stanford/FloatMM | 0.011 | 0.052 | 4.73× | 0.059 | 5.36× | ok |
| Stanford/Puzzle | 0.015 | 0.100 | 6.67× | 0.096 | 6.40× | ok |
| Stanford/Treesort | 0.012 | 0.071 | 5.92× | 0.048 | 4.00× | ok |
| bg/binarytrees/2 | 0.070 | 0.325 | 4.64× | 0.189 | 2.70× | ok |
| bg/fannkuchredux/3 | 0.039 | 0.104 | 2.67× | 0.098 | 2.51× | ok |
| bg/fasta/1 | 0.053 | 0.134 | 2.53× | 0.138 | 2.60× | ok |
| bg/fasta/2 | 0.060 | 0.140 | 2.33× | 0.146 | 2.43× | ok |
| bg/fasta/3 | 0.084 | 0.186 | 2.21× | 0.218 | 2.60× | ok |
| bg/fasta/4 | 0.081 | 0.160 | 1.98× | 0.192 | 2.37× | ok |
| bg/fasta/6 | 0.147 | 0.413 | 2.81× | 0.395 | 2.69× | ok |
| bg/mandelbrot/0 | 0.164 | 0.521 | 3.18× | 0.508 | 3.10× | ok |
| bg/mandelbrot/2 | 0.092 | 0.249 | 2.71× | 0.242 | 2.63× | ok |
| bg/mandelbrot/3 | 0.091 | 0.199 | 2.19× | 0.230 | 2.53× | ok |
| bg/mandelbrot/5 | 0.047 | 0.122 | 2.60× | 0.122 | 2.60× | ok |
| bg/mandelbrot/7 | 0.048 | 0.151 | 3.15× | 0.138 | 2.88× | ok |
| bg/mandelbrot/9 | 0.051 | 0.147 | 2.88× | 0.132 | 2.59× | ok |
| bg/nbody/1 | 0.035 | 0.127 | 3.63× | 0.154 | 4.40× | ok |
| bg/nbody/2 | 0.030 | 0.105 | 3.50× | 0.101 | 3.37× | ok |
| bg/nbody/3 | 0.028 | 0.099 | 3.54× | 0.089 | 3.18× | ok |
| bg/nbody/6 | 0.033 | 0.134 | 4.06× | 0.146 | 4.42× | ok |
| bg/pidigits/4 | 0.121 | 0.299 | 2.47× | 0.354 | 2.93× | ok |
| bg/pidigits/5 | 0.077 | 0.170 | 2.21× | 0.234 | 3.04× | ok |
| bg/revcomp/1 | 0.087 | 0.201 | 2.31× | 0.229 | 2.63× | ok |
| bg/revcomp/5 | 0.084 | 0.355 | 4.23× | 0.245 | 2.92× | ok |
| bg/spectralnorm/1 | 0.113 | 0.422 | 3.73× | 0.329 | 2.91× | ok |
| box2d | 1.711 | 6.354 | 3.71× | 5.871 | 3.43× | ok |
| fmt | 0.864 | 4.447 | 5.15× | 3.351 | 3.88× | ok |
| hexxagon | 0.278 | 0.851 | 3.06× | 0.856 | 3.08× | ok |
| lambda | 0.096 | 0.886 | 9.23× | 0.464 | 4.83× | ok |
| minisat | 0.121 | 0.769 | 6.36× | 0.501 | 4.14× | ok |
| nlohmann-json | 0.610 | 2.540 | 4.16× | 2.328 | 3.82× | ok |
| pugixml | 0.637 | 4.560 | 7.16× | 3.040 | 4.77× | ok |
| pugixml-tests | 5.092 | 28.566 | 5.61× | 19.482 | 3.83× | ok |
| rapidjson | 0.196 | 0.963 | 4.91× | 0.591 | 3.02× | ok |
| re2 | 2.961 | 14.627 | 4.94× | 8.040 | 2.72× | ok |
| re2-test-re2 | 5.426 | 22.208 | 4.09× | 17.727 | 3.27× | ok |
| re2-test-search | 5.457 | 20.122 | 3.69× | 11.672 | 2.14× | ok |
| tramp3d-v4 | 2.211 | 15.532 | 7.02× | 11.127 | 5.03× | ok |

### Frontend only: -fsyntax-only

| Program | cppgm++ (s) | GCC (s) | Speed vs GCC ↑ | Clang (s) | Speed vs Clang ↑ | Status |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| Adobe-C++/functionobjects | 0.061 | 0.115 | 1.89× | 0.132 | 2.16× | ok |
| Adobe-C++/loop_unroll | 0.036 | 0.087 | 2.42× | 0.086 | 2.39× | ok |
| Adobe-C++/simple_types_constant_folding | 0.041 | 0.086 | 2.10× | 0.092 | 2.24× | ok |
| Adobe-C++/simple_types_loop_invariant | 0.034 | 0.081 | 2.38× | 0.084 | 2.47× | ok |
| Adobe-C++/stepanov_abstraction | 0.032 | 0.060 | 1.88× | 0.074 | 2.31× | ok |
| Adobe-C++/stepanov_vector | 0.062 | 0.104 | 1.68× | 0.157 | 2.53× | ok |
| Bullet | 4.475 | 9.659 | 2.16× | 12.090 | 2.70× | ok |
| CLAMR | 1.965 | 4.269 | 2.17× | 4.971 | 2.53× | ok |
| CoyoteBench/almabench | 0.050 | 0.053 | 1.06× | 0.068 | 1.36× | ok |
| CoyoteBench/fftbench | 0.103 | 0.189 | 1.83× | 0.236 | 2.29× | ok |
| CoyoteBench/huffbench | 0.027 | 0.052 | 1.93× | 0.062 | 2.30× | ok |
| CoyoteBench/lpbench | 0.028 | 0.054 | 1.93× | 0.071 | 2.54× | ok |
| DOE/HACCKernels | 0.112 | 0.219 | 1.96× | 0.281 | 2.51× | ok |
| DOE/HPCCG | 0.622 | 1.328 | 2.14× | 1.803 | 2.90× | ok |
| DOE/PENNANT | 1.449 | 2.706 | 1.87× | 2.944 | 2.03× | ok |
| DOE/miniFE | 0.567 | 1.173 | 2.07× | 1.445 | 2.55× | ok |
| Misc-C++-EH/spirit | 0.286 | 0.701 | 2.45× | 1.076 | 3.76× | ok |
| Misc-C++-Large/ray | 0.104 | 0.190 | 1.83× | 0.263 | 2.53× | ok |
| Misc-C++-Large/sphereflake | 0.090 | 0.177 | 1.97× | 0.185 | 2.06× | ok |
| Misc-C++/bigfib | 0.105 | 0.215 | 2.05× | 0.315 | 3.00× | ok |
| Misc-C++/oopack_v1p8 | 0.029 | 0.058 | 2.00× | 0.066 | 2.28× | ok |
| Misc-C++/stepanov_container | 0.137 | 0.258 | 1.88× | 0.393 | 2.87× | ok |
| Misc-C++/stepanov_v1p2 | 0.028 | 0.057 | 2.04× | 0.066 | 2.36× | ok |
| PAQ8p | 0.041 | 0.107 | 2.61× | 0.104 | 2.54× | ok |
| Prolangs-C++/life | 0.003 | 0.009 | — | 0.019 | — | ok |
| Prolangs-C++/ocean | 0.009 | 0.021 | — | 0.027 | — | ok |
| Prolangs-C++/primes | 0.006 | 0.015 | — | 0.022 | — | ok |
| Shootout-C++-EH/except | 0.073 | 0.132 | 1.81× | 0.182 | 2.49× | ok |
| Shootout-C++/ackermann | 0.070 | 0.131 | 1.87× | 0.161 | 2.30× | ok |
| Shootout-C++/ary3 | 0.077 | 0.160 | 2.08× | 0.217 | 2.82× | ok |
| Shootout-C++/fibo | 0.070 | 0.133 | 1.90× | 0.154 | 2.20× | ok |
| Shootout-C++/hash | 0.098 | 0.182 | 1.86× | 0.221 | 2.26× | ok |
| Shootout-C++/hash2 | 0.101 | 0.185 | 1.83× | 0.220 | 2.18× | ok |
| Shootout-C++/heapsort | 0.082 | 0.150 | 1.83× | 0.186 | 2.27× | ok |
| Shootout-C++/lists | 0.075 | 0.141 | 1.88× | 0.161 | 2.15× | ok |
| Shootout-C++/lists1 | 0.081 | 0.162 | 2.00× | 0.185 | 2.28× | ok |
| Shootout-C++/matrix | 0.066 | 0.126 | 1.91× | 0.157 | 2.38× | ok |
| Shootout-C++/methcall | 0.068 | 0.130 | 1.91× | 0.167 | 2.46× | ok |
| Shootout-C++/moments | 0.089 | 0.166 | 1.87× | 0.193 | 2.17× | ok |
| Shootout-C++/random | 0.082 | 0.155 | 1.89× | 0.182 | 2.22× | ok |
| Shootout-C++/sieve | 0.084 | 0.158 | 1.88× | 0.222 | 2.64× | ok |
| SmallPT | 0.025 | 0.046 | 1.84× | 0.062 | 2.48× | ok |
| Stanford/Bubblesort | 0.009 | 0.023 | — | 0.028 | — | ok |
| Stanford/FloatMM | 0.009 | 0.022 | — | 0.027 | — | ok |
| Stanford/Puzzle | 0.009 | 0.023 | — | 0.028 | — | ok |
| Stanford/Treesort | 0.009 | 0.022 | — | 0.028 | — | ok |
| bg/binarytrees/2 | 0.066 | 0.133 | 2.02× | 0.155 | 2.35× | ok |
| bg/fannkuchredux/3 | 0.033 | 0.054 | 1.64× | 0.067 | 2.03× | ok |
| bg/fasta/1 | 0.050 | 0.085 | 1.70× | 0.098 | 1.96× | ok |
| bg/fasta/2 | 0.051 | 0.084 | 1.65× | 0.102 | 2.00× | ok |
| bg/fasta/3 | 0.077 | 0.137 | 1.78× | 0.187 | 2.43× | ok |
| bg/fasta/4 | 0.077 | 0.126 | 1.64× | 0.142 | 1.84× | ok |
| bg/fasta/6 | 0.124 | 0.231 | 1.86× | 0.331 | 2.67× | ok |
| bg/mandelbrot/0 | 0.134 | 0.265 | 1.98× | 0.303 | 2.26× | ok |
| bg/mandelbrot/2 | 0.085 | 0.159 | 1.87× | 0.209 | 2.46× | ok |
| bg/mandelbrot/3 | 0.090 | 0.166 | 1.84× | 0.204 | 2.27× | ok |
| bg/mandelbrot/5 | 0.042 | 0.073 | 1.74× | 0.091 | 2.17× | ok |
| bg/mandelbrot/7 | 0.040 | 0.073 | 1.82× | 0.097 | 2.43× | ok |
| bg/mandelbrot/9 | 0.041 | 0.069 | 1.68× | 0.093 | 2.27× | ok |
| bg/nbody/1 | 0.028 | 0.051 | 1.82× | 0.063 | 2.25× | ok |
| bg/nbody/2 | 0.025 | 0.051 | 2.04× | 0.058 | 2.32× | ok |
| bg/nbody/3 | 0.026 | 0.053 | 2.04× | 0.061 | 2.35× | ok |
| bg/nbody/6 | 0.026 | 0.055 | 2.12× | 0.072 | 2.77× | ok |
| bg/pidigits/4 | 0.108 | 0.220 | 2.04× | 0.299 | 2.77× | ok |
| bg/pidigits/5 | 0.072 | 0.139 | 1.93× | 0.175 | 2.43× | ok |
| bg/revcomp/1 | 0.077 | 0.142 | 1.84× | 0.174 | 2.26× | ok |
| bg/revcomp/5 | 0.073 | 0.134 | 1.84× | 0.185 | 2.53× | ok |
| bg/spectralnorm/1 | 0.103 | 0.190 | 1.84× | 0.235 | 2.28× | ok |
| box2d | 1.368 | 2.718 | 1.99× | 3.472 | 2.54× | ok |
| fmt | 0.413 | 0.889 | 2.15× | 1.302 | 3.15× | ok |
| hexxagon | 0.246 | 0.459 | 1.87× | 0.638 | 2.59× | ok |
| lambda | 0.041 | 0.109 | 2.66× | 0.144 | 3.51× | ok |
| minisat | 0.049 | 0.122 | 2.49× | 0.156 | 3.18× | ok |
| nlohmann-json | 0.290 | 0.637 | 2.20× | 0.903 | 3.11× | ok |
| pugixml | 0.202 | 0.423 | 2.09× | 0.614 | 3.04× | ok |
| pugixml-tests | 2.979 | 6.166 | 2.07× | 8.238 | 2.77× | ok |
| rapidjson | 0.093 | 0.200 | 2.15× | 0.241 | 2.59× | ok |
| re2 | 2.866 | 5.525 | 1.93× | 4.481 | 1.56× | ok |
| re2-test-re2 | 4.053 | 7.954 | 1.96× | 10.458 | 2.58× | ok |
| re2-test-search | 4.075 | 8.092 | 1.99× | 8.197 | 2.01× | ok |
| tramp3d-v4 | 0.398 | 2.104 | 5.29× | 1.191 | 2.99× | ok |

### Building the compiler

| Compiler doing the rebuild | CPU seconds |
| --- | ---: |
| GCC | 279.080 |
| GCC-built cppgm++ | 65.870 |
| Self-built cppgm++ | 67.380 |

These are the initial bootstrap builds. The GCC rebuild is measured once.

#### Paired self-built compiler measurements

| Block | GCC-built cppgm++ (s) | Self-built cppgm++ (s) | Self-built speed ↑ |
| ---: | ---: | ---: | ---: |
| 1 | 67.447 | 70.708 | 0.95× |
| 2 | 68.736 | 69.754 | 0.99× |

Each block averages its two observations per compiler. The badge uses the median block ratio; calibration and every observation are retained in the raw results.
