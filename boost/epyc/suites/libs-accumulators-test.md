# libs/accumulators/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| count.test | RUN | 0.795525 | 2.033694 | 2.318321 | 0.004225 | 0.004742 | 0.004750 |
| covariance.test | RUN | 1.455531 | 6.060379 | 5.669097 | 0.004172 | 0.004752 | 0.004754 |
| droppable.test | RUN | 0.945807 | 3.246529 | 3.179709 | 0.004463 | 0.004838 | 0.004314 |
| error_of.test | RUN | 1.060182 | 3.960066 | 3.537361 | 0.004050 | 0.004317 | 0.004402 |
| extended_p_square.test | RUN | 1.065916 | 2.884223 | 3.308255 | 0.025604 | 0.027891 | 0.030724 |
| extended_p_square_quantile.test | RUN | 1.321181 | 4.729232 | 4.435397 | 0.164629 | 0.179087 | 0.188239 |
| external_accumulator.test | RUN | 0.965882 | 3.381745 | 3.213715 | 0.004304 | 0.004526 | 0.004652 |
| external_weights.test | RUN | 0.837187 | 2.236256 | 2.440131 | 0.004032 | 0.003918 | 0.004324 |
| kurtosis.test | RUN | 1.272554 | 4.433715 | 4.039501 | 0.052903 | 0.054555 | 0.063605 |
| max.test | RUN | 0.870431 | 1.988837 | 2.207184 | 0.004338 | 0.004997 | 0.004463 |
| mean.test | RUN | 1.093257 | 3.495885 | 3.208872 | 0.004087 | 0.004701 | 0.004764 |
| median.test | RUN | 1.338537 | 4.886479 | 4.450473 | 1.521989 | 1.781401 | 2.173112 |
| min.test | RUN | 0.839825 | 1.929891 | 2.220907 | 0.003906 | 0.004585 | 0.004495 |
| moment.test | RUN | 0.936476 | 2.518036 | 2.578215 | 0.004317 | 0.004472 | 0.004186 |
| p_square_cumul_dist.test | RUN | 1.140360 | 3.236152 | 3.431498 | 14.635444 | 15.068263 | 17.918762 |
| p_square_quantile.test | RUN | 0.980618 | 2.598654 | 2.913077 | 0.437989 | 0.465418 | 0.502101 |
| reference.test | RUN | 0.916937 | 2.738810 | 2.539546 | 0.004220 | 0.004500 | 0.004314 |
| rolling_count.test | RUN | 0.909917 | 2.399048 | 2.557901 | 0.004184 | 0.004356 | 0.004386 |
| rolling_sum.test | RUN | 0.860931 | 2.431552 | 2.550889 | 0.004314 | 0.004441 | 0.004537 |
| rolling_mean.test | RUN | 1.176017 | 4.608831 | 3.786640 | 0.004873 | 0.005321 | 0.005279 |
| rolling_variance.test | RUN | 1.304868 | 5.458968 | 4.078792 | 0.004693 | 0.004956 | 0.004808 |
| rolling_moment.test | RUN | 1.072880 | 3.156110 | 3.278099 | 0.004403 | 0.004802 | 0.004208 |
| skewness.test | RUN | 1.158636 | 3.907367 | 3.783466 | 0.046820 | 0.050335 | 0.053705 |
| sum.test | RUN | 0.836217 | 2.122063 | 2.376119 | 0.003984 | 0.004554 | 0.004267 |
| sum_kahan.test | RUN | 0.886448 | 2.338331 | 2.189285 | 0.458148 | 0.630130 | 0.530033 |
| tail.test | RUN | 0.883455 | 2.795140 | 2.865478 | 0.005133 | 0.004790 | 0.004311 |
| tail_mean.test | RUN | 1.273730 | 4.688393 | 4.166264 | 0.224107 | 0.258987 | 0.283984 |
| tail_quantile.test | RUN | 1.029010 | 2.955350 | 2.862383 | 0.212049 | 0.179487 | 0.263435 |
| variance.test | RUN | 1.057820 | 3.519206 | 3.023618 | 0.004105 | 0.004596 | 0.004672 |
| vector.test | RUN | 1.024176 | 3.946298 | 3.359774 | 0.003943 | 0.004708 | 0.004513 |
| weighted_covariance.test | RUN | 1.425367 | 6.374458 | 5.973619 | 0.004191 | 0.004536 | 0.004033 |
| weighted_extended_p_square.test | RUN | 1.037714 | 2.641643 | 3.086951 | 0.483826 | 0.594379 | 0.563123 |
| weighted_kurtosis.test | RUN | 1.191843 | 4.165040 | 3.903555 | 0.061230 | 0.066167 | 0.072348 |
| weighted_mean.test | RUN | 1.016649 | 3.670609 | 3.322825 | 0.003765 | 0.004304 | 0.004117 |
| weighted_median.test | RUN | 1.242992 | 4.380113 | 4.354987 | 16.306172 | 19.866271 | 20.168881 |
| weighted_moment.test | RUN | 0.833893 | 2.337600 | 2.630647 | 0.003853 | 0.004118 | 0.003848 |
| weighted_p_square_cumul_dist.test | RUN | 0.989604 | 2.625981 | 2.965902 | 2.808013 | 3.659287 | 3.243524 |
| weighted_p_square_quantile.test | RUN | 0.893870 | 2.444411 | 2.823508 | 0.703674 | 0.688363 | 0.726246 |
| weighted_skewness.test | RUN | 1.076510 | 3.822707 | 3.257107 | 0.053673 | 0.055644 | 0.063602 |
| weighted_sum.test | RUN | 0.717988 | 1.750456 | 1.775003 | 0.003866 | 0.004007 | 0.004010 |
| weighted_sum_kahan.test | RUN | 0.796115 | 1.907543 | 2.216529 | 0.300897 | 0.362767 | 0.278152 |
| weighted_variance.test | RUN | 1.304939 | 4.660610 | 4.483142 | 0.014812 | 0.018685 | 0.018206 |
| valarray.test | RUN | 1.145167 | 3.886936 | 3.388896 | 0.004282 | 0.004161 | 0.004546 |
| tail_variate_means.test | RUN | 1.365202 | 5.570672 | 5.275793 | 0.004218 | 0.004463 | 0.004098 |
| pot_quantile.test | RUN | 1.784582 | 6.062561 | 5.630411 | 0.193562 | 0.232305 | 0.245318 |
| weighted_pot_quantile.test | RUN | 1.797565 | 6.305433 | 6.840075 | 0.371955 | 0.401656 | 0.456321 |
| weighted_tail_mean.test | RUN | 1.542327 | 4.947959 | 5.728035 | 0.268615 | 0.240587 | 0.297753 |
| weighted_tail_quantile.test | RUN | 1.541337 | 4.320124 | 4.878404 | 0.250137 | 0.174262 | 0.280499 |
| weighted_tail_variate_means.test | RUN | 1.718010 | 6.580823 | 5.623639 | 0.003430 | 0.003530 | 0.003615 |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42570601152) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42570601152) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42570601152).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
