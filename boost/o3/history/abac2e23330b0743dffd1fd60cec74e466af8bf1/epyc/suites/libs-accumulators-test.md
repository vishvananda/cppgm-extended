# libs/accumulators/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| count.test | RUN | 1.109113 | 2.869419 | 3.618091 | 0.004046 | 0.004260 | 0.004169 |
| covariance.test | RUN | 2.024207 | 7.060558 | 9.514268 | 0.004260 | 0.004053 | 0.004329 |
| droppable.test | RUN | 1.362811 | 3.900674 | 4.958290 | 0.004157 | 0.004424 | 0.004224 |
| error_of.test | RUN | 1.453284 | 3.709766 | 5.359399 | 0.003843 | 0.004046 | 0.003935 |
| extended_p_square.test | RUN | 1.578682 | 4.640130 | 5.666252 | 0.006090 | 0.006286 | 0.005946 |
| extended_p_square_quantile.test | RUN | 1.980532 | 7.164347 | 7.578545 | 0.011842 | 0.012216 | 0.010912 |
| external_accumulator.test | RUN | 1.315816 | 3.584995 | 4.774873 | 0.004144 | 0.004628 | 0.004293 |
| external_weights.test | RUN | 1.151762 | 2.932680 | 3.915753 | 0.003640 | 0.003742 | 0.003888 |
| kurtosis.test | RUN | 1.798994 | 4.631786 | 6.499212 | 0.006388 | 0.006031 | 0.005749 |
| max.test | RUN | 1.138103 | 2.866034 | 3.600085 | 0.003805 | 0.003866 | 0.004029 |
| mean.test | RUN | 1.436092 | 3.793247 | 5.236985 | 0.004233 | 0.003890 | 0.003599 |
| median.test | RUN | 1.957098 | 7.684305 | 7.994388 | 0.149612 | 0.167406 | 0.151270 |
| min.test | RUN | 1.108997 | 2.757375 | 3.530514 | 0.003968 | 0.004073 | 0.002997 |
| moment.test | RUN | 1.238898 | 3.034853 | 4.399649 | 0.004115 | 0.004108 | 0.004227 |
| p_square_cumul_dist.test | RUN | 1.608188 | 5.325971 | 6.028495 | 1.146315 | 1.278021 | 1.082763 |
| p_square_quantile.test | RUN | 1.447001 | 4.645883 | 5.104193 | 0.043117 | 0.038205 | 0.034299 |
| reference.test | RUN | 1.251872 | 3.063188 | 4.348727 | 0.004150 | 0.003950 | 0.004349 |
| rolling_count.test | RUN | 1.239525 | 3.423482 | 4.410196 | 0.003730 | 0.003968 | 0.004087 |
| rolling_sum.test | RUN | 1.255787 | 3.466534 | 4.103901 | 0.004322 | 0.004338 | 0.004141 |
| rolling_mean.test | RUN | 1.725609 | 6.244178 | 7.082329 | 0.004185 | 0.004646 | 0.004302 |
| rolling_variance.test | RUN | 2.006537 | 7.084085 | 8.095937 | 0.004191 | 0.004059 | 0.004130 |
| rolling_moment.test | RUN | 1.516332 | 4.209907 | 5.427868 | 0.004124 | 0.003784 | 0.004449 |
| skewness.test | RUN | 1.702844 | 4.395658 | 6.071959 | 0.006627 | 0.005978 | 0.005880 |
| sum.test | RUN | 1.202211 | 3.006459 | 4.020788 | 0.004030 | 0.004179 | 0.004049 |
| sum_kahan.test | RUN | 1.230255 | 3.169031 | 4.260330 | 0.019708 | 0.031732 | 0.020727 |
| tail.test | RUN | 1.304120 | 4.521632 | 4.676448 | 0.004023 | 0.004026 | 0.004263 |
| tail_mean.test | RUN | 1.826412 | 5.115997 | 6.879245 | 0.028695 | 0.034873 | 0.028481 |
| tail_quantile.test | RUN | 1.545083 | 4.348530 | 5.408883 | 0.032499 | 0.035082 | 0.031971 |
| variance.test | RUN | 1.429586 | 3.896821 | 5.300570 | 0.004105 | 0.004330 | 0.004426 |
| vector.test | RUN | 1.491361 | 4.554774 | 6.253495 | 0.003850 | 0.004042 | 0.003772 |
| weighted_covariance.test | RUN | 2.065962 | 7.093730 | 9.651066 | 0.004012 | 0.004233 | 0.003943 |
| weighted_extended_p_square.test | RUN | 1.456305 | 4.267687 | 4.963505 | 0.032916 | 0.031994 | 0.027460 |
| weighted_kurtosis.test | RUN | 1.676765 | 4.361857 | 6.200296 | 0.006981 | 0.006870 | 0.004610 |
| weighted_mean.test | RUN | 1.391319 | 3.752727 | 5.254745 | 0.003456 | 0.004074 | 0.004021 |
| weighted_median.test | RUN | 1.818667 | 5.111826 | 6.864009 | 1.519875 | 1.406860 | 1.308440 |
| weighted_moment.test | RUN | 1.176072 | 2.750980 | 4.026560 | 0.004048 | 0.003757 | 0.003662 |
| weighted_p_square_cumul_dist.test | RUN | 1.468476 | 4.519472 | 4.945623 | 0.159973 | 0.203255 | 0.139282 |
| weighted_p_square_quantile.test | RUN | 1.380239 | 4.303957 | 4.310391 | 0.044558 | 0.040117 | 0.035707 |
| weighted_skewness.test | RUN | 1.630674 | 3.943077 | 5.691555 | 0.007028 | 0.006405 | 0.004993 |
| weighted_sum.test | RUN | 0.903836 | 2.268700 | 3.205788 | 0.003572 | 0.004046 | 0.003344 |
| weighted_sum_kahan.test | RUN | 1.101201 | 2.491855 | 3.442022 | 0.011565 | 0.017215 | 0.011853 |
| weighted_variance.test | RUN | 1.750249 | 4.822318 | 6.780816 | 0.004438 | 0.004371 | 0.004414 |
| valarray.test | RUN | 1.537429 | 4.610070 | 6.268312 | 0.003487 | 0.003663 | 0.003602 |
| tail_variate_means.test | RUN | 2.030430 | 6.799835 | 8.466336 | 0.003683 | 0.003824 | 0.003695 |
| pot_quantile.test | RUN | 2.537850 | 7.652631 | 10.278649 | 0.016673 | 0.016498 | 0.014203 |
| weighted_pot_quantile.test | RUN | 2.599707 | 7.466331 | 10.377974 | 0.044548 | 0.042781 | 0.034159 |
| weighted_tail_mean.test | RUN | 2.300595 | 6.075640 | 8.902376 | 0.029444 | 0.038254 | 0.028826 |
| weighted_tail_quantile.test | RUN | 2.284639 | 5.437428 | 8.134754 | 0.025664 | 0.029044 | 0.023670 |
| weighted_tail_variate_means.test | RUN | 2.485478 | 6.529895 | 8.763271 | 0.002864 | 0.003367 | 0.002841 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.094× | 1.088×, 1.100× | 1.005× |
| clang | 0.917× | 0.921×, 0.912× | 1.002× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| count.test | 1 | 0.003100s | 0.003011s | 0.003044s | byte equality observed |
| covariance.test | 1 | 0.003195s | 0.003155s | 0.003022s | byte equality observed |
| droppable.test | 1 | 0.003121s | 0.003157s | 0.003070s | byte equality observed |
| error_of.test | 1 | 0.003090s | 0.002943s | 0.002752s | byte equality observed |
| extended_p_square.test | 3 | 0.004910s | 0.004830s | 0.004502s | byte equality observed |
| extended_p_square_quantile.test | 2 | 0.010285s | 0.010359s | 0.009479s | byte equality observed |
| external_accumulator.test | 5 | 0.002815s | 0.002723s | 0.002692s | byte equality observed |
| external_weights.test | 5 | 0.002660s | 0.002580s | 0.002524s | byte equality observed |
| kurtosis.test | 4 | 0.005252s | 0.004772s | 0.004082s | byte equality observed |
| max.test | 6 | 0.002787s | 0.002665s | 0.002628s | byte equality observed |
| mean.test | 5 | 0.002829s | 0.002736s | 0.002747s | byte equality observed |
| median.test | 1 | 0.149870s | 0.167881s | 0.142446s | byte equality observed |
| min.test | 6 | 0.002666s | 0.002581s | 0.002612s | byte equality observed |
| moment.test | 6 | 0.002700s | 0.002606s | 0.002606s | byte equality observed |
| p_square_cumul_dist.test | 1 | 0.959969s | 1.167031s | 0.904695s | byte equality observed |
| p_square_quantile.test | 1 | 0.038833s | 0.035451s | 0.031140s | byte equality observed |
| pot_quantile.test | 2 | 0.015076s | 0.013761s | 0.012253s | byte equality observed |
| reference.test | 5 | 0.002783s | 0.002704s | 0.002638s | byte equality observed |
| rolling_count.test | 6 | 0.002701s | 0.002596s | 0.002600s | byte equality observed |
| rolling_mean.test | 6 | 0.002845s | 0.002696s | 0.002684s | byte equality observed |
| rolling_moment.test | 6 | 0.002831s | 0.002717s | 0.002682s | byte equality observed |
| rolling_sum.test | 6 | 0.002707s | 0.002567s | 0.002614s | byte equality observed |
| rolling_variance.test | 7 | 0.002887s | 0.002741s | 0.002689s | byte equality observed |
| skewness.test | 4 | 0.005015s | 0.004672s | 0.004094s | byte equality observed |
| sum.test | 7 | 0.002679s | 0.002643s | 0.002588s | byte equality observed |
| sum_kahan.test | 1 | 0.018562s | 0.029805s | 0.018425s | byte equality observed |
| tail.test | 6 | 0.002763s | 0.002595s | 0.002549s | byte equality observed |
| tail_mean.test | 1 | 0.025842s | 0.033797s | 0.026590s | byte equality observed |
| tail_quantile.test | 1 | 0.031537s | 0.038413s | 0.027049s | byte equality observed |
| tail_variate_means.test | 6 | 0.002759s | 0.002462s | 0.002548s | byte equality observed |
| valarray.test | 6 | 0.002622s | 0.002479s | 0.002508s | byte equality observed |
| variance.test | 6 | 0.002832s | 0.002692s | 0.002693s | byte equality observed |
| vector.test | 7 | 0.002629s | 0.002535s | 0.002538s | byte equality observed |
| weighted_covariance.test | 7 | 0.002620s | 0.002534s | 0.002514s | byte equality observed |
| weighted_extended_p_square.test | 1 | 0.030468s | 0.031578s | 0.025229s | byte equality observed |
| weighted_kurtosis.test | 4 | 0.005745s | 0.005231s | 0.004017s | byte equality observed |
| weighted_mean.test | 7 | 0.002559s | 0.002484s | 0.002463s | byte equality observed |
| weighted_median.test | 1 | 1.259599s | 1.249481s | 1.141512s | byte equality observed |
| weighted_moment.test | 6 | 0.002514s | 0.002449s | 0.002406s | byte equality observed |
| weighted_p_square_cumul_dist.test | 1 | 0.163509s | 0.198859s | 0.138828s | byte equality observed |
| weighted_p_square_quantile.test | 1 | 0.041752s | 0.039470s | 0.032425s | byte equality observed |
| weighted_pot_quantile.test | 1 | 0.032790s | 0.032945s | 0.029063s | byte equality observed |
| weighted_skewness.test | 4 | 0.005756s | 0.005305s | 0.003946s | byte equality observed |
| weighted_sum.test | 7 | 0.002521s | 0.002411s | 0.002374s | byte equality observed |
| weighted_sum_kahan.test | 2 | 0.010411s | 0.016099s | 0.010200s | byte equality observed |
| weighted_tail_mean.test | 1 | 0.025701s | 0.028256s | 0.024712s | byte equality observed |
| weighted_tail_quantile.test | 1 | 0.028864s | 0.033374s | 0.027522s | byte equality observed |
| weighted_tail_variate_means.test | 6 | 0.002652s | 0.002533s | 0.002508s | byte equality observed |
| weighted_variance.test | 5 | 0.003066s | 0.002877s | 0.002747s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
