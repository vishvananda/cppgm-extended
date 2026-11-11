# libs/accumulators/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| count.test | RUN | 0.698218 | 1.767910 | 1.705047 | 0.003428 | 0.003379 | 0.003480 |
| covariance.test | RUN | 1.294915 | 4.773139 | 4.347279 | 0.003649 | 0.003417 | 0.003331 |
| droppable.test | RUN | 0.852917 | 2.393023 | 2.222492 | 0.003343 | 0.003509 | 0.002793 |
| error_of.test | RUN | 0.915100 | 2.340257 | 2.509267 | 0.003490 | 0.003244 | 0.002466 |
| extended_p_square.test | RUN | 0.981267 | 2.945895 | 2.676290 | 0.004636 | 0.009160 | 0.004097 |
| extended_p_square_quantile.test | RUN | 1.267213 | 4.900898 | 3.788892 | 0.007322 | 0.007986 | 0.006718 |
| external_accumulator.test | RUN | 0.868640 | 2.212653 | 2.303890 | 0.003197 | 0.003317 | 0.003088 |
| external_weights.test | RUN | 0.742634 | 1.772020 | 1.819205 | 0.003125 | 0.002964 | 0.002802 |
| kurtosis.test | RUN | 1.105622 | 2.796462 | 2.952438 | 0.005006 | 0.004873 | 0.003936 |
| max.test | RUN | 0.701401 | 1.699948 | 1.688132 | 0.003187 | 0.003257 | 0.003058 |
| mean.test | RUN | 0.911365 | 2.468624 | 2.519796 | 0.003266 | 0.003444 | 0.002790 |
| median.test | RUN | 1.253103 | 4.823367 | 3.842445 | 0.094353 | 0.100176 | 0.091168 |
| min.test | RUN | 0.712620 | 1.725129 | 1.709861 | 0.002972 | 0.003476 | 0.003077 |
| moment.test | RUN | 0.785652 | 1.944668 | 2.102694 | 0.003473 | 0.003502 | 0.003063 |
| p_square_cumul_dist.test | RUN | 1.054741 | 3.408600 | 2.845288 | 0.690915 | 0.766627 | 0.676116 |
| p_square_quantile.test | RUN | 0.933020 | 3.034080 | 2.445493 | 0.024291 | 0.023358 | 0.020444 |
| reference.test | RUN | 0.810092 | 1.849761 | 2.059760 | 0.003420 | 0.003441 | 0.002835 |
| rolling_count.test | RUN | 0.825401 | 2.178943 | 2.042380 | 0.003476 | 0.003224 | 0.003275 |
| rolling_sum.test | RUN | 0.801712 | 2.193843 | 2.209907 | 0.003409 | 0.003425 | 0.002950 |
| rolling_mean.test | RUN | 1.117895 | 3.964443 | 3.542898 | 0.003385 | 0.003197 | 0.003099 |
| rolling_variance.test | RUN | 1.252917 | 4.743309 | 3.992569 | 0.003605 | 0.003346 | 0.003097 |
| rolling_moment.test | RUN | 0.966886 | 2.679105 | 2.657779 | 0.003472 | 0.003564 | 0.003262 |
| skewness.test | RUN | 1.075110 | 2.766284 | 2.889452 | 0.005227 | 0.005086 | 0.003900 |
| sum.test | RUN | 0.746075 | 1.886557 | 1.899850 | 0.003426 | 0.003391 | 0.003252 |
| sum_kahan.test | RUN | 0.809297 | 2.005471 | 2.030318 | 0.012779 | 0.021836 | 0.011600 |
| tail.test | RUN | 0.844314 | 2.807057 | 2.314256 | 0.003247 | 0.003337 | 0.003196 |
| tail_mean.test | RUN | 1.187277 | 3.331918 | 3.408659 | 0.018908 | 0.023223 | 0.019624 |
| tail_quantile.test | RUN | 0.976072 | 2.714005 | 2.609114 | 0.021354 | 0.024309 | 0.021027 |
| variance.test | RUN | 0.921287 | 2.430453 | 2.560792 | 0.003424 | 0.006703 | 0.003218 |
| vector.test | RUN | 0.987427 | 3.033553 | 3.109123 | 0.003222 | 0.003298 | 0.003100 |
| weighted_covariance.test | RUN | 1.334495 | 4.679235 | 4.528089 | 0.003110 | 0.003126 | 0.002750 |
| weighted_extended_p_square.test | RUN | 0.930288 | 2.731982 | 2.300704 | 0.019350 | 0.020105 | 0.016057 |
| weighted_kurtosis.test | RUN | 1.061229 | 2.585679 | 2.848639 | 0.005183 | 0.005400 | 0.003682 |
| weighted_mean.test | RUN | 0.911317 | 2.383127 | 2.465290 | 0.002925 | 0.003121 | 0.002496 |
| weighted_median.test | RUN | 1.159293 | 3.242551 | 3.179928 | 0.977251 | 0.953819 | 0.896044 |
| weighted_moment.test | RUN | 0.749444 | 1.746594 | 1.785697 | 0.003025 | 0.003228 | 0.002632 |
| weighted_p_square_cumul_dist.test | RUN | 0.911707 | 2.658814 | 2.166590 | 0.100479 | 0.105455 | 0.086611 |
| weighted_p_square_quantile.test | RUN | 0.841573 | 2.533816 | 2.012634 | 0.028317 | 0.026324 | 0.019557 |
| weighted_skewness.test | RUN | 1.024664 | 2.533015 | 2.423648 | 0.004945 | 0.005198 | 0.003703 |
| weighted_sum.test | RUN | 0.595050 | 1.435795 | 1.441969 | 0.003054 | 0.003166 | 0.002847 |
| weighted_sum_kahan.test | RUN | 0.690507 | 1.525373 | 1.538971 | 0.007530 | 0.012235 | 0.006916 |
| weighted_variance.test | RUN | 1.110484 | 2.743451 | 3.016542 | 0.003575 | 0.003464 | 0.003103 |
| valarray.test | RUN | 0.984131 | 2.646729 | 2.948903 | 0.003335 | 0.003144 | 0.002806 |
| tail_variate_means.test | RUN | 1.305574 | 4.353983 | 4.038623 | 0.003200 | 0.003166 | 0.002868 |
| pot_quantile.test | RUN | 1.643020 | 4.709280 | 4.805994 | 0.012073 | 0.011531 | 0.009325 |
| weighted_pot_quantile.test | RUN | 1.626180 | 4.720396 | 4.770549 | 0.023061 | 0.025095 | 0.021934 |
| weighted_tail_mean.test | RUN | 1.452364 | 3.737047 | 4.133780 | 0.023231 | 0.023008 | 0.016987 |
| weighted_tail_quantile.test | RUN | 1.411701 | 3.140939 | 3.858153 | 0.018618 | 0.018294 | 0.015685 |
| weighted_tail_variate_means.test | RUN | 1.588085 | 4.026540 | 4.167588 | 0.002428 | 0.002322 | 0.002111 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.088× | 1.078×, 1.099× | 1.004× |
| clang | 0.926× | 0.925×, 0.927× | 0.986× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1362 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| count.test | 1 | 0.002601s | 0.002469s | 0.002437s | byte equality observed |
| covariance.test | 1 | 0.002567s | 0.002295s | 0.002371s | byte equality observed |
| droppable.test | 1 | 0.002589s | 0.002410s | 0.002358s | byte equality observed |
| error_of.test | 1 | 0.002494s | 0.002048s | 0.002274s | byte equality observed |
| extended_p_square.test | 6 | 0.003284s | 0.003149s | 0.002952s | byte equality observed |
| extended_p_square_quantile.test | 3 | 0.005876s | 0.006128s | 0.005359s | byte equality observed |
| external_accumulator.test | 6 | 0.002038s | 0.001940s | 0.001956s | byte equality observed |
| external_weights.test | 7 | 0.001921s | 0.001843s | 0.001853s | byte equality observed |
| kurtosis.test | 5 | 0.003693s | 0.003412s | 0.002653s | byte equality observed |
| max.test | 9 | 0.002015s | 0.001915s | 0.001899s | byte equality observed |
| mean.test | 7 | 0.002013s | 0.001941s | 0.001938s | byte equality observed |
| median.test | 1 | 0.091831s | 0.099802s | 0.088034s | byte equality observed |
| min.test | 9 | 0.002040s | 0.001932s | 0.001931s | byte equality observed |
| moment.test | 8 | 0.002025s | 0.001933s | 0.001985s | byte equality observed |
| p_square_cumul_dist.test | 1 | 0.660421s | 0.763381s | 0.626427s | byte equality observed |
| p_square_quantile.test | 1 | 0.023158s | 0.021789s | 0.020363s | byte equality observed |
| pot_quantile.test | 2 | 0.010692s | 0.009989s | 0.008223s | byte equality observed |
| reference.test | 8 | 0.002078s | 0.002019s | 0.001971s | byte equality observed |
| rolling_count.test | 9 | 0.002065s | 0.001957s | 0.001956s | byte equality observed |
| rolling_mean.test | 9 | 0.002176s | 0.002047s | 0.001984s | byte equality observed |
| rolling_moment.test | 8 | 0.002127s | 0.002012s | 0.002038s | byte equality observed |
| rolling_sum.test | 7 | 0.002064s | 0.001927s | 0.001945s | byte equality observed |
| rolling_variance.test | 8 | 0.002150s | 0.002015s | 0.001971s | byte equality observed |
| skewness.test | 5 | 0.003683s | 0.003419s | 0.002667s | byte equality observed |
| sum.test | 9 | 0.002009s | 0.001964s | 0.001892s | byte equality observed |
| sum_kahan.test | 1 | 0.011690s | 0.020392s | 0.010113s | byte equality observed |
| tail.test | 8 | 0.002095s | 0.001963s | 0.001930s | byte equality observed |
| tail_mean.test | 2 | 0.018563s | 0.019504s | 0.017816s | byte equality observed |
| tail_quantile.test | 1 | 0.021021s | 0.023321s | 0.019500s | byte equality observed |
| tail_variate_means.test | 8 | 0.001967s | 0.001879s | 0.001861s | byte equality observed |
| valarray.test | 9 | 0.001968s | 0.001856s | 0.001825s | byte equality observed |
| variance.test | 8 | 0.002034s | 0.001945s | 0.001929s | byte equality observed |
| vector.test | 7 | 0.001924s | 0.001872s | 0.001852s | byte equality observed |
| weighted_covariance.test | 9 | 0.001937s | 0.001856s | 0.001848s | byte equality observed |
| weighted_extended_p_square.test | 2 | 0.018273s | 0.018651s | 0.014884s | byte equality observed |
| weighted_kurtosis.test | 5 | 0.004286s | 0.003895s | 0.002609s | byte equality observed |
| weighted_mean.test | 5 | 0.001951s | 0.001847s | 0.001846s | byte equality observed |
| weighted_median.test | 1 | 0.894780s | 0.914664s | 0.841643s | byte equality observed |
| weighted_moment.test | 8 | 0.001889s | 0.001860s | 0.001770s | byte equality observed |
| weighted_p_square_cumul_dist.test | 1 | 0.103888s | 0.117991s | 0.088735s | byte equality observed |
| weighted_p_square_quantile.test | 1 | 0.027123s | 0.026096s | 0.020223s | byte equality observed |
| weighted_pot_quantile.test | 1 | 0.023136s | 0.023221s | 0.020096s | byte equality observed |
| weighted_skewness.test | 5 | 0.004447s | 0.004337s | 0.002608s | byte equality observed |
| weighted_sum.test | 9 | 0.001935s | 0.001879s | 0.001809s | byte equality observed |
| weighted_sum_kahan.test | 2 | 0.006720s | 0.010956s | 0.005884s | byte equality observed |
| weighted_tail_mean.test | 2 | 0.019100s | 0.020529s | 0.018599s | byte equality observed |
| weighted_tail_quantile.test | 1 | 0.020635s | 0.022511s | 0.019610s | byte equality observed |
| weighted_tail_variate_means.test | 8 | 0.001980s | 0.001919s | 0.001879s | byte equality observed |
| weighted_variance.test | 8 | 0.002302s | 0.002162s | 0.001903s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
