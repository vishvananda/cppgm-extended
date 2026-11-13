# libs/accumulators/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| count.test | RUN | 0.510570 | 1.438738 | 1.561919 | 0.002976 | 0.002902 | 0.002574 |
| covariance.test | RUN | 1.018515 | 3.814823 | 4.147364 | 0.002809 | 0.002869 | 0.002751 |
| droppable.test | RUN | 0.634107 | 1.920626 | 2.095496 | 0.003838 | 0.002864 | 0.002962 |
| error_of.test | RUN | 0.698807 | 1.874668 | 2.265031 | 0.002452 | 0.002687 | 0.002473 |
| extended_p_square.test | RUN | 0.739969 | 2.423842 | 2.331904 | 0.004058 | 0.004027 | 0.003621 |
| extended_p_square_quantile.test | RUN | 0.961952 | 3.802456 | 3.447671 | 0.006816 | 0.007131 | 0.005575 |
| external_accumulator.test | RUN | 0.631887 | 1.759969 | 2.040092 | 0.002649 | 0.002607 | 0.002826 |
| external_weights.test | RUN | 0.547262 | 1.469207 | 1.590899 | 0.002625 | 0.002597 | 0.002379 |
| kurtosis.test | RUN | 0.831330 | 2.348200 | 2.668654 | 0.004733 | 0.004080 | 0.003317 |
| max.test | RUN | 0.517513 | 1.387181 | 1.552561 | 0.002944 | 0.002963 | 0.002644 |
| mean.test | RUN | 0.690183 | 2.033927 | 2.235442 | 0.002793 | 0.002713 | 0.002702 |
| median.test | RUN | 0.975388 | 3.925814 | 3.431451 | 0.092336 | 0.089707 | 0.085548 |
| min.test | RUN | 0.529131 | 1.431420 | 1.526219 | 0.002691 | 0.002806 | 0.002704 |
| moment.test | RUN | 0.602955 | 1.663473 | 1.777790 | 0.002616 | 0.002732 | 0.002686 |
| p_square_cumul_dist.test | RUN | 0.785830 | 2.758189 | 2.616542 | 0.700994 | 0.712538 | 0.601871 |
| p_square_quantile.test | RUN | 0.682757 | 2.450992 | 2.118119 | 0.021705 | 0.020093 | 0.018312 |
| reference.test | RUN | 0.585554 | 1.601701 | 1.744652 | 0.002903 | 0.002528 | 0.002483 |
| rolling_count.test | RUN | 0.608337 | 1.816132 | 1.905592 | 0.002447 | 0.002830 | 0.002734 |
| rolling_sum.test | RUN | 0.596142 | 1.830790 | 1.904083 | 0.002859 | 0.002729 | 0.002746 |
| rolling_mean.test | RUN | 0.861453 | 3.187604 | 3.007041 | 0.003004 | 0.002762 | 0.002860 |
| rolling_variance.test | RUN | 0.943910 | 3.757572 | 3.479018 | 0.002693 | 0.002668 | 0.002847 |
| rolling_moment.test | RUN | 0.754260 | 2.192857 | 2.309825 | 0.002969 | 0.002950 | 0.002754 |
| skewness.test | RUN | 0.798196 | 2.268272 | 2.528885 | 0.004854 | 0.003880 | 0.003445 |
| sum.test | RUN | 0.557975 | 1.570002 | 1.647157 | 0.002901 | 0.002803 | 0.002701 |
| sum_kahan.test | RUN | 0.586725 | 1.644845 | 1.782947 | 0.011242 | 0.019410 | 0.010188 |
| tail.test | RUN | 0.644651 | 2.281929 | 2.020795 | 0.003441 | 0.002851 | 0.002623 |
| tail_mean.test | RUN | 0.894456 | 2.677799 | 2.885680 | 0.017258 | 0.021220 | 0.017369 |
| tail_quantile.test | RUN | 0.729482 | 2.105040 | 2.266795 | 0.020911 | 0.022731 | 0.018526 |
| variance.test | RUN | 0.687444 | 1.949188 | 2.189193 | 0.002639 | 0.002763 | 0.002726 |
| vector.test | RUN | 0.757323 | 2.397388 | 2.743152 | 0.002585 | 0.002510 | 0.002389 |
| weighted_covariance.test | RUN | 1.003868 | 3.692236 | 3.971701 | 0.002677 | 0.002700 | 0.002171 |
| weighted_extended_p_square.test | RUN | 0.691148 | 2.198598 | 2.086136 | 0.018854 | 0.018113 | 0.014857 |
| weighted_kurtosis.test | RUN | 0.792175 | 2.201469 | 2.571662 | 0.005100 | 0.003973 | 0.003310 |
| weighted_mean.test | RUN | 0.686397 | 1.900513 | 2.217072 | 0.002449 | 0.002565 | 0.002343 |
| weighted_median.test | RUN | 0.877030 | 2.608021 | 2.860660 | 0.877775 | 0.887420 | 0.859646 |
| weighted_moment.test | RUN | 0.552652 | 1.465819 | 1.609335 | 0.002713 | 0.002678 | 0.002720 |
| weighted_p_square_cumul_dist.test | RUN | 0.684699 | 2.182430 | 2.084693 | 0.095181 | 0.098213 | 0.087796 |
| weighted_p_square_quantile.test | RUN | 0.649204 | 2.142556 | 1.923020 | 0.027519 | 0.023452 | 0.020589 |
| weighted_skewness.test | RUN | 0.723770 | 1.981236 | 2.374842 | 0.004864 | 0.004298 | 0.003402 |
| weighted_sum.test | RUN | 0.463556 | 1.219573 | 1.429410 | 0.002444 | 0.002777 | 0.002613 |
| weighted_sum_kahan.test | RUN | 0.507449 | 1.334576 | 1.515841 | 0.006722 | 0.011092 | 0.006646 |
| weighted_variance.test | RUN | 0.851904 | 2.378967 | 2.928959 | 0.002878 | 0.002936 | 0.002757 |
| valarray.test | RUN | 0.787914 | 2.268508 | 2.687257 | 0.002607 | 0.002457 | 0.002554 |
| tail_variate_means.test | RUN | 1.028275 | 3.609263 | 3.724316 | 0.002562 | 0.002801 | 0.002723 |
| pot_quantile.test | RUN | 1.251947 | 4.006311 | 4.429228 | 0.013249 | 0.010286 | 0.008288 |
| weighted_pot_quantile.test | RUN | 1.220649 | 3.938627 | 4.412051 | 0.024188 | 0.022662 | 0.019457 |
| weighted_tail_mean.test | RUN | 1.096562 | 3.161528 | 3.670714 | 0.019083 | 0.020855 | 0.017643 |
| weighted_tail_quantile.test | RUN | 1.092585 | 2.713229 | 3.235275 | 0.017586 | 0.017013 | 0.014597 |
| weighted_tail_variate_means.test | RUN | 1.194957 | 3.434419 | 3.668722 | 0.001974 | 0.001854 | 0.001816 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.037× | 1.041×, 1.033× | 1.008× |
| clang | 0.894× | 0.893×, 0.894× | 1.016× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1364 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43493409407).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| count.test | 1 | 0.002256s | 0.002147s | 0.002073s | byte equality observed |
| covariance.test | 1 | 0.002196s | 0.002124s | 0.002056s | byte equality observed |
| droppable.test | 1 | 0.001982s | 0.002131s | 0.001906s | byte equality observed |
| error_of.test | 1 | 0.002074s | 0.002048s | 0.001895s | byte equality observed |
| extended_p_square.test | 6 | 0.002925s | 0.002848s | 0.002757s | byte equality observed |
| extended_p_square_quantile.test | 3 | 0.005330s | 0.005654s | 0.004797s | byte equality observed |
| external_accumulator.test | 7 | 0.001786s | 0.001747s | 0.001713s | byte equality observed |
| external_weights.test | 7 | 0.001702s | 0.001624s | 0.001597s | byte equality observed |
| kurtosis.test | 5 | 0.003790s | 0.003081s | 0.002329s | byte equality observed |
| max.test | 10 | 0.001762s | 0.001710s | 0.001682s | byte equality observed |
| mean.test | 8 | 0.001760s | 0.001706s | 0.001719s | byte equality observed |
| median.test | 1 | 0.092926s | 0.098558s | 0.082077s | byte equality observed |
| min.test | 5 | 0.001773s | 0.001678s | 0.001690s | byte equality observed |
| moment.test | 7 | 0.001762s | 0.001718s | 0.001698s | byte equality observed |
| p_square_cumul_dist.test | 1 | 0.649115s | 0.701137s | 0.571554s | byte equality observed |
| p_square_quantile.test | 1 | 0.021405s | 0.018625s | 0.017201s | byte equality observed |
| pot_quantile.test | 2 | 0.012094s | 0.008862s | 0.007068s | byte equality observed |
| reference.test | 10 | 0.001758s | 0.001710s | 0.001693s | byte equality observed |
| rolling_count.test | 10 | 0.001756s | 0.001674s | 0.001688s | byte equality observed |
| rolling_mean.test | 9 | 0.001834s | 0.001769s | 0.001780s | byte equality observed |
| rolling_moment.test | 7 | 0.001835s | 0.001773s | 0.001697s | byte equality observed |
| rolling_sum.test | 9 | 0.001770s | 0.001688s | 0.001700s | byte equality observed |
| rolling_variance.test | 9 | 0.001851s | 0.001765s | 0.001754s | byte equality observed |
| skewness.test | 5 | 0.003749s | 0.003088s | 0.002361s | byte equality observed |
| sum.test | 9 | 0.001755s | 0.001676s | 0.001689s | byte equality observed |
| sum_kahan.test | 1 | 0.010391s | 0.018538s | 0.009142s | byte equality observed |
| tail.test | 10 | 0.001759s | 0.001691s | 0.001663s | byte equality observed |
| tail_mean.test | 2 | 0.016452s | 0.017916s | 0.016148s | byte equality observed |
| tail_quantile.test | 1 | 0.019731s | 0.020731s | 0.017518s | byte equality observed |
| tail_variate_means.test | 5 | 0.001729s | 0.001637s | 0.001668s | byte equality observed |
| valarray.test | 7 | 0.001690s | 0.001623s | 0.001626s | byte equality observed |
| variance.test | 8 | 0.001762s | 0.001723s | 0.001682s | byte equality observed |
| vector.test | 8 | 0.001681s | 0.001634s | 0.001601s | byte equality observed |
| weighted_covariance.test | 10 | 0.001678s | 0.001621s | 0.001612s | byte equality observed |
| weighted_extended_p_square.test | 2 | 0.017736s | 0.017699s | 0.013953s | byte equality observed |
| weighted_kurtosis.test | 5 | 0.004218s | 0.003473s | 0.002353s | byte equality observed |
| weighted_mean.test | 8 | 0.001674s | 0.001618s | 0.001589s | byte equality observed |
| weighted_median.test | 1 | 0.827268s | 0.835600s | 0.768672s | byte equality observed |
| weighted_moment.test | 3 | 0.001685s | 0.001643s | 0.001626s | byte equality observed |
| weighted_p_square_cumul_dist.test | 1 | 0.098336s | 0.111698s | 0.085596s | byte equality observed |
| weighted_p_square_quantile.test | 1 | 0.027015s | 0.023838s | 0.018246s | byte equality observed |
| weighted_pot_quantile.test | 1 | 0.023525s | 0.020634s | 0.017943s | byte equality observed |
| weighted_skewness.test | 5 | 0.004256s | 0.003489s | 0.002283s | byte equality observed |
| weighted_sum.test | 10 | 0.001663s | 0.001619s | 0.001571s | byte equality observed |
| weighted_sum_kahan.test | 2 | 0.006014s | 0.010190s | 0.005318s | byte equality observed |
| weighted_tail_mean.test | 2 | 0.017936s | 0.018464s | 0.016731s | byte equality observed |
| weighted_tail_quantile.test | 1 | 0.020554s | 0.020793s | 0.017077s | byte equality observed |
| weighted_tail_variate_means.test | 10 | 0.001705s | 0.001656s | 0.001620s | byte equality observed |
| weighted_variance.test | 9 | 0.002086s | 0.001902s | 0.001732s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43493409407).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
