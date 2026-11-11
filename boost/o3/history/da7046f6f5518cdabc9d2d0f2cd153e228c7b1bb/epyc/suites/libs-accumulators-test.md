# libs/accumulators/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| count.test | RUN | 0.655552 | 1.541631 | 1.532917 | 0.003671 | 0.002981 | 0.002528 |
| covariance.test | RUN | 1.293714 | 3.753072 | 4.126362 | 0.003221 | 0.002842 | 0.003003 |
| droppable.test | RUN | 0.871934 | 1.922342 | 2.203515 | 0.003381 | 0.002970 | 0.002710 |
| error_of.test | RUN | 0.966614 | 1.870917 | 2.433411 | 0.003307 | 0.002568 | 0.002490 |
| extended_p_square.test | RUN | 0.952500 | 2.438014 | 2.422595 | 0.004325 | 0.004128 | 0.003856 |
| extended_p_square_quantile.test | RUN | 1.210975 | 3.811123 | 3.485783 | 0.012436 | 0.007299 | 0.005978 |
| external_accumulator.test | RUN | 0.797172 | 1.817293 | 2.117219 | 0.003312 | 0.002962 | 0.002837 |
| external_weights.test | RUN | 0.711276 | 1.438336 | 1.704935 | 0.002998 | 0.002641 | 0.002676 |
| kurtosis.test | RUN | 1.077237 | 2.338011 | 2.710572 | 0.008295 | 0.004191 | 0.003349 |
| max.test | RUN | 0.686684 | 1.406081 | 1.538583 | 0.003345 | 0.002386 | 0.002639 |
| mean.test | RUN | 0.951829 | 2.009457 | 2.242709 | 0.003085 | 0.002909 | 0.002678 |
| median.test | RUN | 1.248258 | 3.899941 | 3.448691 | 0.090028 | 0.092679 | 0.084440 |
| min.test | RUN | 0.688277 | 1.429012 | 1.486756 | 0.002874 | 0.002589 | 0.002856 |
| moment.test | RUN | 0.764730 | 1.637724 | 1.789565 | 0.003288 | 0.002849 | 0.002683 |
| p_square_cumul_dist.test | RUN | 1.012110 | 2.802868 | 2.577757 | 0.690274 | 0.754628 | 0.604731 |
| p_square_quantile.test | RUN | 0.895023 | 2.528200 | 2.111810 | 0.022307 | 0.020906 | 0.018417 |
| reference.test | RUN | 0.787817 | 1.549342 | 1.808845 | 0.003218 | 0.002892 | 0.002630 |
| rolling_count.test | RUN | 0.784732 | 1.838253 | 1.781537 | 0.003049 | 0.002876 | 0.002699 |
| rolling_sum.test | RUN | 0.760334 | 1.839787 | 1.857740 | 0.003343 | 0.002740 | 0.002681 |
| rolling_mean.test | RUN | 1.105608 | 3.279464 | 3.059356 | 0.003003 | 0.002820 | 0.002801 |
| rolling_variance.test | RUN | 1.224804 | 3.834462 | 3.510799 | 0.003404 | 0.002877 | 0.002620 |
| rolling_moment.test | RUN | 0.913506 | 2.188481 | 2.297036 | 0.003340 | 0.002899 | 0.002873 |
| skewness.test | RUN | 1.017308 | 2.265453 | 2.564982 | 0.004925 | 0.004332 | 0.003438 |
| sum.test | RUN | 0.712321 | 1.539421 | 1.620069 | 0.003020 | 0.002961 | 0.002706 |
| sum_kahan.test | RUN | 0.760129 | 1.660441 | 1.741996 | 0.012305 | 0.019756 | 0.010251 |
| tail.test | RUN | 0.809177 | 2.249514 | 1.997788 | 0.003113 | 0.002958 | 0.002623 |
| tail_mean.test | RUN | 1.126436 | 2.639515 | 2.901599 | 0.019145 | 0.020125 | 0.017661 |
| tail_quantile.test | RUN | 0.939503 | 2.166798 | 2.273430 | 0.021277 | 0.023887 | 0.018752 |
| variance.test | RUN | 0.868220 | 1.930378 | 2.216277 | 0.003020 | 0.002630 | 0.002693 |
| vector.test | RUN | 0.905033 | 2.369118 | 2.692857 | 0.003037 | 0.002948 | 0.002575 |
| weighted_covariance.test | RUN | 1.268297 | 3.658370 | 4.030087 | 0.003090 | 0.002583 | 0.002229 |
| weighted_extended_p_square.test | RUN | 0.900496 | 2.139123 | 2.075883 | 0.018675 | 0.018333 | 0.014666 |
| weighted_kurtosis.test | RUN | 1.001950 | 2.135344 | 2.556666 | 0.004945 | 0.004370 | 0.003447 |
| weighted_mean.test | RUN | 0.859473 | 1.902876 | 2.195856 | 0.002719 | 0.002556 | 0.002578 |
| weighted_median.test | RUN | 1.087727 | 2.708298 | 2.903553 | 0.963181 | 0.917305 | 0.851402 |
| weighted_moment.test | RUN | 0.698362 | 1.421310 | 1.687493 | 0.002955 | 0.002797 | 0.002553 |
| weighted_p_square_cumul_dist.test | RUN | 0.865987 | 2.335570 | 2.043196 | 0.099525 | 0.125371 | 0.081843 |
| weighted_p_square_quantile.test | RUN | 0.810847 | 2.292572 | 1.775556 | 0.025636 | 0.023727 | 0.018665 |
| weighted_skewness.test | RUN | 0.911488 | 2.245937 | 2.447763 | 0.004930 | 0.004363 | 0.003258 |
| weighted_sum.test | RUN | 0.594560 | 1.258940 | 1.307826 | 0.002907 | 0.002711 | 0.002282 |
| weighted_sum_kahan.test | RUN | 0.633089 | 1.342086 | 1.424035 | 0.007294 | 0.011238 | 0.006150 |
| weighted_variance.test | RUN | 1.052394 | 2.410715 | 2.757887 | 0.003363 | 0.003002 | 0.002770 |
| valarray.test | RUN | 0.938905 | 2.292943 | 2.626497 | 0.003162 | 0.002744 | 0.002636 |
| tail_variate_means.test | RUN | 1.239692 | 3.688811 | 3.608419 | 0.003065 | 0.002757 | 0.002595 |
| pot_quantile.test | RUN | 1.533795 | 4.051147 | 4.241803 | 0.011713 | 0.009838 | 0.008246 |
| weighted_pot_quantile.test | RUN | 1.558475 | 3.970414 | 4.305443 | 0.023217 | 0.023126 | 0.019918 |
| weighted_tail_mean.test | RUN | 1.336593 | 3.160948 | 3.639259 | 0.020703 | 0.022051 | 0.017533 |
| weighted_tail_quantile.test | RUN | 1.369978 | 2.701759 | 3.250220 | 0.018279 | 0.017091 | 0.014588 |
| weighted_tail_variate_means.test | RUN | 1.497738 | 3.477253 | 3.734984 | 0.002397 | 0.001921 | 0.001807 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.108×† | 1.135×, 1.081× | 1.060× |
| clang | 0.941× | 0.934×, 0.948× | 0.979× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1361 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| count.test | 1 | 0.002453s | 0.002437s | 0.002445s | byte equality observed |
| covariance.test | 1 | 0.002485s | 0.002430s | 0.002500s | byte equality observed |
| droppable.test | 5 | 0.002108s | 0.002100s | 0.002032s | byte equality observed |
| error_of.test | 1 | 0.002429s | 0.002359s | 0.002195s | byte equality observed |
| extended_p_square.test | 5 | 0.003209s | 0.003276s | 0.003088s | byte equality observed |
| extended_p_square_quantile.test | 3 | 0.005754s | 0.006488s | 0.005300s | byte equality observed |
| external_accumulator.test | 5 | 0.001998s | 0.001982s | 0.001983s | byte equality observed |
| external_weights.test | 9 | 0.001876s | 0.001850s | 0.001852s | byte equality observed |
| kurtosis.test | 4 | 0.003599s | 0.003471s | 0.002644s | byte equality observed |
| max.test | 5 | 0.001966s | 0.001934s | 0.002059s | byte equality observed |
| mean.test | 7 | 0.001991s | 0.001962s | 0.001940s | byte equality observed |
| median.test | 1 | 0.089232s | 0.105861s | 0.087653s | byte equality observed |
| min.test | 8 | 0.001961s | 0.001978s | 0.002014s | byte equality observed |
| moment.test | 5 | 0.002003s | 0.001975s | 0.002032s | byte equality observed |
| p_square_cumul_dist.test | 1 | 0.649333s | 0.771399s | 0.621189s | byte equality observed |
| p_square_quantile.test | 1 | 0.022360s | 0.021746s | 0.020311s | byte equality observed |
| pot_quantile.test | 2 | 0.010323s | 0.010105s | 0.008661s | byte equality observed |
| reference.test | 8 | 0.002003s | 0.002033s | 0.002133s | byte equality observed |
| rolling_count.test | 8 | 0.001988s | 0.001971s | 0.002101s | byte equality observed |
| rolling_mean.test | 9 | 0.002051s | 0.002090s | 0.002116s | byte equality observed |
| rolling_moment.test | 8 | 0.002054s | 0.002068s | 0.002116s | byte equality observed |
| rolling_sum.test | 9 | 0.001994s | 0.001968s | 0.002013s | byte equality observed |
| rolling_variance.test | 9 | 0.002068s | 0.002049s | 0.002079s | byte equality observed |
| skewness.test | 6 | 0.003525s | 0.003489s | 0.002808s | byte equality observed |
| sum.test | 9 | 0.001957s | 0.001942s | 0.002025s | byte equality observed |
| sum_kahan.test | 1 | 0.011132s | 0.020834s | 0.010158s | byte equality observed |
| tail.test | 10 | 0.001912s | 0.001935s | 0.002028s | byte equality observed |
| tail_mean.test | 2 | 0.018153s | 0.020161s | 0.018040s | byte equality observed |
| tail_quantile.test | 1 | 0.020800s | 0.022662s | 0.019517s | byte equality observed |
| tail_variate_means.test | 9 | 0.001933s | 0.001899s | 0.001975s | byte equality observed |
| valarray.test | 9 | 0.001866s | 0.001904s | 0.001920s | byte equality observed |
| variance.test | 9 | 0.001970s | 0.001981s | 0.002012s | byte equality observed |
| vector.test | 10 | 0.001860s | 0.001857s | 0.001901s | byte equality observed |
| weighted_covariance.test | 8 | 0.001889s | 0.001870s | 0.001888s | byte equality observed |
| weighted_extended_p_square.test | 2 | 0.018773s | 0.019375s | 0.014981s | byte equality observed |
| weighted_kurtosis.test | 5 | 0.004117s | 0.003911s | 0.002770s | byte equality observed |
| weighted_mean.test | 10 | 0.001880s | 0.001876s | 0.001946s | byte equality observed |
| weighted_median.test | 1 | 0.880455s | 0.899923s | 0.838434s | byte equality observed |
| weighted_moment.test | 6 | 0.001854s | 0.001846s | 0.001930s | byte equality observed |
| weighted_p_square_cumul_dist.test | 1 | 0.104735s | 0.129516s | 0.092872s | byte equality observed |
| weighted_p_square_quantile.test | 1 | 0.026706s | 0.025876s | 0.021136s | byte equality observed |
| weighted_pot_quantile.test | 1 | 0.022699s | 0.022965s | 0.021141s | byte equality observed |
| weighted_skewness.test | 5 | 0.004163s | 0.003913s | 0.003069s | byte equality observed |
| weighted_sum.test | 10 | 0.001855s | 0.001805s | 0.002044s | byte equality observed |
| weighted_sum_kahan.test | 2 | 0.006703s | 0.011164s | 0.006057s | byte equality observed |
| weighted_tail_mean.test | 2 | 0.018792s | 0.019824s | 0.018920s | byte equality observed |
| weighted_tail_quantile.test | 1 | 0.020563s | 0.022061s | 0.018807s | byte equality observed |
| weighted_tail_variate_means.test | 9 | 0.001919s | 0.001832s | 0.002065s | byte equality observed |
| weighted_variance.test | 9 | 0.002223s | 0.002166s | 0.002111s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
