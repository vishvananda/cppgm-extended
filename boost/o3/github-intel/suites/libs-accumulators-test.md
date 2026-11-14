# libs/accumulators/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| count.test | RUN | 0.585793 | 1.683822 | 1.823345 | 0.002298 | 0.002138 | 0.001809 |
| covariance.test | RUN | 1.112559 | 4.433746 | 4.777875 | 0.002492 | 0.002263 | 0.002006 |
| droppable.test | RUN | 0.712769 | 2.291774 | 2.571248 | 0.002163 | 0.002257 | 0.001917 |
| error_of.test | RUN | 0.797345 | 2.257576 | 2.647294 | 0.001930 | 0.001994 | 0.001691 |
| extended_p_square.test | RUN | 0.861516 | 2.883014 | 2.849169 | 0.003838 | 0.003650 | 0.003223 |
| extended_p_square_quantile.test | RUN | 1.113616 | 4.561690 | 4.096189 | 0.007105 | 0.007428 | 0.006598 |
| external_accumulator.test | RUN | 0.722233 | 2.204855 | 2.445305 | 0.002277 | 0.002260 | 0.001923 |
| external_weights.test | RUN | 0.632669 | 1.645328 | 1.927899 | 0.002042 | 0.002183 | 0.001776 |
| kurtosis.test | RUN | 0.919178 | 2.748853 | 3.197628 | 0.004533 | 0.003781 | 0.002807 |
| max.test | RUN | 0.583632 | 1.638888 | 1.897570 | 0.002175 | 0.002143 | 0.002085 |
| mean.test | RUN | 0.752048 | 2.355800 | 2.708166 | 0.002176 | 0.002331 | 0.002014 |
| median.test | RUN | 1.053180 | 4.731347 | 4.113904 | 0.115032 | 0.122229 | 0.104742 |
| min.test | RUN | 0.593010 | 1.688440 | 1.822733 | 0.002122 | 0.002063 | 0.001892 |
| moment.test | RUN | 0.660228 | 1.858732 | 2.166475 | 0.002324 | 0.001818 | 0.001966 |
| p_square_cumul_dist.test | RUN | 0.893116 | 3.312914 | 3.123880 | 0.815480 | 0.902934 | 0.710872 |
| p_square_quantile.test | RUN | 0.768494 | 2.900200 | 2.572563 | 0.024216 | 0.024021 | 0.020862 |
| reference.test | RUN | 0.659106 | 1.842021 | 2.148516 | 0.002080 | 0.002169 | 0.001939 |
| rolling_count.test | RUN | 0.650186 | 2.139782 | 2.173454 | 0.001937 | 0.002109 | 0.001946 |
| rolling_sum.test | RUN | 0.656374 | 2.136444 | 2.254045 | 0.002162 | 0.002114 | 0.001936 |
| rolling_mean.test | RUN | 0.999667 | 3.839315 | 3.657966 | 0.001967 | 0.002227 | 0.001758 |
| rolling_variance.test | RUN | 1.131261 | 4.528318 | 4.220006 | 0.002444 | 0.002344 | 0.001998 |
| rolling_moment.test | RUN | 0.827411 | 2.576884 | 2.799121 | 0.002142 | 0.002344 | 0.002311 |
| skewness.test | RUN | 0.870183 | 2.632219 | 3.078595 | 0.004660 | 0.003656 | 0.002830 |
| sum.test | RUN | 0.612226 | 1.804418 | 2.040307 | 0.002047 | 0.002180 | 0.001980 |
| sum_kahan.test | RUN | 0.664372 | 1.930472 | 2.156290 | 0.011863 | 0.018810 | 0.011109 |
| tail.test | RUN | 0.715088 | 2.669301 | 2.402654 | 0.001943 | 0.002118 | 0.001985 |
| tail_mean.test | RUN | 1.001023 | 3.121307 | 3.492400 | 0.018915 | 0.018452 | 0.019440 |
| tail_quantile.test | RUN | 0.825445 | 2.583553 | 2.774628 | 0.023609 | 0.024123 | 0.020576 |
| variance.test | RUN | 0.786757 | 2.257534 | 2.702875 | 0.001989 | 0.002160 | 0.001963 |
| vector.test | RUN | 0.846931 | 2.797907 | 3.257737 | 0.001909 | 0.002031 | 0.001906 |
| weighted_covariance.test | RUN | 1.126718 | 4.437632 | 4.846288 | 0.001880 | 0.001911 | 0.001803 |
| weighted_extended_p_square.test | RUN | 0.752935 | 2.594823 | 2.533040 | 0.021453 | 0.021576 | 0.017736 |
| weighted_kurtosis.test | RUN | 0.901449 | 2.529739 | 3.023190 | 0.004616 | 0.004220 | 0.002686 |
| weighted_mean.test | RUN | 0.766273 | 2.263250 | 2.655207 | 0.002416 | 0.002028 | 0.001750 |
| weighted_median.test | RUN | 0.981400 | 3.114022 | 3.436058 | 1.055128 | 1.041641 | 0.973322 |
| weighted_moment.test | RUN | 0.621671 | 1.723940 | 2.029780 | 0.001956 | 0.002227 | 0.001806 |
| weighted_p_square_cumul_dist.test | RUN | 0.763046 | 2.479820 | 2.250861 | 0.122037 | 0.117815 | 0.096081 |
| weighted_p_square_quantile.test | RUN | 0.711223 | 2.559988 | 2.260497 | 0.030682 | 0.025405 | 0.023071 |
| weighted_skewness.test | RUN | 0.873500 | 2.457731 | 2.890586 | 0.004586 | 0.004184 | 0.002836 |
| weighted_sum.test | RUN | 0.528269 | 1.433188 | 1.579168 | 0.002277 | 0.002283 | 0.001783 |
| weighted_sum_kahan.test | RUN | 0.488555 | 1.549701 | 1.706501 | 0.006024 | 0.010347 | 0.006267 |
| weighted_variance.test | RUN | 0.960210 | 2.760264 | 3.367615 | 0.002505 | 0.002489 | 0.001991 |
| valarray.test | RUN | 0.894621 | 2.684369 | 3.106235 | 0.002073 | 0.002206 | 0.001828 |
| tail_variate_means.test | RUN | 1.139176 | 4.316183 | 4.397734 | 0.002135 | 0.002024 | 0.001966 |
| pot_quantile.test | RUN | 1.375774 | 4.729412 | 5.144909 | 0.014231 | 0.010715 | 0.008237 |
| weighted_pot_quantile.test | RUN | 1.378926 | 4.641262 | 5.283707 | 0.024867 | 0.026537 | 0.021148 |
| weighted_tail_mean.test | RUN | 1.198676 | 3.688542 | 4.273455 | 0.020766 | 0.021767 | 0.015402 |
| weighted_tail_quantile.test | RUN | 1.172317 | 3.016021 | 3.947035 | 0.018483 | 0.018030 | 0.015384 |
| weighted_tail_variate_means.test | RUN | 1.310028 | 3.946181 | 4.518630 | 0.001457 | 0.001415 | 0.001368 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.049× | 1.045×, 1.052× | 1.002× |
| clang | 0.886× | 0.882×, 0.889× | 1.003× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1364 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43559814672).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| count.test | 1 | 0.001947s | 0.001850s | 0.001899s | byte equality observed |
| covariance.test | 1 | 0.001986s | 0.002031s | 0.001768s | byte equality observed |
| droppable.test | 1 | 0.001922s | 0.001836s | 0.001825s | byte equality observed |
| error_of.test | 6 | 0.001559s | 0.001562s | 0.001514s | byte equality observed |
| extended_p_square.test | 5 | 0.003023s | 0.003021s | 0.002775s | byte equality observed |
| extended_p_square_quantile.test | 3 | 0.006464s | 0.006598s | 0.005891s | byte equality observed |
| external_accumulator.test | 6 | 0.001582s | 0.001561s | 0.001491s | byte equality observed |
| external_weights.test | 9 | 0.001513s | 0.001477s | 0.001488s | byte equality observed |
| kurtosis.test | 5 | 0.003969s | 0.003057s | 0.002279s | byte equality observed |
| max.test | 9 | 0.001584s | 0.001545s | 0.001531s | byte equality observed |
| mean.test | 9 | 0.001612s | 0.001542s | 0.001505s | byte equality observed |
| median.test | 1 | 0.115466s | 0.131086s | 0.097939s | byte equality observed |
| min.test | 9 | 0.001585s | 0.001534s | 0.001533s | byte equality observed |
| moment.test | 8 | 0.001576s | 0.001530s | 0.001528s | byte equality observed |
| p_square_cumul_dist.test | 1 | 0.721029s | 0.791085s | 0.615525s | byte equality observed |
| p_square_quantile.test | 1 | 0.024559s | 0.023895s | 0.020771s | byte equality observed |
| pot_quantile.test | 2 | 0.013518s | 0.009632s | 0.008197s | byte equality observed |
| reference.test | 10 | 0.001552s | 0.001538s | 0.001493s | byte equality observed |
| rolling_count.test | 10 | 0.001589s | 0.001528s | 0.001536s | byte equality observed |
| rolling_mean.test | 10 | 0.001665s | 0.001586s | 0.001572s | byte equality observed |
| rolling_moment.test | 10 | 0.001628s | 0.001555s | 0.001567s | byte equality observed |
| rolling_sum.test | 10 | 0.001589s | 0.001512s | 0.001530s | byte equality observed |
| rolling_variance.test | 10 | 0.001661s | 0.001571s | 0.001592s | byte equality observed |
| skewness.test | 4 | 0.003960s | 0.003094s | 0.002309s | byte equality observed |
| sum.test | 10 | 0.001564s | 0.001493s | 0.001523s | byte equality observed |
| sum_kahan.test | 2 | 0.010571s | 0.017671s | 0.010650s | byte equality observed |
| tail.test | 9 | 0.001570s | 0.001486s | 0.001486s | byte equality observed |
| tail_mean.test | 2 | 0.018270s | 0.018165s | 0.017943s | byte equality observed |
| tail_quantile.test | 1 | 0.022510s | 0.021907s | 0.020036s | byte equality observed |
| tail_variate_means.test | 8 | 0.001577s | 0.001539s | 0.001490s | byte equality observed |
| valarray.test | 10 | 0.001515s | 0.001472s | 0.001456s | byte equality observed |
| variance.test | 10 | 0.001615s | 0.001525s | 0.001562s | byte equality observed |
| vector.test | 9 | 0.001533s | 0.001493s | 0.001476s | byte equality observed |
| weighted_covariance.test | 9 | 0.001506s | 0.001468s | 0.001444s | byte equality observed |
| weighted_extended_p_square.test | 1 | 0.020046s | 0.020980s | 0.016565s | byte equality observed |
| weighted_kurtosis.test | 5 | 0.004389s | 0.003580s | 0.002383s | byte equality observed |
| weighted_mean.test | 10 | 0.001491s | 0.001460s | 0.001411s | byte equality observed |
| weighted_median.test | 1 | 0.937454s | 0.930913s | 0.863031s | byte equality observed |
| weighted_moment.test | 10 | 0.001468s | 0.001458s | 0.001431s | byte equality observed |
| weighted_p_square_cumul_dist.test | 1 | 0.116027s | 0.144126s | 0.102679s | byte equality observed |
| weighted_p_square_quantile.test | 1 | 0.030422s | 0.026989s | 0.021843s | byte equality observed |
| weighted_pot_quantile.test | 1 | 0.026295s | 0.023564s | 0.020949s | byte equality observed |
| weighted_skewness.test | 4 | 0.004374s | 0.003577s | 0.002335s | byte equality observed |
| weighted_sum.test | 10 | 0.001465s | 0.001431s | 0.001407s | byte equality observed |
| weighted_sum_kahan.test | 3 | 0.005830s | 0.009347s | 0.005721s | byte equality observed |
| weighted_tail_mean.test | 1 | 0.019548s | 0.019317s | 0.018523s | byte equality observed |
| weighted_tail_quantile.test | 1 | 0.022617s | 0.021299s | 0.018746s | byte equality observed |
| weighted_tail_variate_means.test | 7 | 0.001578s | 0.001464s | 0.001468s | byte equality observed |
| weighted_variance.test | 8 | 0.002009s | 0.001789s | 0.001598s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43559814672).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
