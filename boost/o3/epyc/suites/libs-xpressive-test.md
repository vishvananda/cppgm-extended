# libs/xpressive/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| regress.test | RUN | 3.942181 | 23.519150 | 20.680118 | 0.015095 | 0.019312 | 0.013706 |
| regress_u.test | RUN | 4.100458 | 24.358967 | 20.924325 | 0.016254 | 0.019435 | 0.013752 |
| c_traits.test | RUN | 3.937553 | 23.329378 | 20.161948 | 0.010760 | 0.014949 | 0.010695 |
| c_traits_u.test | RUN | 4.075064 | 24.060082 | 20.839204 | 0.013237 | 0.017848 | 0.012246 |
| test1.test | RUN | 1.468040 | 6.186361 | 6.400815 | 0.003441 | 0.003369 | 0.003957 |
| test2.test | RUN | 1.920788 | 8.608215 | 9.199987 | 0.003554 | 0.003655 | 0.003873 |
| test3.test | RUN | 1.962970 | 8.694481 | 9.432663 | 0.003637 | 0.003687 | 0.004160 |
| test4.test | RUN | 1.694976 | 7.108100 | 7.772278 | 0.003766 | 0.003807 | 0.004016 |
| test5.test | RUN | 1.822096 | 7.740585 | 8.349948 | 0.003629 | 0.003539 | 0.003918 |
| test6.test | RUN | 1.626752 | 6.530041 | 7.165689 | 0.003588 | 0.003654 | 0.004132 |
| test7.test | RUN | 1.564908 | 6.697942 | 7.123573 | 0.003481 | 0.003916 | 0.004094 |
| test8.test | RUN | 1.527556 | 6.423599 | 6.989738 | 0.003500 | 0.003683 | 0.003887 |
| test9.test | RUN | 1.655579 | 6.943813 | 7.932088 | 0.004397 | 0.003437 | 0.004184 |
| test10.test | RUN | 1.662071 | 6.666937 | 7.331006 | 0.003590 | 0.003928 | 0.004107 |
| test11.test | RUN | 1.600195 | 6.327030 | 7.397970 | 0.003325 | 0.004117 | 0.004129 |
| test1u.test | RUN | 1.490499 | 5.930525 | 6.592580 | 0.004150 | 0.004167 | 0.003823 |
| test2u.test | RUN | 1.970505 | 8.877317 | 9.602035 | 0.003926 | 0.003520 | 0.004279 |
| test3u.test | RUN | 1.960498 | 9.071294 | 9.819790 | 0.003670 | 0.003647 | 0.004277 |
| test4u.test | RUN | 1.733664 | 7.115108 | 7.978725 | 0.004222 | 0.004981 | 0.004444 |
| test5u.test | RUN | 1.818396 | 7.909057 | 9.094779 | 0.003683 | 0.004813 | 0.004160 |
| test6u.test | RUN | 1.742336 | 6.532574 | 7.545565 | 0.003727 | 0.003885 | 0.004070 |
| test7u.test | RUN | 1.621072 | 6.877597 | 7.551068 | 0.003714 | 0.003655 | 0.004162 |
| test8u.test | RUN | 1.573743 | 6.489035 | 7.080133 | 0.003625 | 0.003937 | 0.004081 |
| test9u.test | RUN | 1.693755 | 7.000145 | 8.053185 | 0.003799 | 0.003731 | 0.003980 |
| test10u.test | RUN | 1.679316 | 6.436946 | 7.399008 | 0.003877 | 0.003902 | 0.004191 |
| test11u.test | RUN | 1.748792 | 6.746495 | 7.683212 | 0.003963 | 0.004058 | 0.004127 |
| misc1.test | RUN | 2.984029 | 16.397033 | 15.893005 | 0.004744 | 0.005098 | 0.005058 |
| misc2.test | RUN | 3.053663 | 15.550522 | 15.428480 | 0.003993 | 0.004079 | 0.004700 |
| test_format.test | RUN | 3.205473 | 18.958543 | 17.528051 | 0.004272 | 0.003852 | 0.003822 |
| test_cycles.test | RUN | 1.394517 | 5.979721 | 6.027908 | 0.003700 | 0.004155 | 0.003937 |
| test_non_char.test | RUN | 2.659773 | 13.123181 | 13.680558 | 0.003774 | 0.004289 | 0.005209 |
| test_static.test | RUN | 1.344517 | 5.464743 | 5.312747 | 0.003907 | 0.003670 | 0.003576 |
| test_actions.test | RUN | 3.118512 | 16.141406 | 16.720180 | 0.004979 | 0.005120 | 0.005735 |
| test_assert.test | RUN | 1.881135 | 6.935076 | 7.804043 | 0.004022 | 0.003754 | 0.004434 |
| test_assert_with_placeholder.test | RUN | 1.720927 | 6.504000 | 7.124189 | 0.003919 | 0.003695 | 0.003721 |
| test_symbols.test | RUN | 3.286562 | 15.495107 | 18.616606 | 0.005078 | 0.004901 | 0.005755 |
| test_dynamic.test | RUN | 2.124050 | 10.381528 | 10.768348 | 0.003845 | 0.003641 | 0.003987 |
| test_dynamic_grammar.test | RUN | 2.214682 | 11.175601 | 10.974840 | 0.004093 | 0.004178 | 0.004402 |
| test_skip.test | RUN | 2.539498 | 11.583950 | 12.909015 | 0.003825 | 0.003599 | 0.004186 |
| multiple_defs.test | LINK | 4.337629 | 21.120831 | 21.520607 | — | — | — |
| test_basic_regex.test | COMPILE | 0.904343 | 1.833910 | 3.127194 | — | — | — |
| test_match_results.test | COMPILE | 0.608678 | 1.230978 | 2.058305 | — | — | — |
| test_regex_algorithms.test | COMPILE | 0.597329 | 1.229330 | 2.053097 | — | — | — |
| test_regex_compiler.test | COMPILE | 0.932516 | 1.951005 | 3.066643 | — | — | — |
| test_regex_constants.test | COMPILE | 0.050875 | 0.079288 | 0.099044 | — | — | — |
| test_regex_error.test | COMPILE | 0.334314 | 0.627700 | 1.062429 | — | — | — |
| test_regex_iterator.test | COMPILE | 0.631852 | 1.241069 | 2.014575 | — | — | — |
| test_regex_primitives.test | COMPILE | 0.884664 | 1.939075 | 2.423900 | — | — | — |
| test_regex_token_iterator.test | COMPILE | 0.594511 | 1.225665 | 1.871276 | — | — | — |
| test_regex_traits.test | COMPILE | 0.354684 | 0.629185 | 1.107094 | — | — | — |
| test_sub_match.test | COMPILE | 0.290572 | 0.480263 | 0.861234 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.079× | 1.072×, 1.086× | 0.995× |
| clang | 0.927× | 0.928×, 0.925× | 0.992× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43133898266).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| c_traits.test | 2 | 0.010521s | 0.014731s | 0.009064s | byte equality observed |
| c_traits_u.test | 2 | 0.012403s | 0.015317s | 0.009982s | byte equality observed |
| misc1.test | 6 | 0.003460s | 0.003352s | 0.003196s | byte equality observed |
| misc2.test | 7 | 0.003023s | 0.002948s | 0.002850s | byte equality observed |
| regress.test | 2 | 0.014145s | 0.017121s | 0.011842s | byte equality observed |
| regress_u.test | 2 | 0.014756s | 0.018012s | 0.012151s | byte equality observed |
| test1.test | 6 | 0.002752s | 0.002646s | 0.002622s | byte equality observed |
| test10.test | 7 | 0.002689s | 0.002669s | 0.002632s | byte equality observed |
| test10u.test | 8 | 0.002640s | 0.002683s | 0.002566s | byte equality observed |
| test11.test | 8 | 0.002739s | 0.002695s | 0.002558s | byte equality observed |
| test11u.test | 5 | 0.002668s | 0.002672s | 0.002591s | byte equality observed |
| test1u.test | 6 | 0.002722s | 0.002719s | 0.002702s | byte equality observed |
| test2.test | 7 | 0.002719s | 0.002824s | 0.002627s | byte equality observed |
| test2u.test | 7 | 0.002725s | 0.002788s | 0.002697s | byte equality observed |
| test3.test | 8 | 0.002733s | 0.002772s | 0.002640s | byte equality observed |
| test3u.test | 7 | 0.002742s | 0.002749s | 0.002663s | byte equality observed |
| test4.test | 8 | 0.002727s | 0.002694s | 0.002668s | byte equality observed |
| test4u.test | 8 | 0.002690s | 0.002726s | 0.002610s | byte equality observed |
| test5.test | 8 | 0.002763s | 0.002786s | 0.002658s | byte equality observed |
| test5u.test | 8 | 0.002784s | 0.002738s | 0.002665s | byte equality observed |
| test6.test | 8 | 0.002700s | 0.002750s | 0.002605s | byte equality observed |
| test6u.test | 8 | 0.002703s | 0.002631s | 0.002703s | byte equality observed |
| test7.test | 7 | 0.002756s | 0.002656s | 0.002634s | byte equality observed |
| test7u.test | 7 | 0.002740s | 0.002681s | 0.002693s | byte equality observed |
| test8.test | 8 | 0.002690s | 0.002639s | 0.002626s | byte equality observed |
| test8u.test | 6 | 0.002719s | 0.002705s | 0.002716s | byte equality observed |
| test9.test | 8 | 0.002697s | 0.002708s | 0.002702s | byte equality observed |
| test9u.test | 8 | 0.002687s | 0.002738s | 0.002609s | byte equality observed |
| test_actions.test | 7 | 0.003422s | 0.003430s | 0.003253s | byte equality observed |
| test_assert.test | 7 | 0.002854s | 0.002817s | 0.002750s | byte equality observed |
| test_assert_with_placeholder.test | 8 | 0.002628s | 0.002613s | 0.002569s | byte equality observed |
| test_cycles.test | 7 | 0.002733s | 0.002672s | 0.002625s | byte equality observed |
| test_dynamic.test | 8 | 0.002732s | 0.002668s | 0.002612s | byte equality observed |
| test_dynamic_grammar.test | 7 | 0.002922s | 0.002833s | 0.002828s | byte equality observed |
| test_format.test | 8 | 0.002828s | 0.002825s | 0.002730s | byte equality observed |
| test_non_char.test | 8 | 0.002796s | 0.002739s | 0.002688s | byte equality observed |
| test_skip.test | 7 | 0.002662s | 0.002589s | 0.002552s | byte equality observed |
| test_static.test | 8 | 0.002536s | 0.002486s | 0.002476s | byte equality observed |
| test_symbols.test | 7 | 0.002887s | 0.003037s | 0.002803s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43133898266).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
