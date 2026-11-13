# libs/xpressive/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| regress.test | RUN | 2.728548 | 18.475782 | 14.366594 | 0.010907 | 0.013368 | 0.009682 |
| regress_u.test | RUN | 2.938258 | 18.970048 | 14.480793 | 0.010079 | 0.013719 | 0.009941 |
| c_traits.test | RUN | 2.809135 | 18.025742 | 13.829316 | 0.008365 | 0.011845 | 0.008292 |
| c_traits_u.test | RUN | 2.956941 | 18.521428 | 14.288916 | 0.009066 | 0.012492 | 0.008449 |
| test1.test | RUN | 1.054992 | 4.561834 | 4.275953 | 0.003011 | 0.003157 | 0.003250 |
| test2.test | RUN | 1.389997 | 6.791522 | 6.250781 | 0.002906 | 0.003139 | 0.003228 |
| test3.test | RUN | 1.395538 | 6.607554 | 6.254115 | 0.003051 | 0.003337 | 0.003161 |
| test4.test | RUN | 1.187991 | 5.448315 | 5.104233 | 0.003096 | 0.003325 | 0.003118 |
| test5.test | RUN | 1.274148 | 5.889961 | 5.464247 | 0.002775 | 0.003379 | 0.003203 |
| test6.test | RUN | 1.167577 | 4.942400 | 4.690770 | 0.002954 | 0.003353 | 0.003037 |
| test7.test | RUN | 1.135686 | 5.123822 | 4.771545 | 0.002982 | 0.003012 | 0.003393 |
| test8.test | RUN | 1.121298 | 4.866529 | 4.751237 | 0.002759 | 0.003067 | 0.003230 |
| test9.test | RUN | 1.216883 | 5.361051 | 5.293332 | 0.003014 | 0.003420 | 0.003305 |
| test10.test | RUN | 1.186548 | 5.066027 | 4.929199 | 0.002707 | 0.002955 | 0.002913 |
| test11.test | RUN | 1.147020 | 4.872545 | 4.840108 | 0.002678 | 0.003282 | 0.003158 |
| test1u.test | RUN | 1.047605 | 4.560784 | 4.213743 | 0.002952 | 0.003093 | 0.002947 |
| test2u.test | RUN | 1.426677 | 6.900132 | 6.431045 | 0.003069 | 0.003347 | 0.002916 |
| test3u.test | RUN | 1.436548 | 7.092826 | 6.579628 | 0.003051 | 0.003246 | 0.003329 |
| test4u.test | RUN | 1.214768 | 5.429593 | 5.094730 | 0.003096 | 0.003413 | 0.002707 |
| test5u.test | RUN | 1.302886 | 6.071748 | 5.717016 | 0.003232 | 0.003376 | 0.003197 |
| test6u.test | RUN | 1.211713 | 5.018062 | 4.815698 | 0.003108 | 0.003404 | 0.003210 |
| test7u.test | RUN | 1.171978 | 5.199927 | 4.868186 | 0.003228 | 0.003415 | 0.003200 |
| test8u.test | RUN | 1.128173 | 4.876302 | 4.579468 | 0.003111 | 0.003367 | 0.002998 |
| test9u.test | RUN | 1.231087 | 5.337817 | 5.108378 | 0.003087 | 0.003421 | 0.003211 |
| test10u.test | RUN | 1.195580 | 4.943303 | 4.755935 | 0.002964 | 0.003250 | 0.003133 |
| test11u.test | RUN | 1.216019 | 5.016887 | 4.859252 | 0.003521 | 0.003209 | 0.003182 |
| misc1.test | RUN | 2.124452 | 12.577735 | 10.509491 | 0.004185 | 0.003917 | 0.003590 |
| misc2.test | RUN | 2.244957 | 11.816410 | 10.319307 | 0.003598 | 0.003552 | 0.003328 |
| test_format.test | RUN | 2.298881 | 14.513471 | 11.427640 | 0.003241 | 0.003325 | 0.003363 |
| test_cycles.test | RUN | 1.062849 | 4.515845 | 3.896175 | 0.003221 | 0.003267 | 0.003210 |
| test_non_char.test | RUN | 1.846510 | 9.950092 | 8.716948 | 0.003147 | 0.003352 | 0.003283 |
| test_static.test | RUN | 0.983421 | 4.162208 | 3.493993 | 0.003123 | 0.003136 | 0.002846 |
| test_actions.test | RUN | 2.294150 | 12.287215 | 10.676335 | 0.004104 | 0.004326 | 0.004268 |
| test_assert.test | RUN | 1.357006 | 5.370712 | 4.965098 | 0.003089 | 0.003466 | 0.002959 |
| test_assert_with_placeholder.test | RUN | 1.248798 | 4.779225 | 4.518339 | 0.003143 | 0.003135 | 0.002859 |
| test_symbols.test | RUN | 2.378203 | 12.145719 | 12.029358 | 0.004333 | 0.004404 | 0.004249 |
| test_dynamic.test | RUN | 1.509586 | 8.133160 | 6.832586 | 0.003331 | 0.003206 | 0.003154 |
| test_dynamic_grammar.test | RUN | 1.528252 | 8.653128 | 7.248409 | 0.003275 | 0.003521 | 0.003405 |
| test_skip.test | RUN | 1.816543 | 9.123618 | 8.467992 | 0.002947 | 0.003366 | 0.003102 |
| multiple_defs.test | LINK | 2.915376 | 16.464560 | 13.780040 | — | — | — |
| test_basic_regex.test | COMPILE | 0.673934 | 1.354695 | 1.935570 | — | — | — |
| test_match_results.test | COMPILE | 0.440638 | 0.953892 | 1.308684 | — | — | — |
| test_regex_algorithms.test | COMPILE | 0.430243 | 0.978722 | 1.301215 | — | — | — |
| test_regex_compiler.test | COMPILE | 0.668867 | 1.521654 | 1.908241 | — | — | — |
| test_regex_constants.test | COMPILE | 0.038305 | 0.068251 | 0.067853 | — | — | — |
| test_regex_error.test | COMPILE | 0.245647 | 0.499284 | 0.709575 | — | — | — |
| test_regex_iterator.test | COMPILE | 0.445900 | 0.954195 | 1.250554 | — | — | — |
| test_regex_primitives.test | COMPILE | 0.642949 | 1.581449 | 1.624605 | — | — | — |
| test_regex_token_iterator.test | COMPILE | 0.466348 | 0.999134 | 1.199230 | — | — | — |
| test_regex_traits.test | COMPILE | 0.249078 | 0.509160 | 0.667747 | — | — | — |
| test_sub_match.test | COMPILE | 0.233993 | 0.410000 | 0.457535 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.089× | 1.082×, 1.096× | 1.002× |
| clang | 0.940× | 0.923×, 0.956× | 1.006× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1363 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43479737952).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| c_traits.test | 3 | 0.007508s | 0.010410s | 0.006538s | byte equality observed |
| c_traits_u.test | 3 | 0.008128s | 0.010641s | 0.007087s | byte equality observed |
| misc1.test | 8 | 0.002697s | 0.002603s | 0.002553s | byte equality observed |
| misc2.test | 9 | 0.002383s | 0.002275s | 0.002277s | byte equality observed |
| regress.test | 3 | 0.009131s | 0.011900s | 0.008022s | byte equality observed |
| regress_u.test | 3 | 0.009594s | 0.011957s | 0.008476s | byte equality observed |
| test1.test | 10 | 0.002122s | 0.002092s | 0.002050s | byte equality observed |
| test10.test | 10 | 0.002078s | 0.002019s | 0.002056s | byte equality observed |
| test10u.test | 10 | 0.002070s | 0.002068s | 0.002042s | byte equality observed |
| test11.test | 10 | 0.002074s | 0.002040s | 0.002013s | byte equality observed |
| test11u.test | 10 | 0.002092s | 0.002065s | 0.001999s | byte equality observed |
| test1u.test | 9 | 0.002073s | 0.002090s | 0.002002s | byte equality observed |
| test2.test | 10 | 0.002123s | 0.002065s | 0.002027s | byte equality observed |
| test2u.test | 10 | 0.002152s | 0.002113s | 0.002075s | byte equality observed |
| test3.test | 10 | 0.002118s | 0.002158s | 0.002027s | byte equality observed |
| test3u.test | 10 | 0.002166s | 0.002176s | 0.002066s | byte equality observed |
| test4.test | 10 | 0.002129s | 0.002073s | 0.002035s | byte equality observed |
| test4u.test | 10 | 0.002116s | 0.002083s | 0.002063s | byte equality observed |
| test5.test | 10 | 0.002149s | 0.002116s | 0.002003s | byte equality observed |
| test5u.test | 10 | 0.002145s | 0.002076s | 0.002070s | byte equality observed |
| test6.test | 9 | 0.002106s | 0.002064s | 0.001990s | byte equality observed |
| test6u.test | 8 | 0.002047s | 0.002085s | 0.002027s | byte equality observed |
| test7.test | 10 | 0.002061s | 0.002131s | 0.002011s | byte equality observed |
| test7u.test | 10 | 0.002111s | 0.002082s | 0.002034s | byte equality observed |
| test8.test | 10 | 0.002077s | 0.002110s | 0.002045s | byte equality observed |
| test8u.test | 9 | 0.002045s | 0.002046s | 0.002043s | byte equality observed |
| test9.test | 8 | 0.002074s | 0.002123s | 0.001991s | byte equality observed |
| test9u.test | 8 | 0.002109s | 0.002087s | 0.001992s | byte equality observed |
| test_actions.test | 8 | 0.002653s | 0.002652s | 0.002521s | byte equality observed |
| test_assert.test | 9 | 0.002183s | 0.002155s | 0.002125s | byte equality observed |
| test_assert_with_placeholder.test | 10 | 0.002029s | 0.001978s | 0.001969s | byte equality observed |
| test_cycles.test | 10 | 0.002100s | 0.002056s | 0.002006s | byte equality observed |
| test_dynamic.test | 10 | 0.002072s | 0.001999s | 0.001995s | byte equality observed |
| test_dynamic_grammar.test | 10 | 0.002227s | 0.002145s | 0.002152s | byte equality observed |
| test_format.test | 10 | 0.002169s | 0.002157s | 0.002075s | byte equality observed |
| test_non_char.test | 10 | 0.002159s | 0.002150s | 0.002076s | byte equality observed |
| test_skip.test | 10 | 0.002048s | 0.002043s | 0.001999s | byte equality observed |
| test_static.test | 10 | 0.001999s | 0.001915s | 0.001895s | byte equality observed |
| test_symbols.test | 9 | 0.002416s | 0.002378s | 0.002274s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43479737952).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
