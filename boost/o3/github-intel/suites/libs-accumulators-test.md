# libs/accumulators/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| count.test | RUN | 0.704229 | 2.049415 | 2.173265 | 0.002666 | 0.002578 | 0.002163 |
| covariance.test | RUN | 1.395704 | 5.226068 | 5.486367 | 0.002507 | 0.002365 | 0.002266 |
| droppable.test | RUN | 0.846066 | 2.749630 | 2.902963 | 0.002654 | 0.002507 | 0.002185 |
| error_of.test | RUN | 0.938980 | 2.665140 | 3.143116 | 0.002394 | 0.002302 | 0.002058 |
| extended_p_square.test | RUN | 1.019644 | 3.448175 | 3.256168 | 0.005090 | 0.004157 | 0.003598 |
| extended_p_square_quantile.test | RUN | 1.314360 | 5.335096 | 4.652181 | 0.007901 | 0.008297 | 0.006528 |
| external_accumulator.test | RUN | 0.833770 | 2.561760 | 2.797564 | 0.002684 | 0.002346 | 0.002166 |
| external_weights.test | RUN | 0.757133 | 2.001013 | 2.249444 | 0.002324 | 0.002288 | 0.001989 |
| kurtosis.test | RUN | 1.129528 | 3.289042 | 3.770046 | 0.005416 | 0.004274 | 0.003262 |
| max.test | RUN | 0.702179 | 1.959065 | 2.105189 | 0.002335 | 0.002469 | 0.002228 |
| mean.test | RUN | 0.913724 | 2.821522 | 3.072480 | 0.002654 | 0.002491 | 0.002183 |
| median.test | RUN | 1.304155 | 5.492992 | 4.730498 | 0.133213 | 0.132497 | 0.113800 |
| min.test | RUN | 0.704349 | 1.973380 | 2.111931 | 0.002462 | 0.002393 | 0.002222 |
| moment.test | RUN | 0.787399 | 2.316353 | 2.490459 | 0.002600 | 0.002429 | 0.002075 |
| p_square_cumul_dist.test | RUN | 1.046386 | 3.941057 | 3.593448 | 1.002609 | 0.994377 | 0.799607 |
| p_square_quantile.test | RUN | 0.906741 | 3.523201 | 2.978722 | 0.032443 | 0.027895 | 0.024394 |
| reference.test | RUN | 0.795900 | 2.198951 | 2.488107 | 0.002840 | 0.002371 | 0.002221 |
| rolling_count.test | RUN | 0.804010 | 2.433461 | 2.475387 | 0.002340 | 0.002503 | 0.001994 |
| rolling_sum.test | RUN | 0.829919 | 2.540007 | 2.642783 | 0.002614 | 0.002489 | 0.002336 |
| rolling_mean.test | RUN | 1.185111 | 4.558448 | 4.312715 | 0.002646 | 0.002508 | 0.002396 |
| rolling_variance.test | RUN | 1.289354 | 5.231507 | 4.876375 | 0.002598 | 0.002500 | 0.002467 |
| rolling_moment.test | RUN | 0.986620 | 3.087571 | 3.274320 | 0.002672 | 0.002550 | 0.002471 |
| skewness.test | RUN | 1.056544 | 3.121867 | 3.573720 | 0.005406 | 0.004209 | 0.003407 |
| sum.test | RUN | 0.733365 | 2.178235 | 2.387795 | 0.002530 | 0.002606 | 0.002334 |
| sum_kahan.test | RUN | 0.781455 | 2.297825 | 2.459444 | 0.012195 | 0.020233 | 0.011929 |
| tail.test | RUN | 0.860229 | 3.190249 | 2.811320 | 0.002575 | 0.002437 | 0.002416 |
| tail_mean.test | RUN | 1.193223 | 3.695253 | 4.106058 | 0.022265 | 0.022805 | 0.021991 |
| tail_quantile.test | RUN | 0.975081 | 3.026974 | 3.177375 | 0.025517 | 0.027722 | 0.023503 |
| variance.test | RUN | 0.922112 | 2.721159 | 3.122827 | 0.002491 | 0.002414 | 0.002408 |
| vector.test | RUN | 1.011280 | 3.328544 | 3.806452 | 0.002894 | 0.002363 | 0.002172 |
| weighted_covariance.test | RUN | 1.366458 | 5.088143 | 5.678847 | 0.002299 | 0.002407 | 0.002094 |
| weighted_extended_p_square.test | RUN | 0.935250 | 3.036622 | 2.932259 | 0.024506 | 0.024495 | 0.017603 |
| weighted_kurtosis.test | RUN | 1.105884 | 3.077584 | 3.602668 | 0.005710 | 0.004811 | 0.003285 |
| weighted_mean.test | RUN | 0.914880 | 2.716649 | 3.090585 | 0.002538 | 0.002507 | 0.002031 |
| weighted_median.test | RUN | 1.195620 | 3.741565 | 4.035665 | 1.213056 | 1.193586 | 1.141640 |
| weighted_moment.test | RUN | 0.745529 | 2.018508 | 2.324393 | 0.002460 | 0.002426 | 0.002150 |
| weighted_p_square_cumul_dist.test | RUN | 0.916470 | 3.140494 | 2.891472 | 0.137526 | 0.155535 | 0.110681 |
| weighted_p_square_quantile.test | RUN | 0.855291 | 2.956979 | 2.420433 | 0.035232 | 0.030704 | 0.023217 |
| weighted_skewness.test | RUN | 0.930528 | 2.923716 | 3.405131 | 0.005353 | 0.004932 | 0.003128 |
| weighted_sum.test | RUN | 0.621111 | 1.705609 | 1.882518 | 0.002742 | 0.002352 | 0.002243 |
| weighted_sum_kahan.test | RUN | 0.669285 | 1.810245 | 2.005658 | 0.006649 | 0.011367 | 0.006847 |
| weighted_variance.test | RUN | 1.126390 | 3.328671 | 3.917372 | 0.003062 | 0.002851 | 0.002370 |
| valarray.test | RUN | 1.054080 | 3.207094 | 3.670546 | 0.002537 | 0.002434 | 0.002072 |
| tail_variate_means.test | RUN | 1.364536 | 5.082207 | 5.098595 | 0.002552 | 0.002446 | 0.002223 |
| pot_quantile.test | RUN | 1.697473 | 5.597268 | 6.070164 | 0.016132 | 0.011997 | 0.010228 |
| weighted_pot_quantile.test | RUN | 1.690118 | 5.533098 | 6.020458 | 0.024195 | 0.029438 | 0.023450 |
| weighted_tail_mean.test | RUN | 1.484304 | 4.351687 | 5.102961 | 0.024142 | 0.024088 | 0.021359 |
| weighted_tail_quantile.test | RUN | 1.503317 | 3.676845 | 4.407608 | 0.021311 | 0.020092 | 0.016876 |
| weighted_tail_variate_means.test | RUN | 1.618915 | 4.736912 | 5.286145 | 0.001774 | 0.001714 | 0.001636 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.014× | 1.022×, 1.006× | 0.980× |
| clang | 0.876× | 0.873×, 0.879× | 0.987× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43395345325).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| count.test | 1 | 0.002174s | 0.001974s | 0.002005s | byte equality observed |
| covariance.test | 1 | 0.002198s | 0.002126s | 0.002057s | byte equality observed |
| droppable.test | 1 | 0.002262s | 0.002129s | 0.002114s | byte equality observed |
| error_of.test | 1 | 0.002179s | 0.002197s | 0.002148s | byte equality observed |
| extended_p_square.test | 4 | 0.003384s | 0.003367s | 0.003132s | byte equality observed |
| extended_p_square_quantile.test | 3 | 0.006943s | 0.007052s | 0.006399s | byte equality observed |
| external_accumulator.test | 6 | 0.001863s | 0.001776s | 0.001758s | byte equality observed |
| external_weights.test | 6 | 0.001758s | 0.001681s | 0.001651s | byte equality observed |
| kurtosis.test | 5 | 0.004440s | 0.003447s | 0.002610s | byte equality observed |
| max.test | 9 | 0.001791s | 0.001731s | 0.001742s | byte equality observed |
| mean.test | 8 | 0.001824s | 0.001781s | 0.001739s | byte equality observed |
| median.test | 1 | 0.125173s | 0.125973s | 0.109695s | byte equality observed |
| min.test | 7 | 0.001815s | 0.001774s | 0.001722s | byte equality observed |
| moment.test | 7 | 0.001829s | 0.001756s | 0.001763s | byte equality observed |
| p_square_cumul_dist.test | 1 | 0.814056s | 0.883444s | 0.694736s | byte equality observed |
| p_square_quantile.test | 1 | 0.030087s | 0.027686s | 0.023863s | byte equality observed |
| pot_quantile.test | 2 | 0.015034s | 0.010652s | 0.009345s | byte equality observed |
| reference.test | 10 | 0.001811s | 0.001746s | 0.001753s | byte equality observed |
| rolling_count.test | 10 | 0.001838s | 0.001759s | 0.001769s | byte equality observed |
| rolling_mean.test | 9 | 0.001937s | 0.001883s | 0.001855s | byte equality observed |
| rolling_moment.test | 10 | 0.001891s | 0.001821s | 0.001812s | byte equality observed |
| rolling_sum.test | 10 | 0.001818s | 0.001761s | 0.001765s | byte equality observed |
| rolling_variance.test | 7 | 0.001907s | 0.001805s | 0.001821s | byte equality observed |
| skewness.test | 5 | 0.004329s | 0.003422s | 0.002687s | byte equality observed |
| sum.test | 9 | 0.001756s | 0.001716s | 0.001701s | byte equality observed |
| sum_kahan.test | 2 | 0.011098s | 0.018908s | 0.011195s | byte equality observed |
| tail.test | 8 | 0.001748s | 0.001696s | 0.001631s | byte equality observed |
| tail_mean.test | 1 | 0.020381s | 0.019712s | 0.021448s | byte equality observed |
| tail_quantile.test | 1 | 0.024445s | 0.025346s | 0.023237s | byte equality observed |
| tail_variate_means.test | 8 | 0.001807s | 0.001697s | 0.001730s | byte equality observed |
| valarray.test | 10 | 0.001759s | 0.001733s | 0.001700s | byte equality observed |
| variance.test | 9 | 0.001810s | 0.001724s | 0.001745s | byte equality observed |
| vector.test | 10 | 0.001717s | 0.001659s | 0.001666s | byte equality observed |
| weighted_covariance.test | 9 | 0.001739s | 0.001670s | 0.001681s | byte equality observed |
| weighted_extended_p_square.test | 1 | 0.023669s | 0.022702s | 0.018053s | byte equality observed |
| weighted_kurtosis.test | 4 | 0.004914s | 0.004000s | 0.002748s | byte equality observed |
| weighted_mean.test | 10 | 0.001703s | 0.001650s | 0.001626s | byte equality observed |
| weighted_median.test | 1 | 1.051256s | 1.043466s | 0.968591s | byte equality observed |
| weighted_moment.test | 10 | 0.001695s | 0.001647s | 0.001575s | byte equality observed |
| weighted_p_square_cumul_dist.test | 1 | 0.133435s | 0.131538s | 0.109243s | byte equality observed |
| weighted_p_square_quantile.test | 1 | 0.034807s | 0.030523s | 0.025268s | byte equality observed |
| weighted_pot_quantile.test | 1 | 0.029620s | 0.025848s | 0.023749s | byte equality observed |
| weighted_skewness.test | 4 | 0.004960s | 0.003979s | 0.002752s | byte equality observed |
| weighted_sum.test | 10 | 0.001710s | 0.001633s | 0.001633s | byte equality observed |
| weighted_sum_kahan.test | 2 | 0.006444s | 0.010152s | 0.006231s | byte equality observed |
| weighted_tail_mean.test | 1 | 0.022309s | 0.022985s | 0.020983s | byte equality observed |
| weighted_tail_quantile.test | 1 | 0.025267s | 0.025934s | 0.022014s | byte equality observed |
| weighted_tail_variate_means.test | 9 | 0.001775s | 0.001662s | 0.001671s | byte equality observed |
| weighted_variance.test | 8 | 0.002223s | 0.001975s | 0.001824s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43395345325).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
