# libs/accumulators/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| count.test | RUN | 0.781723 | 1.993251 | 2.364398 | 0.003295 | 0.003418 | 0.003351 |
| covariance.test | RUN | 1.477942 | 5.128909 | 6.095893 | 0.003420 | 0.003321 | 0.003354 |
| droppable.test | RUN | 0.976361 | 2.667287 | 3.204615 | 0.003462 | 0.003490 | 0.003453 |
| error_of.test | RUN | 1.061735 | 2.630434 | 3.565959 | 0.002903 | 0.003150 | 0.003005 |
| extended_p_square.test | RUN | 1.114970 | 3.342122 | 3.595126 | 0.004791 | 0.005033 | 0.004677 |
| extended_p_square_quantile.test | RUN | 1.407402 | 5.114925 | 5.166152 | 0.008712 | 0.009301 | 0.007916 |
| external_accumulator.test | RUN | 0.991134 | 2.525553 | 3.101696 | 0.003350 | 0.003538 | 0.003436 |
| external_weights.test | RUN | 0.870264 | 1.986361 | 2.446913 | 0.003278 | 0.003076 | 0.003245 |
| kurtosis.test | RUN | 1.255708 | 3.272784 | 4.197420 | 0.005266 | 0.005211 | 0.004355 |
| max.test | RUN | 0.800942 | 1.874383 | 2.254422 | 0.003357 | 0.003297 | 0.003487 |
| mean.test | RUN | 1.043277 | 2.817578 | 3.419870 | 0.003490 | 0.003258 | 0.003243 |
| median.test | RUN | 1.420512 | 5.254121 | 5.097485 | 0.115191 | 0.135522 | 0.112233 |
| min.test | RUN | 0.819613 | 1.984184 | 2.294067 | 0.002978 | 0.003526 | 0.003346 |
| moment.test | RUN | 0.926689 | 2.193770 | 2.777295 | 0.003419 | 0.003471 | 0.003083 |
| p_square_cumul_dist.test | RUN | 1.180346 | 3.818734 | 3.854952 | 0.907709 | 1.033571 | 0.801227 |
| p_square_quantile.test | RUN | 1.053590 | 3.337035 | 3.246109 | 0.030674 | 0.027176 | 0.024681 |
| reference.test | RUN | 0.911673 | 2.208473 | 2.608526 | 0.003296 | 0.003364 | 0.003336 |
| rolling_count.test | RUN | 0.938804 | 2.341059 | 2.856259 | 0.003226 | 0.003407 | 0.003397 |
| rolling_sum.test | RUN | 0.910057 | 2.454589 | 2.873767 | 0.003496 | 0.003295 | 0.003333 |
| rolling_mean.test | RUN | 1.334894 | 4.487010 | 4.670964 | 0.003551 | 0.003169 | 0.003456 |
| rolling_variance.test | RUN | 1.492967 | 5.046470 | 5.365320 | 0.003509 | 0.003418 | 0.003586 |
| rolling_moment.test | RUN | 1.135406 | 3.041882 | 3.488198 | 0.004338 | 0.003689 | 0.003314 |
| skewness.test | RUN | 1.236483 | 3.102228 | 3.904490 | 0.005208 | 0.004991 | 0.004514 |
| sum.test | RUN | 0.857003 | 2.151932 | 2.562640 | 0.003435 | 0.003380 | 0.003494 |
| sum_kahan.test | RUN | 0.938478 | 2.308558 | 2.727836 | 0.017189 | 0.028045 | 0.016899 |
| tail.test | RUN | 0.975040 | 3.112921 | 3.061142 | 0.003068 | 0.003490 | 0.003298 |
| tail_mean.test | RUN | 1.364467 | 3.618152 | 4.501189 | 0.021157 | 0.027727 | 0.021979 |
| tail_quantile.test | RUN | 1.140240 | 2.980892 | 3.526000 | 0.024341 | 0.029681 | 0.024217 |
| variance.test | RUN | 1.108002 | 2.687664 | 3.438046 | 0.003382 | 0.003065 | 0.003277 |
| vector.test | RUN | 1.135719 | 3.300176 | 4.145236 | 0.003042 | 0.003351 | 0.003397 |
| weighted_covariance.test | RUN | 1.493107 | 5.039570 | 6.284378 | 0.002947 | 0.003218 | 0.003240 |
| weighted_extended_p_square.test | RUN | 1.047419 | 2.978990 | 3.220511 | 0.022961 | 0.025137 | 0.019029 |
| weighted_kurtosis.test | RUN | 1.223438 | 3.053381 | 3.974048 | 0.005440 | 0.005520 | 0.004398 |
| weighted_mean.test | RUN | 1.040779 | 2.677825 | 3.424472 | 0.003084 | 0.003101 | 0.002932 |
| weighted_median.test | RUN | 1.316632 | 3.598820 | 4.426715 | 1.210592 | 1.201445 | 1.100480 |
| weighted_moment.test | RUN | 0.901200 | 2.035270 | 2.570021 | 0.002967 | 0.003206 | 0.003066 |
| weighted_p_square_cumul_dist.test | RUN | 1.085202 | 3.083264 | 3.125619 | 0.142738 | 0.180801 | 0.110528 |
| weighted_p_square_quantile.test | RUN | 0.979559 | 3.030155 | 2.672324 | 0.034141 | 0.031408 | 0.026222 |
| weighted_skewness.test | RUN | 1.058153 | 2.747587 | 3.747763 | 0.005808 | 0.005432 | 0.004020 |
| weighted_sum.test | RUN | 0.722043 | 1.673401 | 2.063936 | 0.003044 | 0.003223 | 0.003223 |
| weighted_sum_kahan.test | RUN | 0.778306 | 1.830123 | 2.178798 | 0.009723 | 0.015182 | 0.010076 |
| weighted_variance.test | RUN | 1.311819 | 3.340911 | 4.366696 | 0.003559 | 0.003629 | 0.003259 |
| valarray.test | RUN | 1.115319 | 3.146633 | 4.047665 | 0.003161 | 0.003304 | 0.003118 |
| tail_variate_means.test | RUN | 1.488857 | 4.938624 | 5.655366 | 0.003206 | 0.003234 | 0.003202 |
| pot_quantile.test | RUN | 1.862175 | 5.488219 | 6.720420 | 0.013105 | 0.013085 | 0.011034 |
| weighted_pot_quantile.test | RUN | 1.856541 | 5.369485 | 6.777657 | 0.027777 | 0.030793 | 0.025802 |
| weighted_tail_mean.test | RUN | 1.650495 | 4.342859 | 5.651932 | 0.022017 | 0.028052 | 0.019014 |
| weighted_tail_quantile.test | RUN | 1.654140 | 3.689831 | 5.138714 | 0.021509 | 0.022619 | 0.019374 |
| weighted_tail_variate_means.test | RUN | 1.801396 | 4.664966 | 5.927840 | 0.002386 | 0.002350 | 0.002282 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.118× | 1.124×, 1.111× | 1.007× |
| clang | 0.925× | 0.923×, 0.927× | 0.993× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1362 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| count.test | 1 | 0.002764s | 0.002288s | 0.002584s | byte equality observed |
| covariance.test | 1 | 0.002693s | 0.002358s | 0.002607s | byte equality observed |
| droppable.test | 1 | 0.002703s | 0.002415s | 0.002576s | byte equality observed |
| error_of.test | 1 | 0.002593s | 0.002411s | 0.002557s | byte equality observed |
| extended_p_square.test | 4 | 0.003711s | 0.003674s | 0.003425s | byte equality observed |
| extended_p_square_quantile.test | 3 | 0.007511s | 0.007942s | 0.006627s | byte equality observed |
| external_accumulator.test | 7 | 0.002218s | 0.002142s | 0.002139s | byte equality observed |
| external_weights.test | 5 | 0.002082s | 0.002025s | 0.002021s | byte equality observed |
| kurtosis.test | 5 | 0.004141s | 0.003834s | 0.003062s | byte equality observed |
| max.test | 7 | 0.002154s | 0.002084s | 0.002093s | byte equality observed |
| mean.test | 6 | 0.002182s | 0.002103s | 0.002094s | byte equality observed |
| median.test | 1 | 0.113296s | 0.140653s | 0.107880s | byte equality observed |
| min.test | 8 | 0.002150s | 0.002061s | 0.002083s | byte equality observed |
| moment.test | 4 | 0.002173s | 0.002112s | 0.002103s | byte equality observed |
| p_square_cumul_dist.test | 1 | 0.804295s | 0.982567s | 0.755278s | byte equality observed |
| p_square_quantile.test | 1 | 0.029956s | 0.026538s | 0.024559s | byte equality observed |
| pot_quantile.test | 2 | 0.012200s | 0.011402s | 0.009729s | byte equality observed |
| reference.test | 8 | 0.002179s | 0.002110s | 0.002109s | byte equality observed |
| rolling_count.test | 8 | 0.002177s | 0.002080s | 0.002115s | byte equality observed |
| rolling_mean.test | 8 | 0.002267s | 0.002172s | 0.002179s | byte equality observed |
| rolling_moment.test | 8 | 0.002276s | 0.002198s | 0.002217s | byte equality observed |
| rolling_sum.test | 8 | 0.002168s | 0.002091s | 0.002113s | byte equality observed |
| rolling_variance.test | 8 | 0.002249s | 0.002177s | 0.002184s | byte equality observed |
| skewness.test | 5 | 0.004106s | 0.003817s | 0.003162s | byte equality observed |
| sum.test | 8 | 0.002117s | 0.002051s | 0.002044s | byte equality observed |
| sum_kahan.test | 1 | 0.015818s | 0.026445s | 0.015710s | byte equality observed |
| tail.test | 9 | 0.002156s | 0.002039s | 0.002088s | byte equality observed |
| tail_mean.test | 1 | 0.020313s | 0.022420s | 0.020603s | byte equality observed |
| tail_quantile.test | 1 | 0.023522s | 0.029124s | 0.022037s | byte equality observed |
| tail_variate_means.test | 8 | 0.002097s | 0.001974s | 0.002052s | byte equality observed |
| valarray.test | 8 | 0.002064s | 0.001961s | 0.001985s | byte equality observed |
| variance.test | 8 | 0.002142s | 0.002102s | 0.002107s | byte equality observed |
| vector.test | 8 | 0.002038s | 0.001992s | 0.002006s | byte equality observed |
| weighted_covariance.test | 9 | 0.002070s | 0.002023s | 0.001944s | byte equality observed |
| weighted_extended_p_square.test | 1 | 0.022288s | 0.022496s | 0.018993s | byte equality observed |
| weighted_kurtosis.test | 5 | 0.004623s | 0.004316s | 0.003099s | byte equality observed |
| weighted_mean.test | 9 | 0.002048s | 0.001972s | 0.001963s | byte equality observed |
| weighted_median.test | 1 | 1.073063s | 1.110870s | 1.010413s | byte equality observed |
| weighted_moment.test | 9 | 0.002008s | 0.001936s | 0.001935s | byte equality observed |
| weighted_p_square_cumul_dist.test | 1 | 0.146498s | 0.173875s | 0.115107s | byte equality observed |
| weighted_p_square_quantile.test | 1 | 0.033603s | 0.033118s | 0.025977s | byte equality observed |
| weighted_pot_quantile.test | 1 | 0.026706s | 0.027025s | 0.024135s | byte equality observed |
| weighted_skewness.test | 4 | 0.004626s | 0.004322s | 0.003177s | byte equality observed |
| weighted_sum.test | 8 | 0.002014s | 0.001938s | 0.001958s | byte equality observed |
| weighted_sum_kahan.test | 2 | 0.008816s | 0.013926s | 0.008781s | byte equality observed |
| weighted_tail_mean.test | 1 | 0.021527s | 0.022661s | 0.020226s | byte equality observed |
| weighted_tail_quantile.test | 1 | 0.023632s | 0.026088s | 0.022172s | byte equality observed |
| weighted_tail_variate_means.test | 7 | 0.002074s | 0.001981s | 0.002041s | byte equality observed |
| weighted_variance.test | 6 | 0.002447s | 0.002308s | 0.002149s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
