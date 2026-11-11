# libs/accumulators/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| count.test | RUN | 0.651976 | 1.612422 | 1.556729 | 0.003973 | 0.004361 | 0.003866 |
| covariance.test | RUN | 1.160045 | 5.401559 | 3.928286 | 0.004256 | 0.004224 | 0.004345 |
| droppable.test | RUN | 0.786793 | 2.811124 | 2.097742 | 0.004042 | 0.004502 | 0.003959 |
| error_of.test | RUN | 0.852582 | 3.278753 | 2.514862 | 0.003779 | 0.004028 | 0.003689 |
| extended_p_square.test | RUN | 0.857690 | 2.545155 | 2.228007 | 0.019083 | 0.020828 | 0.022176 |
| extended_p_square_quantile.test | RUN | 1.055831 | 4.385632 | 3.174142 | 0.116073 | 0.124282 | 0.126852 |
| external_accumulator.test | RUN | 0.753825 | 3.185246 | 2.201535 | 0.004090 | 0.004321 | 0.004044 |
| external_weights.test | RUN | 0.679271 | 2.011525 | 1.776284 | 0.003734 | 0.003770 | 0.003359 |
| kurtosis.test | RUN | 1.005605 | 4.009323 | 2.908480 | 0.038162 | 0.034885 | 0.043225 |
| max.test | RUN | 0.664075 | 1.689141 | 1.625327 | 0.004296 | 0.004196 | 0.003722 |
| mean.test | RUN | 0.847640 | 3.070907 | 2.367014 | 0.003789 | 0.004304 | 0.003744 |
| median.test | RUN | 1.090106 | 4.445461 | 3.138664 | 1.133430 | 1.227940 | 1.524150 |
| min.test | RUN | 0.661372 | 1.707038 | 1.578309 | 0.004034 | 0.004564 | 0.003986 |
| moment.test | RUN | 0.736695 | 2.066357 | 1.829609 | 0.004050 | 0.004406 | 0.003844 |
| p_square_cumul_dist.test | RUN | 0.918606 | 2.688287 | 2.136583 | 11.038333 | 9.937414 | 12.367394 |
| p_square_quantile.test | RUN | 0.842112 | 2.270562 | 2.070114 | 0.323871 | 0.286160 | 0.317622 |
| reference.test | RUN | 0.765709 | 2.379707 | 1.992970 | 0.003860 | 0.004282 | 0.003743 |
| rolling_count.test | RUN | 0.712107 | 2.009191 | 1.630772 | 0.003997 | 0.004320 | 0.003839 |
| rolling_sum.test | RUN | 0.745540 | 1.994668 | 1.882513 | 0.003918 | 0.004099 | 0.003894 |
| rolling_mean.test | RUN | 0.950584 | 4.089488 | 2.945622 | 0.004465 | 0.004880 | 0.004266 |
| rolling_variance.test | RUN | 1.044241 | 4.773895 | 3.166665 | 0.004472 | 0.004600 | 0.004241 |
| rolling_moment.test | RUN | 0.863942 | 2.509411 | 2.154665 | 0.004437 | 0.004530 | 0.004123 |
| skewness.test | RUN | 0.958223 | 3.397299 | 2.510438 | 0.032633 | 0.034289 | 0.033422 |
| sum.test | RUN | 0.667782 | 1.887261 | 1.618435 | 0.003903 | 0.003984 | 0.003841 |
| sum_kahan.test | RUN | 0.718595 | 2.023363 | 1.821050 | 0.347903 | 0.348787 | 0.316974 |
| tail.test | RUN | 0.748814 | 2.487299 | 1.996244 | 0.004082 | 0.004148 | 0.003842 |
| tail_mean.test | RUN | 1.045109 | 4.257127 | 2.652552 | 0.154460 | 0.188587 | 0.210750 |
| tail_quantile.test | RUN | 0.865789 | 2.625346 | 2.299262 | 0.143012 | 0.156692 | 0.174305 |
| variance.test | RUN | 0.823019 | 3.042273 | 2.281039 | 0.004000 | 0.011308 | 0.004144 |
| vector.test | RUN | 0.857336 | 3.644250 | 2.379928 | 0.003907 | 0.004187 | 0.003777 |
| weighted_covariance.test | RUN | 1.221625 | 5.697691 | 4.080309 | 0.003963 | 0.004191 | 0.003608 |
| weighted_extended_p_square.test | RUN | 0.870355 | 2.290908 | 2.063540 | 0.364958 | 0.363746 | 0.376820 |
| weighted_kurtosis.test | RUN | 0.993707 | 3.775441 | 2.789397 | 0.041316 | 0.043353 | 0.045323 |
| weighted_mean.test | RUN | 0.832474 | 3.402721 | 2.459305 | 0.003816 | 0.004241 | 0.003880 |
| weighted_median.test | RUN | 1.031782 | 4.104489 | 3.039221 | 11.750145 | 12.678500 | 14.059697 |
| weighted_moment.test | RUN | 0.686854 | 2.074822 | 1.786136 | 0.005063 | 0.003561 | 0.004174 |
| weighted_p_square_cumul_dist.test | RUN | 0.791187 | 2.277135 | 2.089560 | 2.133294 | 2.071122 | 2.297148 |
| weighted_p_square_quantile.test | RUN | 0.750575 | 2.103907 | 1.922543 | 0.481827 | 0.466087 | 0.441188 |
| weighted_skewness.test | RUN | 0.917446 | 3.369281 | 2.293827 | 0.038196 | 0.041709 | 0.045450 |
| weighted_sum.test | RUN | 0.588087 | 1.498395 | 1.220874 | 0.003378 | 0.003931 | 0.003653 |
| weighted_sum_kahan.test | RUN | 0.625488 | 1.657074 | 1.344605 | 0.211309 | 0.212729 | 0.198160 |
| weighted_variance.test | RUN | 0.962187 | 4.080164 | 3.005812 | 0.011361 | 0.012821 | 0.011386 |
| valarray.test | RUN | 0.900066 | 3.465342 | 2.613208 | 0.003533 | 0.003974 | 0.003519 |
| tail_variate_means.test | RUN | 1.081592 | 5.100370 | 3.066334 | 0.003982 | 0.004306 | 0.003829 |
| pot_quantile.test | RUN | 1.428827 | 5.132067 | 4.203678 | 0.147729 | 0.169939 | 0.162276 |
| weighted_pot_quantile.test | RUN | 1.440606 | 5.481752 | 4.421072 | 0.260769 | 0.276997 | 0.284625 |
| weighted_tail_mean.test | RUN | 1.300258 | 4.084507 | 3.378899 | 0.179991 | 0.190362 | 0.175831 |
| weighted_tail_quantile.test | RUN | 1.325203 | 3.886212 | 2.765974 | 0.167059 | 0.106365 | 0.136103 |
| weighted_tail_variate_means.test | RUN | 1.572497 | 5.497441 | 3.959711 | 0.003355 | 0.003228 | 0.002902 |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42562778965) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42562778965) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42562778965).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
