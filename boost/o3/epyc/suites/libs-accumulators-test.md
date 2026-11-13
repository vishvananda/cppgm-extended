# libs/accumulators/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| count.test | RUN | 0.728234 | 2.052845 | 2.403386 | 0.003436 | 0.003453 | 0.003071 |
| covariance.test | RUN | 1.450912 | 5.170937 | 6.285113 | 0.003493 | 0.003458 | 0.003406 |
| droppable.test | RUN | 0.918043 | 2.806288 | 3.325693 | 0.003214 | 0.003452 | 0.003358 |
| error_of.test | RUN | 1.025240 | 2.630341 | 3.566920 | 0.003036 | 0.003253 | 0.003363 |
| extended_p_square.test | RUN | 1.085409 | 3.351315 | 3.584465 | 0.005015 | 0.005096 | 0.004272 |
| extended_p_square_quantile.test | RUN | 1.361258 | 5.307110 | 5.251760 | 0.008642 | 0.009457 | 0.007713 |
| external_accumulator.test | RUN | 0.907873 | 2.574107 | 3.176780 | 0.003145 | 0.003055 | 0.003628 |
| external_weights.test | RUN | 0.805713 | 2.064950 | 2.556436 | 0.003054 | 0.002977 | 0.002996 |
| kurtosis.test | RUN | 1.184040 | 3.414471 | 4.165603 | 0.005807 | 0.005056 | 0.004453 |
| max.test | RUN | 0.735347 | 1.900609 | 2.324390 | 0.003353 | 0.003341 | 0.003336 |
| mean.test | RUN | 0.971255 | 2.981419 | 3.410280 | 0.003367 | 0.003481 | 0.002955 |
| median.test | RUN | 1.329929 | 5.574313 | 5.134223 | 0.131262 | 0.135293 | 0.109336 |
| min.test | RUN | 0.734582 | 2.003778 | 2.343144 | 0.003397 | 0.003342 | 0.003278 |
| moment.test | RUN | 0.851566 | 2.304988 | 2.790696 | 0.003376 | 0.003278 | 0.003303 |
| p_square_cumul_dist.test | RUN | 1.141947 | 3.933051 | 3.888253 | 0.995037 | 1.049062 | 0.813701 |
| p_square_quantile.test | RUN | 0.997211 | 3.353467 | 3.337107 | 0.028744 | 0.029331 | 0.024785 |
| reference.test | RUN | 0.838465 | 2.241079 | 2.793265 | 0.003269 | 0.003491 | 0.003399 |
| rolling_count.test | RUN | 0.887335 | 2.535287 | 2.816152 | 0.003363 | 0.003196 | 0.003446 |
| rolling_sum.test | RUN | 0.829989 | 2.548963 | 2.714470 | 0.003340 | 0.003242 | 0.003178 |
| rolling_mean.test | RUN | 1.261070 | 4.542872 | 4.813547 | 0.003172 | 0.003519 | 0.003500 |
| rolling_variance.test | RUN | 1.412374 | 5.197029 | 5.488856 | 0.003633 | 0.003623 | 0.003263 |
| rolling_moment.test | RUN | 1.077140 | 2.964640 | 3.593220 | 0.003807 | 0.003435 | 0.003682 |
| skewness.test | RUN | 1.168365 | 3.110477 | 4.019702 | 0.006071 | 0.004897 | 0.004483 |
| sum.test | RUN | 0.764925 | 2.124901 | 2.533736 | 0.003631 | 0.003436 | 0.003359 |
| sum_kahan.test | RUN | 0.831284 | 2.265879 | 2.756692 | 0.016987 | 0.027762 | 0.016955 |
| tail.test | RUN | 0.927775 | 3.144858 | 3.156479 | 0.003435 | 0.003323 | 0.003396 |
| tail_mean.test | RUN | 1.355769 | 3.620446 | 4.671566 | 0.022300 | 0.028003 | 0.022369 |
| tail_quantile.test | RUN | 1.054752 | 2.936699 | 3.623885 | 0.026069 | 0.032088 | 0.024090 |
| variance.test | RUN | 1.042649 | 2.689882 | 3.492390 | 0.003501 | 0.003406 | 0.003493 |
| vector.test | RUN | 1.121396 | 3.290359 | 4.227523 | 0.003170 | 0.003378 | 0.003131 |
| weighted_covariance.test | RUN | 1.447466 | 5.051825 | 6.287958 | 0.002888 | 0.003238 | 0.002706 |
| weighted_extended_p_square.test | RUN | 0.975660 | 2.977921 | 3.208872 | 0.026446 | 0.025100 | 0.019418 |
| weighted_kurtosis.test | RUN | 1.142889 | 2.994593 | 4.024621 | 0.006588 | 0.005458 | 0.004284 |
| weighted_mean.test | RUN | 0.990874 | 2.713642 | 3.480169 | 0.003124 | 0.003125 | 0.003106 |
| weighted_median.test | RUN | 1.281057 | 3.598359 | 4.464448 | 1.139794 | 1.202455 | 1.100378 |
| weighted_moment.test | RUN | 0.809546 | 1.965176 | 2.577507 | 0.003004 | 0.003249 | 0.003311 |
| weighted_p_square_cumul_dist.test | RUN | 0.945662 | 3.043397 | 3.296650 | 0.131674 | 0.184644 | 0.110049 |
| weighted_p_square_quantile.test | RUN | 0.894641 | 2.876177 | 3.044018 | 0.037301 | 0.032205 | 0.025429 |
| weighted_skewness.test | RUN | 1.097713 | 2.858321 | 3.640881 | 0.006237 | 0.005210 | 0.004213 |
| weighted_sum.test | RUN | 0.608753 | 1.693237 | 2.161511 | 0.002930 | 0.003311 | 0.002984 |
| weighted_sum_kahan.test | RUN | 0.732349 | 1.826425 | 2.201452 | 0.009900 | 0.015324 | 0.009806 |
| weighted_variance.test | RUN | 1.215151 | 3.401241 | 4.487422 | 0.003594 | 0.003566 | 0.003418 |
| valarray.test | RUN | 1.131599 | 3.160534 | 4.162761 | 0.003228 | 0.003294 | 0.003209 |
| tail_variate_means.test | RUN | 1.438557 | 4.863112 | 5.835887 | 0.003165 | 0.003220 | 0.003386 |
| pot_quantile.test | RUN | 1.739972 | 5.437992 | 6.959455 | 0.016222 | 0.012672 | 0.010651 |
| weighted_pot_quantile.test | RUN | 1.773457 | 5.379773 | 7.024030 | 0.030903 | 0.030802 | 0.025242 |
| weighted_tail_mean.test | RUN | 1.580204 | 4.245202 | 5.947927 | 0.023083 | 0.028839 | 0.023253 |
| weighted_tail_quantile.test | RUN | 1.577126 | 3.767759 | 5.343013 | 0.022866 | 0.022258 | 0.019567 |
| weighted_tail_variate_means.test | RUN | 1.734826 | 4.749663 | 5.989200 | 0.002452 | 0.002262 | 0.002272 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.072× | 1.077×, 1.067× | 1.005× |
| clang | 0.887× | 0.893×, 0.882× | 0.997× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1363 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43479737952).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| count.test | 1 | 0.002592s | 0.002568s | 0.002512s | byte equality observed |
| covariance.test | 1 | 0.002677s | 0.002560s | 0.002590s | byte equality observed |
| droppable.test | 1 | 0.002652s | 0.002475s | 0.002540s | byte equality observed |
| error_of.test | 1 | 0.002533s | 0.002451s | 0.002400s | byte equality observed |
| extended_p_square.test | 4 | 0.003743s | 0.003759s | 0.003432s | byte equality observed |
| extended_p_square_quantile.test | 3 | 0.007403s | 0.007969s | 0.006241s | byte equality observed |
| external_accumulator.test | 5 | 0.002190s | 0.002112s | 0.002047s | byte equality observed |
| external_weights.test | 6 | 0.002066s | 0.001984s | 0.001989s | byte equality observed |
| kurtosis.test | 4 | 0.004832s | 0.003801s | 0.002981s | byte equality observed |
| max.test | 6 | 0.002154s | 0.002085s | 0.001993s | byte equality observed |
| mean.test | 7 | 0.002172s | 0.002081s | 0.002055s | byte equality observed |
| median.test | 1 | 0.129000s | 0.130346s | 0.108213s | byte equality observed |
| min.test | 6 | 0.002136s | 0.002081s | 0.002042s | byte equality observed |
| moment.test | 4 | 0.002172s | 0.002097s | 0.002026s | byte equality observed |
| p_square_cumul_dist.test | 1 | 0.866549s | 0.975200s | 0.749284s | byte equality observed |
| p_square_quantile.test | 1 | 0.029719s | 0.028540s | 0.024389s | byte equality observed |
| pot_quantile.test | 2 | 0.015476s | 0.011449s | 0.009662s | byte equality observed |
| reference.test | 7 | 0.002266s | 0.002149s | 0.002089s | byte equality observed |
| rolling_count.test | 7 | 0.002195s | 0.002103s | 0.002089s | byte equality observed |
| rolling_mean.test | 7 | 0.002286s | 0.002204s | 0.002132s | byte equality observed |
| rolling_moment.test | 7 | 0.002253s | 0.002168s | 0.002130s | byte equality observed |
| rolling_sum.test | 8 | 0.002144s | 0.002087s | 0.002051s | byte equality observed |
| rolling_variance.test | 8 | 0.002246s | 0.002202s | 0.002134s | byte equality observed |
| skewness.test | 4 | 0.004782s | 0.003864s | 0.003128s | byte equality observed |
| sum.test | 8 | 0.002120s | 0.002065s | 0.002009s | byte equality observed |
| sum_kahan.test | 1 | 0.015757s | 0.026279s | 0.015541s | byte equality observed |
| tail.test | 9 | 0.002184s | 0.002036s | 0.002055s | byte equality observed |
| tail_mean.test | 1 | 0.020435s | 0.025228s | 0.020163s | byte equality observed |
| tail_quantile.test | 1 | 0.024178s | 0.028362s | 0.022004s | byte equality observed |
| tail_variate_means.test | 8 | 0.002026s | 0.002019s | 0.001940s | byte equality observed |
| valarray.test | 6 | 0.002092s | 0.002027s | 0.001935s | byte equality observed |
| variance.test | 8 | 0.002137s | 0.002097s | 0.002043s | byte equality observed |
| vector.test | 7 | 0.002061s | 0.002033s | 0.001942s | byte equality observed |
| weighted_covariance.test | 8 | 0.002038s | 0.002015s | 0.001952s | byte equality observed |
| weighted_extended_p_square.test | 1 | 0.024620s | 0.025386s | 0.019124s | byte equality observed |
| weighted_kurtosis.test | 4 | 0.005295s | 0.004259s | 0.003076s | byte equality observed |
| weighted_mean.test | 9 | 0.002027s | 0.001962s | 0.001892s | byte equality observed |
| weighted_median.test | 1 | 1.083372s | 1.105379s | 1.007491s | byte equality observed |
| weighted_moment.test | 8 | 0.002020s | 0.001960s | 0.001902s | byte equality observed |
| weighted_p_square_cumul_dist.test | 1 | 0.137573s | 0.174895s | 0.119471s | byte equality observed |
| weighted_p_square_quantile.test | 1 | 0.036209s | 0.033767s | 0.026175s | byte equality observed |
| weighted_pot_quantile.test | 1 | 0.030512s | 0.027537s | 0.024045s | byte equality observed |
| weighted_skewness.test | 4 | 0.005639s | 0.004604s | 0.003138s | byte equality observed |
| weighted_sum.test | 9 | 0.002087s | 0.002083s | 0.001938s | byte equality observed |
| weighted_sum_kahan.test | 2 | 0.008908s | 0.014259s | 0.008725s | byte equality observed |
| weighted_tail_mean.test | 1 | 0.021662s | 0.023518s | 0.020553s | byte equality observed |
| weighted_tail_quantile.test | 1 | 0.025511s | 0.026222s | 0.022103s | byte equality observed |
| weighted_tail_variate_means.test | 9 | 0.002128s | 0.002144s | 0.002025s | byte equality observed |
| weighted_variance.test | 7 | 0.002593s | 0.002400s | 0.002096s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43479737952).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
