# libs/accumulators/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| count.test | RUN | 0.698145 | 2.011759 | 2.342804 | 0.003486 | 0.003189 | 0.002837 |
| covariance.test | RUN | 1.387385 | 5.055309 | 6.026518 | 0.003239 | 0.003391 | 0.003361 |
| droppable.test | RUN | 0.890170 | 2.676135 | 3.191257 | 0.003308 | 0.003230 | 0.003252 |
| error_of.test | RUN | 0.979479 | 2.635379 | 3.442469 | 0.002890 | 0.003315 | 0.002968 |
| extended_p_square.test | RUN | 0.998703 | 3.277901 | 3.503518 | 0.004817 | 0.005065 | 0.004157 |
| extended_p_square_quantile.test | RUN | 1.301589 | 5.085178 | 5.009163 | 0.008701 | 0.009270 | 0.007913 |
| external_accumulator.test | RUN | 0.887104 | 2.603425 | 3.064844 | 0.003234 | 0.003252 | 0.003375 |
| external_weights.test | RUN | 0.776029 | 1.987610 | 2.478657 | 0.003584 | 0.002809 | 0.002972 |
| kurtosis.test | RUN | 1.163854 | 3.188559 | 4.104103 | 0.005668 | 0.004935 | 0.004168 |
| max.test | RUN | 0.732783 | 1.946572 | 2.303623 | 0.003511 | 0.003219 | 0.002780 |
| mean.test | RUN | 0.983711 | 2.781444 | 3.328718 | 0.003134 | 0.003306 | 0.003302 |
| median.test | RUN | 1.306931 | 5.294308 | 5.006032 | 0.123457 | 0.132874 | 0.112540 |
| min.test | RUN | 0.732965 | 1.868583 | 2.332941 | 0.003017 | 0.003336 | 0.003154 |
| moment.test | RUN | 0.833219 | 2.226734 | 2.722161 | 0.003670 | 0.003384 | 0.003168 |
| p_square_cumul_dist.test | RUN | 1.080514 | 3.733511 | 3.758596 | 0.996648 | 1.038603 | 0.805234 |
| p_square_quantile.test | RUN | 0.950453 | 3.339657 | 3.248952 | 0.029363 | 0.029272 | 0.024523 |
| reference.test | RUN | 0.806918 | 2.185288 | 2.596779 | 0.003023 | 0.003013 | 0.002825 |
| rolling_count.test | RUN | 0.848146 | 2.440356 | 2.824047 | 0.003331 | 0.003290 | 0.003365 |
| rolling_sum.test | RUN | 0.766486 | 2.497336 | 2.800744 | 0.003362 | 0.003130 | 0.003087 |
| rolling_mean.test | RUN | 1.192930 | 4.447524 | 4.537701 | 0.003024 | 0.003202 | 0.003238 |
| rolling_variance.test | RUN | 1.344107 | 5.168550 | 5.236017 | 0.003170 | 0.003326 | 0.003340 |
| rolling_moment.test | RUN | 1.023018 | 2.999698 | 3.453860 | 0.003348 | 0.003270 | 0.003470 |
| skewness.test | RUN | 1.096067 | 3.135475 | 3.862255 | 0.006362 | 0.004991 | 0.004426 |
| sum.test | RUN | 0.760598 | 2.125843 | 2.491900 | 0.003220 | 0.003515 | 0.003367 |
| sum_kahan.test | RUN | 0.810790 | 2.277227 | 2.638414 | 0.016712 | 0.028095 | 0.017113 |
| tail.test | RUN | 0.889247 | 3.179959 | 3.000020 | 0.003135 | 0.003480 | 0.003378 |
| tail_mean.test | RUN | 1.225792 | 3.625178 | 4.538398 | 0.020788 | 0.027997 | 0.022258 |
| tail_quantile.test | RUN | 1.001427 | 3.028956 | 3.457298 | 0.026030 | 0.030767 | 0.023661 |
| variance.test | RUN | 0.974732 | 2.754555 | 3.317092 | 0.003358 | 0.003415 | 0.003367 |
| vector.test | RUN | 1.059048 | 3.212058 | 4.135363 | 0.003195 | 0.003304 | 0.003270 |
| weighted_covariance.test | RUN | 1.404176 | 5.130521 | 6.150717 | 0.003443 | 0.003137 | 0.003089 |
| weighted_extended_p_square.test | RUN | 0.965078 | 2.952169 | 3.104645 | 0.028232 | 0.025064 | 0.020011 |
| weighted_kurtosis.test | RUN | 1.148588 | 3.018395 | 3.928409 | 0.006063 | 0.005491 | 0.004220 |
| weighted_mean.test | RUN | 0.955606 | 2.732595 | 3.420018 | 0.003038 | 0.003009 | 0.002737 |
| weighted_median.test | RUN | 1.262715 | 3.667726 | 4.323356 | 1.140587 | 1.182124 | 1.102449 |
| weighted_moment.test | RUN | 0.795246 | 2.082476 | 2.483068 | 0.003126 | 0.003287 | 0.003126 |
| weighted_p_square_cumul_dist.test | RUN | 0.956233 | 3.061614 | 3.083885 | 0.144801 | 0.136380 | 0.108957 |
| weighted_p_square_quantile.test | RUN | 0.896937 | 3.015910 | 2.847360 | 0.036820 | 0.030929 | 0.026361 |
| weighted_skewness.test | RUN | 1.050945 | 2.724076 | 3.422343 | 0.006204 | 0.005372 | 0.004112 |
| weighted_sum.test | RUN | 0.661359 | 1.723876 | 2.040439 | 0.002836 | 0.002818 | 0.003129 |
| weighted_sum_kahan.test | RUN | 0.616699 | 1.813641 | 2.137173 | 0.009540 | 0.015019 | 0.009800 |
| weighted_variance.test | RUN | 1.152616 | 3.376639 | 4.262559 | 0.003584 | 0.003549 | 0.003239 |
| valarray.test | RUN | 1.077802 | 3.202177 | 3.906523 | 0.002858 | 0.003084 | 0.003025 |
| tail_variate_means.test | RUN | 1.370725 | 4.943662 | 5.515303 | 0.002907 | 0.003284 | 0.002537 |
| pot_quantile.test | RUN | 1.703217 | 5.515348 | 6.561465 | 0.016288 | 0.012414 | 0.010695 |
| weighted_pot_quantile.test | RUN | 1.689218 | 5.516278 | 6.666362 | 0.029499 | 0.031079 | 0.025810 |
| weighted_tail_mean.test | RUN | 1.473220 | 4.349810 | 5.612465 | 0.022680 | 0.026465 | 0.022488 |
| weighted_tail_quantile.test | RUN | 1.430167 | 3.833516 | 4.869804 | 0.026348 | 0.022524 | 0.019197 |
| weighted_tail_variate_means.test | RUN | 1.672112 | 4.795317 | 5.565131 | 0.002313 | 0.002235 | 0.002142 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.072× | 1.074×, 1.069× | 1.012× |
| clang | 0.888× | 0.890×, 0.886× | 0.999× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1363 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43601267198).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| count.test | 1 | 0.002618s | 0.002590s | 0.002571s | byte equality observed |
| covariance.test | 1 | 0.002607s | 0.002299s | 0.002377s | byte equality observed |
| droppable.test | 1 | 0.002638s | 0.002520s | 0.002387s | byte equality observed |
| error_of.test | 1 | 0.002473s | 0.002216s | 0.002339s | byte equality observed |
| extended_p_square.test | 4 | 0.003689s | 0.003648s | 0.003393s | byte equality observed |
| extended_p_square_quantile.test | 3 | 0.007361s | 0.007889s | 0.006394s | byte equality observed |
| external_accumulator.test | 5 | 0.002176s | 0.002090s | 0.002091s | byte equality observed |
| external_weights.test | 7 | 0.002021s | 0.001957s | 0.001972s | byte equality observed |
| kurtosis.test | 4 | 0.004836s | 0.003748s | 0.003098s | byte equality observed |
| max.test | 5 | 0.002136s | 0.002053s | 0.002056s | byte equality observed |
| mean.test | 6 | 0.002152s | 0.002092s | 0.002047s | byte equality observed |
| median.test | 1 | 0.121313s | 0.132377s | 0.109826s | byte equality observed |
| min.test | 6 | 0.002103s | 0.002039s | 0.002074s | byte equality observed |
| moment.test | 6 | 0.002125s | 0.002072s | 0.002036s | byte equality observed |
| p_square_cumul_dist.test | 1 | 0.857807s | 0.962357s | 0.752331s | byte equality observed |
| p_square_quantile.test | 1 | 0.030264s | 0.028213s | 0.023797s | byte equality observed |
| pot_quantile.test | 2 | 0.015168s | 0.011378s | 0.009730s | byte equality observed |
| reference.test | 7 | 0.002155s | 0.002063s | 0.002064s | byte equality observed |
| rolling_count.test | 8 | 0.002140s | 0.002063s | 0.002056s | byte equality observed |
| rolling_mean.test | 6 | 0.002223s | 0.002156s | 0.002107s | byte equality observed |
| rolling_moment.test | 8 | 0.002240s | 0.002130s | 0.002131s | byte equality observed |
| rolling_sum.test | 7 | 0.002160s | 0.002003s | 0.002054s | byte equality observed |
| rolling_variance.test | 8 | 0.002235s | 0.002144s | 0.002128s | byte equality observed |
| skewness.test | 4 | 0.004819s | 0.003815s | 0.003091s | byte equality observed |
| sum.test | 6 | 0.002103s | 0.002004s | 0.002022s | byte equality observed |
| sum_kahan.test | 1 | 0.015696s | 0.026389s | 0.015602s | byte equality observed |
| tail.test | 9 | 0.002199s | 0.001982s | 0.002038s | byte equality observed |
| tail_mean.test | 1 | 0.020039s | 0.022801s | 0.019209s | byte equality observed |
| tail_quantile.test | 1 | 0.024783s | 0.027388s | 0.022211s | byte equality observed |
| tail_variate_means.test | 8 | 0.002069s | 0.001924s | 0.001946s | byte equality observed |
| valarray.test | 8 | 0.002078s | 0.001981s | 0.001964s | byte equality observed |
| variance.test | 8 | 0.002175s | 0.002067s | 0.002019s | byte equality observed |
| vector.test | 8 | 0.002060s | 0.001953s | 0.001950s | byte equality observed |
| weighted_covariance.test | 9 | 0.002037s | 0.001961s | 0.001935s | byte equality observed |
| weighted_extended_p_square.test | 1 | 0.023622s | 0.023259s | 0.017940s | byte equality observed |
| weighted_kurtosis.test | 4 | 0.005312s | 0.004234s | 0.003090s | byte equality observed |
| weighted_mean.test | 8 | 0.002027s | 0.001938s | 0.001924s | byte equality observed |
| weighted_median.test | 1 | 1.079229s | 1.117836s | 1.005883s | byte equality observed |
| weighted_moment.test | 8 | 0.002022s | 0.001927s | 0.001884s | byte equality observed |
| weighted_p_square_cumul_dist.test | 1 | 0.148618s | 0.176139s | 0.116792s | byte equality observed |
| weighted_p_square_quantile.test | 1 | 0.036110s | 0.033292s | 0.025676s | byte equality observed |
| weighted_pot_quantile.test | 1 | 0.030537s | 0.027149s | 0.023886s | byte equality observed |
| weighted_skewness.test | 4 | 0.005549s | 0.004291s | 0.003122s | byte equality observed |
| weighted_sum.test | 7 | 0.002060s | 0.001913s | 0.001900s | byte equality observed |
| weighted_sum_kahan.test | 2 | 0.008842s | 0.013907s | 0.008650s | byte equality observed |
| weighted_tail_mean.test | 1 | 0.021671s | 0.022688s | 0.021241s | byte equality observed |
| weighted_tail_quantile.test | 1 | 0.025265s | 0.025892s | 0.022407s | byte equality observed |
| weighted_tail_variate_means.test | 5 | 0.002156s | 0.002014s | 0.002049s | byte equality observed |
| weighted_variance.test | 6 | 0.002580s | 0.002301s | 0.002133s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43601267198).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
