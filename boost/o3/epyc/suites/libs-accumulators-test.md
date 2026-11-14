# libs/accumulators/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| count.test | RUN | 0.943620 | 2.747911 | 3.536683 | 0.004227 | 0.004305 | 0.004078 |
| covariance.test | RUN | 1.880022 | 6.839306 | 9.177954 | 0.003905 | 0.004234 | 0.004034 |
| droppable.test | RUN | 1.167037 | 3.573753 | 4.684586 | 0.003591 | 0.004249 | 0.004772 |
| error_of.test | RUN | 1.280534 | 3.594047 | 5.242550 | 0.003562 | 0.004075 | 0.003696 |
| extended_p_square.test | RUN | 1.407368 | 4.435999 | 5.361827 | 0.005884 | 0.006172 | 0.005960 |
| extended_p_square_quantile.test | RUN | 1.755472 | 6.802887 | 7.653535 | 0.011522 | 0.012136 | 0.010818 |
| external_accumulator.test | RUN | 1.185177 | 3.393476 | 4.673289 | 0.003856 | 0.003606 | 0.004141 |
| external_weights.test | RUN | 1.041344 | 2.828612 | 3.724957 | 0.003489 | 0.003424 | 0.003870 |
| kurtosis.test | RUN | 1.534423 | 4.451101 | 6.368420 | 0.006822 | 0.005857 | 0.005528 |
| max.test | RUN | 0.987745 | 2.758961 | 3.432837 | 0.003891 | 0.003751 | 0.004248 |
| mean.test | RUN | 1.246076 | 3.669731 | 5.111083 | 0.003902 | 0.004248 | 0.003898 |
| median.test | RUN | 1.690859 | 7.187120 | 7.500232 | 0.165473 | 0.164552 | 0.152480 |
| min.test | RUN | 0.967722 | 2.603667 | 3.539048 | 0.003813 | 0.004000 | 0.003358 |
| moment.test | RUN | 1.070078 | 3.016724 | 4.150870 | 0.004082 | 0.003768 | 0.003710 |
| p_square_cumul_dist.test | RUN | 1.439115 | 5.166224 | 5.926244 | 1.351367 | 1.285113 | 1.071307 |
| p_square_quantile.test | RUN | 1.263692 | 4.527535 | 4.930648 | 0.038535 | 0.038326 | 0.032887 |
| reference.test | RUN | 1.042179 | 3.013624 | 4.137321 | 0.003492 | 0.003787 | 0.003403 |
| rolling_count.test | RUN | 1.143779 | 3.106100 | 3.921038 | 0.003424 | 0.003901 | 0.003655 |
| rolling_sum.test | RUN | 1.105717 | 3.257386 | 4.256305 | 0.004209 | 0.004219 | 0.003586 |
| rolling_mean.test | RUN | 1.599563 | 6.066442 | 6.838854 | 0.004418 | 0.003923 | 0.003818 |
| rolling_variance.test | RUN | 1.751220 | 6.945365 | 7.639784 | 0.004137 | 0.003801 | 0.004045 |
| rolling_moment.test | RUN | 1.396488 | 4.039570 | 5.205156 | 0.004297 | 0.003923 | 0.004027 |
| skewness.test | RUN | 1.486481 | 4.276685 | 5.751951 | 0.006998 | 0.006347 | 0.005545 |
| sum.test | RUN | 1.007619 | 2.851589 | 3.803015 | 0.003871 | 0.004107 | 0.003972 |
| sum_kahan.test | RUN | 1.131027 | 3.028924 | 3.948217 | 0.019350 | 0.031212 | 0.019548 |
| tail.test | RUN | 1.196404 | 4.246032 | 4.464044 | 0.003714 | 0.004086 | 0.004245 |
| tail_mean.test | RUN | 1.639292 | 5.029870 | 6.639454 | 0.026263 | 0.038507 | 0.028145 |
| tail_quantile.test | RUN | 1.329706 | 4.149268 | 5.103382 | 0.033545 | 0.039084 | 0.030558 |
| variance.test | RUN | 1.327806 | 3.626370 | 4.975388 | 0.003831 | 0.003773 | 0.003880 |
| vector.test | RUN | 1.412206 | 4.502440 | 5.950044 | 0.003060 | 0.004084 | 0.003689 |
| weighted_covariance.test | RUN | 1.892548 | 6.911750 | 9.099402 | 0.003690 | 0.003630 | 0.003896 |
| weighted_extended_p_square.test | RUN | 1.259355 | 4.053164 | 4.757160 | 0.032183 | 0.032395 | 0.027042 |
| weighted_kurtosis.test | RUN | 1.491944 | 4.166892 | 5.887818 | 0.007168 | 0.006301 | 0.005181 |
| weighted_mean.test | RUN | 1.275064 | 3.711021 | 5.067355 | 0.003228 | 0.003564 | 0.003995 |
| weighted_median.test | RUN | 1.636182 | 4.849863 | 6.610747 | 1.384272 | 1.402343 | 1.297881 |
| weighted_moment.test | RUN | 1.057117 | 2.778700 | 3.950282 | 0.003404 | 0.004130 | 0.003658 |
| weighted_p_square_cumul_dist.test | RUN | 1.246722 | 4.070472 | 4.688644 | 0.193352 | 0.165505 | 0.139149 |
| weighted_p_square_quantile.test | RUN | 1.156851 | 4.041226 | 3.956602 | 0.045215 | 0.041222 | 0.034452 |
| weighted_skewness.test | RUN | 1.394310 | 3.667768 | 5.675939 | 0.007299 | 0.006236 | 0.005284 |
| weighted_sum.test | RUN | 0.755910 | 2.339007 | 2.997510 | 0.003356 | 0.003996 | 0.003774 |
| weighted_sum_kahan.test | RUN | 0.934356 | 2.380608 | 3.216499 | 0.011462 | 0.017689 | 0.011628 |
| weighted_variance.test | RUN | 1.538471 | 4.509693 | 6.223476 | 0.004449 | 0.004414 | 0.003828 |
| valarray.test | RUN | 1.431691 | 4.219624 | 5.873310 | 0.003594 | 0.003937 | 0.003834 |
| tail_variate_means.test | RUN | 1.838870 | 6.624155 | 8.061393 | 0.003542 | 0.003884 | 0.003759 |
| pot_quantile.test | RUN | 2.247176 | 7.478638 | 9.950916 | 0.019902 | 0.014076 | 0.013861 |
| weighted_pot_quantile.test | RUN | 2.222607 | 7.147523 | 9.874700 | 0.038209 | 0.041165 | 0.032226 |
| weighted_tail_mean.test | RUN | 2.009472 | 5.855578 | 8.163626 | 0.028683 | 0.036508 | 0.022262 |
| weighted_tail_quantile.test | RUN | 1.943243 | 4.840605 | 7.328184 | 0.026563 | 0.028511 | 0.022810 |
| weighted_tail_variate_means.test | RUN | 2.123451 | 5.923371 | 7.943331 | 0.002678 | 0.002572 | 0.002417 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.050× | 1.053×, 1.048× | 1.007× |
| clang | 0.883× | 0.885×, 0.881× | 0.992× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43609274616).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| count.test | 1 | 0.003033s | 0.002925s | 0.002954s | byte equality observed |
| covariance.test | 1 | 0.003191s | 0.002979s | 0.002982s | byte equality observed |
| droppable.test | 1 | 0.003051s | 0.003018s | 0.002878s | byte equality observed |
| error_of.test | 1 | 0.002979s | 0.002776s | 0.002986s | byte equality observed |
| extended_p_square.test | 4 | 0.004652s | 0.004689s | 0.004231s | byte equality observed |
| extended_p_square_quantile.test | 2 | 0.009981s | 0.010503s | 0.009248s | byte equality observed |
| external_accumulator.test | 6 | 0.002710s | 0.002615s | 0.002648s | byte equality observed |
| external_weights.test | 4 | 0.002553s | 0.002465s | 0.002440s | byte equality observed |
| kurtosis.test | 3 | 0.005727s | 0.004717s | 0.004035s | byte equality observed |
| max.test | 6 | 0.002692s | 0.002619s | 0.002581s | byte equality observed |
| mean.test | 7 | 0.002705s | 0.002588s | 0.002603s | byte equality observed |
| median.test | 1 | 0.158732s | 0.179626s | 0.142747s | byte equality observed |
| min.test | 6 | 0.002634s | 0.002507s | 0.002548s | byte equality observed |
| moment.test | 5 | 0.002656s | 0.002497s | 0.002564s | byte equality observed |
| p_square_cumul_dist.test | 1 | 1.059564s | 1.183694s | 0.916690s | byte equality observed |
| p_square_quantile.test | 1 | 0.037003s | 0.034541s | 0.029537s | byte equality observed |
| pot_quantile.test | 2 | 0.017646s | 0.013599s | 0.012057s | byte equality observed |
| reference.test | 8 | 0.002610s | 0.002608s | 0.002497s | byte equality observed |
| rolling_count.test | 7 | 0.002648s | 0.002529s | 0.002558s | byte equality observed |
| rolling_mean.test | 7 | 0.002718s | 0.002630s | 0.002661s | byte equality observed |
| rolling_moment.test | 7 | 0.002721s | 0.002616s | 0.002654s | byte equality observed |
| rolling_sum.test | 7 | 0.002623s | 0.002464s | 0.002555s | byte equality observed |
| rolling_variance.test | 6 | 0.002776s | 0.002607s | 0.002691s | byte equality observed |
| skewness.test | 4 | 0.005602s | 0.004654s | 0.004010s | byte equality observed |
| sum.test | 6 | 0.002540s | 0.002555s | 0.002475s | byte equality observed |
| sum_kahan.test | 1 | 0.018312s | 0.030006s | 0.017988s | byte equality observed |
| tail.test | 7 | 0.002691s | 0.002372s | 0.002566s | byte equality observed |
| tail_mean.test | 1 | 0.023798s | 0.029055s | 0.025027s | byte equality observed |
| tail_quantile.test | 1 | 0.031600s | 0.038633s | 0.026995s | byte equality observed |
| tail_variate_means.test | 7 | 0.002550s | 0.002351s | 0.002485s | byte equality observed |
| valarray.test | 7 | 0.002573s | 0.002380s | 0.002435s | byte equality observed |
| variance.test | 7 | 0.002674s | 0.002510s | 0.002594s | byte equality observed |
| vector.test | 7 | 0.002508s | 0.002404s | 0.002395s | byte equality observed |
| weighted_covariance.test | 7 | 0.002515s | 0.002446s | 0.002419s | byte equality observed |
| weighted_extended_p_square.test | 1 | 0.030553s | 0.030850s | 0.024869s | byte equality observed |
| weighted_kurtosis.test | 4 | 0.006251s | 0.005112s | 0.003765s | byte equality observed |
| weighted_mean.test | 6 | 0.002473s | 0.002338s | 0.002400s | byte equality observed |
| weighted_median.test | 1 | 1.242939s | 1.266758s | 1.141319s | byte equality observed |
| weighted_moment.test | 8 | 0.002399s | 0.002309s | 0.002272s | byte equality observed |
| weighted_p_square_cumul_dist.test | 1 | 0.189117s | 0.181873s | 0.141562s | byte equality observed |
| weighted_p_square_quantile.test | 1 | 0.043138s | 0.039462s | 0.033010s | byte equality observed |
| weighted_pot_quantile.test | 1 | 0.035631s | 0.032008s | 0.029050s | byte equality observed |
| weighted_skewness.test | 4 | 0.006161s | 0.005008s | 0.003745s | byte equality observed |
| weighted_sum.test | 6 | 0.002391s | 0.002292s | 0.002289s | byte equality observed |
| weighted_sum_kahan.test | 2 | 0.010129s | 0.015773s | 0.009986s | byte equality observed |
| weighted_tail_mean.test | 1 | 0.025072s | 0.028690s | 0.026409s | byte equality observed |
| weighted_tail_quantile.test | 1 | 0.030051s | 0.034516s | 0.029057s | byte equality observed |
| weighted_tail_variate_means.test | 6 | 0.002482s | 0.002346s | 0.002493s | byte equality observed |
| weighted_variance.test | 6 | 0.003079s | 0.002793s | 0.002625s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43609274616).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
