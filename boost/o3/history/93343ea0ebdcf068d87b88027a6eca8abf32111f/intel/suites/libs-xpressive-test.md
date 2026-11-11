# libs/xpressive/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| regress.test | RUN | 4.508261 | 23.606804 | 18.248590 | 0.013023 | 0.017057 | 0.013193 |
| regress_u.test | RUN | 4.789510 | 24.560882 | 17.786889 | 0.014134 | 0.016608 | 0.012929 |
| c_traits.test | RUN | 4.450068 | 22.663942 | 17.148648 | 0.010605 | 0.017146 | 0.010860 |
| c_traits_u.test | RUN | 4.657356 | 23.986190 | 17.720747 | 0.012290 | 0.014938 | 0.010785 |
| test1.test | RUN | 2.065457 | 5.989726 | 5.306670 | 0.003378 | 0.004782 | 0.003574 |
| test2.test | RUN | 2.353649 | 8.289548 | 7.722659 | 0.003019 | 0.003395 | 0.003511 |
| test3.test | RUN | 1.915633 | 9.056596 | 8.169345 | 0.004080 | 0.003687 | 0.003301 |
| test4.test | RUN | 1.672998 | 7.117866 | 6.421559 | 0.002764 | 0.003281 | 0.003549 |
| test5.test | RUN | 1.778958 | 7.470443 | 6.985296 | 0.003145 | 0.013645 | 0.003623 |
| test6.test | RUN | 2.240177 | 6.375466 | 5.720830 | 0.003507 | 0.004075 | 0.003605 |
| test7.test | RUN | 1.785248 | 6.522461 | 5.842854 | 0.003441 | 0.003897 | 0.003780 |
| test8.test | RUN | 1.606406 | 6.425749 | 5.581150 | 0.003106 | 0.003537 | 0.003321 |
| test9.test | RUN | 1.706250 | 6.667211 | 6.547448 | 0.003408 | 0.002893 | 0.003290 |
| test10.test | RUN | 1.685148 | 6.591433 | 5.884108 | 0.003134 | 0.003190 | 0.003401 |
| test11.test | RUN | 1.708434 | 6.111664 | 5.902390 | 0.003348 | 0.003608 | 0.003186 |
| test1u.test | RUN | 1.525616 | 5.977447 | 5.197121 | 0.003345 | 0.003638 | 0.003339 |
| test2u.test | RUN | 1.985354 | 8.569869 | 7.741870 | 0.003211 | 0.003780 | 0.003596 |
| test3u.test | RUN | 1.921471 | 9.082077 | 8.082051 | 0.004698 | 0.003829 | 0.003834 |
| test4u.test | RUN | 1.779568 | 7.249476 | 6.631527 | 0.003235 | 0.005009 | 0.003148 |
| test5u.test | RUN | 1.771103 | 8.244688 | 7.080014 | 0.003300 | 0.004712 | 0.003100 |
| test6u.test | RUN | 1.646677 | 6.193140 | 6.125178 | 0.003600 | 0.003738 | 0.003742 |
| test7u.test | RUN | 1.584631 | 6.906243 | 6.192053 | 0.003174 | 0.003785 | 0.002657 |
| test8u.test | RUN | 1.576338 | 6.356589 | 5.998310 | 0.002909 | 0.003010 | 0.005025 |
| test9u.test | RUN | 1.690493 | 7.369050 | 6.591756 | 0.003370 | 0.003416 | 0.004636 |
| test10u.test | RUN | 1.652306 | 6.174429 | 6.057824 | 0.003168 | 0.003597 | 0.004137 |
| test11u.test | RUN | 1.586147 | 6.415013 | 6.217142 | 0.003082 | 0.003797 | 0.003699 |
| misc1.test | RUN | 3.245234 | 16.560465 | 13.115642 | 0.004070 | 0.004331 | 0.005313 |
| misc2.test | RUN | 3.041387 | 15.492258 | 13.094178 | 0.003702 | 0.003554 | 0.003708 |
| test_format.test | RUN | 3.382585 | 18.739639 | 14.831734 | 0.003274 | 0.003292 | 0.003460 |
| test_cycles.test | RUN | 1.363223 | 5.795733 | 5.297816 | 0.003547 | 0.004370 | 0.003750 |
| test_non_char.test | RUN | 2.668542 | 13.221805 | 10.983454 | 0.003490 | 0.003438 | 0.003776 |
| test_static.test | RUN | 1.287475 | 5.144355 | 4.326739 | 0.003259 | 0.004399 | 0.003426 |
| test_actions.test | RUN | 3.275645 | 15.933851 | 13.266987 | 0.004178 | 0.004940 | 0.005562 |
| test_assert.test | RUN | 1.731502 | 6.635833 | 7.077015 | 0.003371 | 0.005166 | 0.003545 |
| test_assert_with_placeholder.test | RUN | 1.635154 | 6.057576 | 6.138568 | 0.003313 | 0.004075 | 0.003260 |
| test_symbols.test | RUN | 3.254814 | 16.064795 | 15.208056 | 0.005355 | 0.004607 | 0.005642 |
| test_dynamic.test | RUN | 2.210373 | 10.668953 | 8.765907 | 0.003551 | 0.003610 | 0.004763 |
| test_dynamic_grammar.test | RUN | 2.265689 | 11.278150 | 9.235379 | 0.003509 | 0.003504 | 0.003522 |
| test_skip.test | RUN | 2.440330 | 11.274891 | 10.274268 | 0.003429 | 0.003669 | 0.003561 |
| multiple_defs.test | LINK | 4.305106 | 21.766702 | 17.609448 | — | — | — |
| test_basic_regex.test | COMPILE | 0.863444 | 1.643766 | 2.389507 | — | — | — |
| test_match_results.test | COMPILE | 0.578118 | 1.142444 | 1.661501 | — | — | — |
| test_regex_algorithms.test | COMPILE | 0.603422 | 1.133747 | 1.600548 | — | — | — |
| test_regex_compiler.test | COMPILE | 0.889642 | 1.852952 | 2.474679 | — | — | — |
| test_regex_constants.test | COMPILE | 0.052499 | 0.071911 | 0.103456 | — | — | — |
| test_regex_error.test | COMPILE | 0.314612 | 0.606945 | 0.921730 | — | — | — |
| test_regex_iterator.test | COMPILE | 0.817137 | 1.133708 | 1.651108 | — | — | — |
| test_regex_primitives.test | COMPILE | 1.117399 | 1.812771 | 2.306742 | — | — | — |
| test_regex_token_iterator.test | COMPILE | 0.574410 | 1.154547 | 1.692909 | — | — | — |
| test_regex_traits.test | COMPILE | 0.333182 | 0.620850 | 0.965609 | — | — | — |
| test_sub_match.test | COMPILE | 0.279361 | 0.501817 | 0.693314 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.078× | 1.070×, 1.087× | 0.961× |
| clang | 0.870×† | 0.797×, 0.944× | 0.932× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1353 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| c_traits.test | 2 | 0.010033s | 0.013416s | 0.009705s | byte equality observed |
| c_traits_u.test | 2 | 0.011768s | 0.014160s | 0.010616s | byte equality observed |
| misc1.test | 4 | 0.003607s | 0.003410s | 0.003208s | byte equality observed |
| misc2.test | 5 | 0.003145s | 0.003495s | 0.002841s | byte equality observed |
| regress.test | 1 | 0.012962s | 0.015851s | 0.011209s | byte equality observed |
| regress_u.test | 1 | 0.013677s | 0.015705s | 0.011683s | byte equality observed |
| test1.test | 5 | 0.002850s | 0.002962s | 0.002592s | byte equality observed |
| test10.test | 6 | 0.002834s | 0.002790s | 0.002559s | byte equality observed |
| test10u.test | 6 | 0.002900s | 0.002696s | 0.002611s | byte equality observed |
| test11.test | 5 | 0.002823s | 0.002889s | 0.002571s | byte equality observed |
| test11u.test | 6 | 0.002981s | 0.002688s | 0.002678s | byte equality observed |
| test1u.test | 5 | 0.002902s | 0.002976s | 0.002657s | byte equality observed |
| test2.test | 5 | 0.002863s | 0.002875s | 0.002558s | byte equality observed |
| test2u.test | 5 | 0.003060s | 0.002881s | 0.002721s | byte equality observed |
| test3.test | 5 | 0.003006s | 0.002929s | 0.002802s | byte equality observed |
| test3u.test | 5 | 0.002952s | 0.002991s | 0.002785s | byte equality observed |
| test4.test | 6 | 0.002892s | 0.002788s | 0.002630s | byte equality observed |
| test4u.test | 5 | 0.002923s | 0.002916s | 0.002565s | byte equality observed |
| test5.test | 3 | 0.002816s | 0.003008s | 0.002515s | byte equality observed |
| test5u.test | 4 | 0.002917s | 0.002995s | 0.002763s | byte equality observed |
| test6.test | 4 | 0.002911s | 0.002911s | 0.002628s | byte equality observed |
| test6u.test | 4 | 0.002951s | 0.002996s | 0.002689s | byte equality observed |
| test7.test | 4 | 0.002757s | 0.002816s | 0.002591s | byte equality observed |
| test7u.test | 5 | 0.002923s | 0.002951s | 0.002700s | byte equality observed |
| test8.test | 5 | 0.002703s | 0.003003s | 0.002610s | byte equality observed |
| test8u.test | 5 | 0.002804s | 0.003008s | 0.002660s | byte equality observed |
| test9.test | 4 | 0.002875s | 0.003090s | 0.002802s | byte equality observed |
| test9u.test | 5 | 0.002794s | 0.002868s | 0.002720s | byte equality observed |
| test_actions.test | 4 | 0.003394s | 0.003568s | 0.003239s | byte equality observed |
| test_assert.test | 4 | 0.003011s | 0.002812s | 0.002732s | byte equality observed |
| test_assert_with_placeholder.test | 5 | 0.002665s | 0.002892s | 0.002828s | byte equality observed |
| test_cycles.test | 5 | 0.002891s | 0.002708s | 0.002539s | byte equality observed |
| test_dynamic.test | 5 | 0.002732s | 0.002639s | 0.002769s | byte equality observed |
| test_dynamic_grammar.test | 5 | 0.002985s | 0.002880s | 0.002769s | byte equality observed |
| test_format.test | 5 | 0.003123s | 0.002926s | 0.002764s | byte equality observed |
| test_non_char.test | 5 | 0.002865s | 0.002706s | 0.002773s | byte equality observed |
| test_skip.test | 5 | 0.002736s | 0.002642s | 0.002505s | byte equality observed |
| test_static.test | 4 | 0.002665s | 0.002851s | 0.002559s | byte equality observed |
| test_symbols.test | 3 | 0.003481s | 0.003257s | 0.003132s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
