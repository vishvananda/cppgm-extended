# libs/accumulators/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| count.test | RUN | 0.775717 | 2.043371 | 2.343398 | 0.003499 | 0.003539 | 0.003194 |
| covariance.test | RUN | 1.451270 | 5.088216 | 5.943926 | 0.003399 | 0.003222 | 0.003325 |
| droppable.test | RUN | 0.948420 | 2.628276 | 3.137772 | 0.003247 | 0.003453 | 0.003399 |
| error_of.test | RUN | 1.075163 | 2.599460 | 3.460350 | 0.003148 | 0.003095 | 0.002965 |
| extended_p_square.test | RUN | 1.128222 | 3.332790 | 3.478602 | 0.004819 | 0.005021 | 0.004497 |
| extended_p_square_quantile.test | RUN | 1.440411 | 5.210765 | 5.042394 | 0.009022 | 0.009315 | 0.008069 |
| external_accumulator.test | RUN | 0.938284 | 2.520787 | 3.109749 | 0.003237 | 0.003412 | 0.003276 |
| external_weights.test | RUN | 0.833376 | 2.038202 | 2.396729 | 0.003040 | 0.002653 | 0.002915 |
| kurtosis.test | RUN | 1.262532 | 3.321414 | 4.111044 | 0.005290 | 0.004812 | 0.004182 |
| max.test | RUN | 0.790399 | 2.036246 | 2.276907 | 0.003013 | 0.003454 | 0.003081 |
| mean.test | RUN | 1.033586 | 2.848262 | 3.328943 | 0.003232 | 0.003474 | 0.003076 |
| median.test | RUN | 1.422362 | 5.397297 | 5.005387 | 0.115159 | 0.131725 | 0.109149 |
| min.test | RUN | 0.796080 | 1.897239 | 2.279339 | 0.003079 | 0.003361 | 0.003063 |
| moment.test | RUN | 0.927387 | 2.335259 | 2.732849 | 0.003163 | 0.003487 | 0.003114 |
| p_square_cumul_dist.test | RUN | 1.158189 | 3.882891 | 3.781967 | 0.886014 | 1.034569 | 0.805260 |
| p_square_quantile.test | RUN | 1.046833 | 3.480359 | 3.258354 | 0.030969 | 0.026426 | 0.024503 |
| reference.test | RUN | 0.901704 | 2.145202 | 2.593910 | 0.003272 | 0.003465 | 0.002889 |
| rolling_count.test | RUN | 0.894319 | 2.344400 | 2.827211 | 0.003068 | 0.003529 | 0.003063 |
| rolling_sum.test | RUN | 0.914435 | 2.382788 | 2.852129 | 0.003720 | 0.003400 | 0.003275 |
| rolling_mean.test | RUN | 1.236345 | 4.472121 | 4.588591 | 0.003200 | 0.003557 | 0.003139 |
| rolling_variance.test | RUN | 1.443953 | 5.094639 | 5.208957 | 0.003242 | 0.003416 | 0.003157 |
| rolling_moment.test | RUN | 1.083849 | 3.059913 | 3.479376 | 0.003259 | 0.003180 | 0.003423 |
| skewness.test | RUN | 1.192066 | 3.120010 | 3.869419 | 0.005311 | 0.005010 | 0.004326 |
| sum.test | RUN | 0.811674 | 2.155759 | 2.551522 | 0.003270 | 0.003466 | 0.003061 |
| sum_kahan.test | RUN | 0.929096 | 2.317177 | 2.680509 | 0.016815 | 0.027586 | 0.016824 |
| tail.test | RUN | 0.946657 | 3.184593 | 3.002847 | 0.003010 | 0.003508 | 0.003054 |
| tail_mean.test | RUN | 1.340546 | 3.709952 | 4.439608 | 0.022113 | 0.027988 | 0.022668 |
| tail_quantile.test | RUN | 1.094056 | 3.029888 | 3.443970 | 0.025569 | 0.031721 | 0.023437 |
| variance.test | RUN | 1.069916 | 2.771206 | 3.339341 | 0.003329 | 0.003185 | 0.003346 |
| vector.test | RUN | 1.082285 | 3.296845 | 3.998465 | 0.002900 | 0.003332 | 0.003019 |
| weighted_covariance.test | RUN | 1.491993 | 5.083834 | 6.075222 | 0.002916 | 0.002890 | 0.003057 |
| weighted_extended_p_square.test | RUN | 1.038384 | 2.949110 | 3.094329 | 0.023540 | 0.024723 | 0.018291 |
| weighted_kurtosis.test | RUN | 1.192277 | 2.984894 | 3.902798 | 0.005646 | 0.005547 | 0.004074 |
| weighted_mean.test | RUN | 1.028537 | 2.674588 | 3.359172 | 0.003032 | 0.002846 | 0.002889 |
| weighted_median.test | RUN | 1.303912 | 3.609411 | 4.347109 | 1.220598 | 1.180617 | 1.104919 |
| weighted_moment.test | RUN | 0.850344 | 2.040426 | 2.477388 | 0.003280 | 0.003241 | 0.003117 |
| weighted_p_square_cumul_dist.test | RUN | 1.025811 | 2.918742 | 3.097080 | 0.126117 | 0.137834 | 0.108555 |
| weighted_p_square_quantile.test | RUN | 0.971433 | 3.000740 | 2.885517 | 0.035407 | 0.031163 | 0.026340 |
| weighted_skewness.test | RUN | 1.063697 | 2.809082 | 3.402833 | 0.005550 | 0.005282 | 0.004048 |
| weighted_sum.test | RUN | 0.713766 | 1.695005 | 1.996651 | 0.002987 | 0.002848 | 0.003132 |
| weighted_sum_kahan.test | RUN | 0.760445 | 1.796617 | 2.121655 | 0.009340 | 0.015385 | 0.009945 |
| weighted_variance.test | RUN | 1.253143 | 3.220805 | 4.294114 | 0.003498 | 0.003619 | 0.002698 |
| valarray.test | RUN | 1.118512 | 3.148919 | 3.906061 | 0.003162 | 0.003229 | 0.002923 |
| tail_variate_means.test | RUN | 1.462466 | 4.930047 | 5.479728 | 0.003175 | 0.003226 | 0.003064 |
| pot_quantile.test | RUN | 1.835754 | 5.432883 | 6.578328 | 0.013211 | 0.012694 | 0.010800 |
| weighted_pot_quantile.test | RUN | 1.851564 | 5.405798 | 6.643686 | 0.027293 | 0.031078 | 0.025183 |
| weighted_tail_mean.test | RUN | 1.603671 | 4.238707 | 5.609519 | 0.022517 | 0.026095 | 0.022142 |
| weighted_tail_quantile.test | RUN | 1.611285 | 3.773302 | 4.974728 | 0.021528 | 0.022435 | 0.019434 |
| weighted_tail_variate_means.test | RUN | 1.676622 | 4.654860 | 5.559854 | 0.002328 | 0.002254 | 0.002134 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.133× | 1.127×, 1.140× | 0.998× |
| clang | 0.934× | 0.932×, 0.935× | 1.004× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1363 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| count.test | 1 | 0.002599s | 0.002567s | 0.002487s | byte equality observed |
| covariance.test | 1 | 0.002661s | 0.002602s | 0.002557s | byte equality observed |
| droppable.test | 1 | 0.002660s | 0.002485s | 0.002561s | byte equality observed |
| error_of.test | 1 | 0.002530s | 0.002393s | 0.002359s | byte equality observed |
| extended_p_square.test | 4 | 0.003800s | 0.003731s | 0.003443s | byte equality observed |
| extended_p_square_quantile.test | 3 | 0.007466s | 0.008004s | 0.006371s | byte equality observed |
| external_accumulator.test | 4 | 0.002209s | 0.002135s | 0.002159s | byte equality observed |
| external_weights.test | 7 | 0.002070s | 0.002019s | 0.002012s | byte equality observed |
| kurtosis.test | 5 | 0.004121s | 0.003817s | 0.003083s | byte equality observed |
| max.test | 6 | 0.002170s | 0.002102s | 0.002091s | byte equality observed |
| mean.test | 4 | 0.002212s | 0.002121s | 0.002127s | byte equality observed |
| median.test | 1 | 0.116194s | 0.143724s | 0.109478s | byte equality observed |
| min.test | 6 | 0.002150s | 0.002073s | 0.002073s | byte equality observed |
| moment.test | 7 | 0.002180s | 0.002058s | 0.002075s | byte equality observed |
| p_square_cumul_dist.test | 1 | 0.787584s | 1.012188s | 0.757922s | byte equality observed |
| p_square_quantile.test | 1 | 0.029478s | 0.027011s | 0.024507s | byte equality observed |
| pot_quantile.test | 2 | 0.012235s | 0.011524s | 0.009754s | byte equality observed |
| reference.test | 6 | 0.002201s | 0.002137s | 0.002108s | byte equality observed |
| rolling_count.test | 8 | 0.002172s | 0.002080s | 0.002104s | byte equality observed |
| rolling_mean.test | 8 | 0.002282s | 0.002197s | 0.002175s | byte equality observed |
| rolling_moment.test | 8 | 0.002272s | 0.002186s | 0.002183s | byte equality observed |
| rolling_sum.test | 7 | 0.002186s | 0.002082s | 0.002111s | byte equality observed |
| rolling_variance.test | 7 | 0.002273s | 0.002184s | 0.002185s | byte equality observed |
| skewness.test | 4 | 0.004020s | 0.003830s | 0.003218s | byte equality observed |
| sum.test | 6 | 0.002143s | 0.002090s | 0.002018s | byte equality observed |
| sum_kahan.test | 1 | 0.015809s | 0.026549s | 0.015572s | byte equality observed |
| tail.test | 9 | 0.002158s | 0.002013s | 0.002104s | byte equality observed |
| tail_mean.test | 1 | 0.021053s | 0.023141s | 0.021271s | byte equality observed |
| tail_quantile.test | 1 | 0.024083s | 0.028252s | 0.022964s | byte equality observed |
| tail_variate_means.test | 8 | 0.002090s | 0.001978s | 0.002001s | byte equality observed |
| valarray.test | 9 | 0.002087s | 0.001991s | 0.001987s | byte equality observed |
| variance.test | 8 | 0.002194s | 0.002103s | 0.002102s | byte equality observed |
| vector.test | 9 | 0.002076s | 0.001968s | 0.001955s | byte equality observed |
| weighted_covariance.test | 9 | 0.002081s | 0.001980s | 0.001988s | byte equality observed |
| weighted_extended_p_square.test | 1 | 0.022163s | 0.022679s | 0.018637s | byte equality observed |
| weighted_kurtosis.test | 4 | 0.004637s | 0.004292s | 0.002992s | byte equality observed |
| weighted_mean.test | 7 | 0.002051s | 0.001984s | 0.001939s | byte equality observed |
| weighted_median.test | 1 | 1.074340s | 1.122980s | 1.008167s | byte equality observed |
| weighted_moment.test | 6 | 0.002029s | 0.001926s | 0.001955s | byte equality observed |
| weighted_p_square_cumul_dist.test | 1 | 0.138761s | 0.157500s | 0.110112s | byte equality observed |
| weighted_p_square_quantile.test | 1 | 0.034366s | 0.033777s | 0.025904s | byte equality observed |
| weighted_pot_quantile.test | 1 | 0.027008s | 0.027563s | 0.024194s | byte equality observed |
| weighted_skewness.test | 4 | 0.004749s | 0.004445s | 0.003198s | byte equality observed |
| weighted_sum.test | 8 | 0.002062s | 0.001968s | 0.001947s | byte equality observed |
| weighted_sum_kahan.test | 2 | 0.008872s | 0.014022s | 0.008793s | byte equality observed |
| weighted_tail_mean.test | 1 | 0.021511s | 0.024054s | 0.021062s | byte equality observed |
| weighted_tail_quantile.test | 1 | 0.023923s | 0.026612s | 0.022875s | byte equality observed |
| weighted_tail_variate_means.test | 4 | 0.002181s | 0.002083s | 0.002137s | byte equality observed |
| weighted_variance.test | 8 | 0.002448s | 0.002330s | 0.002080s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
