# libs/accumulators/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| count.test | RUN | 1.537578 | 2.609804 | 3.833273 | 0.003530 | 0.004246 | 0.003426 |
| covariance.test | RUN | 2.005515 | 6.529495 | 8.033222 | 0.003867 | 0.006821 | 0.003245 |
| droppable.test | RUN | 1.124670 | 3.640526 | 6.000035 | 0.003562 | 0.003999 | 0.003661 |
| error_of.test | RUN | 1.214957 | 3.354945 | 6.310575 | 0.003153 | 0.003459 | 0.003297 |
| extended_p_square.test | RUN | 1.320000 | 4.448279 | 4.781096 | 0.005254 | 0.005518 | 0.005705 |
| extended_p_square_quantile.test | RUN | 1.679605 | 6.744916 | 6.534960 | 0.009112 | 0.010872 | 0.010483 |
| external_accumulator.test | RUN | 1.107045 | 3.446227 | 3.786219 | 0.003959 | 0.003646 | 0.003119 |
| external_weights.test | RUN | 0.937980 | 2.658391 | 3.122136 | 0.003786 | 0.003496 | 0.002767 |
| kurtosis.test | RUN | 1.552454 | 4.367231 | 5.337380 | 0.005286 | 0.006400 | 0.005281 |
| max.test | RUN | 0.993258 | 2.569877 | 2.969021 | 0.003374 | 0.004211 | 0.003574 |
| mean.test | RUN | 1.234176 | 3.882050 | 4.219688 | 0.003274 | 0.003492 | 0.003571 |
| median.test | RUN | 1.683467 | 7.285294 | 6.711537 | 0.134541 | 0.150522 | 0.128593 |
| min.test | RUN | 0.927858 | 2.658684 | 2.817025 | 0.003434 | 0.003451 | 0.005041 |
| moment.test | RUN | 1.038153 | 3.154773 | 3.302173 | 0.003573 | 0.003757 | 0.003428 |
| p_square_cumul_dist.test | RUN | 1.455502 | 4.945026 | 4.808966 | 0.960918 | 1.183965 | 0.927581 |
| p_square_quantile.test | RUN | 1.243664 | 4.491771 | 4.031707 | 0.035445 | 0.032302 | 0.052751 |
| reference.test | RUN | 1.042441 | 2.897580 | 3.387680 | 0.003278 | 0.003129 | 0.003733 |
| rolling_count.test | RUN | 1.077772 | 3.229971 | 3.425971 | 0.003191 | 0.003962 | 0.003696 |
| rolling_sum.test | RUN | 1.158309 | 3.505206 | 3.483198 | 0.003011 | 0.003490 | 0.002875 |
| rolling_mean.test | RUN | 1.590523 | 6.199634 | 5.674325 | 0.003681 | 0.003521 | 0.003683 |
| rolling_variance.test | RUN | 1.992749 | 7.048368 | 6.949624 | 0.003582 | 0.003859 | 0.003980 |
| rolling_moment.test | RUN | 1.294622 | 3.948699 | 4.418288 | 0.005105 | 0.004407 | 0.003159 |
| skewness.test | RUN | 1.412696 | 4.105847 | 4.818281 | 0.005186 | 0.005768 | 0.004497 |
| sum.test | RUN | 1.011843 | 2.889014 | 3.271914 | 0.003399 | 0.004238 | 0.003567 |
| sum_kahan.test | RUN | 1.037590 | 2.902909 | 3.364530 | 0.022383 | 0.030251 | 0.021096 |
| tail.test | RUN | 1.124291 | 4.233546 | 3.966836 | 0.003408 | 0.004655 | 0.003544 |
| tail_mean.test | RUN | 1.582164 | 4.882577 | 5.545714 | 0.029603 | 0.037079 | 0.028643 |
| tail_quantile.test | RUN | 1.357310 | 4.004840 | 4.420833 | 0.032775 | 0.042546 | 0.030270 |
| variance.test | RUN | 1.228972 | 3.757859 | 4.091839 | 0.003535 | 0.003718 | 0.004475 |
| vector.test | RUN | 1.307905 | 4.267394 | 5.172161 | 0.002961 | 0.010788 | 0.003965 |
| weighted_covariance.test | RUN | 1.763963 | 6.632079 | 8.048356 | 0.003390 | 0.003604 | 0.003233 |
| weighted_extended_p_square.test | RUN | 1.216341 | 3.998233 | 4.038983 | 0.026229 | 0.027519 | 0.022005 |
| weighted_kurtosis.test | RUN | 1.493688 | 4.094385 | 5.013991 | 0.005851 | 0.005940 | 0.004849 |
| weighted_mean.test | RUN | 1.232831 | 3.459835 | 4.213259 | 0.003095 | 0.004552 | 0.003491 |
| weighted_median.test | RUN | 1.591449 | 4.887440 | 5.613013 | 1.335967 | 1.359851 | 1.306517 |
| weighted_moment.test | RUN | 0.990515 | 2.597209 | 3.250625 | 0.003365 | 0.002962 | 0.003459 |
| weighted_p_square_cumul_dist.test | RUN | 1.376254 | 4.024025 | 3.880564 | 0.141062 | 0.207811 | 0.133397 |
| weighted_p_square_quantile.test | RUN | 1.123438 | 4.052814 | 3.760903 | 0.037393 | 0.035599 | 0.028611 |
| weighted_skewness.test | RUN | 1.356583 | 3.885967 | 4.777108 | 0.006570 | 0.005732 | 0.004448 |
| weighted_sum.test | RUN | 0.836955 | 2.193496 | 2.508837 | 0.003664 | 0.003531 | 0.003503 |
| weighted_sum_kahan.test | RUN | 0.926886 | 2.334443 | 2.692058 | 0.013331 | 0.016927 | 0.012372 |
| weighted_variance.test | RUN | 1.480258 | 4.330906 | 5.653853 | 0.004527 | 0.003676 | 0.003660 |
| valarray.test | RUN | 1.357516 | 4.064667 | 4.968190 | 0.003472 | 0.003541 | 0.003394 |
| tail_variate_means.test | RUN | 1.776370 | 6.307475 | 6.929602 | 0.003427 | 0.003499 | 0.004986 |
| pot_quantile.test | RUN | 2.158926 | 6.933469 | 8.645009 | 0.013815 | 0.013437 | 0.012402 |
| weighted_pot_quantile.test | RUN | 2.200643 | 7.104346 | 8.196829 | 0.030635 | 0.035862 | 0.030111 |
| weighted_tail_mean.test | RUN | 2.231063 | 5.508277 | 7.183012 | 0.028087 | 0.034234 | 0.034315 |
| weighted_tail_quantile.test | RUN | 1.932409 | 6.091275 | 7.056770 | 0.030997 | 0.040060 | 0.033493 |
| weighted_tail_variate_means.test | RUN | 2.372962 | 9.073883 | 10.115841 | 0.003685 | 0.003252 | 0.004526 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.089× | 1.113×, 1.065× | 0.993× |
| clang | 0.916×† | 0.826×, 1.006× | 1.001× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1353 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| count.test | 2 | 0.003213s | 0.003161s | 0.003159s | byte equality observed |
| covariance.test | 2 | 0.003440s | 0.003256s | 0.003084s | byte equality observed |
| droppable.test | 1 | 0.003476s | 0.003378s | 0.003776s | byte equality observed |
| error_of.test | 1 | 0.003577s | 0.003639s | 0.003322s | byte equality observed |
| extended_p_square.test | 2 | 0.004897s | 0.004992s | 0.005139s | byte equality observed |
| extended_p_square_quantile.test | 1 | 0.011135s | 0.009444s | 0.009655s | byte equality observed |
| external_accumulator.test | 3 | 0.003320s | 0.003131s | 0.003066s | byte equality observed |
| external_weights.test | 2 | 0.003354s | 0.002993s | 0.002943s | byte equality observed |
| kurtosis.test | 1 | 0.006150s | 0.005018s | 0.004847s | byte equality observed |
| max.test | 2 | 0.003224s | 0.003037s | 0.003160s | byte equality observed |
| mean.test | 1 | 0.004034s | 0.003379s | 0.003151s | byte equality observed |
| median.test | 1 | 0.137723s | 0.186103s | 0.128336s | byte equality observed |
| min.test | 1 | 0.003657s | 0.003640s | 0.003386s | byte equality observed |
| moment.test | 1 | 0.003221s | 0.003218s | 0.003070s | byte equality observed |
| p_square_cumul_dist.test | 1 | 0.978098s | 1.198361s | 0.935920s | byte equality observed |
| p_square_quantile.test | 1 | 0.039113s | 0.032451s | 0.032265s | byte equality observed |
| pot_quantile.test | 1 | 0.013301s | 0.012588s | 0.011176s | byte equality observed |
| reference.test | 2 | 0.003373s | 0.003188s | 0.003022s | byte equality observed |
| rolling_count.test | 3 | 0.003266s | 0.002828s | 0.003137s | byte equality observed |
| rolling_mean.test | 4 | 0.003601s | 0.003128s | 0.003055s | byte equality observed |
| rolling_moment.test | 4 | 0.003124s | 0.003351s | 0.003048s | byte equality observed |
| rolling_sum.test | 3 | 0.003288s | 0.002873s | 0.003191s | byte equality observed |
| rolling_variance.test | 3 | 0.003196s | 0.003203s | 0.003034s | byte equality observed |
| skewness.test | 3 | 0.005281s | 0.005066s | 0.004054s | byte equality observed |
| sum.test | 4 | 0.003005s | 0.003187s | 0.003192s | byte equality observed |
| sum_kahan.test | 1 | 0.022097s | 0.029811s | 0.023684s | byte equality observed |
| tail.test | 3 | 0.003168s | 0.003028s | 0.002688s | byte equality observed |
| tail_mean.test | 1 | 0.029716s | 0.037942s | 0.027437s | byte equality observed |
| tail_quantile.test | 1 | 0.033202s | 0.040607s | 0.030090s | byte equality observed |
| tail_variate_means.test | 4 | 0.003024s | 0.003066s | 0.002810s | byte equality observed |
| valarray.test | 4 | 0.003097s | 0.002884s | 0.002800s | byte equality observed |
| variance.test | 4 | 0.003103s | 0.002977s | 0.002868s | byte equality observed |
| vector.test | 4 | 0.002995s | 0.002948s | 0.002660s | byte equality observed |
| weighted_covariance.test | 4 | 0.003007s | 0.002863s | 0.002849s | byte equality observed |
| weighted_extended_p_square.test | 1 | 0.028137s | 0.033140s | 0.026996s | byte equality observed |
| weighted_kurtosis.test | 3 | 0.006274s | 0.005311s | 0.003823s | byte equality observed |
| weighted_mean.test | 4 | 0.002996s | 0.002988s | 0.002902s | byte equality observed |
| weighted_median.test | 1 | 1.406979s | 1.354887s | 1.310660s | byte equality observed |
| weighted_moment.test | 4 | 0.003171s | 0.002892s | 0.002733s | byte equality observed |
| weighted_p_square_cumul_dist.test | 1 | 0.144197s | 0.189598s | 0.145978s | byte equality observed |
| weighted_p_square_quantile.test | 1 | 0.037710s | 0.034584s | 0.034473s | byte equality observed |
| weighted_pot_quantile.test | 1 | 0.031298s | 0.037191s | 0.028650s | byte equality observed |
| weighted_skewness.test | 3 | 0.005994s | 0.005395s | 0.004090s | byte equality observed |
| weighted_sum.test | 5 | 0.003032s | 0.002819s | 0.002525s | byte equality observed |
| weighted_sum_kahan.test | 1 | 0.012726s | 0.016144s | 0.011318s | byte equality observed |
| weighted_tail_mean.test | 1 | 0.026937s | 0.035458s | 0.029793s | byte equality observed |
| weighted_tail_quantile.test | 1 | 0.031720s | 0.038916s | 0.028475s | byte equality observed |
| weighted_tail_variate_means.test | 5 | 0.002972s | 0.002950s | 0.002699s | byte equality observed |
| weighted_variance.test | 4 | 0.003446s | 0.003266s | 0.002881s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
