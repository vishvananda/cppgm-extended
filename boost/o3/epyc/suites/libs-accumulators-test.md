# libs/accumulators/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| count.test | RUN | 0.707983 | 1.876042 | 1.831486 | 0.003352 | 0.003451 | 0.003260 |
| covariance.test | RUN | 1.300279 | 5.214663 | 4.973339 | 0.003419 | 0.003619 | 0.003238 |
| droppable.test | RUN | 0.843182 | 2.563717 | 2.776108 | 0.003543 | 0.003793 | 0.003144 |
| error_of.test | RUN | 0.917562 | 2.478065 | 2.863688 | 0.003072 | 0.003457 | 0.002960 |
| extended_p_square.test | RUN | 0.974701 | 3.104273 | 2.729019 | 0.004560 | 0.004850 | 0.004062 |
| extended_p_square_quantile.test | RUN | 1.259652 | 5.388996 | 4.063280 | 0.007479 | 0.008173 | 0.006608 |
| external_accumulator.test | RUN | 0.852915 | 2.416101 | 2.391303 | 0.003558 | 0.003388 | 0.003245 |
| external_weights.test | RUN | 0.726074 | 1.861938 | 1.989478 | 0.003095 | 0.003383 | 0.003354 |
| kurtosis.test | RUN | 1.123622 | 3.098621 | 3.228529 | 0.005256 | 0.004444 | 0.003931 |
| max.test | RUN | 0.716132 | 1.776533 | 1.866043 | 0.004109 | 0.003597 | 0.003473 |
| mean.test | RUN | 0.938961 | 2.581655 | 2.673228 | 0.003258 | 0.003452 | 0.003494 |
| median.test | RUN | 1.282708 | 5.531079 | 4.175474 | 0.100031 | 0.103899 | 0.091581 |
| min.test | RUN | 0.719993 | 1.835286 | 1.784651 | 0.002985 | 0.003412 | 0.003383 |
| moment.test | RUN | 0.807295 | 2.084824 | 2.208965 | 0.003230 | 0.004088 | 0.003141 |
| p_square_cumul_dist.test | RUN | 1.018589 | 3.722085 | 3.203427 | 0.690863 | 0.797335 | 0.647408 |
| p_square_quantile.test | RUN | 0.903596 | 3.362199 | 2.559787 | 0.025957 | 0.023154 | 0.020562 |
| reference.test | RUN | 0.785654 | 2.149473 | 2.197330 | 0.003133 | 0.003283 | 0.003376 |
| rolling_count.test | RUN | 0.800724 | 2.368865 | 2.227691 | 0.003228 | 0.003743 | 0.003212 |
| rolling_sum.test | RUN | 0.803172 | 2.419460 | 2.241465 | 0.003534 | 0.003312 | 0.003394 |
| rolling_mean.test | RUN | 1.135119 | 4.464309 | 3.807594 | 0.003262 | 0.003494 | 0.003254 |
| rolling_variance.test | RUN | 1.318722 | 5.120768 | 4.316134 | 0.003373 | 0.003632 | 0.003134 |
| rolling_moment.test | RUN | 0.972807 | 2.960804 | 2.788597 | 0.003745 | 0.003900 | 0.003386 |
| skewness.test | RUN | 1.053361 | 2.936846 | 3.067896 | 0.004990 | 0.004958 | 0.003994 |
| sum.test | RUN | 0.751235 | 2.028361 | 1.998111 | 0.003350 | 0.003502 | 0.003167 |
| sum_kahan.test | RUN | 0.799017 | 2.153634 | 2.114321 | 0.012784 | 0.022607 | 0.011612 |
| tail.test | RUN | 0.842914 | 3.163854 | 2.423495 | 0.003250 | 0.003487 | 0.002572 |
| tail_mean.test | RUN | 1.160354 | 3.675655 | 3.550337 | 0.019215 | 0.024978 | 0.020469 |
| tail_quantile.test | RUN | 0.965369 | 2.944050 | 2.726839 | 0.021799 | 0.029375 | 0.020405 |
| variance.test | RUN | 0.913437 | 2.678584 | 2.611459 | 0.003205 | 0.003854 | 0.003316 |
| vector.test | RUN | 0.972407 | 3.390872 | 3.273159 | 0.003067 | 0.003572 | 0.003264 |
| weighted_covariance.test | RUN | 1.333189 | 5.181474 | 4.967177 | 0.002775 | 0.003310 | 0.003299 |
| weighted_extended_p_square.test | RUN | 0.931716 | 2.971349 | 2.449299 | 0.020691 | 0.019782 | 0.016720 |
| weighted_kurtosis.test | RUN | 1.083450 | 2.884043 | 3.146701 | 0.005382 | 0.005381 | 0.003996 |
| weighted_mean.test | RUN | 0.910322 | 2.558462 | 2.663402 | 0.002968 | 0.003279 | 0.003004 |
| weighted_median.test | RUN | 1.142935 | 3.505715 | 3.528309 | 0.985136 | 0.975150 | 0.910477 |
| weighted_moment.test | RUN | 0.751313 | 1.866774 | 2.057007 | 0.003025 | 0.003221 | 0.002978 |
| weighted_p_square_cumul_dist.test | RUN | 0.905692 | 3.028375 | 2.425442 | 0.101952 | 0.108098 | 0.088931 |
| weighted_p_square_quantile.test | RUN | 0.843087 | 2.916636 | 2.219159 | 0.028451 | 0.027007 | 0.020719 |
| weighted_skewness.test | RUN | 1.081530 | 2.719914 | 2.802379 | 0.005400 | 0.005372 | 0.003394 |
| weighted_sum.test | RUN | 0.619759 | 1.577266 | 1.555741 | 0.003090 | 0.003009 | 0.002746 |
| weighted_sum_kahan.test | RUN | 0.718895 | 1.730499 | 1.714309 | 0.007799 | 0.013087 | 0.007290 |
| weighted_variance.test | RUN | 1.141520 | 3.149807 | 3.342648 | 0.003451 | 0.003736 | 0.003501 |
| valarray.test | RUN | 1.025551 | 3.053886 | 3.087946 | 0.004144 | 0.003634 | 0.002755 |
| tail_variate_means.test | RUN | 1.358791 | 4.834638 | 4.296800 | 0.003427 | 0.003873 | 0.003144 |
| pot_quantile.test | RUN | 1.670678 | 5.387732 | 5.047290 | 0.013126 | 0.011946 | 0.009627 |
| weighted_pot_quantile.test | RUN | 1.566005 | 5.295077 | 5.212089 | 0.026217 | 0.025540 | 0.022540 |
| weighted_tail_mean.test | RUN | 1.418794 | 4.055987 | 4.483268 | 0.022576 | 0.022745 | 0.020141 |
| weighted_tail_quantile.test | RUN | 1.360857 | 3.600470 | 4.108457 | 0.018590 | 0.019424 | 0.016217 |
| weighted_tail_variate_means.test | RUN | 1.521857 | 4.740834 | 4.994853 | 0.002599 | 0.002477 | 0.002275 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.058×† | 1.031×, 1.085× | 1.049× |
| clang | 0.916× | 0.909×, 0.923× | 0.998× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1361 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| count.test | 1 | 0.002549s | 0.002466s | 0.002444s | byte equality observed |
| covariance.test | 1 | 0.002554s | 0.002588s | 0.002358s | byte equality observed |
| droppable.test | 1 | 0.002651s | 0.002478s | 0.002371s | byte equality observed |
| error_of.test | 1 | 0.002492s | 0.002411s | 0.002306s | byte equality observed |
| extended_p_square.test | 5 | 0.003485s | 0.003344s | 0.003072s | byte equality observed |
| extended_p_square_quantile.test | 3 | 0.006223s | 0.006380s | 0.005434s | byte equality observed |
| external_accumulator.test | 4 | 0.002086s | 0.002019s | 0.001950s | byte equality observed |
| external_weights.test | 4 | 0.001978s | 0.001898s | 0.001876s | byte equality observed |
| kurtosis.test | 4 | 0.003876s | 0.003593s | 0.002610s | byte equality observed |
| max.test | 5 | 0.002152s | 0.002182s | 0.002007s | byte equality observed |
| mean.test | 8 | 0.002104s | 0.002158s | 0.001936s | byte equality observed |
| median.test | 1 | 0.092667s | 0.103606s | 0.090110s | byte equality observed |
| min.test | 7 | 0.002085s | 0.002046s | 0.001954s | byte equality observed |
| moment.test | 8 | 0.002109s | 0.002042s | 0.001925s | byte equality observed |
| p_square_cumul_dist.test | 1 | 0.644464s | 0.715128s | 0.607556s | byte equality observed |
| p_square_quantile.test | 1 | 0.025514s | 0.021972s | 0.019534s | byte equality observed |
| pot_quantile.test | 2 | 0.011641s | 0.010214s | 0.008388s | byte equality observed |
| reference.test | 6 | 0.002204s | 0.002115s | 0.002019s | byte equality observed |
| rolling_count.test | 6 | 0.002119s | 0.001999s | 0.001986s | byte equality observed |
| rolling_mean.test | 6 | 0.002209s | 0.002080s | 0.002110s | byte equality observed |
| rolling_moment.test | 7 | 0.002216s | 0.002093s | 0.002193s | byte equality observed |
| rolling_sum.test | 8 | 0.002022s | 0.002002s | 0.002125s | byte equality observed |
| rolling_variance.test | 7 | 0.002161s | 0.002082s | 0.002130s | byte equality observed |
| skewness.test | 4 | 0.003645s | 0.003501s | 0.002794s | byte equality observed |
| sum.test | 7 | 0.001994s | 0.001921s | 0.002047s | byte equality observed |
| sum_kahan.test | 1 | 0.011398s | 0.020098s | 0.010133s | byte equality observed |
| tail.test | 7 | 0.002056s | 0.001915s | 0.001992s | byte equality observed |
| tail_mean.test | 1 | 0.018370s | 0.019851s | 0.017976s | byte equality observed |
| tail_quantile.test | 1 | 0.021607s | 0.023434s | 0.019545s | byte equality observed |
| tail_variate_means.test | 7 | 0.002052s | 0.001881s | 0.002019s | byte equality observed |
| valarray.test | 6 | 0.001984s | 0.002016s | 0.001896s | byte equality observed |
| variance.test | 7 | 0.002031s | 0.002032s | 0.001912s | byte equality observed |
| vector.test | 7 | 0.001946s | 0.001920s | 0.001848s | byte equality observed |
| weighted_covariance.test | 7 | 0.001952s | 0.001768s | 0.001862s | byte equality observed |
| weighted_extended_p_square.test | 1 | 0.017900s | 0.020507s | 0.014972s | byte equality observed |
| weighted_kurtosis.test | 5 | 0.004376s | 0.003855s | 0.002707s | byte equality observed |
| weighted_mean.test | 7 | 0.002026s | 0.001977s | 0.001944s | byte equality observed |
| weighted_median.test | 1 | 0.877326s | 0.886476s | 0.820854s | byte equality observed |
| weighted_moment.test | 6 | 0.001994s | 0.002203s | 0.001944s | byte equality observed |
| weighted_p_square_cumul_dist.test | 1 | 0.108346s | 0.114695s | 0.092584s | byte equality observed |
| weighted_p_square_quantile.test | 1 | 0.028068s | 0.027121s | 0.020441s | byte equality observed |
| weighted_pot_quantile.test | 1 | 0.024146s | 0.023308s | 0.020476s | byte equality observed |
| weighted_skewness.test | 4 | 0.004672s | 0.004528s | 0.002965s | byte equality observed |
| weighted_sum.test | 8 | 0.002028s | 0.002127s | 0.001949s | byte equality observed |
| weighted_sum_kahan.test | 2 | 0.006987s | 0.011367s | 0.006304s | byte equality observed |
| weighted_tail_mean.test | 1 | 0.020203s | 0.019940s | 0.018818s | byte equality observed |
| weighted_tail_quantile.test | 1 | 0.021222s | 0.021908s | 0.019357s | byte equality observed |
| weighted_tail_variate_means.test | 5 | 0.002152s | 0.002112s | 0.002208s | byte equality observed |
| weighted_variance.test | 5 | 0.002316s | 0.002247s | 0.002084s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
