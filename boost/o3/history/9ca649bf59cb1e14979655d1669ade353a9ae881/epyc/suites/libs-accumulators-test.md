# libs/accumulators/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| count.test | RUN | 0.788615 | 1.990795 | 2.373364 | 0.003562 | 0.003321 | 0.002953 |
| covariance.test | RUN | 1.513105 | 5.055849 | 6.003352 | 0.003516 | 0.003462 | 0.003431 |
| droppable.test | RUN | 0.975226 | 2.658195 | 3.212432 | 0.003333 | 0.003535 | 0.003480 |
| error_of.test | RUN | 1.059633 | 2.664961 | 3.512616 | 0.003207 | 0.003282 | 0.003020 |
| extended_p_square.test | RUN | 1.171489 | 3.292802 | 3.517983 | 0.004919 | 0.005064 | 0.004617 |
| extended_p_square_quantile.test | RUN | 1.404100 | 5.137804 | 5.034441 | 0.008783 | 0.009408 | 0.007862 |
| external_accumulator.test | RUN | 0.959984 | 2.644064 | 3.121628 | 0.003497 | 0.003243 | 0.003349 |
| external_weights.test | RUN | 0.829007 | 1.931127 | 2.465904 | 0.003110 | 0.003191 | 0.003168 |
| kurtosis.test | RUN | 1.244898 | 3.261444 | 4.103159 | 0.005395 | 0.004730 | 0.004454 |
| max.test | RUN | 0.808878 | 2.035602 | 2.286932 | 0.003579 | 0.003112 | 0.003202 |
| mean.test | RUN | 1.051616 | 2.696469 | 3.368514 | 0.003328 | 0.003467 | 0.003037 |
| median.test | RUN | 1.430375 | 5.410256 | 5.043769 | 0.114943 | 0.129435 | 0.113637 |
| min.test | RUN | 0.822970 | 1.936771 | 2.257593 | 0.003169 | 0.003467 | 0.003272 |
| moment.test | RUN | 0.926293 | 2.233847 | 2.712658 | 0.003498 | 0.003276 | 0.003292 |
| p_square_cumul_dist.test | RUN | 1.193842 | 3.849916 | 3.838360 | 0.884781 | 1.044597 | 0.817011 |
| p_square_quantile.test | RUN | 1.073594 | 3.331402 | 3.234158 | 0.029474 | 0.029346 | 0.024314 |
| reference.test | RUN | 0.922067 | 2.206959 | 2.820601 | 0.003209 | 0.003034 | 0.003485 |
| rolling_count.test | RUN | 0.951247 | 2.456806 | 2.804805 | 0.003087 | 0.003210 | 0.003243 |
| rolling_sum.test | RUN | 0.922011 | 2.488205 | 2.654874 | 0.003602 | 0.003178 | 0.003221 |
| rolling_mean.test | RUN | 1.297542 | 4.486147 | 4.605833 | 0.003306 | 0.003666 | 0.003446 |
| rolling_variance.test | RUN | 1.448102 | 5.203379 | 5.294651 | 0.003229 | 0.003366 | 0.003245 |
| rolling_moment.test | RUN | 1.124442 | 3.034704 | 3.466208 | 0.003575 | 0.003651 | 0.003666 |
| skewness.test | RUN | 1.227143 | 3.090526 | 3.887220 | 0.005278 | 0.005011 | 0.003962 |
| sum.test | RUN | 0.840327 | 2.142946 | 2.451117 | 0.003073 | 0.003608 | 0.003221 |
| sum_kahan.test | RUN | 0.908008 | 2.274774 | 2.711211 | 0.017013 | 0.027796 | 0.017011 |
| tail.test | RUN | 0.959794 | 3.160186 | 3.065181 | 0.003090 | 0.003589 | 0.003249 |
| tail_mean.test | RUN | 1.337518 | 3.687725 | 4.482024 | 0.022116 | 0.026566 | 0.022167 |
| tail_quantile.test | RUN | 1.115960 | 3.001260 | 3.418337 | 0.024564 | 0.030696 | 0.023908 |
| variance.test | RUN | 1.078391 | 2.744580 | 3.355788 | 0.003573 | 0.003375 | 0.003445 |
| vector.test | RUN | 1.109944 | 3.289157 | 4.117277 | 0.003165 | 0.003313 | 0.003116 |
| weighted_covariance.test | RUN | 1.515038 | 5.195020 | 6.202280 | 0.002761 | 0.003140 | 0.002944 |
| weighted_extended_p_square.test | RUN | 1.063871 | 2.981445 | 3.148215 | 0.024284 | 0.025309 | 0.019802 |
| weighted_kurtosis.test | RUN | 1.237926 | 3.026898 | 3.915960 | 0.005763 | 0.005496 | 0.004105 |
| weighted_mean.test | RUN | 1.043656 | 2.710184 | 3.414123 | 0.003357 | 0.003139 | 0.003098 |
| weighted_median.test | RUN | 1.304783 | 3.602512 | 4.472409 | 1.215621 | 1.184099 | 1.099035 |
| weighted_moment.test | RUN | 0.859600 | 2.040760 | 2.592480 | 0.003301 | 0.003407 | 0.003062 |
| weighted_p_square_cumul_dist.test | RUN | 1.066370 | 3.054872 | 3.157121 | 0.140835 | 0.137880 | 0.109901 |
| weighted_p_square_quantile.test | RUN | 0.988086 | 2.992854 | 2.595400 | 0.034044 | 0.035431 | 0.026326 |
| weighted_skewness.test | RUN | 1.186077 | 2.735317 | 3.811083 | 0.005412 | 0.005387 | 0.004202 |
| weighted_sum.test | RUN | 0.746081 | 1.747334 | 2.014312 | 0.003087 | 0.003113 | 0.003338 |
| weighted_sum_kahan.test | RUN | 0.710512 | 1.846513 | 2.148091 | 0.009688 | 0.015175 | 0.009831 |
| weighted_variance.test | RUN | 1.264960 | 3.378655 | 4.303861 | 0.003605 | 0.003578 | 0.003491 |
| valarray.test | RUN | 1.142958 | 3.218376 | 4.016056 | 0.003145 | 0.003145 | 0.003078 |
| tail_variate_means.test | RUN | 1.493675 | 4.967739 | 5.580584 | 0.002795 | 0.003130 | 0.002838 |
| pot_quantile.test | RUN | 1.850640 | 5.540370 | 6.669129 | 0.013416 | 0.012980 | 0.010838 |
| weighted_pot_quantile.test | RUN | 1.911913 | 5.310538 | 6.750352 | 0.026490 | 0.022983 | 0.025278 |
| weighted_tail_mean.test | RUN | 1.663406 | 4.366362 | 5.690025 | 0.022830 | 0.027650 | 0.022372 |
| weighted_tail_quantile.test | RUN | 1.705663 | 3.846897 | 5.004574 | 0.024650 | 0.022713 | 0.019720 |
| weighted_tail_variate_means.test | RUN | 1.865874 | 4.836727 | 5.809789 | 0.002388 | 0.002299 | 0.002231 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.129× | 1.117×, 1.141× | 0.999× |
| clang | 0.934× | 0.930×, 0.938× | 0.996× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1361 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| count.test | 1 | 0.002679s | 0.002474s | 0.002659s | byte equality observed |
| covariance.test | 1 | 0.002690s | 0.002610s | 0.002662s | byte equality observed |
| droppable.test | 1 | 0.002673s | 0.002346s | 0.002506s | byte equality observed |
| error_of.test | 4 | 0.002176s | 0.002090s | 0.002129s | byte equality observed |
| extended_p_square.test | 5 | 0.003708s | 0.003732s | 0.003389s | byte equality observed |
| extended_p_square_quantile.test | 3 | 0.007297s | 0.007888s | 0.006615s | byte equality observed |
| external_accumulator.test | 8 | 0.002193s | 0.002115s | 0.002071s | byte equality observed |
| external_weights.test | 5 | 0.002073s | 0.001992s | 0.002017s | byte equality observed |
| kurtosis.test | 4 | 0.004117s | 0.003818s | 0.002956s | byte equality observed |
| max.test | 6 | 0.002179s | 0.002118s | 0.002145s | byte equality observed |
| mean.test | 6 | 0.002181s | 0.002125s | 0.002144s | byte equality observed |
| median.test | 1 | 0.112547s | 0.141923s | 0.108348s | byte equality observed |
| min.test | 6 | 0.002172s | 0.002083s | 0.002107s | byte equality observed |
| moment.test | 6 | 0.002191s | 0.002130s | 0.002090s | byte equality observed |
| p_square_cumul_dist.test | 1 | 0.792601s | 0.964021s | 0.749010s | byte equality observed |
| p_square_quantile.test | 1 | 0.029471s | 0.026864s | 0.024564s | byte equality observed |
| pot_quantile.test | 2 | 0.012226s | 0.011559s | 0.009743s | byte equality observed |
| reference.test | 8 | 0.002239s | 0.002165s | 0.002112s | byte equality observed |
| rolling_count.test | 8 | 0.002213s | 0.002113s | 0.002089s | byte equality observed |
| rolling_mean.test | 8 | 0.002317s | 0.002231s | 0.002192s | byte equality observed |
| rolling_moment.test | 8 | 0.002279s | 0.002213s | 0.002197s | byte equality observed |
| rolling_sum.test | 8 | 0.002225s | 0.002128s | 0.002096s | byte equality observed |
| rolling_variance.test | 7 | 0.002315s | 0.002243s | 0.002220s | byte equality observed |
| skewness.test | 5 | 0.004098s | 0.003868s | 0.002971s | byte equality observed |
| sum.test | 9 | 0.002186s | 0.002104s | 0.002079s | byte equality observed |
| sum_kahan.test | 1 | 0.015913s | 0.026370s | 0.015611s | byte equality observed |
| tail.test | 7 | 0.002203s | 0.002081s | 0.002064s | byte equality observed |
| tail_mean.test | 1 | 0.021307s | 0.023100s | 0.020904s | byte equality observed |
| tail_quantile.test | 1 | 0.023827s | 0.027414s | 0.022883s | byte equality observed |
| tail_variate_means.test | 8 | 0.002115s | 0.002025s | 0.002051s | byte equality observed |
| valarray.test | 8 | 0.002103s | 0.002015s | 0.002008s | byte equality observed |
| variance.test | 8 | 0.002203s | 0.002090s | 0.002062s | byte equality observed |
| vector.test | 8 | 0.002110s | 0.002033s | 0.002039s | byte equality observed |
| weighted_covariance.test | 9 | 0.002095s | 0.001973s | 0.001983s | byte equality observed |
| weighted_extended_p_square.test | 1 | 0.022388s | 0.025274s | 0.017468s | byte equality observed |
| weighted_kurtosis.test | 4 | 0.004673s | 0.004283s | 0.003074s | byte equality observed |
| weighted_mean.test | 8 | 0.002077s | 0.001987s | 0.001954s | byte equality observed |
| weighted_median.test | 1 | 1.076405s | 1.120413s | 1.013577s | byte equality observed |
| weighted_moment.test | 8 | 0.002058s | 0.001998s | 0.001933s | byte equality observed |
| weighted_p_square_cumul_dist.test | 1 | 0.135867s | 0.177783s | 0.119842s | byte equality observed |
| weighted_p_square_quantile.test | 1 | 0.034422s | 0.033369s | 0.026163s | byte equality observed |
| weighted_pot_quantile.test | 1 | 0.026977s | 0.027441s | 0.024146s | byte equality observed |
| weighted_skewness.test | 4 | 0.004805s | 0.004511s | 0.003230s | byte equality observed |
| weighted_sum.test | 8 | 0.002105s | 0.001990s | 0.001951s | byte equality observed |
| weighted_sum_kahan.test | 2 | 0.008859s | 0.014012s | 0.008716s | byte equality observed |
| weighted_tail_mean.test | 1 | 0.021580s | 0.022984s | 0.020913s | byte equality observed |
| weighted_tail_quantile.test | 1 | 0.023980s | 0.025888s | 0.021569s | byte equality observed |
| weighted_tail_variate_means.test | 8 | 0.002205s | 0.001997s | 0.002049s | byte equality observed |
| weighted_variance.test | 8 | 0.002477s | 0.002341s | 0.002183s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
