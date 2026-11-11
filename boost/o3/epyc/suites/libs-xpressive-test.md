# libs/xpressive/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| regress.test | RUN | 2.624976 | 16.814093 | 12.175387 | 0.010433 | 0.011485 | 0.008094 |
| regress_u.test | RUN | 2.857262 | 17.325376 | 12.431601 | 0.010124 | 0.011327 | 0.009332 |
| c_traits.test | RUN | 2.665800 | 16.632073 | 11.755465 | 0.007582 | 0.010304 | 0.008057 |
| c_traits_u.test | RUN | 2.850280 | 17.283426 | 12.374616 | 0.007555 | 0.011174 | 0.007684 |
| test1.test | RUN | 1.132209 | 4.293570 | 3.568496 | 0.003460 | 0.003266 | 0.003184 |
| test2.test | RUN | 1.370162 | 5.934665 | 5.258557 | 0.003051 | 0.003060 | 0.003166 |
| test3.test | RUN | 1.366711 | 6.138570 | 5.332434 | 0.002899 | 0.003320 | 0.003222 |
| test4.test | RUN | 1.214557 | 5.045798 | 4.299582 | 0.003198 | 0.003252 | 0.003367 |
| test5.test | RUN | 1.285324 | 5.495384 | 4.604595 | 0.003435 | 0.003283 | 0.003117 |
| test6.test | RUN | 1.165149 | 4.505104 | 3.838686 | 0.003118 | 0.003589 | 0.003064 |
| test7.test | RUN | 1.121318 | 4.728229 | 3.929083 | 0.003379 | 0.003236 | 0.002818 |
| test8.test | RUN | 1.135413 | 4.538754 | 3.761562 | 0.003399 | 0.003316 | 0.003244 |
| test9.test | RUN | 1.214373 | 4.887470 | 4.272979 | 0.003286 | 0.003310 | 0.003032 |
| test10.test | RUN | 1.168638 | 4.665952 | 3.947648 | 0.002692 | 0.003468 | 0.002956 |
| test11.test | RUN | 1.148018 | 4.496983 | 3.906479 | 0.002974 | 0.003321 | 0.003273 |
| test1u.test | RUN | 1.075949 | 4.229340 | 3.352770 | 0.002596 | 0.003118 | 0.003088 |
| test2u.test | RUN | 1.405782 | 6.311458 | 5.250671 | 0.003547 | 0.003285 | 0.003397 |
| test3u.test | RUN | 1.439311 | 6.381273 | 5.355999 | 0.003466 | 0.003336 | 0.003197 |
| test4u.test | RUN | 1.236602 | 4.969510 | 4.282277 | 0.003154 | 0.003258 | 0.003114 |
| test5u.test | RUN | 1.310169 | 5.430417 | 4.865918 | 0.003315 | 0.003623 | 0.003172 |
| test6u.test | RUN | 1.237716 | 4.632716 | 3.937827 | 0.003214 | 0.003472 | 0.003088 |
| test7u.test | RUN | 1.176644 | 4.868761 | 4.018947 | 0.003459 | 0.003348 | 0.003121 |
| test8u.test | RUN | 1.130228 | 4.445405 | 3.722051 | 0.003085 | 0.003292 | 0.003192 |
| test9u.test | RUN | 1.210808 | 4.874286 | 4.300357 | 0.003352 | 0.003169 | 0.003412 |
| test10u.test | RUN | 1.215308 | 4.612741 | 3.852238 | 0.003561 | 0.003611 | 0.003050 |
| test11u.test | RUN | 1.211313 | 4.622628 | 4.068698 | 0.003402 | 0.003592 | 0.003286 |
| misc1.test | RUN | 2.134723 | 11.848007 | 9.155574 | 0.004425 | 0.003772 | 0.003715 |
| misc2.test | RUN | 2.202713 | 11.090788 | 8.830468 | 0.003739 | 0.003598 | 0.003462 |
| test_format.test | RUN | 2.256094 | 13.719418 | 9.835929 | 0.003673 | 0.003718 | 0.003064 |
| test_cycles.test | RUN | 1.066024 | 4.083876 | 3.177053 | 0.003559 | 0.003224 | 0.003029 |
| test_non_char.test | RUN | 1.849160 | 9.581844 | 7.268376 | 0.003534 | 0.003434 | 0.003054 |
| test_static.test | RUN | 0.961768 | 3.747079 | 2.735706 | 0.003199 | 0.003225 | 0.002954 |
| test_actions.test | RUN | 2.217752 | 11.353121 | 8.915253 | 0.004278 | 0.003939 | 0.003651 |
| test_assert.test | RUN | 1.312100 | 4.858377 | 3.929645 | 0.003091 | 0.003926 | 0.002772 |
| test_assert_with_placeholder.test | RUN | 1.215571 | 4.454195 | 3.573489 | 0.003246 | 0.003022 | 0.002861 |
| test_symbols.test | RUN | 2.416469 | 11.089876 | 9.949903 | 0.004772 | 0.004158 | 0.004104 |
| test_dynamic.test | RUN | 1.512283 | 7.348702 | 5.706847 | 0.003527 | 0.003323 | 0.002945 |
| test_dynamic_grammar.test | RUN | 1.541487 | 7.890586 | 5.940982 | 0.003693 | 0.003417 | 0.003206 |
| test_skip.test | RUN | 1.803742 | 8.299650 | 7.004138 | 0.003306 | 0.003248 | 0.003411 |
| multiple_defs.test | LINK | 2.930484 | 14.435831 | 11.597133 | — | — | — |
| test_basic_regex.test | COMPILE | 0.660756 | 1.157540 | 1.414188 | — | — | — |
| test_match_results.test | COMPILE | 0.450169 | 0.786267 | 0.930317 | — | — | — |
| test_regex_algorithms.test | COMPILE | 0.469722 | 0.800831 | 0.935377 | — | — | — |
| test_regex_compiler.test | COMPILE | 0.697017 | 1.341430 | 1.375717 | — | — | — |
| test_regex_constants.test | COMPILE | 0.040118 | 0.059839 | 0.056121 | — | — | — |
| test_regex_error.test | COMPILE | 0.255894 | 0.450033 | 0.508662 | — | — | — |
| test_regex_iterator.test | COMPILE | 0.481704 | 0.799923 | 0.913301 | — | — | — |
| test_regex_primitives.test | COMPILE | 0.703537 | 1.298297 | 1.231985 | — | — | — |
| test_regex_token_iterator.test | COMPILE | 0.462171 | 0.786523 | 0.905320 | — | — | — |
| test_regex_traits.test | COMPILE | 0.282018 | 0.424057 | 0.479973 | — | — | — |
| test_sub_match.test | COMPILE | 0.234524 | 0.345978 | 0.290212 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.064× | 1.084×, 1.043× | 0.977× |
| clang | 0.902× | 0.910×, 0.894× | 0.970× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1361 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| c_traits.test | 3 | 0.006311s | 0.008158s | 0.005978s | byte equality observed |
| c_traits_u.test | 3 | 0.006787s | 0.008642s | 0.006163s | byte equality observed |
| misc1.test | 8 | 0.002565s | 0.002328s | 0.002383s | byte equality observed |
| misc2.test | 9 | 0.002258s | 0.002064s | 0.002182s | byte equality observed |
| regress.test | 3 | 0.008157s | 0.009438s | 0.006759s | byte equality observed |
| regress_u.test | 3 | 0.008027s | 0.009549s | 0.007232s | byte equality observed |
| test1.test | 9 | 0.002034s | 0.001919s | 0.001959s | byte equality observed |
| test10.test | 10 | 0.001966s | 0.001892s | 0.001925s | byte equality observed |
| test10u.test | 10 | 0.001945s | 0.001900s | 0.001932s | byte equality observed |
| test11.test | 10 | 0.001962s | 0.001885s | 0.001968s | byte equality observed |
| test11u.test | 7 | 0.001953s | 0.001889s | 0.001896s | byte equality observed |
| test1u.test | 10 | 0.001956s | 0.001928s | 0.001908s | byte equality observed |
| test2.test | 10 | 0.001999s | 0.001955s | 0.001919s | byte equality observed |
| test2u.test | 10 | 0.001996s | 0.001986s | 0.001960s | byte equality observed |
| test3.test | 10 | 0.001997s | 0.001962s | 0.001948s | byte equality observed |
| test3u.test | 10 | 0.002056s | 0.001977s | 0.001938s | byte equality observed |
| test4.test | 10 | 0.002024s | 0.001890s | 0.001873s | byte equality observed |
| test4u.test | 10 | 0.002014s | 0.001909s | 0.001902s | byte equality observed |
| test5.test | 10 | 0.001950s | 0.001908s | 0.001875s | byte equality observed |
| test5u.test | 10 | 0.001977s | 0.001916s | 0.001895s | byte equality observed |
| test6.test | 10 | 0.001902s | 0.001867s | 0.001845s | byte equality observed |
| test6u.test | 10 | 0.001935s | 0.001861s | 0.001859s | byte equality observed |
| test7.test | 8 | 0.001983s | 0.001904s | 0.001851s | byte equality observed |
| test7u.test | 9 | 0.001949s | 0.001888s | 0.001888s | byte equality observed |
| test8.test | 10 | 0.001909s | 0.001834s | 0.001844s | byte equality observed |
| test8u.test | 10 | 0.001952s | 0.001871s | 0.001858s | byte equality observed |
| test9.test | 10 | 0.001920s | 0.001847s | 0.001824s | byte equality observed |
| test9u.test | 10 | 0.001972s | 0.001913s | 0.001847s | byte equality observed |
| test_actions.test | 9 | 0.002429s | 0.002358s | 0.002311s | byte equality observed |
| test_assert.test | 10 | 0.002043s | 0.001961s | 0.001959s | byte equality observed |
| test_assert_with_placeholder.test | 10 | 0.001875s | 0.001856s | 0.001818s | byte equality observed |
| test_cycles.test | 10 | 0.001998s | 0.001888s | 0.001847s | byte equality observed |
| test_dynamic.test | 10 | 0.002013s | 0.001832s | 0.001855s | byte equality observed |
| test_dynamic_grammar.test | 7 | 0.002121s | 0.001922s | 0.001989s | byte equality observed |
| test_format.test | 9 | 0.002077s | 0.001922s | 0.001947s | byte equality observed |
| test_non_char.test | 10 | 0.002070s | 0.001871s | 0.001960s | byte equality observed |
| test_skip.test | 10 | 0.001964s | 0.001838s | 0.001852s | byte equality observed |
| test_static.test | 10 | 0.001908s | 0.001747s | 0.001811s | byte equality observed |
| test_symbols.test | 9 | 0.002457s | 0.002193s | 0.002277s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
