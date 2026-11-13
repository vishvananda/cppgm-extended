# libs/accumulators/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| count.test | RUN | 0.713195 | 2.011089 | 2.342557 | 0.003394 | 0.003170 | 0.002974 |
| covariance.test | RUN | 1.344767 | 5.002800 | 6.073803 | 0.003381 | 0.003229 | 0.003336 |
| droppable.test | RUN | 0.887738 | 2.638824 | 3.160974 | 0.003149 | 0.003307 | 0.003410 |
| error_of.test | RUN | 0.959377 | 2.619574 | 3.477937 | 0.002814 | 0.003046 | 0.002943 |
| extended_p_square.test | RUN | 0.999548 | 3.324107 | 3.556650 | 0.004819 | 0.004960 | 0.004553 |
| extended_p_square_quantile.test | RUN | 1.310211 | 5.142208 | 5.108542 | 0.008760 | 0.009332 | 0.007954 |
| external_accumulator.test | RUN | 0.862242 | 2.499112 | 3.129574 | 0.003305 | 0.002735 | 0.003422 |
| external_weights.test | RUN | 0.727164 | 1.995238 | 2.450587 | 0.002779 | 0.003056 | 0.002841 |
| kurtosis.test | RUN | 1.158113 | 3.174014 | 4.161813 | 0.005701 | 0.005099 | 0.004200 |
| max.test | RUN | 0.734232 | 1.917311 | 2.254363 | 0.003384 | 0.003473 | 0.003320 |
| mean.test | RUN | 0.935656 | 2.716183 | 3.381176 | 0.003324 | 0.003194 | 0.003225 |
| median.test | RUN | 1.297422 | 5.252469 | 5.026111 | 0.120991 | 0.134373 | 0.113508 |
| min.test | RUN | 0.720777 | 1.879344 | 2.293346 | 0.003056 | 0.003323 | 0.003400 |
| moment.test | RUN | 0.802097 | 2.256251 | 2.741840 | 0.003264 | 0.003244 | 0.002926 |
| p_square_cumul_dist.test | RUN | 1.049423 | 3.742558 | 3.819733 | 1.006092 | 1.042074 | 0.807892 |
| p_square_quantile.test | RUN | 0.931825 | 3.285161 | 3.207867 | 0.029140 | 0.026869 | 0.024385 |
| reference.test | RUN | 0.803684 | 2.175203 | 2.809172 | 0.003303 | 0.003189 | 0.002872 |
| rolling_count.test | RUN | 0.795097 | 2.381206 | 2.683557 | 0.002864 | 0.003383 | 0.002838 |
| rolling_sum.test | RUN | 0.819897 | 2.425097 | 2.828652 | 0.003368 | 0.003326 | 0.003313 |
| rolling_mean.test | RUN | 1.171839 | 4.375690 | 4.604351 | 0.003395 | 0.003393 | 0.003394 |
| rolling_variance.test | RUN | 1.322650 | 5.041689 | 5.211568 | 0.003409 | 0.003119 | 0.003461 |
| rolling_moment.test | RUN | 1.019283 | 3.050194 | 3.490355 | 0.003175 | 0.003497 | 0.003404 |
| skewness.test | RUN | 1.080373 | 3.091679 | 3.901726 | 0.005923 | 0.004685 | 0.004207 |
| sum.test | RUN | 0.737763 | 2.082067 | 2.522223 | 0.003138 | 0.003394 | 0.003146 |
| sum_kahan.test | RUN | 0.803609 | 2.234964 | 2.639772 | 0.016891 | 0.027866 | 0.016952 |
| tail.test | RUN | 0.872526 | 3.103890 | 3.029261 | 0.003240 | 0.003418 | 0.003274 |
| tail_mean.test | RUN | 1.234865 | 3.544421 | 4.406847 | 0.021409 | 0.027942 | 0.021920 |
| tail_quantile.test | RUN | 1.002663 | 2.930343 | 3.431893 | 0.027366 | 0.031866 | 0.023644 |
| variance.test | RUN | 0.971848 | 2.643650 | 3.349338 | 0.003070 | 0.003397 | 0.003309 |
| vector.test | RUN | 1.042974 | 3.200043 | 4.067045 | 0.003017 | 0.003145 | 0.003152 |
| weighted_covariance.test | RUN | 1.392764 | 4.969037 | 6.185415 | 0.002664 | 0.003035 | 0.002835 |
| weighted_extended_p_square.test | RUN | 0.934963 | 2.937927 | 3.200862 | 0.024189 | 0.024917 | 0.020060 |
| weighted_kurtosis.test | RUN | 1.121352 | 3.016532 | 3.958339 | 0.006210 | 0.005314 | 0.004136 |
| weighted_mean.test | RUN | 0.942717 | 2.660645 | 3.376277 | 0.002900 | 0.003181 | 0.002964 |
| weighted_median.test | RUN | 1.220904 | 3.593553 | 4.312744 | 1.190700 | 1.180406 | 1.097974 |
| weighted_moment.test | RUN | 0.763529 | 1.978003 | 2.567976 | 0.002981 | 0.003128 | 0.002983 |
| weighted_p_square_cumul_dist.test | RUN | 0.945051 | 3.004050 | 2.866945 | 0.137236 | 0.138640 | 0.109121 |
| weighted_p_square_quantile.test | RUN | 0.882381 | 2.854820 | 2.909549 | 0.035466 | 0.035529 | 0.025975 |
| weighted_skewness.test | RUN | 0.968998 | 2.813736 | 3.705896 | 0.006254 | 0.005270 | 0.004352 |
| weighted_sum.test | RUN | 0.634095 | 1.714091 | 2.002687 | 0.002932 | 0.003137 | 0.002921 |
| weighted_sum_kahan.test | RUN | 0.694453 | 1.766581 | 2.171913 | 0.009661 | 0.015250 | 0.009903 |
| weighted_variance.test | RUN | 1.179371 | 3.197038 | 4.259951 | 0.003545 | 0.003662 | 0.003269 |
| valarray.test | RUN | 1.058050 | 3.142938 | 3.957575 | 0.003022 | 0.002973 | 0.003078 |
| tail_variate_means.test | RUN | 1.389914 | 4.961801 | 5.541021 | 0.002994 | 0.003129 | 0.003014 |
| pot_quantile.test | RUN | 1.692791 | 5.296623 | 6.588075 | 0.016776 | 0.012957 | 0.010702 |
| weighted_pot_quantile.test | RUN | 1.711208 | 5.323008 | 6.696840 | 0.030339 | 0.030952 | 0.024102 |
| weighted_tail_mean.test | RUN | 1.474230 | 4.301558 | 5.693703 | 0.023296 | 0.027900 | 0.022532 |
| weighted_tail_quantile.test | RUN | 1.452248 | 3.843824 | 5.038060 | 0.022790 | 0.022651 | 0.019400 |
| weighted_tail_variate_means.test | RUN | 1.634227 | 4.762261 | 5.661514 | 0.002313 | 0.002207 | 0.002169 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.086× | 1.088×, 1.084× | 0.994× |
| clang | 0.891× | 0.891×, 0.891× | 1.013× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1362 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43357224918).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| count.test | 1 | 0.002648s | 0.002548s | 0.002550s | byte equality observed |
| covariance.test | 1 | 0.002676s | 0.002530s | 0.002605s | byte equality observed |
| droppable.test | 1 | 0.002589s | 0.002602s | 0.002537s | byte equality observed |
| error_of.test | 4 | 0.002141s | 0.002070s | 0.002070s | byte equality observed |
| extended_p_square.test | 5 | 0.003625s | 0.003591s | 0.003356s | byte equality observed |
| extended_p_square_quantile.test | 2 | 0.007412s | 0.007836s | 0.006643s | byte equality observed |
| external_accumulator.test | 7 | 0.002137s | 0.002038s | 0.002054s | byte equality observed |
| external_weights.test | 5 | 0.001993s | 0.001903s | 0.001901s | byte equality observed |
| kurtosis.test | 4 | 0.004765s | 0.003681s | 0.003010s | byte equality observed |
| max.test | 6 | 0.002101s | 0.002043s | 0.002000s | byte equality observed |
| mean.test | 6 | 0.002128s | 0.002024s | 0.002067s | byte equality observed |
| median.test | 1 | 0.120396s | 0.143859s | 0.112495s | byte equality observed |
| min.test | 6 | 0.002085s | 0.002035s | 0.002031s | byte equality observed |
| moment.test | 8 | 0.002084s | 0.002045s | 0.002031s | byte equality observed |
| p_square_cumul_dist.test | 1 | 0.861698s | 0.989462s | 0.749254s | byte equality observed |
| p_square_quantile.test | 1 | 0.029057s | 0.026769s | 0.024024s | byte equality observed |
| pot_quantile.test | 2 | 0.015062s | 0.011382s | 0.009625s | byte equality observed |
| reference.test | 8 | 0.002122s | 0.002038s | 0.002064s | byte equality observed |
| rolling_count.test | 9 | 0.002114s | 0.002014s | 0.002062s | byte equality observed |
| rolling_mean.test | 8 | 0.002213s | 0.002098s | 0.002128s | byte equality observed |
| rolling_moment.test | 8 | 0.002185s | 0.002108s | 0.002122s | byte equality observed |
| rolling_sum.test | 7 | 0.002118s | 0.002033s | 0.002070s | byte equality observed |
| rolling_variance.test | 8 | 0.002198s | 0.002134s | 0.002165s | byte equality observed |
| skewness.test | 4 | 0.004748s | 0.003748s | 0.003033s | byte equality observed |
| sum.test | 9 | 0.002079s | 0.001961s | 0.002019s | byte equality observed |
| sum_kahan.test | 1 | 0.015657s | 0.026520s | 0.015429s | byte equality observed |
| tail.test | 9 | 0.002084s | 0.001984s | 0.002011s | byte equality observed |
| tail_mean.test | 1 | 0.020654s | 0.023667s | 0.020094s | byte equality observed |
| tail_quantile.test | 1 | 0.025944s | 0.027878s | 0.022249s | byte equality observed |
| tail_variate_means.test | 8 | 0.002028s | 0.001934s | 0.001976s | byte equality observed |
| valarray.test | 8 | 0.002029s | 0.001921s | 0.001957s | byte equality observed |
| variance.test | 8 | 0.002106s | 0.002021s | 0.002004s | byte equality observed |
| vector.test | 7 | 0.002033s | 0.001937s | 0.001933s | byte equality observed |
| weighted_covariance.test | 8 | 0.002001s | 0.001933s | 0.001931s | byte equality observed |
| weighted_extended_p_square.test | 1 | 0.023165s | 0.022530s | 0.019674s | byte equality observed |
| weighted_kurtosis.test | 4 | 0.005247s | 0.004233s | 0.003024s | byte equality observed |
| weighted_mean.test | 9 | 0.001984s | 0.001906s | 0.001896s | byte equality observed |
| weighted_median.test | 1 | 1.081789s | 1.099398s | 1.006438s | byte equality observed |
| weighted_moment.test | 9 | 0.001971s | 0.001879s | 0.001922s | byte equality observed |
| weighted_p_square_cumul_dist.test | 1 | 0.136412s | 0.175292s | 0.117134s | byte equality observed |
| weighted_p_square_quantile.test | 1 | 0.035431s | 0.033642s | 0.025398s | byte equality observed |
| weighted_pot_quantile.test | 1 | 0.029730s | 0.027355s | 0.023279s | byte equality observed |
| weighted_skewness.test | 2 | 0.005314s | 0.004221s | 0.003034s | byte equality observed |
| weighted_sum.test | 9 | 0.001971s | 0.001875s | 0.001871s | byte equality observed |
| weighted_sum_kahan.test | 2 | 0.008750s | 0.013899s | 0.008593s | byte equality observed |
| weighted_tail_mean.test | 1 | 0.021547s | 0.022572s | 0.020193s | byte equality observed |
| weighted_tail_quantile.test | 1 | 0.025043s | 0.025499s | 0.021587s | byte equality observed |
| weighted_tail_variate_means.test | 8 | 0.002016s | 0.001934s | 0.001948s | byte equality observed |
| weighted_variance.test | 8 | 0.002517s | 0.002286s | 0.002133s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43357224918).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
