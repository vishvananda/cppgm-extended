# libs/accumulators/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| count.test | RUN | 0.629346 | 1.630556 | 1.670645 | 0.004077 | 0.003900 | 0.004502 |
| covariance.test | RUN | 1.176208 | 5.337424 | 4.402018 | 0.003863 | 0.004079 | 0.004499 |
| droppable.test | RUN | 0.788588 | 2.816531 | 2.497299 | 0.003915 | 0.004064 | 0.004146 |
| error_of.test | RUN | 0.895608 | 3.521013 | 2.788920 | 0.003857 | 0.003969 | 0.004217 |
| extended_p_square.test | RUN | 0.888548 | 2.472610 | 2.555723 | 0.020809 | 0.020834 | 0.022121 |
| extended_p_square_quantile.test | RUN | 1.071265 | 4.205806 | 3.425798 | 0.116889 | 0.126304 | 0.127588 |
| external_accumulator.test | RUN | 0.768640 | 2.821572 | 2.458708 | 0.003971 | 0.004110 | 0.003922 |
| external_weights.test | RUN | 0.687199 | 1.892435 | 1.872157 | 0.003525 | 0.003691 | 0.003699 |
| kurtosis.test | RUN | 1.012542 | 3.805977 | 3.027040 | 0.032716 | 0.037238 | 0.040172 |
| max.test | RUN | 0.636776 | 1.665828 | 1.643449 | 0.003965 | 0.004137 | 0.004252 |
| mean.test | RUN | 0.819173 | 2.925857 | 2.449572 | 0.003943 | 0.004115 | 0.004316 |
| median.test | RUN | 1.041132 | 4.285404 | 3.306711 | 1.104324 | 1.276868 | 1.524901 |
| min.test | RUN | 0.629957 | 1.611164 | 1.636190 | 0.003924 | 0.004374 | 0.004119 |
| moment.test | RUN | 0.730400 | 2.083276 | 1.885198 | 0.003725 | 0.004142 | 0.004001 |
| p_square_cumul_dist.test | RUN | 0.873295 | 2.783355 | 2.434325 | 10.665021 | 9.994971 | 12.344328 |
| p_square_quantile.test | RUN | 0.797685 | 2.374759 | 2.116263 | 0.316839 | 0.323856 | 0.327527 |
| reference.test | RUN | 0.729352 | 2.377069 | 1.914387 | 0.003669 | 0.004067 | 0.003935 |
| rolling_count.test | RUN | 0.698333 | 1.962708 | 1.888306 | 0.004204 | 0.004182 | 0.004175 |
| rolling_sum.test | RUN | 0.716472 | 1.931807 | 1.718627 | 0.003855 | 0.004120 | 0.004129 |
| rolling_mean.test | RUN | 0.978058 | 3.829577 | 2.870996 | 0.004394 | 0.004716 | 0.004368 |
| rolling_variance.test | RUN | 1.070143 | 4.633632 | 3.472765 | 0.004535 | 0.004718 | 0.004309 |
| rolling_moment.test | RUN | 0.849309 | 2.587128 | 2.357236 | 0.004047 | 0.004367 | 0.004320 |
| skewness.test | RUN | 0.979168 | 3.354713 | 2.375149 | 0.031839 | 0.032136 | 0.036256 |
| sum.test | RUN | 0.665748 | 1.752551 | 1.718404 | 0.003863 | 0.003983 | 0.004174 |
| sum_kahan.test | RUN | 0.667696 | 1.988142 | 1.889799 | 0.337707 | 0.356461 | 0.311394 |
| tail.test | RUN | 0.695993 | 2.464275 | 1.960553 | 0.003930 | 0.004265 | 0.004406 |
| tail_mean.test | RUN | 1.012390 | 4.232137 | 3.165449 | 0.148784 | 0.180417 | 0.219088 |
| tail_quantile.test | RUN | 0.860987 | 2.678109 | 2.191476 | 0.138031 | 0.149235 | 0.170787 |
| variance.test | RUN | 0.813976 | 3.075684 | 2.449197 | 0.003913 | 0.005573 | 0.003970 |
| vector.test | RUN | 0.794148 | 3.487513 | 2.790083 | 0.003814 | 0.004313 | 0.003643 |
| weighted_covariance.test | RUN | 1.080019 | 5.637997 | 4.348800 | 0.003703 | 0.003924 | 0.003782 |
| weighted_extended_p_square.test | RUN | 0.801123 | 2.274601 | 2.179924 | 0.358311 | 0.346745 | 0.356052 |
| weighted_kurtosis.test | RUN | 0.953457 | 3.733035 | 2.807934 | 0.039638 | 0.044210 | 0.047551 |
| weighted_mean.test | RUN | 0.780601 | 3.414906 | 2.488364 | 0.003694 | 0.003771 | 0.003797 |
| weighted_median.test | RUN | 0.954684 | 3.927758 | 3.050731 | 11.779437 | 12.469435 | 13.814615 |
| weighted_moment.test | RUN | 0.645088 | 1.956059 | 1.848585 | 0.003290 | 0.003951 | 0.003790 |
| weighted_p_square_cumul_dist.test | RUN | 0.756848 | 2.176857 | 2.132341 | 2.034217 | 2.101636 | 2.300688 |
| weighted_p_square_quantile.test | RUN | 0.686773 | 2.008870 | 1.960148 | 0.471138 | 0.463784 | 0.438535 |
| weighted_skewness.test | RUN | 0.862728 | 3.231405 | 2.580312 | 0.038879 | 0.044882 | 0.044516 |
| weighted_sum.test | RUN | 0.565712 | 1.402902 | 1.317270 | 0.003674 | 0.003941 | 0.003555 |
| weighted_sum_kahan.test | RUN | 0.579612 | 1.563128 | 1.573710 | 0.209459 | 0.207304 | 0.201713 |
| weighted_variance.test | RUN | 0.988921 | 4.036169 | 3.186522 | 0.011413 | 0.012561 | 0.011494 |
| valarray.test | RUN | 0.870007 | 3.232115 | 2.700602 | 0.003461 | 0.003916 | 0.003815 |
| tail_variate_means.test | RUN | 1.132279 | 4.866898 | 3.398107 | 0.003763 | 0.004170 | 0.003943 |
| pot_quantile.test | RUN | 1.408463 | 4.878353 | 4.586512 | 0.142456 | 0.165537 | 0.154867 |
| weighted_pot_quantile.test | RUN | 1.414585 | 5.229634 | 4.700525 | 0.275497 | 0.258973 | 0.287305 |
| weighted_tail_mean.test | RUN | 1.221974 | 4.181626 | 3.536179 | 0.177439 | 0.147411 | 0.177013 |
| weighted_tail_quantile.test | RUN | 1.200392 | 3.459737 | 3.395151 | 0.126176 | 0.099677 | 0.135044 |
| weighted_tail_variate_means.test | RUN | 1.476778 | 5.133526 | 3.998637 | 0.003049 | 0.003012 | 0.003180 |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42570114108) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42570114108) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42570114108).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
