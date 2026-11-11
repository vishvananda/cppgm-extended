# libs/xpressive/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| regress.test | RUN | 3.158249 | 17.949949 | 14.308990 | 0.010666 | 0.013742 | 0.009878 |
| regress_u.test | RUN | 3.343261 | 18.383780 | 14.217803 | 0.010892 | 0.013269 | 0.009858 |
| c_traits.test | RUN | 3.155116 | 17.468813 | 13.618035 | 0.008409 | 0.012222 | 0.008062 |
| c_traits_u.test | RUN | 3.325887 | 18.221510 | 14.172519 | 0.009020 | 0.012825 | 0.008584 |
| test1.test | RUN | 1.185561 | 4.639166 | 4.240161 | 0.003058 | 0.003346 | 0.003258 |
| test2.test | RUN | 1.451291 | 6.599719 | 6.200288 | 0.003485 | 0.003323 | 0.003221 |
| test3.test | RUN | 1.509252 | 6.472034 | 6.227839 | 0.002901 | 0.003337 | 0.003042 |
| test4.test | RUN | 1.336579 | 5.354844 | 5.142634 | 0.003258 | 0.003215 | 0.003235 |
| test5.test | RUN | 1.395571 | 5.762117 | 5.520173 | 0.003180 | 0.003342 | 0.003228 |
| test6.test | RUN | 1.260035 | 4.915320 | 4.724586 | 0.003315 | 0.003110 | 0.003219 |
| test7.test | RUN | 1.255938 | 4.926549 | 4.769606 | 0.003069 | 0.003191 | 0.003181 |
| test8.test | RUN | 1.239826 | 4.861651 | 4.621987 | 0.002965 | 0.003231 | 0.003095 |
| test9.test | RUN | 1.324969 | 5.216526 | 5.168394 | 0.003213 | 0.003157 | 0.002837 |
| test10.test | RUN | 1.294495 | 4.978455 | 4.788722 | 0.002957 | 0.003307 | 0.002723 |
| test11.test | RUN | 1.265443 | 4.770677 | 4.806718 | 0.002984 | 0.003336 | 0.003169 |
| test1u.test | RUN | 1.195298 | 4.513557 | 4.183746 | 0.003144 | 0.003218 | 0.003112 |
| test2u.test | RUN | 1.497893 | 6.733103 | 6.441412 | 0.003353 | 0.003342 | 0.003310 |
| test3u.test | RUN | 1.541641 | 6.877623 | 6.636015 | 0.003192 | 0.003247 | 0.002991 |
| test4u.test | RUN | 1.364525 | 5.326123 | 5.151681 | 0.002989 | 0.003368 | 0.003236 |
| test5u.test | RUN | 1.388883 | 5.911036 | 5.900947 | 0.003004 | 0.003624 | 0.003342 |
| test6u.test | RUN | 1.297563 | 4.995985 | 4.904590 | 0.003289 | 0.003289 | 0.003240 |
| test7u.test | RUN | 1.284630 | 5.084012 | 4.947434 | 0.002979 | 0.003317 | 0.002718 |
| test8u.test | RUN | 1.240138 | 4.821387 | 4.640716 | 0.003083 | 0.003371 | 0.003190 |
| test9u.test | RUN | 1.345783 | 5.152183 | 5.212835 | 0.003182 | 0.003034 | 0.002968 |
| test10u.test | RUN | 1.322091 | 4.988742 | 4.860820 | 0.003114 | 0.003280 | 0.003182 |
| test11u.test | RUN | 1.298083 | 4.989386 | 4.943359 | 0.003385 | 0.003184 | 0.003174 |
| misc1.test | RUN | 2.300270 | 12.364168 | 10.662895 | 0.004132 | 0.004273 | 0.004024 |
| misc2.test | RUN | 2.354656 | 11.845810 | 10.357779 | 0.003682 | 0.003803 | 0.004055 |
| test_format.test | RUN | 2.540683 | 14.179124 | 11.566672 | 0.003397 | 0.003483 | 0.003142 |
| test_cycles.test | RUN | 1.152659 | 4.383014 | 3.934576 | 0.003073 | 0.003237 | 0.003225 |
| test_non_char.test | RUN | 2.038850 | 10.098266 | 8.906467 | 0.003278 | 0.003606 | 0.003386 |
| test_static.test | RUN | 1.053072 | 4.080868 | 3.567174 | 0.003182 | 0.003281 | 0.003140 |
| test_actions.test | RUN | 2.457057 | 11.969626 | 10.775855 | 0.004133 | 0.004160 | 0.004477 |
| test_assert.test | RUN | 1.388962 | 5.290351 | 5.103594 | 0.003418 | 0.003492 | 0.003446 |
| test_assert_with_placeholder.test | RUN | 1.343156 | 4.822320 | 4.601720 | 0.003066 | 0.003327 | 0.003187 |
| test_symbols.test | RUN | 2.579526 | 11.863412 | 12.305312 | 0.004125 | 0.004381 | 0.004417 |
| test_dynamic.test | RUN | 1.692783 | 8.048084 | 6.901955 | 0.003353 | 0.003212 | 0.003317 |
| test_dynamic_grammar.test | RUN | 1.765447 | 8.644420 | 7.318639 | 0.003324 | 0.003629 | 0.003351 |
| test_skip.test | RUN | 1.943693 | 8.916810 | 8.555114 | 0.003157 | 0.003162 | 0.003265 |
| multiple_defs.test | LINK | 3.313585 | 16.068109 | 14.056983 | — | — | — |
| test_basic_regex.test | COMPILE | 0.751218 | 1.329795 | 1.943912 | — | — | — |
| test_match_results.test | COMPILE | 0.495609 | 0.887947 | 1.319176 | — | — | — |
| test_regex_algorithms.test | COMPILE | 0.525656 | 0.855103 | 1.319800 | — | — | — |
| test_regex_compiler.test | COMPILE | 0.768967 | 1.431014 | 1.930456 | — | — | — |
| test_regex_constants.test | COMPILE | 0.047916 | 0.060345 | 0.067680 | — | — | — |
| test_regex_error.test | COMPILE | 0.273998 | 0.482586 | 0.703899 | — | — | — |
| test_regex_iterator.test | COMPILE | 0.507050 | 0.841136 | 1.261230 | — | — | — |
| test_regex_primitives.test | COMPILE | 0.765942 | 1.421445 | 1.682438 | — | — | — |
| test_regex_token_iterator.test | COMPILE | 0.493167 | 0.885522 | 1.257759 | — | — | — |
| test_regex_traits.test | COMPILE | 0.292576 | 0.469541 | 0.666057 | — | — | — |
| test_sub_match.test | COMPILE | 0.230746 | 0.388287 | 0.503703 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.093× | 1.089×, 1.097× | 0.989× |
| clang | 0.958× | 0.966×, 0.949× | 1.001× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1362 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| c_traits.test | 3 | 0.007121s | 0.010385s | 0.006739s | byte equality observed |
| c_traits_u.test | 3 | 0.007873s | 0.010780s | 0.007119s | byte equality observed |
| misc1.test | 8 | 0.002650s | 0.002579s | 0.002599s | byte equality observed |
| misc2.test | 9 | 0.002377s | 0.002245s | 0.002315s | byte equality observed |
| regress.test | 3 | 0.009324s | 0.011911s | 0.008176s | byte equality observed |
| regress_u.test | 3 | 0.009323s | 0.012020s | 0.008452s | byte equality observed |
| test1.test | 9 | 0.002102s | 0.002056s | 0.002092s | byte equality observed |
| test10.test | 10 | 0.002079s | 0.002041s | 0.002013s | byte equality observed |
| test10u.test | 10 | 0.002092s | 0.002045s | 0.002033s | byte equality observed |
| test11.test | 10 | 0.002102s | 0.002069s | 0.002020s | byte equality observed |
| test11u.test | 10 | 0.002106s | 0.002059s | 0.002046s | byte equality observed |
| test1u.test | 10 | 0.002104s | 0.002083s | 0.002025s | byte equality observed |
| test2.test | 10 | 0.002154s | 0.002173s | 0.002071s | byte equality observed |
| test2u.test | 10 | 0.002125s | 0.002183s | 0.002077s | byte equality observed |
| test3.test | 10 | 0.002142s | 0.002154s | 0.002055s | byte equality observed |
| test3u.test | 10 | 0.002135s | 0.002204s | 0.002090s | byte equality observed |
| test4.test | 10 | 0.002114s | 0.002083s | 0.002056s | byte equality observed |
| test4u.test | 10 | 0.002101s | 0.002074s | 0.002047s | byte equality observed |
| test5.test | 10 | 0.002133s | 0.002101s | 0.002059s | byte equality observed |
| test5u.test | 10 | 0.002137s | 0.002124s | 0.002079s | byte equality observed |
| test6.test | 10 | 0.002070s | 0.002079s | 0.002050s | byte equality observed |
| test6u.test | 10 | 0.002117s | 0.002044s | 0.002024s | byte equality observed |
| test7.test | 10 | 0.002059s | 0.002055s | 0.002023s | byte equality observed |
| test7u.test | 10 | 0.002124s | 0.002083s | 0.002072s | byte equality observed |
| test8.test | 10 | 0.002074s | 0.002055s | 0.001999s | byte equality observed |
| test8u.test | 9 | 0.002115s | 0.002072s | 0.002009s | byte equality observed |
| test9.test | 10 | 0.002085s | 0.002076s | 0.002048s | byte equality observed |
| test9u.test | 10 | 0.002117s | 0.002080s | 0.002012s | byte equality observed |
| test_actions.test | 6 | 0.002654s | 0.002620s | 0.002586s | byte equality observed |
| test_assert.test | 9 | 0.002223s | 0.002177s | 0.002154s | byte equality observed |
| test_assert_with_placeholder.test | 10 | 0.002063s | 0.002048s | 0.002005s | byte equality observed |
| test_cycles.test | 9 | 0.002110s | 0.002049s | 0.002014s | byte equality observed |
| test_dynamic.test | 8 | 0.002071s | 0.002015s | 0.002031s | byte equality observed |
| test_dynamic_grammar.test | 8 | 0.002250s | 0.002178s | 0.002137s | byte equality observed |
| test_format.test | 9 | 0.002194s | 0.002144s | 0.002091s | byte equality observed |
| test_non_char.test | 9 | 0.002178s | 0.002108s | 0.002121s | byte equality observed |
| test_skip.test | 10 | 0.002099s | 0.002053s | 0.001986s | byte equality observed |
| test_static.test | 10 | 0.001925s | 0.001848s | 0.001903s | byte equality observed |
| test_symbols.test | 8 | 0.002338s | 0.002444s | 0.002331s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
