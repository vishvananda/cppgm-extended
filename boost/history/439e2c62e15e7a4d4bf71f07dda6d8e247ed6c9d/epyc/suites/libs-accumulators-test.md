# libs/accumulators/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| count.test | RUN | 0.562929 | 1.470440 | 1.446768 | 0.003672 | 0.003523 | 0.003596 |
| covariance.test | RUN | 1.042754 | 4.443055 | 3.668457 | 0.003532 | 0.003922 | 0.003699 |
| droppable.test | RUN | 0.664926 | 2.410571 | 2.055134 | 0.003697 | 0.003595 | 0.003793 |
| error_of.test | RUN | 0.751662 | 3.016545 | 2.212776 | 0.003428 | 0.003583 | 0.003344 |
| extended_p_square.test | RUN | 0.782896 | 2.150110 | 2.049797 | 0.017908 | 0.018836 | 0.020934 |
| extended_p_square_quantile.test | RUN | 0.944005 | 3.709726 | 2.877698 | 0.111845 | 0.116221 | 0.119715 |
| external_accumulator.test | RUN | 0.665414 | 2.632070 | 2.021881 | 0.003551 | 0.003793 | 0.003699 |
| external_weights.test | RUN | 0.605381 | 1.620419 | 1.582660 | 0.003162 | 0.003361 | 0.003313 |
| kurtosis.test | RUN | 0.886928 | 3.359959 | 2.593101 | 0.032080 | 0.034873 | 0.040578 |
| max.test | RUN | 0.555796 | 1.424393 | 1.428355 | 0.003435 | 0.003521 | 0.003462 |
| mean.test | RUN | 0.712138 | 2.652174 | 2.101371 | 0.003406 | 0.003003 | 0.003781 |
| median.test | RUN | 0.933576 | 3.757159 | 2.877491 | 1.043476 | 1.159356 | 1.430902 |
| min.test | RUN | 0.566452 | 1.435493 | 1.399603 | 0.004299 | 0.003519 | 0.003615 |
| moment.test | RUN | 0.650622 | 1.769024 | 1.745906 | 0.003462 | 0.003593 | 0.002777 |
| p_square_cumul_dist.test | RUN | 0.789975 | 2.111875 | 2.118870 | 10.292004 | 9.316606 | 11.656154 |
| p_square_quantile.test | RUN | 0.711840 | 1.912105 | 1.711434 | 0.300818 | 0.281332 | 0.310688 |
| reference.test | RUN | 0.624231 | 1.974041 | 1.807274 | 0.003378 | 0.003502 | 0.003325 |
| rolling_count.test | RUN | 0.625859 | 1.740131 | 1.625814 | 0.003402 | 0.003737 | 0.003606 |
| rolling_sum.test | RUN | 0.643460 | 1.764031 | 1.639575 | 0.003375 | 0.003684 | 0.003369 |
| rolling_mean.test | RUN | 0.791716 | 3.357514 | 2.433642 | 0.003829 | 0.004171 | 0.003933 |
| rolling_variance.test | RUN | 0.906439 | 4.071731 | 3.025959 | 0.004056 | 0.004141 | 0.003970 |
| rolling_moment.test | RUN | 0.737491 | 2.279746 | 2.032126 | 0.003576 | 0.003952 | 0.003608 |
| skewness.test | RUN | 0.812694 | 2.820668 | 2.219453 | 0.030883 | 0.032437 | 0.034536 |
| sum.test | RUN | 0.577567 | 1.539165 | 1.533701 | 0.003267 | 0.003707 | 0.003448 |
| sum_kahan.test | RUN | 0.638023 | 1.732756 | 1.460779 | 0.327598 | 0.324662 | 0.312965 |
| tail.test | RUN | 0.622046 | 2.078298 | 1.839970 | 0.003407 | 0.003991 | 0.003223 |
| tail_mean.test | RUN | 0.879461 | 3.517707 | 2.732764 | 0.144407 | 0.168594 | 0.192468 |
| tail_quantile.test | RUN | 0.775014 | 2.212701 | 1.797955 | 0.139383 | 0.138694 | 0.155930 |
| variance.test | RUN | 0.705044 | 2.617091 | 2.001580 | 0.003398 | 0.003740 | 0.003393 |
| vector.test | RUN | 0.722987 | 3.080641 | 2.306946 | 0.003426 | 0.003463 | 0.003619 |
| weighted_covariance.test | RUN | 0.982225 | 4.876795 | 3.784906 | 0.003036 | 0.003574 | 0.003402 |
| weighted_extended_p_square.test | RUN | 0.742007 | 1.994422 | 1.806174 | 0.336566 | 0.347037 | 0.349732 |
| weighted_kurtosis.test | RUN | 0.813811 | 3.331982 | 2.469873 | 0.038636 | 0.042232 | 0.045220 |
| weighted_mean.test | RUN | 0.715115 | 2.924240 | 2.180312 | 0.003254 | 0.003532 | 0.003309 |
| weighted_median.test | RUN | 0.880757 | 3.444594 | 2.716202 | 10.393729 | 12.512067 | 13.179495 |
| weighted_moment.test | RUN | 0.602708 | 1.718104 | 1.606591 | 0.003069 | 0.003451 | 0.003075 |
| weighted_p_square_cumul_dist.test | RUN | 0.712299 | 1.943715 | 1.878434 | 2.001254 | 2.059803 | 2.129121 |
| weighted_p_square_quantile.test | RUN | 0.615661 | 1.826780 | 1.664229 | 0.442774 | 0.432227 | 0.417030 |
| weighted_skewness.test | RUN | 0.786425 | 2.862387 | 2.045249 | 0.034567 | 0.036363 | 0.040940 |
| weighted_sum.test | RUN | 0.531651 | 1.304106 | 1.117704 | 0.003146 | 0.003169 | 0.003011 |
| weighted_sum_kahan.test | RUN | 0.533968 | 1.383550 | 1.371801 | 0.201368 | 0.195420 | 0.176030 |
| weighted_variance.test | RUN | 0.868239 | 3.611633 | 2.561923 | 0.010323 | 0.012027 | 0.011079 |
| valarray.test | RUN | 0.771657 | 2.951137 | 2.141982 | 0.003312 | 0.003526 | 0.003247 |
| tail_variate_means.test | RUN | 0.929157 | 4.447795 | 3.192445 | 0.003555 | 0.003724 | 0.003638 |
| pot_quantile.test | RUN | 1.220442 | 4.430776 | 3.432542 | 0.136946 | 0.155100 | 0.144808 |
| weighted_pot_quantile.test | RUN | 1.225959 | 4.807753 | 4.125192 | 0.240984 | 0.258298 | 0.269303 |
| weighted_tail_mean.test | RUN | 1.030808 | 3.470286 | 3.385922 | 0.164217 | 0.103775 | 0.168050 |
| weighted_tail_quantile.test | RUN | 1.110699 | 3.331855 | 3.076354 | 0.149750 | 0.101304 | 0.157132 |
| weighted_tail_variate_means.test | RUN | 1.326613 | 4.397142 | 3.543628 | 0.002702 | 0.002800 | 0.002689 |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42568872153) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42568872153) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42568872153).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
