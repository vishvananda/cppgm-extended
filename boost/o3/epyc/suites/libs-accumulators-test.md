# libs/accumulators/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| count.test | RUN | 1.029750 | 2.755708 | 3.426698 | 0.004153 | 0.003949 | 0.003907 |
| covariance.test | RUN | 1.999196 | 6.891741 | 8.768556 | 0.004043 | 0.004182 | 0.004356 |
| droppable.test | RUN | 1.246636 | 3.798825 | 4.652602 | 0.004105 | 0.004452 | 0.004092 |
| error_of.test | RUN | 1.374476 | 3.628288 | 5.216757 | 0.003653 | 0.003544 | 0.003805 |
| extended_p_square.test | RUN | 1.432875 | 4.528551 | 5.176826 | 0.006152 | 0.006180 | 0.005817 |
| extended_p_square_quantile.test | RUN | 1.879046 | 6.953296 | 7.470070 | 0.012264 | 0.012465 | 0.010271 |
| external_accumulator.test | RUN | 1.186972 | 3.549760 | 4.665136 | 0.004247 | 0.003837 | 0.003920 |
| external_weights.test | RUN | 1.094374 | 2.863834 | 3.636519 | 0.003708 | 0.004242 | 0.003440 |
| kurtosis.test | RUN | 1.652222 | 4.537975 | 6.163405 | 0.007152 | 0.007185 | 0.005126 |
| max.test | RUN | 1.055995 | 2.593250 | 3.365815 | 0.003920 | 0.004273 | 0.004100 |
| mean.test | RUN | 1.281489 | 3.941153 | 4.958075 | 0.003787 | 0.004162 | 0.004154 |
| median.test | RUN | 1.829740 | 7.147428 | 7.434243 | 0.178561 | 0.167680 | 0.147717 |
| min.test | RUN | 1.015855 | 2.721522 | 3.387090 | 0.003786 | 0.004880 | 0.003988 |
| moment.test | RUN | 1.156314 | 3.167562 | 4.107858 | 0.003969 | 0.004328 | 0.004117 |
| p_square_cumul_dist.test | RUN | 1.450612 | 5.203719 | 5.640099 | 1.222820 | 1.287065 | 1.077472 |
| p_square_quantile.test | RUN | 1.257140 | 4.647820 | 4.795900 | 0.038206 | 0.038319 | 0.032764 |
| reference.test | RUN | 1.136531 | 3.103882 | 4.159523 | 0.004107 | 0.004217 | 0.004215 |
| rolling_count.test | RUN | 1.182155 | 3.240910 | 3.865781 | 0.003340 | 0.004145 | 0.004037 |
| rolling_sum.test | RUN | 1.130220 | 3.424971 | 4.232561 | 0.004323 | 0.004133 | 0.004165 |
| rolling_mean.test | RUN | 1.576920 | 5.920722 | 6.657135 | 0.004069 | 0.004676 | 0.004182 |
| rolling_variance.test | RUN | 1.837597 | 6.837532 | 7.646943 | 0.004060 | 0.003901 | 0.004469 |
| rolling_moment.test | RUN | 1.397538 | 4.256505 | 5.287043 | 0.004006 | 0.004352 | 0.004695 |
| skewness.test | RUN | 1.564800 | 4.359917 | 5.802885 | 0.007198 | 0.005965 | 0.005236 |
| sum.test | RUN | 1.052597 | 2.947529 | 3.784761 | 0.003392 | 0.004054 | 0.003740 |
| sum_kahan.test | RUN | 1.107083 | 3.136468 | 4.073705 | 0.019495 | 0.030849 | 0.019215 |
| tail.test | RUN | 1.264756 | 4.347111 | 4.551597 | 0.003852 | 0.003960 | 0.003596 |
| tail_mean.test | RUN | 1.691688 | 5.087603 | 6.666937 | 0.026362 | 0.034773 | 0.027972 |
| tail_quantile.test | RUN | 1.395074 | 3.977457 | 5.072612 | 0.033330 | 0.041698 | 0.030567 |
| variance.test | RUN | 1.331260 | 3.693273 | 4.944287 | 0.003851 | 0.004149 | 0.003992 |
| vector.test | RUN | 1.435407 | 4.556779 | 6.002670 | 0.003713 | 0.003815 | 0.003691 |
| weighted_covariance.test | RUN | 1.970570 | 7.031960 | 9.213053 | 0.004101 | 0.004061 | 0.003567 |
| weighted_extended_p_square.test | RUN | 1.359168 | 4.121362 | 4.661042 | 0.031632 | 0.031668 | 0.026065 |
| weighted_kurtosis.test | RUN | 1.549186 | 4.173661 | 5.846594 | 0.007595 | 0.006358 | 0.005615 |
| weighted_mean.test | RUN | 1.286485 | 3.751025 | 5.035831 | 0.003456 | 0.003850 | 0.003732 |
| weighted_median.test | RUN | 1.667701 | 4.924427 | 6.439601 | 1.472518 | 1.386142 | 1.299201 |
| weighted_moment.test | RUN | 1.048283 | 2.759378 | 3.733398 | 0.003766 | 0.003620 | 0.003909 |
| weighted_p_square_cumul_dist.test | RUN | 1.261910 | 4.197780 | 4.313920 | 0.168650 | 0.166069 | 0.140002 |
| weighted_p_square_quantile.test | RUN | 1.201491 | 3.925811 | 4.385960 | 0.044753 | 0.041096 | 0.036547 |
| weighted_skewness.test | RUN | 1.505438 | 4.070596 | 5.531712 | 0.007913 | 0.006256 | 0.004610 |
| weighted_sum.test | RUN | 0.792560 | 2.307523 | 2.956575 | 0.003888 | 0.003991 | 0.003448 |
| weighted_sum_kahan.test | RUN | 0.962832 | 2.524256 | 3.206761 | 0.011695 | 0.017671 | 0.011605 |
| weighted_variance.test | RUN | 1.653711 | 4.471383 | 6.307272 | 0.004541 | 0.004488 | 0.004049 |
| valarray.test | RUN | 1.524117 | 4.249791 | 5.866171 | 0.004956 | 0.004033 | 0.003602 |
| tail_variate_means.test | RUN | 1.952703 | 6.793831 | 8.158056 | 0.003802 | 0.003559 | 0.003594 |
| pot_quantile.test | RUN | 2.357947 | 7.476657 | 9.864927 | 0.019416 | 0.015421 | 0.013834 |
| weighted_pot_quantile.test | RUN | 2.354335 | 7.356471 | 9.719446 | 0.037013 | 0.042464 | 0.034213 |
| weighted_tail_mean.test | RUN | 2.114245 | 5.948018 | 8.277037 | 0.023362 | 0.039281 | 0.021753 |
| weighted_tail_quantile.test | RUN | 2.076492 | 4.742131 | 7.385547 | 0.027427 | 0.028722 | 0.022760 |
| weighted_tail_variate_means.test | RUN | 2.238200 | 6.239088 | 8.215649 | 0.002946 | 0.002508 | 0.002394 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.069× | 1.068×, 1.071× | 0.989× |
| clang | 0.889× | 0.888×, 0.890× | 0.997× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43133898266).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| count.test | 1 | 0.003208s | 0.003039s | 0.002933s | byte equality observed |
| covariance.test | 1 | 0.003116s | 0.003056s | 0.003048s | byte equality observed |
| droppable.test | 1 | 0.002897s | 0.003083s | 0.003086s | byte equality observed |
| error_of.test | 1 | 0.003038s | 0.002917s | 0.003013s | byte equality observed |
| extended_p_square.test | 4 | 0.004701s | 0.004771s | 0.004331s | byte equality observed |
| extended_p_square_quantile.test | 2 | 0.010883s | 0.010490s | 0.008831s | byte equality observed |
| external_accumulator.test | 4 | 0.002744s | 0.002704s | 0.002629s | byte equality observed |
| external_weights.test | 5 | 0.002626s | 0.002546s | 0.002543s | byte equality observed |
| kurtosis.test | 3 | 0.005803s | 0.004725s | 0.004051s | byte equality observed |
| max.test | 6 | 0.002688s | 0.002637s | 0.002620s | byte equality observed |
| mean.test | 5 | 0.002792s | 0.002709s | 0.002647s | byte equality observed |
| median.test | 1 | 0.173195s | 0.165990s | 0.141425s | byte equality observed |
| min.test | 6 | 0.002679s | 0.002661s | 0.002516s | byte equality observed |
| moment.test | 5 | 0.002630s | 0.002550s | 0.002555s | byte equality observed |
| p_square_cumul_dist.test | 1 | 1.000846s | 1.170009s | 0.902291s | byte equality observed |
| p_square_quantile.test | 1 | 0.039536s | 0.035120s | 0.030973s | byte equality observed |
| pot_quantile.test | 2 | 0.017463s | 0.013639s | 0.011862s | byte equality observed |
| reference.test | 5 | 0.002644s | 0.002636s | 0.002595s | byte equality observed |
| rolling_count.test | 7 | 0.002637s | 0.002610s | 0.002507s | byte equality observed |
| rolling_mean.test | 6 | 0.002738s | 0.002796s | 0.002646s | byte equality observed |
| rolling_moment.test | 6 | 0.002739s | 0.002769s | 0.002656s | byte equality observed |
| rolling_sum.test | 7 | 0.002642s | 0.002590s | 0.002529s | byte equality observed |
| rolling_variance.test | 6 | 0.002776s | 0.002771s | 0.002646s | byte equality observed |
| skewness.test | 3 | 0.005663s | 0.004718s | 0.003967s | byte equality observed |
| sum.test | 7 | 0.002600s | 0.002563s | 0.002603s | byte equality observed |
| sum_kahan.test | 1 | 0.018295s | 0.030451s | 0.018222s | byte equality observed |
| tail.test | 7 | 0.002571s | 0.002496s | 0.002545s | byte equality observed |
| tail_mean.test | 1 | 0.024735s | 0.029205s | 0.025109s | byte equality observed |
| tail_quantile.test | 1 | 0.032097s | 0.040667s | 0.028737s | byte equality observed |
| tail_variate_means.test | 5 | 0.002598s | 0.002449s | 0.002563s | byte equality observed |
| valarray.test | 6 | 0.002604s | 0.002423s | 0.002430s | byte equality observed |
| variance.test | 6 | 0.002712s | 0.002564s | 0.002667s | byte equality observed |
| vector.test | 7 | 0.002581s | 0.002488s | 0.002488s | byte equality observed |
| weighted_covariance.test | 7 | 0.002510s | 0.002433s | 0.002483s | byte equality observed |
| weighted_extended_p_square.test | 1 | 0.029768s | 0.031254s | 0.024934s | byte equality observed |
| weighted_kurtosis.test | 3 | 0.006238s | 0.005198s | 0.003872s | byte equality observed |
| weighted_mean.test | 6 | 0.002493s | 0.002427s | 0.002409s | byte equality observed |
| weighted_median.test | 1 | 1.266475s | 1.277372s | 1.141927s | byte equality observed |
| weighted_moment.test | 5 | 0.002472s | 0.002471s | 0.002360s | byte equality observed |
| weighted_p_square_cumul_dist.test | 1 | 0.167910s | 0.187103s | 0.140649s | byte equality observed |
| weighted_p_square_quantile.test | 1 | 0.043472s | 0.039720s | 0.031933s | byte equality observed |
| weighted_pot_quantile.test | 1 | 0.035141s | 0.032713s | 0.028789s | byte equality observed |
| weighted_skewness.test | 3 | 0.006184s | 0.005236s | 0.003750s | byte equality observed |
| weighted_sum.test | 6 | 0.002429s | 0.002425s | 0.002337s | byte equality observed |
| weighted_sum_kahan.test | 2 | 0.010143s | 0.016012s | 0.009996s | byte equality observed |
| weighted_tail_mean.test | 1 | 0.025109s | 0.030139s | 0.024411s | byte equality observed |
| weighted_tail_quantile.test | 1 | 0.030094s | 0.034030s | 0.026561s | byte equality observed |
| weighted_tail_variate_means.test | 6 | 0.002514s | 0.002592s | 0.002355s | byte equality observed |
| weighted_variance.test | 5 | 0.003123s | 0.002983s | 0.002607s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43133898266).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
