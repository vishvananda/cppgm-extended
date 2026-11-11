# libs/accumulators/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| count.test | RUN | 0.914966 | 2.440175 | 2.613817 | 0.003205 | 0.003187 | 0.002766 |
| covariance.test | RUN | 1.729204 | 6.262351 | 6.703550 | 0.003200 | 0.003262 | 0.003120 |
| droppable.test | RUN | 1.109143 | 3.354942 | 3.599394 | 0.003330 | 0.003095 | 0.002646 |
| error_of.test | RUN | 1.223522 | 3.254571 | 3.840710 | 0.003004 | 0.002997 | 0.002626 |
| extended_p_square.test | RUN | 1.302993 | 4.162532 | 3.994810 | 0.005509 | 0.005176 | 0.004790 |
| extended_p_square_quantile.test | RUN | 1.726512 | 6.397108 | 5.731442 | 0.009507 | 0.010111 | 0.008930 |
| external_accumulator.test | RUN | 1.120092 | 3.138907 | 3.476467 | 0.002989 | 0.003214 | 0.002775 |
| external_weights.test | RUN | 1.000367 | 2.501539 | 2.744427 | 0.003008 | 0.003100 | 0.002627 |
| kurtosis.test | RUN | 1.466293 | 4.025695 | 4.527346 | 0.005668 | 0.005373 | 0.004300 |
| max.test | RUN | 0.934297 | 2.371274 | 2.651779 | 0.003170 | 0.002973 | 0.002910 |
| mean.test | RUN | 1.246097 | 3.459332 | 3.783225 | 0.003156 | 0.003201 | 0.002842 |
| median.test | RUN | 1.651055 | 6.625938 | 5.791847 | 0.143093 | 0.161076 | 0.135281 |
| min.test | RUN | 0.944186 | 2.428671 | 2.555231 | 0.003087 | 0.003031 | 0.002998 |
| moment.test | RUN | 1.038417 | 2.751941 | 3.047381 | 0.003200 | 0.003175 | 0.002657 |
| p_square_cumul_dist.test | RUN | 1.410237 | 4.821898 | 4.390261 | 1.063869 | 1.182736 | 0.943559 |
| p_square_quantile.test | RUN | 1.217116 | 4.288190 | 3.620573 | 0.036432 | 0.033687 | 0.028806 |
| reference.test | RUN | 1.021378 | 2.667985 | 3.020262 | 0.003310 | 0.002985 | 0.003010 |
| rolling_count.test | RUN | 1.072666 | 3.081567 | 3.022527 | 0.003027 | 0.003073 | 0.002697 |
| rolling_sum.test | RUN | 1.041193 | 3.119275 | 3.152904 | 0.003387 | 0.003248 | 0.003026 |
| rolling_mean.test | RUN | 1.554111 | 5.542531 | 5.184944 | 0.003264 | 0.003378 | 0.003138 |
| rolling_variance.test | RUN | 1.702734 | 6.405628 | 5.889224 | 0.003418 | 0.003334 | 0.003095 |
| rolling_moment.test | RUN | 1.306419 | 3.853230 | 3.892675 | 0.003307 | 0.003394 | 0.003149 |
| skewness.test | RUN | 1.357909 | 3.865877 | 4.269835 | 0.005655 | 0.005245 | 0.004181 |
| sum.test | RUN | 0.957705 | 2.599208 | 2.875416 | 0.003289 | 0.003206 | 0.002946 |
| sum_kahan.test | RUN | 1.053737 | 2.809718 | 2.971335 | 0.014461 | 0.024288 | 0.014214 |
| tail.test | RUN | 1.104532 | 3.862336 | 3.381141 | 0.003589 | 0.003299 | 0.002924 |
| tail_mean.test | RUN | 1.550954 | 4.528926 | 4.866652 | 0.028128 | 0.027452 | 0.025472 |
| tail_quantile.test | RUN | 1.276232 | 3.724581 | 3.835671 | 0.031383 | 0.031482 | 0.027402 |
| variance.test | RUN | 1.212915 | 3.325274 | 3.747331 | 0.003021 | 0.003133 | 0.003106 |
| vector.test | RUN | 1.311139 | 4.112656 | 4.593366 | 0.003172 | 0.003066 | 0.002865 |
| weighted_covariance.test | RUN | 1.787324 | 6.146599 | 6.857133 | 0.002886 | 0.003140 | 0.002583 |
| weighted_extended_p_square.test | RUN | 1.222752 | 3.723212 | 3.461248 | 0.028551 | 0.029053 | 0.023371 |
| weighted_kurtosis.test | RUN | 1.430679 | 3.759518 | 4.282501 | 0.005961 | 0.005948 | 0.004238 |
| weighted_mean.test | RUN | 1.222515 | 3.293149 | 3.755661 | 0.002732 | 0.003044 | 0.002510 |
| weighted_median.test | RUN | 1.547222 | 4.566320 | 4.817871 | 1.465368 | 1.406421 | 1.317341 |
| weighted_moment.test | RUN | 0.978261 | 2.438132 | 2.818656 | 0.003026 | 0.003129 | 0.002793 |
| weighted_p_square_cumul_dist.test | RUN | 1.222231 | 3.837390 | 3.481622 | 0.158207 | 0.158425 | 0.131225 |
| weighted_p_square_quantile.test | RUN | 1.148600 | 3.576182 | 3.182964 | 0.037819 | 0.035743 | 0.027722 |
| weighted_skewness.test | RUN | 1.346450 | 3.575247 | 3.787386 | 0.005947 | 0.005897 | 0.004195 |
| weighted_sum.test | RUN | 0.782465 | 2.068116 | 2.250179 | 0.002683 | 0.003103 | 0.002770 |
| weighted_sum_kahan.test | RUN | 0.900225 | 2.192681 | 2.373554 | 0.008678 | 0.013648 | 0.008539 |
| weighted_variance.test | RUN | 1.488070 | 4.099123 | 4.717372 | 0.003757 | 0.003610 | 0.003139 |
| valarray.test | RUN | 1.297696 | 3.882908 | 4.416877 | 0.002656 | 0.003108 | 0.002875 |
| tail_variate_means.test | RUN | 1.746789 | 6.064874 | 6.139569 | 0.002998 | 0.003144 | 0.002928 |
| pot_quantile.test | RUN | 2.129974 | 6.756593 | 7.250422 | 0.015210 | 0.014531 | 0.011923 |
| weighted_pot_quantile.test | RUN | 2.166608 | 6.376064 | 7.284573 | 0.036398 | 0.025239 | 0.030674 |
| weighted_tail_mean.test | RUN | 1.854481 | 5.288878 | 6.213617 | 0.027672 | 0.028973 | 0.026271 |
| weighted_tail_quantile.test | RUN | 1.831449 | 4.626557 | 5.515636 | 0.023083 | 0.024041 | 0.020448 |
| weighted_tail_variate_means.test | RUN | 2.086194 | 5.714132 | 6.228526 | 0.002263 | 0.002115 | 0.002089 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.065× | 1.061×, 1.069× | 1.004× |
| clang | 0.912× | 0.912×, 0.913× | 1.000× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| count.test | 1 | 0.002514s | 0.002497s | 0.002388s | byte equality observed |
| covariance.test | 1 | 0.002511s | 0.002318s | 0.002453s | byte equality observed |
| droppable.test | 1 | 0.002489s | 0.002313s | 0.002311s | byte equality observed |
| error_of.test | 4 | 0.002217s | 0.002110s | 0.002166s | byte equality observed |
| extended_p_square.test | 4 | 0.004212s | 0.003942s | 0.003667s | byte equality observed |
| extended_p_square_quantile.test | 3 | 0.008001s | 0.008142s | 0.007315s | byte equality observed |
| external_accumulator.test | 6 | 0.002177s | 0.002107s | 0.002081s | byte equality observed |
| external_weights.test | 6 | 0.002039s | 0.002010s | 0.001988s | byte equality observed |
| kurtosis.test | 4 | 0.004403s | 0.004020s | 0.003094s | byte equality observed |
| max.test | 7 | 0.002145s | 0.002091s | 0.002078s | byte equality observed |
| mean.test | 7 | 0.002171s | 0.002113s | 0.002068s | byte equality observed |
| median.test | 1 | 0.134629s | 0.169185s | 0.125180s | byte equality observed |
| min.test | 7 | 0.002167s | 0.002122s | 0.002046s | byte equality observed |
| moment.test | 4 | 0.002175s | 0.002074s | 0.002068s | byte equality observed |
| p_square_cumul_dist.test | 1 | 0.874642s | 1.009084s | 0.787389s | byte equality observed |
| p_square_quantile.test | 1 | 0.035495s | 0.031571s | 0.028170s | byte equality observed |
| pot_quantile.test | 2 | 0.014200s | 0.012804s | 0.010937s | byte equality observed |
| reference.test | 4 | 0.002219s | 0.002084s | 0.002152s | byte equality observed |
| rolling_count.test | 7 | 0.002229s | 0.002130s | 0.002128s | byte equality observed |
| rolling_mean.test | 8 | 0.002299s | 0.002130s | 0.002199s | byte equality observed |
| rolling_moment.test | 8 | 0.002244s | 0.002104s | 0.002147s | byte equality observed |
| rolling_sum.test | 8 | 0.002193s | 0.002057s | 0.002100s | byte equality observed |
| rolling_variance.test | 8 | 0.002316s | 0.002142s | 0.002170s | byte equality observed |
| skewness.test | 4 | 0.004313s | 0.004056s | 0.003186s | byte equality observed |
| sum.test | 8 | 0.002167s | 0.001987s | 0.002024s | byte equality observed |
| sum_kahan.test | 1 | 0.013339s | 0.022537s | 0.012863s | byte equality observed |
| tail.test | 8 | 0.002146s | 0.002094s | 0.002038s | byte equality observed |
| tail_mean.test | 1 | 0.025888s | 0.023753s | 0.024088s | byte equality observed |
| tail_quantile.test | 1 | 0.028871s | 0.028852s | 0.025232s | byte equality observed |
| tail_variate_means.test | 7 | 0.002173s | 0.001995s | 0.002016s | byte equality observed |
| valarray.test | 6 | 0.002136s | 0.002052s | 0.002000s | byte equality observed |
| variance.test | 8 | 0.002162s | 0.002100s | 0.002066s | byte equality observed |
| vector.test | 7 | 0.002073s | 0.001993s | 0.001992s | byte equality observed |
| weighted_covariance.test | 8 | 0.002105s | 0.001983s | 0.001969s | byte equality observed |
| weighted_extended_p_square.test | 1 | 0.028592s | 0.026858s | 0.021513s | byte equality observed |
| weighted_kurtosis.test | 4 | 0.004941s | 0.004695s | 0.003083s | byte equality observed |
| weighted_mean.test | 9 | 0.002058s | 0.001996s | 0.001915s | byte equality observed |
| weighted_median.test | 1 | 1.195444s | 1.212636s | 1.116882s | byte equality observed |
| weighted_moment.test | 6 | 0.002062s | 0.001994s | 0.001926s | byte equality observed |
| weighted_p_square_cumul_dist.test | 1 | 0.153569s | 0.181966s | 0.126913s | byte equality observed |
| weighted_p_square_quantile.test | 1 | 0.038014s | 0.036004s | 0.028672s | byte equality observed |
| weighted_pot_quantile.test | 1 | 0.031456s | 0.030755s | 0.027903s | byte equality observed |
| weighted_skewness.test | 4 | 0.005219s | 0.004893s | 0.003252s | byte equality observed |
| weighted_sum.test | 8 | 0.002049s | 0.001992s | 0.001917s | byte equality observed |
| weighted_sum_kahan.test | 2 | 0.007459s | 0.012057s | 0.007358s | byte equality observed |
| weighted_tail_mean.test | 1 | 0.025594s | 0.024938s | 0.024100s | byte equality observed |
| weighted_tail_quantile.test | 1 | 0.027311s | 0.029188s | 0.025631s | byte equality observed |
| weighted_tail_variate_means.test | 4 | 0.002165s | 0.002115s | 0.002017s | byte equality observed |
| weighted_variance.test | 6 | 0.002564s | 0.002436s | 0.002167s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
