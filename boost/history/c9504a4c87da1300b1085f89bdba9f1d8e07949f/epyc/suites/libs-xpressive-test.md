# libs/xpressive/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| regress.test | RUN | 1.591989 | 9.443863 | 7.600090 | 0.048446 | 0.054438 | 0.056817 |
| regress_u.test | RUN | 1.654521 | 9.772842 | 7.866573 | 0.049934 | 0.054288 | 0.059690 |
| c_traits.test | RUN | 1.549163 | 9.039811 | 7.137212 | 0.037119 | 0.050462 | 0.050710 |
| c_traits_u.test | RUN | 1.644391 | 9.700063 | 7.808491 | 0.044180 | 0.046397 | 0.054281 |
| test1.test | RUN | 1.088805 | 3.270655 | 3.564691 | 0.003852 | 0.004496 | 0.004522 |
| test2.test | RUN | 1.190033 | 4.811801 | 4.745688 | 0.004228 | 0.004978 | 0.004820 |
| test3.test | RUN | 1.203551 | 4.943907 | 4.907864 | 0.004056 | 0.004754 | 0.005238 |
| test4.test | RUN | 1.093212 | 4.009879 | 4.107019 | 0.003921 | 0.004267 | 0.004414 |
| test5.test | RUN | 1.119871 | 4.364953 | 4.328418 | 0.004047 | 0.004655 | 0.004863 |
| test6.test | RUN | 1.048397 | 3.834867 | 3.958774 | 0.003868 | 0.004675 | 0.004863 |
| test7.test | RUN | 1.055167 | 3.627992 | 3.891643 | 0.003900 | 0.004744 | 0.004313 |
| test8.test | RUN | 1.057306 | 3.603758 | 3.771672 | 0.003874 | 0.004564 | 0.004509 |
| test9.test | RUN | 1.099139 | 4.063197 | 4.197465 | 0.004151 | 0.004507 | 0.004664 |
| test10.test | RUN | 1.118808 | 3.982850 | 4.136162 | 0.003750 | 0.004133 | 0.004640 |
| test11.test | RUN | 1.079158 | 3.774764 | 3.936310 | 0.003824 | 0.004231 | 0.004531 |
| test1u.test | RUN | 1.010793 | 3.242031 | 3.491148 | 0.003704 | 0.004131 | 0.004595 |
| test2u.test | RUN | 1.261038 | 5.049282 | 5.011013 | 0.004308 | 0.004438 | 0.005134 |
| test3u.test | RUN | 1.232703 | 5.402408 | 5.177313 | 0.004092 | 0.004556 | 0.004795 |
| test4u.test | RUN | 1.110651 | 4.113528 | 4.096378 | 0.003962 | 0.004572 | 0.004914 |
| test5u.test | RUN | 1.154245 | 4.461885 | 4.447700 | 0.003823 | 0.004712 | 0.004798 |
| test6u.test | RUN | 1.122535 | 3.984271 | 4.255251 | 0.003820 | 0.004329 | 0.004560 |
| test7u.test | RUN | 1.095410 | 3.941833 | 4.020674 | 0.003970 | 0.004424 | 0.004684 |
| test8u.test | RUN | 1.072040 | 3.640269 | 3.712270 | 0.003849 | 0.004690 | 0.004532 |
| test9u.test | RUN | 1.118117 | 4.241658 | 4.198750 | 0.004048 | 0.004342 | 0.004516 |
| test10u.test | RUN | 1.121687 | 3.979520 | 4.084279 | 0.003935 | 0.004640 | 0.004465 |
| test11u.test | RUN | 1.114324 | 4.056632 | 4.147110 | 0.004034 | 0.004545 | 0.004644 |
| misc1.test | RUN | 1.438565 | 8.121131 | 6.821748 | 0.005520 | 0.006070 | 0.005371 |
| misc2.test | RUN | 1.612256 | 7.977981 | 6.826150 | 0.004458 | 0.005145 | 0.004611 |
| test_format.test | RUN | 1.422450 | 7.725241 | 6.107609 | 0.004284 | 0.004288 | 0.004732 |
| test_cycles.test | RUN | 1.006141 | 3.089258 | 3.284625 | 0.004366 | 0.004775 | 0.004617 |
| test_non_char.test | RUN | 1.320053 | 6.589616 | 5.592244 | 0.004261 | 0.004305 | 0.004562 |
| test_static.test | RUN | 0.948874 | 2.857027 | 3.103662 | 0.003937 | 0.004172 | 0.004318 |
| test_actions.test | RUN | 1.857676 | 9.677420 | 8.844650 | 0.005344 | 0.006400 | 0.006223 |
| test_assert.test | RUN | 1.288213 | 4.243537 | 4.558242 | 0.004069 | 0.004830 | 0.004690 |
| test_assert_with_placeholder.test | RUN | 1.233284 | 3.379343 | 3.923736 | 0.004089 | 0.003801 | 0.004575 |
| test_symbols.test | RUN | 2.036993 | 11.136770 | 9.413031 | 0.005643 | 0.006223 | 0.006568 |
| test_dynamic.test | RUN | 1.142769 | 4.895251 | 4.439009 | 0.003819 | 0.004445 | 0.004388 |
| test_dynamic_grammar.test | RUN | 1.207171 | 4.891934 | 4.581048 | 0.004373 | 0.005096 | 0.004962 |
| test_skip.test | RUN | 1.551240 | 6.858236 | 6.465523 | 0.004032 | 0.004544 | 0.004580 |
| multiple_defs.test | LINK | 2.225185 | 9.362811 | 8.705273 | — | — | — |
| test_basic_regex.test | COMPILE | 0.769845 | 1.332725 | 1.868884 | — | — | — |
| test_match_results.test | COMPILE | 0.507750 | 0.895056 | 1.225550 | — | — | — |
| test_regex_algorithms.test | COMPILE | 0.511904 | 0.917602 | 1.264932 | — | — | — |
| test_regex_compiler.test | COMPILE | 0.773111 | 1.480210 | 1.947962 | — | — | — |
| test_regex_constants.test | COMPILE | 0.044647 | 0.055339 | 0.073363 | — | — | — |
| test_regex_error.test | COMPILE | 0.278493 | 0.454693 | 0.714975 | — | — | — |
| test_regex_iterator.test | COMPILE | 0.516681 | 0.882597 | 1.279724 | — | — | — |
| test_regex_primitives.test | COMPILE | 0.763285 | 1.372618 | 1.749353 | — | — | — |
| test_regex_token_iterator.test | COMPILE | 0.511078 | 0.913344 | 1.292759 | — | — | — |
| test_regex_traits.test | COMPILE | 0.300194 | 0.490090 | 0.704280 | — | — | — |
| test_sub_match.test | COMPILE | 0.247496 | 0.366749 | 0.455873 | — | — | — |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42570601152) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42570601152) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42570601152).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
