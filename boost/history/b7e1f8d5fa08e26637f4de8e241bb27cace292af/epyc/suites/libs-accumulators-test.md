# libs/accumulators/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| count.test | RUN | 0.765143 | 1.999802 | 2.224111 | 0.004343 | 0.004555 | 0.004504 |
| covariance.test | RUN | 1.392431 | 5.904877 | 5.539323 | 0.004316 | 0.004859 | 0.004664 |
| droppable.test | RUN | 0.903726 | 3.191673 | 3.122968 | 0.004123 | 0.004356 | 0.004454 |
| error_of.test | RUN | 1.043127 | 3.943559 | 3.496696 | 0.003975 | 0.004125 | 0.004305 |
| extended_p_square.test | RUN | 1.029516 | 2.850710 | 3.210049 | 0.024826 | 0.027396 | 0.031992 |
| extended_p_square_quantile.test | RUN | 1.286056 | 4.838427 | 4.400851 | 0.160676 | 0.175623 | 0.190159 |
| external_accumulator.test | RUN | 0.911187 | 3.449602 | 3.039837 | 0.004338 | 0.004538 | 0.004507 |
| external_weights.test | RUN | 0.843846 | 2.241662 | 2.492578 | 0.003670 | 0.004064 | 0.003647 |
| kurtosis.test | RUN | 1.195355 | 4.329292 | 3.968014 | 0.048504 | 0.056861 | 0.063666 |
| max.test | RUN | 0.780018 | 2.002366 | 2.204135 | 0.004025 | 0.004450 | 0.004177 |
| mean.test | RUN | 1.002138 | 3.528382 | 3.264539 | 0.004232 | 0.004466 | 0.004643 |
| median.test | RUN | 1.277771 | 4.863402 | 4.376920 | 1.570464 | 1.817172 | 2.157824 |
| min.test | RUN | 0.773788 | 1.948041 | 2.161636 | 0.003975 | 0.004376 | 0.004555 |
| moment.test | RUN | 0.881768 | 2.436937 | 2.579818 | 0.004141 | 0.004340 | 0.004265 |
| p_square_cumul_dist.test | RUN | 1.087908 | 3.170936 | 2.975076 | 14.692322 | 14.994185 | 17.857325 |
| p_square_quantile.test | RUN | 0.988125 | 2.740834 | 2.848501 | 0.438877 | 0.456878 | 0.466460 |
| reference.test | RUN | 0.887608 | 2.760731 | 2.669441 | 0.004174 | 0.004487 | 0.004279 |
| rolling_count.test | RUN | 0.837073 | 2.378648 | 2.185265 | 0.004157 | 0.004180 | 0.004243 |
| rolling_sum.test | RUN | 0.859122 | 2.388767 | 2.596206 | 0.004226 | 0.004422 | 0.004432 |
| rolling_mean.test | RUN | 1.148199 | 4.565801 | 4.084301 | 0.004586 | 0.005226 | 0.005383 |
| rolling_variance.test | RUN | 1.258661 | 5.486990 | 3.999655 | 0.004208 | 0.005151 | 0.004865 |
| rolling_moment.test | RUN | 1.021218 | 3.138446 | 3.257923 | 0.004465 | 0.004426 | 0.004600 |
| skewness.test | RUN | 1.112072 | 3.906951 | 3.724105 | 0.047022 | 0.050371 | 0.051314 |
| sum.test | RUN | 0.808837 | 2.204221 | 2.053261 | 0.004213 | 0.004480 | 0.004282 |
| sum_kahan.test | RUN | 0.856569 | 2.327074 | 2.485947 | 0.436275 | 0.526487 | 0.532088 |
| tail.test | RUN | 0.849384 | 2.776027 | 2.863306 | 0.004209 | 0.004600 | 0.004624 |
| tail_mean.test | RUN | 1.153456 | 4.688353 | 4.329412 | 0.223194 | 0.258807 | 0.301767 |
| tail_quantile.test | RUN | 1.050162 | 2.979891 | 2.736675 | 0.203013 | 0.238305 | 0.272713 |
| variance.test | RUN | 1.021296 | 3.542296 | 3.275094 | 0.004013 | 0.004194 | 0.004591 |
| vector.test | RUN | 0.948985 | 3.932416 | 3.141586 | 0.003865 | 0.004596 | 0.004449 |
| weighted_covariance.test | RUN | 1.387906 | 6.384196 | 5.909006 | 0.003980 | 0.004471 | 0.003955 |
| weighted_extended_p_square.test | RUN | 0.984929 | 2.618034 | 3.027615 | 0.494572 | 0.563721 | 0.564815 |
| weighted_kurtosis.test | RUN | 1.107753 | 4.213516 | 3.863389 | 0.060951 | 0.068581 | 0.074204 |
| weighted_mean.test | RUN | 0.976193 | 3.751450 | 3.320086 | 0.004136 | 0.004430 | 0.004096 |
| weighted_median.test | RUN | 1.208635 | 4.258102 | 4.168418 | 16.246194 | 19.822397 | 19.878381 |
| weighted_moment.test | RUN | 0.814693 | 2.156763 | 2.516001 | 0.003755 | 0.004047 | 0.004123 |
| weighted_p_square_cumul_dist.test | RUN | 0.936566 | 2.523710 | 2.877628 | 2.873854 | 3.371678 | 3.463223 |
| weighted_p_square_quantile.test | RUN | 0.875228 | 2.396615 | 2.724763 | 0.705515 | 0.756385 | 0.721381 |
| weighted_skewness.test | RUN | 1.091609 | 3.851980 | 3.135095 | 0.052907 | 0.063517 | 0.062578 |
| weighted_sum.test | RUN | 0.693951 | 1.749647 | 1.710237 | 0.004027 | 0.004074 | 0.003921 |
| weighted_sum_kahan.test | RUN | 0.755541 | 1.920732 | 2.131973 | 0.285162 | 0.323293 | 0.277589 |
| weighted_variance.test | RUN | 1.210787 | 4.845339 | 4.221136 | 0.016014 | 0.019163 | 0.018046 |
| valarray.test | RUN | 1.082346 | 3.895368 | 3.119322 | 0.003968 | 0.004366 | 0.003969 |
| tail_variate_means.test | RUN | 1.356852 | 5.844086 | 5.095523 | 0.004262 | 0.004625 | 0.004086 |
| pot_quantile.test | RUN | 1.604885 | 6.044063 | 5.412858 | 0.218533 | 0.254932 | 0.255353 |
| weighted_pot_quantile.test | RUN | 1.740005 | 6.242975 | 6.304995 | 0.391373 | 0.401164 | 0.448611 |
| weighted_tail_mean.test | RUN | 1.502530 | 4.974545 | 4.570477 | 0.269097 | 0.244146 | 0.289376 |
| weighted_tail_quantile.test | RUN | 1.516661 | 4.502089 | 5.381980 | 0.248465 | 0.179567 | 0.269417 |
| weighted_tail_variate_means.test | RUN | 1.579098 | 6.181291 | 5.876113 | 0.003249 | 0.003537 | 0.003344 |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42557497688) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42557497688) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42557497688).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
