# libs/accumulators/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| count.test | RUN | 0.555601 | 1.522082 | 1.820930 | 0.003282 | 0.003148 | 0.003358 |
| covariance.test | RUN | 1.176028 | 3.947608 | 4.626945 | 0.003194 | 0.003055 | 0.003175 |
| droppable.test | RUN | 0.718516 | 2.058224 | 2.491292 | 0.003130 | 0.003083 | 0.003152 |
| error_of.test | RUN | 0.796113 | 2.018162 | 2.604207 | 0.003000 | 0.002624 | 0.002965 |
| extended_p_square.test | RUN | 0.849345 | 2.571016 | 2.740596 | 0.004722 | 0.004259 | 0.004022 |
| extended_p_square_quantile.test | RUN | 1.137797 | 3.998195 | 3.923942 | 0.007920 | 0.007182 | 0.006853 |
| external_accumulator.test | RUN | 0.764623 | 2.020026 | 2.405195 | 0.003136 | 0.002753 | 0.003086 |
| external_weights.test | RUN | 0.711265 | 1.487531 | 1.859932 | 0.003058 | 0.002894 | 0.002950 |
| kurtosis.test | RUN | 1.017662 | 2.500856 | 3.086845 | 0.005572 | 0.004302 | 0.003647 |
| max.test | RUN | 0.611894 | 1.562477 | 1.771010 | 0.003088 | 0.003089 | 0.003082 |
| mean.test | RUN | 0.787024 | 2.071608 | 2.551488 | 0.003092 | 0.003099 | 0.003233 |
| median.test | RUN | 1.110062 | 4.166774 | 3.968588 | 0.105907 | 0.095943 | 0.090006 |
| min.test | RUN | 0.598747 | 1.509247 | 1.700876 | 0.002983 | 0.003018 | 0.003087 |
| moment.test | RUN | 0.680032 | 1.747288 | 2.045752 | 0.003222 | 0.002943 | 0.003019 |
| p_square_cumul_dist.test | RUN | 0.915174 | 2.901305 | 2.922011 | 0.774555 | 0.728477 | 0.642038 |
| p_square_quantile.test | RUN | 0.806299 | 2.580562 | 2.511377 | 0.025779 | 0.021201 | 0.019582 |
| reference.test | RUN | 0.680647 | 1.620439 | 2.022994 | 0.003293 | 0.002729 | 0.003194 |
| rolling_count.test | RUN | 0.710678 | 1.785689 | 2.166301 | 0.002968 | 0.002701 | 0.003153 |
| rolling_sum.test | RUN | 0.688200 | 1.798464 | 2.133063 | 0.003280 | 0.002640 | 0.003226 |
| rolling_mean.test | RUN | 1.000484 | 3.387313 | 3.637136 | 0.003042 | 0.003052 | 0.003416 |
| rolling_variance.test | RUN | 1.133397 | 3.963397 | 4.119173 | 0.003569 | 0.003065 | 0.003368 |
| rolling_moment.test | RUN | 0.873886 | 2.296144 | 2.649381 | 0.003385 | 0.002980 | 0.003358 |
| skewness.test | RUN | 0.933655 | 2.359307 | 3.018180 | 0.005694 | 0.004213 | 0.004128 |
| sum.test | RUN | 0.628502 | 1.619102 | 2.000587 | 0.003038 | 0.003036 | 0.003086 |
| sum_kahan.test | RUN | 0.684945 | 1.770366 | 2.117762 | 0.012638 | 0.020917 | 0.011184 |
| tail.test | RUN | 0.741427 | 2.378527 | 2.392433 | 0.003025 | 0.002823 | 0.003251 |
| tail_mean.test | RUN | 1.045711 | 2.802002 | 3.534369 | 0.020354 | 0.022268 | 0.019148 |
| tail_quantile.test | RUN | 0.854001 | 2.354337 | 2.719220 | 0.023630 | 0.025863 | 0.021697 |
| variance.test | RUN | 0.809160 | 2.084099 | 2.594989 | 0.002860 | 0.002689 | 0.003202 |
| vector.test | RUN | 0.876868 | 2.482574 | 3.227642 | 0.003049 | 0.002896 | 0.002863 |
| weighted_covariance.test | RUN | 1.149605 | 3.937587 | 4.628072 | 0.002611 | 0.002651 | 0.002745 |
| weighted_extended_p_square.test | RUN | 0.781499 | 2.265344 | 2.338761 | 0.022245 | 0.018849 | 0.016350 |
| weighted_kurtosis.test | RUN | 0.919464 | 2.297581 | 2.898118 | 0.006015 | 0.004408 | 0.003708 |
| weighted_mean.test | RUN | 0.783483 | 2.093343 | 2.548572 | 0.002835 | 0.002888 | 0.002873 |
| weighted_median.test | RUN | 1.029833 | 2.848743 | 3.302061 | 0.994016 | 0.952574 | 0.915341 |
| weighted_moment.test | RUN | 0.639354 | 1.577011 | 1.837048 | 0.002790 | 0.002960 | 0.003123 |
| weighted_p_square_cumul_dist.test | RUN | 0.804011 | 2.329040 | 2.444598 | 0.111116 | 0.128730 | 0.087903 |
| weighted_p_square_quantile.test | RUN | 0.737543 | 2.274396 | 2.186046 | 0.032165 | 0.024800 | 0.020543 |
| weighted_skewness.test | RUN | 0.876663 | 2.259592 | 2.729131 | 0.005658 | 0.004573 | 0.003866 |
| weighted_sum.test | RUN | 0.500982 | 1.380173 | 1.580767 | 0.002682 | 0.002902 | 0.003054 |
| weighted_sum_kahan.test | RUN | 0.578814 | 1.430595 | 1.627312 | 0.007125 | 0.011694 | 0.007082 |
| weighted_variance.test | RUN | 0.957396 | 2.555157 | 3.444794 | 0.003467 | 0.003062 | 0.003342 |
| valarray.test | RUN | 0.907409 | 2.401700 | 3.179694 | 0.003059 | 0.002914 | 0.002717 |
| tail_variate_means.test | RUN | 1.159510 | 4.027629 | 4.452454 | 0.003283 | 0.002854 | 0.003083 |
| pot_quantile.test | RUN | 1.422892 | 4.407166 | 5.110743 | 0.015272 | 0.010734 | 0.009479 |
| weighted_pot_quantile.test | RUN | 1.438798 | 4.324583 | 5.104289 | 0.028729 | 0.025493 | 0.022259 |
| weighted_tail_mean.test | RUN | 1.254951 | 3.494429 | 4.318827 | 0.023690 | 0.022568 | 0.019798 |
| weighted_tail_quantile.test | RUN | 1.268665 | 2.955305 | 3.908383 | 0.020971 | 0.018020 | 0.016174 |
| weighted_tail_variate_means.test | RUN | 1.450375 | 3.878253 | 4.607720 | 0.002377 | 0.001939 | 0.002235 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.023× | 1.008×, 1.038× | 0.992× |
| clang | 0.911× | 0.926×, 0.895× | 1.002× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43558051632).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| count.test | 1 | 0.002389s | 0.002312s | 0.002234s | byte equality observed |
| covariance.test | 1 | 0.002416s | 0.002350s | 0.002295s | byte equality observed |
| droppable.test | 1 | 0.002420s | 0.002276s | 0.002279s | byte equality observed |
| error_of.test | 5 | 0.001907s | 0.001807s | 0.001790s | byte equality observed |
| extended_p_square.test | 4 | 0.003082s | 0.003143s | 0.002855s | byte equality observed |
| extended_p_square_quantile.test | 3 | 0.005476s | 0.006121s | 0.005067s | byte equality observed |
| external_accumulator.test | 4 | 0.001924s | 0.001878s | 0.001807s | byte equality observed |
| external_weights.test | 4 | 0.001773s | 0.001774s | 0.001691s | byte equality observed |
| kurtosis.test | 5 | 0.004049s | 0.003299s | 0.002484s | byte equality observed |
| max.test | 8 | 0.001929s | 0.001808s | 0.001793s | byte equality observed |
| mean.test | 5 | 0.001951s | 0.001809s | 0.001822s | byte equality observed |
| median.test | 1 | 0.096779s | 0.095426s | 0.086894s | byte equality observed |
| min.test | 7 | 0.001868s | 0.001835s | 0.001852s | byte equality observed |
| moment.test | 7 | 0.001925s | 0.001812s | 0.001847s | byte equality observed |
| p_square_cumul_dist.test | 1 | 0.648674s | 0.701738s | 0.595652s | byte equality observed |
| p_square_quantile.test | 1 | 0.023202s | 0.020679s | 0.019734s | byte equality observed |
| pot_quantile.test | 2 | 0.013242s | 0.009412s | 0.007945s | byte equality observed |
| reference.test | 5 | 0.001938s | 0.001831s | 0.001866s | byte equality observed |
| rolling_count.test | 9 | 0.001888s | 0.001790s | 0.001862s | byte equality observed |
| rolling_mean.test | 7 | 0.001988s | 0.001895s | 0.001922s | byte equality observed |
| rolling_moment.test | 8 | 0.001945s | 0.001921s | 0.001928s | byte equality observed |
| rolling_sum.test | 7 | 0.001912s | 0.001819s | 0.001808s | byte equality observed |
| rolling_variance.test | 6 | 0.002046s | 0.001946s | 0.001874s | byte equality observed |
| skewness.test | 5 | 0.004047s | 0.003280s | 0.002475s | byte equality observed |
| sum.test | 8 | 0.001923s | 0.001824s | 0.001782s | byte equality observed |
| sum_kahan.test | 1 | 0.011116s | 0.019643s | 0.009727s | byte equality observed |
| tail.test | 8 | 0.001862s | 0.001794s | 0.001793s | byte equality observed |
| tail_mean.test | 1 | 0.017669s | 0.018320s | 0.017639s | byte equality observed |
| tail_quantile.test | 1 | 0.021465s | 0.021251s | 0.018691s | byte equality observed |
| tail_variate_means.test | 9 | 0.001814s | 0.001745s | 0.001744s | byte equality observed |
| valarray.test | 8 | 0.001818s | 0.001713s | 0.001704s | byte equality observed |
| variance.test | 6 | 0.001949s | 0.001815s | 0.001842s | byte equality observed |
| vector.test | 9 | 0.001840s | 0.001731s | 0.001734s | byte equality observed |
| weighted_covariance.test | 9 | 0.001823s | 0.001723s | 0.001712s | byte equality observed |
| weighted_extended_p_square.test | 1 | 0.019364s | 0.018052s | 0.014746s | byte equality observed |
| weighted_kurtosis.test | 4 | 0.004587s | 0.003717s | 0.002501s | byte equality observed |
| weighted_mean.test | 9 | 0.001816s | 0.001727s | 0.001690s | byte equality observed |
| weighted_median.test | 1 | 0.873858s | 0.866299s | 0.809876s | byte equality observed |
| weighted_moment.test | 7 | 0.001833s | 0.001742s | 0.001687s | byte equality observed |
| weighted_p_square_cumul_dist.test | 1 | 0.105538s | 0.127949s | 0.090562s | byte equality observed |
| weighted_p_square_quantile.test | 1 | 0.028610s | 0.025225s | 0.019346s | byte equality observed |
| weighted_pot_quantile.test | 1 | 0.025587s | 0.022463s | 0.019470s | byte equality observed |
| weighted_skewness.test | 4 | 0.004945s | 0.004036s | 0.002516s | byte equality observed |
| weighted_sum.test | 8 | 0.001892s | 0.001805s | 0.001682s | byte equality observed |
| weighted_sum_kahan.test | 2 | 0.006547s | 0.010801s | 0.005684s | byte equality observed |
| weighted_tail_mean.test | 1 | 0.019840s | 0.018894s | 0.017656s | byte equality observed |
| weighted_tail_quantile.test | 1 | 0.021413s | 0.020660s | 0.018891s | byte equality observed |
| weighted_tail_variate_means.test | 8 | 0.001923s | 0.001822s | 0.001756s | byte equality observed |
| weighted_variance.test | 7 | 0.002286s | 0.002066s | 0.001916s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43558051632).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
