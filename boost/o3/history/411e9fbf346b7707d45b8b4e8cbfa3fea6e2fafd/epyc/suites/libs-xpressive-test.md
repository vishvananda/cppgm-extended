# libs/xpressive/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| regress.test | RUN | 2.187558 | 15.526353 | 11.609193 | 0.008421 | 0.011008 | 0.008224 |
| regress_u.test | RUN | 2.391968 | 15.827234 | 11.929928 | 0.008383 | 0.011178 | 0.008472 |
| c_traits.test | RUN | 2.226072 | 15.258497 | 11.372956 | 0.007602 | 0.009750 | 0.006887 |
| c_traits_u.test | RUN | 2.391576 | 15.769836 | 11.951778 | 0.007560 | 0.010001 | 0.007805 |
| test1.test | RUN | 0.843172 | 3.859351 | 3.341242 | 0.002740 | 0.002988 | 0.003030 |
| test2.test | RUN | 1.113945 | 5.564734 | 5.249839 | 0.002730 | 0.003036 | 0.002957 |
| test3.test | RUN | 1.119260 | 5.567185 | 5.273120 | 0.002708 | 0.003197 | 0.003075 |
| test4.test | RUN | 0.981713 | 4.613045 | 4.261325 | 0.002526 | 0.003092 | 0.003027 |
| test5.test | RUN | 1.038499 | 4.955389 | 4.579990 | 0.002661 | 0.003156 | 0.003001 |
| test6.test | RUN | 0.931115 | 4.149215 | 3.829093 | 0.002645 | 0.002712 | 0.003068 |
| test7.test | RUN | 0.904529 | 4.249642 | 3.819466 | 0.002807 | 0.003010 | 0.002853 |
| test8.test | RUN | 0.890240 | 4.075729 | 3.628530 | 0.002831 | 0.003061 | 0.002833 |
| test9.test | RUN | 0.959214 | 4.521573 | 4.140814 | 0.002789 | 0.003064 | 0.003053 |
| test10.test | RUN | 0.950134 | 4.217471 | 3.800841 | 0.002789 | 0.003094 | 0.002966 |
| test11.test | RUN | 0.925047 | 4.186383 | 3.693392 | 0.002770 | 0.003475 | 0.003009 |
| test1u.test | RUN | 0.843466 | 3.967714 | 3.197194 | 0.002848 | 0.002908 | 0.002868 |
| test2u.test | RUN | 1.131706 | 5.829986 | 4.994230 | 0.002979 | 0.003388 | 0.003027 |
| test3u.test | RUN | 1.188807 | 5.988383 | 5.191300 | 0.002916 | 0.003052 | 0.002852 |
| test4u.test | RUN | 0.978242 | 4.576373 | 3.930405 | 0.002767 | 0.003235 | 0.002859 |
| test5u.test | RUN | 1.071157 | 5.073101 | 4.450777 | 0.002739 | 0.003251 | 0.002949 |
| test6u.test | RUN | 0.964473 | 4.282983 | 3.526956 | 0.002926 | 0.003208 | 0.002892 |
| test7u.test | RUN | 0.925593 | 4.438159 | 3.618530 | 0.002975 | 0.003195 | 0.002714 |
| test8u.test | RUN | 0.888584 | 4.214063 | 3.348961 | 0.002626 | 0.003157 | 0.002783 |
| test9u.test | RUN | 0.975634 | 4.461184 | 3.767988 | 0.002832 | 0.003441 | 0.002778 |
| test10u.test | RUN | 0.950085 | 4.046766 | 3.526793 | 0.002841 | 0.003032 | 0.002830 |
| test11u.test | RUN | 0.970124 | 4.171877 | 3.718995 | 0.002801 | 0.003210 | 0.002964 |
| misc1.test | RUN | 1.708919 | 10.884506 | 8.310435 | 0.003428 | 0.003829 | 0.003904 |
| misc2.test | RUN | 1.692194 | 10.133899 | 8.234410 | 0.002745 | 0.003371 | 0.003233 |
| test_format.test | RUN | 1.789692 | 12.656388 | 9.322331 | 0.002681 | 0.003373 | 0.002916 |
| test_cycles.test | RUN | 0.818034 | 3.796090 | 2.840412 | 0.002695 | 0.003069 | 0.002956 |
| test_non_char.test | RUN | 1.454356 | 8.895693 | 7.199778 | 0.002792 | 0.003245 | 0.003075 |
| test_static.test | RUN | 0.723658 | 3.508796 | 2.672982 | 0.002890 | 0.003073 | 0.002880 |
| test_actions.test | RUN | 1.773948 | 10.870550 | 8.475098 | 0.003330 | 0.003958 | 0.003776 |
| test_assert.test | RUN | 1.073239 | 4.617428 | 3.834209 | 0.002895 | 0.003653 | 0.003179 |
| test_assert_with_placeholder.test | RUN | 0.957404 | 4.203969 | 3.453947 | 0.002607 | 0.002993 | 0.002912 |
| test_symbols.test | RUN | 1.855597 | 10.851980 | 9.760965 | 0.003374 | 0.003861 | 0.004025 |
| test_dynamic.test | RUN | 1.128930 | 7.267072 | 5.507052 | 0.002834 | 0.003270 | 0.003186 |
| test_dynamic_grammar.test | RUN | 1.236099 | 7.718179 | 5.818446 | 0.002958 | 0.003360 | 0.003121 |
| test_skip.test | RUN | 1.465141 | 7.861980 | 6.811046 | 0.002793 | 0.002624 | 0.002917 |
| multiple_defs.test | LINK | 2.308314 | 13.544513 | 11.492638 | — | — | — |
| test_basic_regex.test | COMPILE | 0.490149 | 1.115819 | 1.373705 | — | — | — |
| test_match_results.test | COMPILE | 0.322545 | 0.731961 | 0.872904 | — | — | — |
| test_regex_algorithms.test | COMPILE | 0.326612 | 0.734468 | 0.888685 | — | — | — |
| test_regex_compiler.test | COMPILE | 0.502235 | 1.226581 | 1.398908 | — | — | — |
| test_regex_constants.test | COMPILE | 0.028165 | 0.050245 | 0.058531 | — | — | — |
| test_regex_error.test | COMPILE | 0.199400 | 0.387734 | 0.511089 | — | — | — |
| test_regex_iterator.test | COMPILE | 0.324007 | 0.770338 | 0.921308 | — | — | — |
| test_regex_primitives.test | COMPILE | 0.497247 | 1.172384 | 1.409472 | — | — | — |
| test_regex_token_iterator.test | COMPILE | 0.322713 | 0.724683 | 0.944556 | — | — | — |
| test_regex_traits.test | COMPILE | 0.188784 | 0.368975 | 0.482724 | — | — | — |
| test_sub_match.test | COMPILE | 0.158727 | 0.254704 | 0.395761 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.136×† | 1.066×, 1.206× | 1.044× |
| clang | 0.977× | 0.997×, 0.956× | 0.985× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43558051632).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| c_traits.test | 3 | 0.005878s | 0.008305s | 0.005652s | byte equality observed |
| c_traits_u.test | 3 | 0.006415s | 0.008534s | 0.005990s | byte equality observed |
| misc1.test | 9 | 0.002283s | 0.002380s | 0.002232s | byte equality observed |
| misc2.test | 9 | 0.002058s | 0.002072s | 0.001990s | byte equality observed |
| regress.test | 3 | 0.007266s | 0.009263s | 0.006590s | byte equality observed |
| regress_u.test | 2 | 0.007396s | 0.009410s | 0.006782s | byte equality observed |
| test1.test | 4 | 0.001872s | 0.001951s | 0.001765s | byte equality observed |
| test10.test | 7 | 0.001813s | 0.001842s | 0.001710s | byte equality observed |
| test10u.test | 5 | 0.001812s | 0.001903s | 0.001752s | byte equality observed |
| test11.test | 9 | 0.001815s | 0.001856s | 0.001731s | byte equality observed |
| test11u.test | 6 | 0.001833s | 0.001857s | 0.001752s | byte equality observed |
| test1u.test | 7 | 0.001806s | 0.001838s | 0.001740s | byte equality observed |
| test2.test | 7 | 0.001816s | 0.001911s | 0.001758s | byte equality observed |
| test2u.test | 6 | 0.001874s | 0.001978s | 0.001762s | byte equality observed |
| test3.test | 8 | 0.001847s | 0.001913s | 0.001779s | byte equality observed |
| test3u.test | 6 | 0.001886s | 0.001972s | 0.001777s | byte equality observed |
| test4.test | 9 | 0.001777s | 0.001829s | 0.001743s | byte equality observed |
| test4u.test | 6 | 0.001812s | 0.001852s | 0.001765s | byte equality observed |
| test5.test | 7 | 0.001823s | 0.001920s | 0.001749s | byte equality observed |
| test5u.test | 7 | 0.001788s | 0.001909s | 0.001734s | byte equality observed |
| test6.test | 7 | 0.001790s | 0.001823s | 0.001748s | byte equality observed |
| test6u.test | 7 | 0.001803s | 0.001878s | 0.001730s | byte equality observed |
| test7.test | 7 | 0.001783s | 0.001831s | 0.001770s | byte equality observed |
| test7u.test | 10 | 0.001795s | 0.001846s | 0.001760s | byte equality observed |
| test8.test | 10 | 0.001776s | 0.001827s | 0.001731s | byte equality observed |
| test8u.test | 10 | 0.001750s | 0.001831s | 0.001749s | byte equality observed |
| test9.test | 10 | 0.001793s | 0.001829s | 0.001730s | byte equality observed |
| test9u.test | 10 | 0.001764s | 0.001846s | 0.001749s | byte equality observed |
| test_actions.test | 9 | 0.002242s | 0.002328s | 0.002159s | byte equality observed |
| test_assert.test | 10 | 0.001840s | 0.001888s | 0.001812s | byte equality observed |
| test_assert_with_placeholder.test | 10 | 0.001766s | 0.001815s | 0.001688s | byte equality observed |
| test_cycles.test | 10 | 0.001777s | 0.001873s | 0.001745s | byte equality observed |
| test_dynamic.test | 10 | 0.001749s | 0.001840s | 0.001754s | byte equality observed |
| test_dynamic_grammar.test | 10 | 0.001887s | 0.001914s | 0.001806s | byte equality observed |
| test_format.test | 10 | 0.001854s | 0.001870s | 0.001805s | byte equality observed |
| test_non_char.test | 10 | 0.001821s | 0.001856s | 0.001761s | byte equality observed |
| test_skip.test | 10 | 0.001764s | 0.001774s | 0.001686s | byte equality observed |
| test_static.test | 10 | 0.001668s | 0.001744s | 0.001656s | byte equality observed |
| test_symbols.test | 9 | 0.002105s | 0.002170s | 0.002121s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43558051632).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
