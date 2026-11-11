# libs/xpressive/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| regress.test | RUN | 2.811816 | 15.164295 | 11.488197 | 0.010130 | 0.013794 | 0.007322 |
| regress_u.test | RUN | 3.038380 | 15.579187 | 11.293803 | 0.009704 | 0.011310 | 0.008299 |
| c_traits.test | RUN | 2.866269 | 15.037715 | 10.891993 | 0.007601 | 0.009317 | 0.006867 |
| c_traits_u.test | RUN | 3.066908 | 15.535325 | 11.324957 | 0.008364 | 0.010032 | 0.007593 |
| test1.test | RUN | 1.069240 | 3.812400 | 3.143231 | 0.003482 | 0.003052 | 0.002948 |
| test2.test | RUN | 1.364552 | 5.324072 | 4.829167 | 0.003276 | 0.003161 | 0.003203 |
| test3.test | RUN | 1.371347 | 5.490498 | 4.810913 | 0.004076 | 0.003249 | 0.002967 |
| test4.test | RUN | 1.231425 | 4.593680 | 3.910795 | 0.003175 | 0.003277 | 0.003100 |
| test5.test | RUN | 1.267474 | 4.915561 | 4.134091 | 0.003494 | 0.003260 | 0.003026 |
| test6.test | RUN | 1.194818 | 4.137945 | 3.494571 | 0.003362 | 0.003073 | 0.002664 |
| test7.test | RUN | 1.126553 | 4.270447 | 3.504630 | 0.003049 | 0.002983 | 0.002876 |
| test8.test | RUN | 1.111104 | 4.149596 | 3.461075 | 0.002870 | 0.003200 | 0.002783 |
| test9.test | RUN | 1.209984 | 4.425352 | 3.818850 | 0.003492 | 0.003078 | 0.002424 |
| test10.test | RUN | 1.187048 | 4.138932 | 3.528642 | 0.003308 | 0.003180 | 0.002564 |
| test11.test | RUN | 1.150682 | 4.096347 | 3.518750 | 0.002961 | 0.002946 | 0.002357 |
| test1u.test | RUN | 1.044729 | 3.810499 | 3.063227 | 0.003127 | 0.003051 | 0.002586 |
| test2u.test | RUN | 1.426777 | 5.685150 | 4.775467 | 0.003426 | 0.003273 | 0.002908 |
| test3u.test | RUN | 1.417709 | 5.830824 | 4.899231 | 0.003482 | 0.003402 | 0.002989 |
| test4u.test | RUN | 1.272953 | 4.562303 | 3.782409 | 0.003345 | 0.003185 | 0.002894 |
| test5u.test | RUN | 1.330673 | 4.959770 | 4.247781 | 0.003133 | 0.003284 | 0.002871 |
| test6u.test | RUN | 1.217858 | 4.213827 | 3.644744 | 0.003495 | 0.003031 | 0.002712 |
| test7u.test | RUN | 1.237618 | 4.368433 | 3.711829 | 0.003293 | 0.003072 | 0.003213 |
| test8u.test | RUN | 1.194176 | 3.995438 | 3.500285 | 0.003287 | 0.003148 | 0.003027 |
| test9u.test | RUN | 1.263909 | 4.268714 | 4.068190 | 0.003376 | 0.003191 | 0.002925 |
| test10u.test | RUN | 1.319534 | 4.080237 | 3.673597 | 0.003283 | 0.003084 | 0.003096 |
| test11u.test | RUN | 1.273220 | 4.027235 | 3.795730 | 0.003239 | 0.003050 | 0.002937 |
| misc1.test | RUN | 2.235632 | 10.538113 | 8.467707 | 0.003957 | 0.003906 | 0.003725 |
| misc2.test | RUN | 2.251064 | 9.955278 | 8.183669 | 0.003592 | 0.003251 | 0.003520 |
| test_format.test | RUN | 2.405672 | 12.290661 | 9.754385 | 0.003503 | 0.003132 | 0.003151 |
| test_cycles.test | RUN | 1.073106 | 3.626290 | 2.977104 | 0.003336 | 0.003019 | 0.003010 |
| test_non_char.test | RUN | 1.922407 | 8.644778 | 7.493037 | 0.003272 | 0.003095 | 0.003257 |
| test_static.test | RUN | 0.943647 | 3.277465 | 2.759788 | 0.003291 | 0.002685 | 0.003158 |
| test_actions.test | RUN | 2.358321 | 10.153689 | 9.147973 | 0.003879 | 0.003954 | 0.003786 |
| test_assert.test | RUN | 1.409636 | 4.381162 | 4.070903 | 0.003604 | 0.003188 | 0.003305 |
| test_assert_with_placeholder.test | RUN | 1.269121 | 3.840393 | 3.681815 | 0.003104 | 0.003071 | 0.003044 |
| test_symbols.test | RUN | 2.473405 | 10.024424 | 9.836496 | 0.004342 | 0.003920 | 0.004058 |
| test_dynamic.test | RUN | 1.562467 | 6.849691 | 5.734770 | 0.003490 | 0.002991 | 0.003099 |
| test_dynamic_grammar.test | RUN | 1.549363 | 7.127265 | 5.950737 | 0.003240 | 0.003345 | 0.003204 |
| test_skip.test | RUN | 1.821631 | 7.571950 | 6.829183 | 0.003295 | 0.003089 | 0.003095 |
| multiple_defs.test | LINK | 3.100726 | 13.541845 | 11.415140 | — | — | — |
| test_basic_regex.test | COMPILE | 0.667290 | 1.130964 | 1.386582 | — | — | — |
| test_match_results.test | COMPILE | 0.453100 | 0.730321 | 0.939045 | — | — | — |
| test_regex_algorithms.test | COMPILE | 0.450374 | 0.722465 | 0.955634 | — | — | — |
| test_regex_compiler.test | COMPILE | 0.677454 | 1.198100 | 1.453214 | — | — | — |
| test_regex_constants.test | COMPILE | 0.044418 | 0.056731 | 0.058408 | — | — | — |
| test_regex_error.test | COMPILE | 0.262850 | 0.389893 | 0.515830 | — | — | — |
| test_regex_iterator.test | COMPILE | 0.449762 | 0.750328 | 0.915707 | — | — | — |
| test_regex_primitives.test | COMPILE | 0.631066 | 1.257664 | 1.255497 | — | — | — |
| test_regex_token_iterator.test | COMPILE | 0.415287 | 0.766347 | 0.878578 | — | — | — |
| test_regex_traits.test | COMPILE | 0.264465 | 0.399216 | 0.451788 | — | — | — |
| test_sub_match.test | COMPILE | 0.228895 | 0.326996 | 0.377204 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.075× | 1.078×, 1.072× | 0.965× |
| clang | 0.959×† | 0.931×, 0.987× | 1.005× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1362 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| c_traits.test | 3 | 0.005776s | 0.008550s | 0.005931s | byte equality observed |
| c_traits_u.test | 3 | 0.006545s | 0.008894s | 0.006328s | byte equality observed |
| misc1.test | 9 | 0.002446s | 0.002453s | 0.002349s | byte equality observed |
| misc2.test | 8 | 0.002179s | 0.002162s | 0.002101s | byte equality observed |
| regress.test | 3 | 0.008157s | 0.009481s | 0.006886s | byte equality observed |
| regress_u.test | 3 | 0.008090s | 0.009687s | 0.007121s | byte equality observed |
| test1.test | 10 | 0.001912s | 0.001977s | 0.001861s | byte equality observed |
| test10.test | 10 | 0.001890s | 0.001982s | 0.001865s | byte equality observed |
| test10u.test | 10 | 0.001876s | 0.001931s | 0.001871s | byte equality observed |
| test11.test | 10 | 0.001915s | 0.001937s | 0.001852s | byte equality observed |
| test11u.test | 10 | 0.001918s | 0.001935s | 0.001880s | byte equality observed |
| test1u.test | 10 | 0.001905s | 0.001930s | 0.001854s | byte equality observed |
| test2.test | 9 | 0.001930s | 0.001969s | 0.001882s | byte equality observed |
| test2u.test | 10 | 0.001971s | 0.002012s | 0.001948s | byte equality observed |
| test3.test | 10 | 0.001971s | 0.001993s | 0.001893s | byte equality observed |
| test3u.test | 10 | 0.001952s | 0.001980s | 0.001932s | byte equality observed |
| test4.test | 10 | 0.001933s | 0.001920s | 0.001837s | byte equality observed |
| test4u.test | 10 | 0.001947s | 0.001944s | 0.001877s | byte equality observed |
| test5.test | 10 | 0.001909s | 0.001967s | 0.001882s | byte equality observed |
| test5u.test | 8 | 0.001952s | 0.001914s | 0.001896s | byte equality observed |
| test6.test | 10 | 0.001938s | 0.001880s | 0.001851s | byte equality observed |
| test6u.test | 10 | 0.001921s | 0.001921s | 0.001900s | byte equality observed |
| test7.test | 10 | 0.001896s | 0.001915s | 0.001845s | byte equality observed |
| test7u.test | 10 | 0.001926s | 0.001898s | 0.001873s | byte equality observed |
| test8.test | 10 | 0.001910s | 0.001920s | 0.001838s | byte equality observed |
| test8u.test | 10 | 0.001910s | 0.001903s | 0.001855s | byte equality observed |
| test9.test | 10 | 0.001898s | 0.001911s | 0.001847s | byte equality observed |
| test9u.test | 10 | 0.001918s | 0.001942s | 0.001862s | byte equality observed |
| test_actions.test | 6 | 0.002410s | 0.002448s | 0.002296s | byte equality observed |
| test_assert.test | 10 | 0.001982s | 0.001991s | 0.001932s | byte equality observed |
| test_assert_with_placeholder.test | 10 | 0.001909s | 0.001893s | 0.001813s | byte equality observed |
| test_cycles.test | 10 | 0.001906s | 0.001914s | 0.001844s | byte equality observed |
| test_dynamic.test | 10 | 0.001922s | 0.001911s | 0.001849s | byte equality observed |
| test_dynamic_grammar.test | 10 | 0.002024s | 0.002015s | 0.001953s | byte equality observed |
| test_format.test | 10 | 0.002000s | 0.001979s | 0.001923s | byte equality observed |
| test_non_char.test | 10 | 0.001967s | 0.001944s | 0.001914s | byte equality observed |
| test_skip.test | 10 | 0.001877s | 0.001877s | 0.001851s | byte equality observed |
| test_static.test | 10 | 0.001751s | 0.001691s | 0.001707s | byte equality observed |
| test_symbols.test | 9 | 0.002162s | 0.002173s | 0.002108s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
