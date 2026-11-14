# libs/accumulators/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| count.test | RUN | 0.627317 | 1.524724 | 1.520412 | 0.003462 | 0.002979 | 0.002449 |
| covariance.test | RUN | 1.203046 | 4.118558 | 4.031862 | 0.003431 | 0.003000 | 0.002947 |
| droppable.test | RUN | 0.772974 | 2.081945 | 2.080586 | 0.003211 | 0.003161 | 0.002768 |
| error_of.test | RUN | 0.844543 | 2.094994 | 2.275981 | 0.002831 | 0.002633 | 0.002580 |
| extended_p_square.test | RUN | 0.898874 | 2.585344 | 2.376379 | 0.004795 | 0.004290 | 0.003809 |
| extended_p_square_quantile.test | RUN | 1.190613 | 4.196641 | 3.320618 | 0.007281 | 0.007247 | 0.006242 |
| external_accumulator.test | RUN | 0.770325 | 1.947646 | 2.033797 | 0.003580 | 0.002897 | 0.002502 |
| external_weights.test | RUN | 0.674121 | 1.542042 | 1.648271 | 0.002974 | 0.002740 | 0.002446 |
| kurtosis.test | RUN | 0.980733 | 2.456795 | 2.686941 | 0.005133 | 0.004224 | 0.003304 |
| max.test | RUN | 0.615324 | 1.483886 | 1.514625 | 0.003025 | 0.002712 | 0.002642 |
| mean.test | RUN | 0.799936 | 2.088027 | 2.192843 | 0.003012 | 0.003070 | 0.002712 |
| median.test | RUN | 1.063022 | 4.306818 | 3.401792 | 0.094203 | 0.095826 | 0.084107 |
| min.test | RUN | 0.578668 | 1.485030 | 1.531937 | 0.003046 | 0.003049 | 0.002628 |
| moment.test | RUN | 0.656107 | 1.757146 | 1.796127 | 0.003311 | 0.002939 | 0.002714 |
| p_square_cumul_dist.test | RUN | 0.890373 | 3.025237 | 2.575190 | 0.711993 | 0.757423 | 0.607426 |
| p_square_quantile.test | RUN | 0.788182 | 2.653802 | 2.143593 | 0.023522 | 0.021614 | 0.018617 |
| reference.test | RUN | 0.681070 | 1.726886 | 1.814613 | 0.003113 | 0.002925 | 0.002575 |
| rolling_count.test | RUN | 0.679227 | 1.915337 | 1.792537 | 0.002740 | 0.002918 | 0.002714 |
| rolling_sum.test | RUN | 0.695949 | 1.910661 | 1.847901 | 0.003135 | 0.002833 | 0.002739 |
| rolling_mean.test | RUN | 0.995097 | 3.464511 | 3.084241 | 0.003096 | 0.003234 | 0.002831 |
| rolling_variance.test | RUN | 1.117336 | 4.022124 | 3.507698 | 0.002992 | 0.003118 | 0.002827 |
| rolling_moment.test | RUN | 0.863489 | 2.341392 | 2.312404 | 0.002860 | 0.003178 | 0.002793 |
| skewness.test | RUN | 0.889506 | 2.373379 | 2.619784 | 0.005377 | 0.004282 | 0.003426 |
| sum.test | RUN | 0.607257 | 1.636658 | 1.678616 | 0.003136 | 0.002986 | 0.002649 |
| sum_kahan.test | RUN | 0.665559 | 1.765535 | 1.769183 | 0.012424 | 0.020291 | 0.010117 |
| tail.test | RUN | 0.732349 | 2.445609 | 2.019759 | 0.003219 | 0.002953 | 0.002582 |
| tail_mean.test | RUN | 1.062826 | 2.812930 | 2.921659 | 0.018627 | 0.021088 | 0.017674 |
| tail_quantile.test | RUN | 0.838026 | 2.316427 | 2.300068 | 0.022536 | 0.024870 | 0.018735 |
| variance.test | RUN | 0.808479 | 2.065574 | 2.274662 | 0.003062 | 0.002966 | 0.002732 |
| vector.test | RUN | 0.883408 | 2.965688 | 2.694865 | 0.003044 | 0.006694 | 0.002547 |
| weighted_covariance.test | RUN | 1.169716 | 4.849302 | 4.117550 | 0.002821 | 0.002868 | 0.002539 |
| weighted_extended_p_square.test | RUN | 0.789136 | 3.070707 | 2.098637 | 0.019288 | 0.019324 | 0.014878 |
| weighted_kurtosis.test | RUN | 0.920267 | 3.122156 | 2.529710 | 0.005254 | 0.004866 | 0.003264 |
| weighted_mean.test | RUN | 0.784086 | 2.112183 | 2.235185 | 0.002777 | 0.002782 | 0.002405 |
| weighted_median.test | RUN | 1.012433 | 2.721253 | 2.901490 | 0.932978 | 0.895666 | 0.837366 |
| weighted_moment.test | RUN | 0.636077 | 1.538889 | 1.650070 | 0.003121 | 0.002863 | 0.002541 |
| weighted_p_square_cumul_dist.test | RUN | 0.781959 | 2.258596 | 2.074909 | 0.104244 | 0.096420 | 0.082143 |
| weighted_p_square_quantile.test | RUN | 0.727419 | 2.273504 | 1.724424 | 0.029342 | 0.023945 | 0.018996 |
| weighted_skewness.test | RUN | 0.905262 | 2.147235 | 2.435613 | 0.005480 | 0.004626 | 0.003012 |
| weighted_sum.test | RUN | 0.510738 | 1.284740 | 1.300964 | 0.002639 | 0.002597 | 0.002525 |
| weighted_sum_kahan.test | RUN | 0.587901 | 1.382910 | 1.393260 | 0.007368 | 0.011156 | 0.006373 |
| weighted_variance.test | RUN | 1.001085 | 2.514223 | 2.815638 | 0.003721 | 0.003104 | 0.002689 |
| valarray.test | RUN | 0.942294 | 2.389163 | 2.613302 | 0.003127 | 0.002778 | 0.002520 |
| tail_variate_means.test | RUN | 1.208679 | 3.801137 | 3.717398 | 0.003027 | 0.002697 | 0.002551 |
| pot_quantile.test | RUN | 1.483962 | 4.211035 | 4.319255 | 0.014527 | 0.010065 | 0.008218 |
| weighted_pot_quantile.test | RUN | 1.493373 | 4.119897 | 4.390149 | 0.031638 | 0.023483 | 0.019910 |
| weighted_tail_mean.test | RUN | 1.303275 | 3.318233 | 3.596388 | 0.021018 | 0.019944 | 0.018111 |
| weighted_tail_quantile.test | RUN | 1.260881 | 2.894549 | 3.258609 | 0.020418 | 0.017189 | 0.014501 |
| weighted_tail_variate_means.test | RUN | 1.462539 | 3.526254 | 3.864587 | 0.002376 | 0.001970 | 0.001854 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.035× | 1.022×, 1.048× | 0.991× |
| clang | 0.888× | 0.899×, 0.878× | 1.005× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1364 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43572026855).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| count.test | 1 | 0.002267s | 0.002214s | 0.002056s | byte equality observed |
| covariance.test | 1 | 0.002273s | 0.002201s | 0.002145s | byte equality observed |
| droppable.test | 1 | 0.002254s | 0.002220s | 0.002150s | byte equality observed |
| error_of.test | 1 | 0.002134s | 0.002076s | 0.001989s | byte equality observed |
| extended_p_square.test | 3 | 0.002975s | 0.002960s | 0.002756s | byte equality observed |
| extended_p_square_quantile.test | 3 | 0.005184s | 0.005702s | 0.004873s | byte equality observed |
| external_accumulator.test | 6 | 0.001797s | 0.001759s | 0.001745s | byte equality observed |
| external_weights.test | 6 | 0.001685s | 0.001657s | 0.001671s | byte equality observed |
| kurtosis.test | 5 | 0.003844s | 0.003111s | 0.002381s | byte equality observed |
| max.test | 10 | 0.001760s | 0.001719s | 0.001707s | byte equality observed |
| mean.test | 9 | 0.001779s | 0.001726s | 0.001717s | byte equality observed |
| median.test | 1 | 0.088838s | 0.091839s | 0.080591s | byte equality observed |
| min.test | 8 | 0.001757s | 0.001721s | 0.001692s | byte equality observed |
| moment.test | 8 | 0.001771s | 0.001752s | 0.001739s | byte equality observed |
| p_square_cumul_dist.test | 1 | 0.645220s | 0.703920s | 0.566572s | byte equality observed |
| p_square_quantile.test | 1 | 0.020994s | 0.020154s | 0.018519s | byte equality observed |
| pot_quantile.test | 2 | 0.012188s | 0.009054s | 0.007454s | byte equality observed |
| reference.test | 9 | 0.001793s | 0.001750s | 0.001749s | byte equality observed |
| rolling_count.test | 10 | 0.001803s | 0.001732s | 0.001755s | byte equality observed |
| rolling_mean.test | 10 | 0.001852s | 0.001809s | 0.001784s | byte equality observed |
| rolling_moment.test | 9 | 0.001831s | 0.001785s | 0.001809s | byte equality observed |
| rolling_sum.test | 8 | 0.001789s | 0.001728s | 0.001726s | byte equality observed |
| rolling_variance.test | 9 | 0.001876s | 0.001803s | 0.001789s | byte equality observed |
| skewness.test | 3 | 0.003830s | 0.003124s | 0.002384s | byte equality observed |
| sum.test | 5 | 0.001767s | 0.001700s | 0.001690s | byte equality observed |
| sum_kahan.test | 2 | 0.010568s | 0.018570s | 0.009183s | byte equality observed |
| tail.test | 6 | 0.001749s | 0.001724s | 0.001696s | byte equality observed |
| tail_mean.test | 2 | 0.016363s | 0.017542s | 0.016289s | byte equality observed |
| tail_quantile.test | 1 | 0.020389s | 0.021026s | 0.017456s | byte equality observed |
| tail_variate_means.test | 9 | 0.001726s | 0.001654s | 0.001680s | byte equality observed |
| valarray.test | 10 | 0.001692s | 0.001637s | 0.001642s | byte equality observed |
| variance.test | 10 | 0.001775s | 0.001714s | 0.001702s | byte equality observed |
| vector.test | 9 | 0.001696s | 0.001657s | 0.001629s | byte equality observed |
| weighted_covariance.test | 9 | 0.001711s | 0.001646s | 0.001644s | byte equality observed |
| weighted_extended_p_square.test | 1 | 0.018488s | 0.018016s | 0.013875s | byte equality observed |
| weighted_kurtosis.test | 5 | 0.004247s | 0.003525s | 0.002392s | byte equality observed |
| weighted_mean.test | 8 | 0.001693s | 0.001637s | 0.001639s | byte equality observed |
| weighted_median.test | 1 | 0.838568s | 0.845109s | 0.775208s | byte equality observed |
| weighted_moment.test | 10 | 0.001683s | 0.001629s | 0.001615s | byte equality observed |
| weighted_p_square_cumul_dist.test | 1 | 0.105578s | 0.109540s | 0.086591s | byte equality observed |
| weighted_p_square_quantile.test | 1 | 0.026859s | 0.024498s | 0.018398s | byte equality observed |
| weighted_pot_quantile.test | 1 | 0.023581s | 0.020680s | 0.018334s | byte equality observed |
| weighted_skewness.test | 5 | 0.004294s | 0.003497s | 0.002325s | byte equality observed |
| weighted_sum.test | 8 | 0.001685s | 0.001637s | 0.001595s | byte equality observed |
| weighted_sum_kahan.test | 2 | 0.006236s | 0.010201s | 0.005351s | byte equality observed |
| weighted_tail_mean.test | 2 | 0.017886s | 0.018419s | 0.016787s | byte equality observed |
| weighted_tail_quantile.test | 1 | 0.020514s | 0.020622s | 0.017467s | byte equality observed |
| weighted_tail_variate_means.test | 9 | 0.001724s | 0.001656s | 0.001657s | byte equality observed |
| weighted_variance.test | 9 | 0.002104s | 0.001903s | 0.001752s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43572026855).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
