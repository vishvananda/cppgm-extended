# libs/accumulators/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| count.test | RUN | 1.056124 | 2.754933 | 3.614239 | 0.003927 | 0.004085 | 0.004363 |
| covariance.test | RUN | 1.956176 | 6.866917 | 9.069353 | 0.003859 | 0.004071 | 0.004081 |
| droppable.test | RUN | 1.268034 | 3.655729 | 4.702177 | 0.004124 | 0.004110 | 0.004352 |
| error_of.test | RUN | 1.401446 | 3.669575 | 5.237740 | 0.003778 | 0.004162 | 0.003706 |
| extended_p_square.test | RUN | 1.557175 | 4.518651 | 5.273884 | 0.006166 | 0.006484 | 0.005706 |
| extended_p_square_quantile.test | RUN | 1.867464 | 6.929753 | 7.537081 | 0.011804 | 0.012234 | 0.010775 |
| external_accumulator.test | RUN | 1.294389 | 3.707183 | 4.596378 | 0.003947 | 0.004094 | 0.004131 |
| external_weights.test | RUN | 1.148718 | 2.703115 | 3.635392 | 0.003703 | 0.003990 | 0.003804 |
| kurtosis.test | RUN | 1.704478 | 4.529069 | 6.238843 | 0.006175 | 0.006259 | 0.005467 |
| max.test | RUN | 1.115289 | 2.791013 | 3.436076 | 0.004038 | 0.003833 | 0.004184 |
| mean.test | RUN | 1.360765 | 3.712170 | 5.079364 | 0.004299 | 0.004414 | 0.003852 |
| median.test | RUN | 1.857515 | 7.327694 | 7.542367 | 0.151357 | 0.164452 | 0.148718 |
| min.test | RUN | 1.076656 | 2.644483 | 3.468570 | 0.003364 | 0.004132 | 0.003853 |
| moment.test | RUN | 1.235817 | 3.066127 | 4.190012 | 0.004169 | 0.003770 | 0.003858 |
| p_square_cumul_dist.test | RUN | 1.592814 | 5.272821 | 5.627450 | 1.162254 | 1.287693 | 1.063792 |
| p_square_quantile.test | RUN | 1.380128 | 4.606787 | 4.941516 | 0.043423 | 0.037645 | 0.032793 |
| reference.test | RUN | 1.223752 | 3.057652 | 3.917421 | 0.004066 | 0.003681 | 0.003352 |
| rolling_count.test | RUN | 1.251302 | 3.332209 | 4.298786 | 0.003496 | 0.003814 | 0.003867 |
| rolling_sum.test | RUN | 1.241067 | 3.214600 | 4.288863 | 0.004163 | 0.003891 | 0.004055 |
| rolling_mean.test | RUN | 1.750002 | 6.140841 | 6.709719 | 0.004227 | 0.004088 | 0.004275 |
| rolling_variance.test | RUN | 1.909218 | 6.980793 | 7.739874 | 0.004193 | 0.003898 | 0.004067 |
| rolling_moment.test | RUN | 1.444731 | 4.058979 | 5.321441 | 0.004081 | 0.004077 | 0.004268 |
| skewness.test | RUN | 1.590286 | 4.220660 | 5.934293 | 0.006479 | 0.005636 | 0.005279 |
| sum.test | RUN | 1.123355 | 2.949312 | 3.718871 | 0.004207 | 0.004230 | 0.003859 |
| sum_kahan.test | RUN | 1.263973 | 3.153040 | 4.102096 | 0.019515 | 0.031624 | 0.019958 |
| tail.test | RUN | 1.283222 | 4.203851 | 4.658338 | 0.003516 | 0.003891 | 0.004115 |
| tail_mean.test | RUN | 1.744760 | 4.975637 | 6.675765 | 0.027888 | 0.039241 | 0.027338 |
| tail_quantile.test | RUN | 1.476287 | 4.106867 | 5.187727 | 0.032139 | 0.037027 | 0.031597 |
| variance.test | RUN | 1.402872 | 3.682339 | 4.987458 | 0.003797 | 0.003785 | 0.003929 |
| vector.test | RUN | 1.463310 | 4.684007 | 5.971189 | 0.003749 | 0.004011 | 0.003846 |
| weighted_covariance.test | RUN | 1.929740 | 6.899412 | 9.238561 | 0.003693 | 0.003726 | 0.003718 |
| weighted_extended_p_square.test | RUN | 1.379168 | 4.008252 | 4.620661 | 0.030445 | 0.031584 | 0.027191 |
| weighted_kurtosis.test | RUN | 1.602886 | 4.103088 | 5.926348 | 0.006674 | 0.006807 | 0.005268 |
| weighted_mean.test | RUN | 1.341473 | 3.751820 | 4.975034 | 0.003570 | 0.004012 | 0.003845 |
| weighted_median.test | RUN | 1.770666 | 4.995313 | 6.382897 | 1.395857 | 1.382642 | 1.306132 |
| weighted_moment.test | RUN | 1.129496 | 2.747493 | 3.833494 | 0.004004 | 0.003573 | 0.003772 |
| weighted_p_square_cumul_dist.test | RUN | 1.399733 | 4.235636 | 4.754818 | 0.186065 | 0.164744 | 0.138889 |
| weighted_p_square_quantile.test | RUN | 1.346067 | 4.075117 | 4.317378 | 0.041851 | 0.041106 | 0.036204 |
| weighted_skewness.test | RUN | 1.531393 | 3.966959 | 5.094462 | 0.006573 | 0.006672 | 0.004164 |
| weighted_sum.test | RUN | 0.860461 | 2.260699 | 3.080680 | 0.003396 | 0.003561 | 0.003227 |
| weighted_sum_kahan.test | RUN | 1.080865 | 2.527896 | 3.233006 | 0.011511 | 0.017634 | 0.011593 |
| weighted_variance.test | RUN | 1.710171 | 4.513892 | 6.440256 | 0.004202 | 0.004515 | 0.004413 |
| valarray.test | RUN | 1.472556 | 4.228224 | 5.942752 | 0.003692 | 0.003626 | 0.003159 |
| tail_variate_means.test | RUN | 1.991849 | 6.790894 | 8.009653 | 0.003863 | 0.003972 | 0.004089 |
| pot_quantile.test | RUN | 2.403481 | 7.473057 | 10.051458 | 0.015960 | 0.015632 | 0.014086 |
| weighted_pot_quantile.test | RUN | 2.398853 | 7.413782 | 10.073516 | 0.026653 | 0.042550 | 0.033438 |
| weighted_tail_mean.test | RUN | 2.130504 | 5.815721 | 8.458856 | 0.027399 | 0.033588 | 0.029632 |
| weighted_tail_quantile.test | RUN | 2.107969 | 5.257274 | 7.772173 | 0.025215 | 0.028248 | 0.022913 |
| weighted_tail_variate_means.test | RUN | 2.231860 | 5.864662 | 8.238615 | 0.002740 | 0.002732 | 0.002430 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.092× | 1.088×, 1.095× | 0.985× |
| clang | 0.917× | 0.921×, 0.913× | 1.001× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| count.test | 1 | 0.003073s | 0.002966s | 0.002902s | byte equality observed |
| covariance.test | 1 | 0.003049s | 0.002974s | 0.002846s | byte equality observed |
| droppable.test | 1 | 0.003100s | 0.003037s | 0.002849s | byte equality observed |
| error_of.test | 4 | 0.002680s | 0.002611s | 0.002550s | byte equality observed |
| extended_p_square.test | 4 | 0.004747s | 0.004664s | 0.004364s | byte equality observed |
| extended_p_square_quantile.test | 2 | 0.010089s | 0.010416s | 0.009139s | byte equality observed |
| external_accumulator.test | 4 | 0.002746s | 0.002631s | 0.002642s | byte equality observed |
| external_weights.test | 5 | 0.002607s | 0.002487s | 0.002511s | byte equality observed |
| kurtosis.test | 4 | 0.005136s | 0.004774s | 0.003967s | byte equality observed |
| max.test | 3 | 0.002779s | 0.002741s | 0.002686s | byte equality observed |
| mean.test | 6 | 0.002765s | 0.002694s | 0.002661s | byte equality observed |
| median.test | 1 | 0.149379s | 0.165501s | 0.142478s | byte equality observed |
| min.test | 6 | 0.002704s | 0.002634s | 0.002560s | byte equality observed |
| moment.test | 6 | 0.002656s | 0.002585s | 0.002535s | byte equality observed |
| p_square_cumul_dist.test | 1 | 0.952086s | 1.143999s | 0.902664s | byte equality observed |
| p_square_quantile.test | 1 | 0.038525s | 0.034990s | 0.030873s | byte equality observed |
| pot_quantile.test | 2 | 0.014629s | 0.013403s | 0.011931s | byte equality observed |
| reference.test | 7 | 0.002691s | 0.002538s | 0.002521s | byte equality observed |
| rolling_count.test | 7 | 0.002690s | 0.002503s | 0.002533s | byte equality observed |
| rolling_mean.test | 6 | 0.002834s | 0.002664s | 0.002624s | byte equality observed |
| rolling_moment.test | 7 | 0.002787s | 0.002633s | 0.002608s | byte equality observed |
| rolling_sum.test | 7 | 0.002703s | 0.002518s | 0.002539s | byte equality observed |
| rolling_variance.test | 7 | 0.002789s | 0.002678s | 0.002607s | byte equality observed |
| skewness.test | 4 | 0.004922s | 0.004631s | 0.003939s | byte equality observed |
| sum.test | 6 | 0.002604s | 0.002526s | 0.002458s | byte equality observed |
| sum_kahan.test | 1 | 0.018320s | 0.029812s | 0.018270s | byte equality observed |
| tail.test | 7 | 0.002642s | 0.002490s | 0.002501s | byte equality observed |
| tail_mean.test | 1 | 0.028257s | 0.030143s | 0.026148s | byte equality observed |
| tail_quantile.test | 1 | 0.030687s | 0.039006s | 0.027940s | byte equality observed |
| tail_variate_means.test | 7 | 0.002599s | 0.002431s | 0.002534s | byte equality observed |
| valarray.test | 7 | 0.002628s | 0.002427s | 0.002490s | byte equality observed |
| variance.test | 7 | 0.002754s | 0.002590s | 0.002631s | byte equality observed |
| vector.test | 7 | 0.002578s | 0.002484s | 0.002467s | byte equality observed |
| weighted_covariance.test | 6 | 0.002538s | 0.002544s | 0.002451s | byte equality observed |
| weighted_extended_p_square.test | 1 | 0.029667s | 0.030198s | 0.023961s | byte equality observed |
| weighted_kurtosis.test | 4 | 0.005595s | 0.005204s | 0.003950s | byte equality observed |
| weighted_mean.test | 6 | 0.002494s | 0.002445s | 0.002419s | byte equality observed |
| weighted_median.test | 1 | 1.230424s | 1.260102s | 1.137656s | byte equality observed |
| weighted_moment.test | 7 | 0.002456s | 0.002370s | 0.002341s | byte equality observed |
| weighted_p_square_cumul_dist.test | 1 | 0.182056s | 0.199812s | 0.140653s | byte equality observed |
| weighted_p_square_quantile.test | 1 | 0.041569s | 0.039108s | 0.032920s | byte equality observed |
| weighted_pot_quantile.test | 1 | 0.031833s | 0.032384s | 0.029242s | byte equality observed |
| weighted_skewness.test | 4 | 0.005448s | 0.005083s | 0.003794s | byte equality observed |
| weighted_sum.test | 6 | 0.002455s | 0.002360s | 0.002329s | byte equality observed |
| weighted_sum_kahan.test | 2 | 0.010181s | 0.015794s | 0.009993s | byte equality observed |
| weighted_tail_mean.test | 1 | 0.025138s | 0.027262s | 0.024004s | byte equality observed |
| weighted_tail_quantile.test | 1 | 0.028282s | 0.032205s | 0.026316s | byte equality observed |
| weighted_tail_variate_means.test | 4 | 0.002500s | 0.002407s | 0.002374s | byte equality observed |
| weighted_variance.test | 6 | 0.002965s | 0.002814s | 0.002652s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
