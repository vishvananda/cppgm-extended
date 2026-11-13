# libs/accumulators/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| count.test | RUN | 0.967474 | 2.767443 | 3.507666 | 0.004387 | 0.003966 | 0.003872 |
| covariance.test | RUN | 1.781694 | 6.639100 | 8.874260 | 0.004319 | 0.004605 | 0.004046 |
| droppable.test | RUN | 1.166448 | 3.678310 | 4.723877 | 0.003817 | 0.004061 | 0.003874 |
| error_of.test | RUN | 1.298277 | 3.575751 | 4.996363 | 0.003561 | 0.003771 | 0.003623 |
| extended_p_square.test | RUN | 1.348001 | 4.475069 | 5.239056 | 0.005830 | 0.005973 | 0.005647 |
| extended_p_square_quantile.test | RUN | 1.757291 | 6.763388 | 7.329320 | 0.011537 | 0.012164 | 0.011003 |
| external_accumulator.test | RUN | 1.178563 | 3.476715 | 4.579185 | 0.003906 | 0.003739 | 0.004095 |
| external_weights.test | RUN | 1.022431 | 2.777943 | 3.600807 | 0.003572 | 0.004028 | 0.004001 |
| kurtosis.test | RUN | 1.547211 | 4.351174 | 6.117521 | 0.007159 | 0.006206 | 0.005085 |
| max.test | RUN | 0.959787 | 2.730021 | 3.423561 | 0.003725 | 0.004003 | 0.003875 |
| mean.test | RUN | 1.280488 | 3.709328 | 4.869312 | 0.004044 | 0.003860 | 0.003893 |
| median.test | RUN | 1.759199 | 6.979286 | 7.407826 | 0.165208 | 0.169824 | 0.149374 |
| min.test | RUN | 0.948077 | 2.655178 | 3.493399 | 0.003624 | 0.003724 | 0.003892 |
| moment.test | RUN | 1.088942 | 3.100443 | 4.033318 | 0.004208 | 0.003858 | 0.003500 |
| p_square_cumul_dist.test | RUN | 1.418733 | 5.245644 | 5.596668 | 1.350426 | 1.292246 | 1.080899 |
| p_square_quantile.test | RUN | 1.289664 | 4.579201 | 4.888995 | 0.037387 | 0.035809 | 0.033606 |
| reference.test | RUN | 1.070905 | 2.861568 | 4.092598 | 0.003715 | 0.003736 | 0.003854 |
| rolling_count.test | RUN | 1.086518 | 3.342945 | 4.154459 | 0.003702 | 0.003826 | 0.004134 |
| rolling_sum.test | RUN | 1.104510 | 3.357348 | 3.847068 | 0.004026 | 0.003705 | 0.003772 |
| rolling_mean.test | RUN | 1.479552 | 5.928397 | 6.764831 | 0.003965 | 0.003305 | 0.004130 |
| rolling_variance.test | RUN | 1.787660 | 6.725323 | 7.682102 | 0.004171 | 0.003844 | 0.004151 |
| rolling_moment.test | RUN | 1.308695 | 4.042922 | 5.206942 | 0.004662 | 0.004013 | 0.004331 |
| skewness.test | RUN | 1.434242 | 4.219705 | 5.756058 | 0.006750 | 0.005871 | 0.005415 |
| sum.test | RUN | 1.003186 | 2.870514 | 3.853303 | 0.003747 | 0.003772 | 0.003985 |
| sum_kahan.test | RUN | 1.097233 | 2.988533 | 3.854982 | 0.019235 | 0.030799 | 0.019727 |
| tail.test | RUN | 1.165106 | 4.398366 | 4.460190 | 0.003957 | 0.004013 | 0.003537 |
| tail_mean.test | RUN | 1.637413 | 5.084754 | 6.654114 | 0.026884 | 0.035538 | 0.026518 |
| tail_quantile.test | RUN | 1.365627 | 3.953676 | 5.106231 | 0.033610 | 0.042358 | 0.031501 |
| variance.test | RUN | 1.291582 | 3.649185 | 4.965851 | 0.003813 | 0.003479 | 0.004084 |
| vector.test | RUN | 1.401196 | 4.459054 | 5.942069 | 0.003709 | 0.003773 | 0.004273 |
| weighted_covariance.test | RUN | 1.851975 | 6.845578 | 9.016967 | 0.003615 | 0.003719 | 0.003860 |
| weighted_extended_p_square.test | RUN | 1.259417 | 4.049651 | 4.609027 | 0.031919 | 0.032073 | 0.026716 |
| weighted_kurtosis.test | RUN | 1.497080 | 4.081211 | 5.891538 | 0.006669 | 0.006055 | 0.005353 |
| weighted_mean.test | RUN | 1.271402 | 3.631752 | 5.045681 | 0.003523 | 0.003410 | 0.003738 |
| weighted_median.test | RUN | 1.646102 | 5.003650 | 6.478048 | 1.489990 | 1.391749 | 1.307693 |
| weighted_moment.test | RUN | 0.999099 | 2.720165 | 3.826737 | 0.003834 | 0.003476 | 0.003682 |
| weighted_p_square_cumul_dist.test | RUN | 1.258148 | 4.055345 | 4.642932 | 0.176958 | 0.165678 | 0.137962 |
| weighted_p_square_quantile.test | RUN | 1.190177 | 3.979696 | 4.255085 | 0.046019 | 0.041841 | 0.036881 |
| weighted_skewness.test | RUN | 1.404080 | 3.880378 | 4.980949 | 0.006994 | 0.006612 | 0.005720 |
| weighted_sum.test | RUN | 0.876751 | 2.278385 | 2.990444 | 0.003481 | 0.003954 | 0.003656 |
| weighted_sum_kahan.test | RUN | 0.797441 | 2.499147 | 3.348307 | 0.010626 | 0.017020 | 0.011643 |
| weighted_variance.test | RUN | 1.552518 | 4.435337 | 6.342816 | 0.003981 | 0.003874 | 0.004119 |
| valarray.test | RUN | 1.443967 | 4.185838 | 5.908744 | 0.003768 | 0.003509 | 0.003792 |
| tail_variate_means.test | RUN | 1.830194 | 6.559698 | 7.980527 | 0.003574 | 0.003793 | 0.003671 |
| pot_quantile.test | RUN | 2.213669 | 7.497989 | 9.752337 | 0.019074 | 0.015824 | 0.014009 |
| weighted_pot_quantile.test | RUN | 2.249991 | 7.022162 | 9.881419 | 0.032069 | 0.040918 | 0.033462 |
| weighted_tail_mean.test | RUN | 1.977629 | 5.762404 | 8.300390 | 0.028608 | 0.036612 | 0.028309 |
| weighted_tail_quantile.test | RUN | 2.042821 | 5.053425 | 7.433712 | 0.026630 | 0.028299 | 0.023247 |
| weighted_tail_variate_means.test | RUN | 2.244113 | 5.701449 | 7.986723 | 0.002564 | 0.002382 | 0.002497 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.043× | 1.037×, 1.048× | 0.985× |
| clang | 0.876× | 0.872×, 0.881× | 1.005× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43348824563).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| count.test | 1 | 0.002979s | 0.002934s | 0.002691s | byte equality observed |
| covariance.test | 1 | 0.003035s | 0.003006s | 0.002742s | byte equality observed |
| droppable.test | 1 | 0.002999s | 0.002935s | 0.002763s | byte equality observed |
| error_of.test | 1 | 0.002791s | 0.002760s | 0.002898s | byte equality observed |
| extended_p_square.test | 4 | 0.004729s | 0.004593s | 0.004254s | byte equality observed |
| extended_p_square_quantile.test | 2 | 0.010176s | 0.010156s | 0.008739s | byte equality observed |
| external_accumulator.test | 5 | 0.002687s | 0.002640s | 0.002635s | byte equality observed |
| external_weights.test | 6 | 0.002464s | 0.002414s | 0.002450s | byte equality observed |
| kurtosis.test | 3 | 0.005738s | 0.004792s | 0.004005s | byte equality observed |
| max.test | 5 | 0.002622s | 0.002503s | 0.002463s | byte equality observed |
| mean.test | 6 | 0.002647s | 0.002614s | 0.002610s | byte equality observed |
| median.test | 1 | 0.160436s | 0.166945s | 0.141030s | byte equality observed |
| min.test | 6 | 0.002586s | 0.002542s | 0.002454s | byte equality observed |
| moment.test | 3 | 0.002613s | 0.002537s | 0.002502s | byte equality observed |
| p_square_cumul_dist.test | 1 | 1.062936s | 1.165171s | 0.908869s | byte equality observed |
| p_square_quantile.test | 1 | 0.036872s | 0.035134s | 0.030702s | byte equality observed |
| pot_quantile.test | 2 | 0.017461s | 0.013438s | 0.011968s | byte equality observed |
| reference.test | 6 | 0.002566s | 0.002442s | 0.002464s | byte equality observed |
| rolling_count.test | 7 | 0.002601s | 0.002458s | 0.002469s | byte equality observed |
| rolling_mean.test | 7 | 0.002667s | 0.002618s | 0.002591s | byte equality observed |
| rolling_moment.test | 7 | 0.002680s | 0.002602s | 0.002568s | byte equality observed |
| rolling_sum.test | 7 | 0.002584s | 0.002493s | 0.002455s | byte equality observed |
| rolling_variance.test | 7 | 0.002712s | 0.002636s | 0.002588s | byte equality observed |
| skewness.test | 4 | 0.005658s | 0.004604s | 0.003976s | byte equality observed |
| sum.test | 6 | 0.002558s | 0.002498s | 0.002450s | byte equality observed |
| sum_kahan.test | 1 | 0.017991s | 0.029977s | 0.018333s | byte equality observed |
| tail.test | 6 | 0.002588s | 0.002435s | 0.002452s | byte equality observed |
| tail_mean.test | 1 | 0.023515s | 0.028920s | 0.026257s | byte equality observed |
| tail_quantile.test | 1 | 0.030436s | 0.036910s | 0.029073s | byte equality observed |
| tail_variate_means.test | 7 | 0.002555s | 0.002346s | 0.002501s | byte equality observed |
| valarray.test | 6 | 0.002508s | 0.002398s | 0.002377s | byte equality observed |
| variance.test | 7 | 0.002632s | 0.002553s | 0.002585s | byte equality observed |
| vector.test | 7 | 0.002525s | 0.002475s | 0.002503s | byte equality observed |
| weighted_covariance.test | 7 | 0.002565s | 0.002409s | 0.002379s | byte equality observed |
| weighted_extended_p_square.test | 1 | 0.030286s | 0.029989s | 0.024878s | byte equality observed |
| weighted_kurtosis.test | 4 | 0.006233s | 0.005109s | 0.003807s | byte equality observed |
| weighted_mean.test | 8 | 0.002447s | 0.002312s | 0.002304s | byte equality observed |
| weighted_median.test | 1 | 1.264040s | 1.250967s | 1.143122s | byte equality observed |
| weighted_moment.test | 7 | 0.002396s | 0.002319s | 0.002307s | byte equality observed |
| weighted_p_square_cumul_dist.test | 1 | 0.167969s | 0.198695s | 0.142493s | byte equality observed |
| weighted_p_square_quantile.test | 1 | 0.043923s | 0.039144s | 0.032897s | byte equality observed |
| weighted_pot_quantile.test | 1 | 0.035435s | 0.032208s | 0.028916s | byte equality observed |
| weighted_skewness.test | 4 | 0.006123s | 0.004993s | 0.003772s | byte equality observed |
| weighted_sum.test | 8 | 0.002380s | 0.002355s | 0.002303s | byte equality observed |
| weighted_sum_kahan.test | 2 | 0.010140s | 0.015775s | 0.009963s | byte equality observed |
| weighted_tail_mean.test | 1 | 0.027010s | 0.027059s | 0.024547s | byte equality observed |
| weighted_tail_quantile.test | 1 | 0.031348s | 0.032161s | 0.027553s | byte equality observed |
| weighted_tail_variate_means.test | 7 | 0.002537s | 0.002357s | 0.002394s | byte equality observed |
| weighted_variance.test | 6 | 0.003073s | 0.002752s | 0.002620s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43348824563).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
