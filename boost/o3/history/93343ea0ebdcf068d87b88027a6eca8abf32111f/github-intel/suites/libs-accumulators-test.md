# libs/accumulators/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| count.test | RUN | 0.984321 | 2.579531 | 2.785683 | 0.003045 | 0.003034 | 0.002951 |
| covariance.test | RUN | 1.803789 | 6.682300 | 7.296022 | 0.003032 | 0.003050 | 0.002897 |
| droppable.test | RUN | 1.175684 | 3.578839 | 3.956508 | 0.003198 | 0.003098 | 0.002948 |
| error_of.test | RUN | 1.305778 | 3.534531 | 4.211512 | 0.002931 | 0.002902 | 0.002744 |
| extended_p_square.test | RUN | 1.364893 | 4.352264 | 4.294233 | 0.004972 | 0.005026 | 0.004470 |
| extended_p_square_quantile.test | RUN | 1.719502 | 6.724107 | 6.183509 | 0.010774 | 0.010676 | 0.009260 |
| external_accumulator.test | RUN | 1.147928 | 3.405815 | 3.750919 | 0.002878 | 0.003013 | 0.002856 |
| external_weights.test | RUN | 1.023747 | 2.578432 | 3.008312 | 0.002795 | 0.002972 | 0.002643 |
| kurtosis.test | RUN | 1.512796 | 4.325890 | 4.952351 | 0.005350 | 0.005154 | 0.004404 |
| max.test | RUN | 0.968623 | 2.608629 | 2.878998 | 0.003026 | 0.002783 | 0.002904 |
| mean.test | RUN | 1.282190 | 3.594804 | 4.084809 | 0.003119 | 0.003019 | 0.002890 |
| median.test | RUN | 1.728078 | 7.045472 | 6.134438 | 0.143521 | 0.155218 | 0.141879 |
| min.test | RUN | 0.982353 | 2.567752 | 2.806447 | 0.003047 | 0.003141 | 0.002732 |
| moment.test | RUN | 1.108581 | 2.956433 | 3.268534 | 0.003281 | 0.003102 | 0.002882 |
| p_square_cumul_dist.test | RUN | 1.455804 | 4.982940 | 4.693630 | 1.070588 | 1.232458 | 0.956089 |
| p_square_quantile.test | RUN | 1.285650 | 4.564392 | 3.907234 | 0.039005 | 0.032664 | 0.029945 |
| reference.test | RUN | 1.080864 | 2.903482 | 3.199081 | 0.002981 | 0.003063 | 0.002609 |
| rolling_count.test | RUN | 1.123524 | 3.111054 | 3.426702 | 0.003094 | 0.003010 | 0.002866 |
| rolling_sum.test | RUN | 1.122687 | 3.258988 | 3.457776 | 0.003116 | 0.003144 | 0.002892 |
| rolling_mean.test | RUN | 1.570680 | 5.926997 | 5.661770 | 0.003357 | 0.003240 | 0.003064 |
| rolling_variance.test | RUN | 1.750670 | 6.626098 | 6.400814 | 0.003048 | 0.003188 | 0.003019 |
| rolling_moment.test | RUN | 1.327719 | 3.948279 | 4.238926 | 0.003383 | 0.003144 | 0.003071 |
| skewness.test | RUN | 1.459086 | 4.078239 | 4.636498 | 0.005512 | 0.005307 | 0.004248 |
| sum.test | RUN | 1.045866 | 2.842616 | 3.066449 | 0.003070 | 0.003041 | 0.002834 |
| sum_kahan.test | RUN | 1.085269 | 2.987934 | 3.243323 | 0.023704 | 0.030100 | 0.022030 |
| tail.test | RUN | 1.162644 | 4.229836 | 3.673946 | 0.003020 | 0.003169 | 0.002922 |
| tail_mean.test | RUN | 1.650481 | 4.810716 | 5.386677 | 0.027419 | 0.026875 | 0.022897 |
| tail_quantile.test | RUN | 1.338793 | 3.953934 | 4.160730 | 0.030606 | 0.031456 | 0.025121 |
| variance.test | RUN | 1.287545 | 3.527373 | 4.082124 | 0.003084 | 0.003082 | 0.002814 |
| vector.test | RUN | 1.323815 | 4.360181 | 4.889670 | 0.002894 | 0.003086 | 0.002659 |
| weighted_covariance.test | RUN | 1.881820 | 6.549881 | 7.381552 | 0.002915 | 0.002897 | 0.002757 |
| weighted_extended_p_square.test | RUN | 1.324955 | 3.945956 | 3.790235 | 0.030620 | 0.030022 | 0.023692 |
| weighted_kurtosis.test | RUN | 1.506187 | 3.961633 | 4.731987 | 0.005989 | 0.005571 | 0.004206 |
| weighted_mean.test | RUN | 1.247829 | 3.554320 | 4.049505 | 0.002924 | 0.002999 | 0.002836 |
| weighted_median.test | RUN | 1.607756 | 4.811329 | 5.250438 | 1.409674 | 1.337413 | 1.270162 |
| weighted_moment.test | RUN | 1.044787 | 2.637594 | 3.036515 | 0.002898 | 0.002903 | 0.002745 |
| weighted_p_square_cumul_dist.test | RUN | 1.289459 | 4.074935 | 3.744565 | 0.166810 | 0.182354 | 0.131159 |
| weighted_p_square_quantile.test | RUN | 1.194968 | 4.028265 | 3.448032 | 0.043395 | 0.036569 | 0.032249 |
| weighted_skewness.test | RUN | 1.400724 | 3.877848 | 4.223561 | 0.005857 | 0.005670 | 0.004300 |
| weighted_sum.test | RUN | 0.863858 | 2.152015 | 2.442951 | 0.002930 | 0.002973 | 0.002808 |
| weighted_sum_kahan.test | RUN | 0.938514 | 2.394571 | 2.588632 | 0.013004 | 0.016289 | 0.012351 |
| weighted_variance.test | RUN | 1.570980 | 4.452867 | 5.139981 | 0.003418 | 0.003375 | 0.003136 |
| valarray.test | RUN | 1.369842 | 4.252066 | 4.784768 | 0.002908 | 0.002907 | 0.002786 |
| tail_variate_means.test | RUN | 1.808331 | 6.471740 | 6.675461 | 0.002954 | 0.002803 | 0.002816 |
| pot_quantile.test | RUN | 2.251388 | 7.239006 | 7.869860 | 0.014363 | 0.013351 | 0.011757 |
| weighted_pot_quantile.test | RUN | 2.208824 | 7.202480 | 7.908291 | 0.032891 | 0.033153 | 0.028921 |
| weighted_tail_mean.test | RUN | 1.994731 | 5.683156 | 6.797354 | 0.024829 | 0.026947 | 0.023934 |
| weighted_tail_quantile.test | RUN | 2.026945 | 5.230056 | 5.986326 | 0.020855 | 0.022096 | 0.018285 |
| weighted_tail_variate_means.test | RUN | 2.113263 | 5.657753 | 6.732409 | 0.002420 | 0.002028 | 0.001859 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.040× | 1.040×, 1.040× | 1.006× |
| clang | 0.908× | 0.911×, 0.905× | 1.005× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| count.test | 1 | 0.002526s | 0.002420s | 0.002283s | byte equality observed |
| covariance.test | 1 | 0.002540s | 0.002534s | 0.002452s | byte equality observed |
| droppable.test | 1 | 0.002440s | 0.002515s | 0.002530s | byte equality observed |
| error_of.test | 1 | 0.002436s | 0.002452s | 0.002377s | byte equality observed |
| extended_p_square.test | 4 | 0.004144s | 0.004007s | 0.003666s | byte equality observed |
| extended_p_square_quantile.test | 2 | 0.008894s | 0.009843s | 0.008278s | byte equality observed |
| external_accumulator.test | 5 | 0.002240s | 0.002146s | 0.002146s | byte equality observed |
| external_weights.test | 6 | 0.002123s | 0.002041s | 0.001985s | byte equality observed |
| kurtosis.test | 4 | 0.004415s | 0.004076s | 0.003407s | byte equality observed |
| max.test | 8 | 0.002199s | 0.002119s | 0.002056s | byte equality observed |
| mean.test | 6 | 0.002228s | 0.002105s | 0.002091s | byte equality observed |
| median.test | 1 | 0.141518s | 0.158665s | 0.135884s | byte equality observed |
| min.test | 6 | 0.002207s | 0.002101s | 0.002120s | byte equality observed |
| moment.test | 7 | 0.002212s | 0.002095s | 0.002080s | byte equality observed |
| p_square_cumul_dist.test | 1 | 0.882891s | 1.000569s | 0.798040s | byte equality observed |
| p_square_quantile.test | 1 | 0.040854s | 0.035334s | 0.029467s | byte equality observed |
| pot_quantile.test | 2 | 0.014050s | 0.011949s | 0.012107s | byte equality observed |
| reference.test | 8 | 0.002245s | 0.002131s | 0.002119s | byte equality observed |
| rolling_count.test | 8 | 0.002272s | 0.002136s | 0.002128s | byte equality observed |
| rolling_mean.test | 8 | 0.002385s | 0.002263s | 0.002245s | byte equality observed |
| rolling_moment.test | 8 | 0.002322s | 0.002193s | 0.002196s | byte equality observed |
| rolling_sum.test | 7 | 0.002276s | 0.002135s | 0.002162s | byte equality observed |
| rolling_variance.test | 8 | 0.002352s | 0.002225s | 0.002231s | byte equality observed |
| skewness.test | 5 | 0.004447s | 0.004040s | 0.003579s | byte equality observed |
| sum.test | 8 | 0.002152s | 0.002054s | 0.002022s | byte equality observed |
| sum_kahan.test | 1 | 0.022907s | 0.028946s | 0.021563s | byte equality observed |
| tail.test | 8 | 0.002211s | 0.002103s | 0.002090s | byte equality observed |
| tail_mean.test | 1 | 0.025206s | 0.022496s | 0.022929s | byte equality observed |
| tail_quantile.test | 1 | 0.029364s | 0.027884s | 0.024885s | byte equality observed |
| tail_variate_means.test | 7 | 0.002185s | 0.001981s | 0.002030s | byte equality observed |
| valarray.test | 8 | 0.002159s | 0.002053s | 0.002021s | byte equality observed |
| variance.test | 6 | 0.002189s | 0.002107s | 0.002071s | byte equality observed |
| vector.test | 9 | 0.002148s | 0.002051s | 0.002042s | byte equality observed |
| weighted_covariance.test | 7 | 0.002175s | 0.002005s | 0.002076s | byte equality observed |
| weighted_extended_p_square.test | 1 | 0.028370s | 0.031141s | 0.023387s | byte equality observed |
| weighted_kurtosis.test | 4 | 0.005155s | 0.004610s | 0.003674s | byte equality observed |
| weighted_mean.test | 5 | 0.002166s | 0.002028s | 0.002014s | byte equality observed |
| weighted_median.test | 1 | 1.172238s | 1.152973s | 1.084242s | byte equality observed |
| weighted_moment.test | 8 | 0.002135s | 0.002018s | 0.001967s | byte equality observed |
| weighted_p_square_cumul_dist.test | 1 | 0.175099s | 0.180816s | 0.142985s | byte equality observed |
| weighted_p_square_quantile.test | 1 | 0.042991s | 0.038294s | 0.033338s | byte equality observed |
| weighted_pot_quantile.test | 1 | 0.031759s | 0.030246s | 0.028624s | byte equality observed |
| weighted_skewness.test | 4 | 0.005287s | 0.004694s | 0.003772s | byte equality observed |
| weighted_sum.test | 6 | 0.002146s | 0.001995s | 0.001995s | byte equality observed |
| weighted_sum_kahan.test | 2 | 0.012423s | 0.015251s | 0.011490s | byte equality observed |
| weighted_tail_mean.test | 1 | 0.024225s | 0.023228s | 0.022172s | byte equality observed |
| weighted_tail_quantile.test | 1 | 0.026342s | 0.026585s | 0.024390s | byte equality observed |
| weighted_tail_variate_means.test | 7 | 0.002243s | 0.002063s | 0.002049s | byte equality observed |
| weighted_variance.test | 7 | 0.002634s | 0.002435s | 0.002335s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
