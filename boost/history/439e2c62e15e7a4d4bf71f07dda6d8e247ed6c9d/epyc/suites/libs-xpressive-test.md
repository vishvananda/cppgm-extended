# libs/xpressive/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| regress.test | RUN | 1.142610 | 7.137803 | 4.910852 | 0.029192 | 0.031253 | 0.034476 |
| regress_u.test | RUN | 1.232987 | 7.311228 | 5.137351 | 0.030669 | 0.032746 | 0.037003 |
| c_traits.test | RUN | 1.105383 | 6.995499 | 4.795345 | 0.025514 | 0.033282 | 0.028679 |
| c_traits_u.test | RUN | 1.212022 | 7.414574 | 5.088663 | 0.027241 | 0.035821 | 0.033235 |
| test1.test | RUN | 0.743684 | 2.416620 | 2.266984 | 0.003177 | 0.003543 | 0.003632 |
| test2.test | RUN | 0.870293 | 3.594836 | 3.039988 | 0.003439 | 0.004004 | 0.003793 |
| test3.test | RUN | 0.872846 | 3.752734 | 3.124847 | 0.003082 | 0.003582 | 0.003492 |
| test4.test | RUN | 0.799762 | 3.038976 | 2.582775 | 0.003362 | 0.003724 | 0.003766 |
| test5.test | RUN | 0.816956 | 3.223216 | 2.759206 | 0.003228 | 0.003779 | 0.003517 |
| test6.test | RUN | 0.779712 | 2.853059 | 2.511401 | 0.003158 | 0.003722 | 0.003457 |
| test7.test | RUN | 0.760915 | 2.770283 | 2.433600 | 0.003181 | 0.003449 | 0.003438 |
| test8.test | RUN | 0.763254 | 2.752351 | 2.401433 | 0.003147 | 0.003573 | 0.003763 |
| test9.test | RUN | 0.809442 | 3.112779 | 2.626986 | 0.003209 | 0.003796 | 0.003640 |
| test10.test | RUN | 0.796996 | 2.998155 | 2.565246 | 0.003231 | 0.003771 | 0.003393 |
| test11.test | RUN | 0.784813 | 2.881828 | 2.478620 | 0.003132 | 0.003498 | 0.003601 |
| test1u.test | RUN | 0.726288 | 2.445686 | 2.222241 | 0.003409 | 0.003220 | 0.003460 |
| test2u.test | RUN | 0.911203 | 3.886565 | 3.214205 | 0.003585 | 0.003983 | 0.003866 |
| test3u.test | RUN | 0.907742 | 4.249074 | 3.262590 | 0.003253 | 0.003740 | 0.003784 |
| test4u.test | RUN | 0.819307 | 3.057735 | 2.643277 | 0.003405 | 0.003902 | 0.003253 |
| test5u.test | RUN | 0.840877 | 3.314174 | 2.812412 | 0.003456 | 0.003716 | 0.003599 |
| test6u.test | RUN | 0.842203 | 2.991275 | 2.593937 | 0.003230 | 0.003548 | 0.003415 |
| test7u.test | RUN | 0.796998 | 2.941177 | 2.622926 | 0.003193 | 0.003435 | 0.003612 |
| test8u.test | RUN | 0.791179 | 2.750672 | 2.409232 | 0.003181 | 0.003657 | 0.003592 |
| test9u.test | RUN | 0.805196 | 3.181413 | 2.713670 | 0.003314 | 0.003881 | 0.003579 |
| test10u.test | RUN | 0.822192 | 2.958531 | 2.633348 | 0.003378 | 0.003279 | 0.003595 |
| test11u.test | RUN | 0.802451 | 3.004990 | 2.614546 | 0.003409 | 0.003686 | 0.003487 |
| misc1.test | RUN | 1.051212 | 6.038873 | 4.369390 | 0.004641 | 0.004533 | 0.004532 |
| misc2.test | RUN | 1.186686 | 6.031429 | 4.508378 | 0.003738 | 0.004152 | 0.003899 |
| test_format.test | RUN | 1.053720 | 6.038042 | 3.955758 | 0.003602 | 0.003643 | 0.003743 |
| test_cycles.test | RUN | 0.713461 | 2.297096 | 2.162549 | 0.003586 | 0.003667 | 0.003457 |
| test_non_char.test | RUN | 0.949913 | 5.167484 | 3.567201 | 0.003517 | 0.003798 | 0.003644 |
| test_static.test | RUN | 0.698893 | 2.050607 | 1.945516 | 0.003531 | 0.003552 | 0.003381 |
| test_actions.test | RUN | 1.386983 | 7.502502 | 5.862863 | 0.004819 | 0.005050 | 0.004675 |
| test_assert.test | RUN | 0.911992 | 3.173941 | 2.913968 | 0.003754 | 0.003917 | 0.003782 |
| test_assert_with_placeholder.test | RUN | 0.905933 | 2.790167 | 2.548766 | 0.003490 | 0.003665 | 0.003487 |
| test_symbols.test | RUN | 1.532054 | 8.530798 | 6.246549 | 0.004157 | 0.004028 | 0.005012 |
| test_dynamic.test | RUN | 0.850094 | 3.701339 | 2.802726 | 0.003306 | 0.003668 | 0.003481 |
| test_dynamic_grammar.test | RUN | 0.871977 | 3.722780 | 3.037084 | 0.003691 | 0.004037 | 0.003867 |
| test_skip.test | RUN | 1.124616 | 5.340845 | 4.240831 | 0.003399 | 0.003613 | 0.003388 |
| multiple_defs.test | LINK | 1.603730 | 7.112100 | 5.701243 | — | — | — |
| test_basic_regex.test | COMPILE | 0.549012 | 1.021341 | 1.206227 | — | — | — |
| test_match_results.test | COMPILE | 0.366787 | 0.617824 | 0.794939 | — | — | — |
| test_regex_algorithms.test | COMPILE | 0.367720 | 0.661837 | 0.802763 | — | — | — |
| test_regex_compiler.test | COMPILE | 0.552841 | 1.080872 | 1.181488 | — | — | — |
| test_regex_constants.test | COMPILE | 0.035586 | 0.038436 | 0.051459 | — | — | — |
| test_regex_error.test | COMPILE | 0.197234 | 0.334386 | 0.437286 | — | — | — |
| test_regex_iterator.test | COMPILE | 0.357541 | 0.647191 | 0.781125 | — | — | — |
| test_regex_primitives.test | COMPILE | 0.559105 | 0.991372 | 1.085915 | — | — | — |
| test_regex_token_iterator.test | COMPILE | 0.368266 | 0.663703 | 0.802764 | — | — | — |
| test_regex_traits.test | COMPILE | 0.214913 | 0.341859 | 0.427296 | — | — | — |
| test_sub_match.test | COMPILE | 0.158602 | 0.272743 | 0.294064 | — | — | — |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42568872153) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42568872153) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42568872153).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
