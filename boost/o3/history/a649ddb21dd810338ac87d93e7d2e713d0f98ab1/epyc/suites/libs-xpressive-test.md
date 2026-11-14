# libs/xpressive/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| regress.test | RUN | 3.651362 | 23.819110 | 19.848015 | 0.014845 | 0.019027 | 0.013705 |
| regress_u.test | RUN | 3.825830 | 24.538558 | 19.933870 | 0.014325 | 0.018074 | 0.013453 |
| c_traits.test | RUN | 3.566945 | 23.398654 | 19.126751 | 0.011196 | 0.016016 | 0.011243 |
| c_traits_u.test | RUN | 3.965796 | 24.345053 | 19.657166 | 0.012790 | 0.018182 | 0.012318 |
| test1.test | RUN | 1.404579 | 6.273761 | 6.241777 | 0.003514 | 0.003888 | 0.003822 |
| test2.test | RUN | 1.883846 | 8.822150 | 8.744318 | 0.003678 | 0.004197 | 0.003565 |
| test3.test | RUN | 1.875934 | 8.797311 | 8.922547 | 0.003735 | 0.004132 | 0.003624 |
| test4.test | RUN | 1.617528 | 7.285800 | 7.452401 | 0.003420 | 0.003887 | 0.003792 |
| test5.test | RUN | 1.742057 | 7.756391 | 7.912775 | 0.003235 | 0.003621 | 0.003829 |
| test6.test | RUN | 1.551883 | 6.657496 | 6.811722 | 0.003262 | 0.004083 | 0.003486 |
| test7.test | RUN | 1.511990 | 6.797623 | 6.896102 | 0.003664 | 0.003806 | 0.003425 |
| test8.test | RUN | 1.541612 | 6.561430 | 6.664857 | 0.003658 | 0.003972 | 0.003598 |
| test9.test | RUN | 1.644380 | 7.133269 | 7.500886 | 0.003721 | 0.004124 | 0.003388 |
| test10.test | RUN | 1.570371 | 6.806390 | 6.940237 | 0.003391 | 0.004121 | 0.003218 |
| test11.test | RUN | 1.573020 | 6.567414 | 6.764294 | 0.003583 | 0.004099 | 0.003723 |
| test1u.test | RUN | 1.443040 | 6.160208 | 6.047210 | 0.003737 | 0.003984 | 0.003630 |
| test2u.test | RUN | 1.900639 | 9.158711 | 9.191800 | 0.003725 | 0.004025 | 0.003895 |
| test3u.test | RUN | 1.853385 | 9.462545 | 9.455078 | 0.003515 | 0.004005 | 0.003855 |
| test4u.test | RUN | 1.597438 | 7.207150 | 7.366856 | 0.003398 | 0.003989 | 0.003962 |
| test5u.test | RUN | 1.751674 | 7.948946 | 8.318694 | 0.003735 | 0.004196 | 0.003608 |
| test6u.test | RUN | 1.620174 | 6.758365 | 7.110149 | 0.003723 | 0.004140 | 0.003591 |
| test7u.test | RUN | 1.574055 | 7.090669 | 7.139071 | 0.003902 | 0.004059 | 0.003815 |
| test8u.test | RUN | 1.515563 | 6.541632 | 6.701604 | 0.003680 | 0.004055 | 0.003587 |
| test9u.test | RUN | 1.601240 | 7.264956 | 7.489034 | 0.003740 | 0.004262 | 0.003859 |
| test10u.test | RUN | 1.619727 | 6.791438 | 6.958502 | 0.003710 | 0.003900 | 0.003699 |
| test11u.test | RUN | 1.618133 | 6.704957 | 7.194927 | 0.003946 | 0.004124 | 0.003307 |
| misc1.test | RUN | 2.755247 | 16.629873 | 14.979619 | 0.004727 | 0.005283 | 0.004930 |
| misc2.test | RUN | 2.788475 | 15.859028 | 14.531924 | 0.004055 | 0.004736 | 0.004369 |
| test_format.test | RUN | 2.922501 | 19.281878 | 16.295881 | 0.003918 | 0.003881 | 0.003818 |
| test_cycles.test | RUN | 1.336980 | 6.061086 | 5.729824 | 0.003582 | 0.003680 | 0.003795 |
| test_non_char.test | RUN | 2.444806 | 13.474923 | 12.489557 | 0.004092 | 0.004053 | 0.004022 |
| test_static.test | RUN | 1.267771 | 5.521483 | 5.084258 | 0.003626 | 0.003481 | 0.003630 |
| test_actions.test | RUN | 3.031531 | 16.365186 | 15.478050 | 0.005069 | 0.004815 | 0.004548 |
| test_assert.test | RUN | 1.767240 | 7.129748 | 7.434182 | 0.003822 | 0.004257 | 0.004173 |
| test_assert_with_placeholder.test | RUN | 1.655942 | 6.662931 | 6.736058 | 0.003438 | 0.003873 | 0.003889 |
| test_symbols.test | RUN | 3.080097 | 16.056626 | 17.184581 | 0.005077 | 0.005400 | 0.005386 |
| test_dynamic.test | RUN | 1.958677 | 10.826989 | 10.159199 | 0.003752 | 0.003892 | 0.003864 |
| test_dynamic_grammar.test | RUN | 2.072608 | 11.496740 | 10.139886 | 0.003815 | 0.004475 | 0.004026 |
| test_skip.test | RUN | 2.420218 | 11.936998 | 11.876962 | 0.003681 | 0.004164 | 0.003468 |
| multiple_defs.test | LINK | 3.899456 | 21.587321 | 19.750453 | — | — | — |
| test_basic_regex.test | COMPILE | 0.907389 | 1.875079 | 2.854986 | — | — | — |
| test_match_results.test | COMPILE | 0.590457 | 1.345847 | 1.893697 | — | — | — |
| test_regex_algorithms.test | COMPILE | 0.580819 | 1.326715 | 2.024393 | — | — | — |
| test_regex_compiler.test | COMPILE | 0.899966 | 2.057595 | 2.938672 | — | — | — |
| test_regex_constants.test | COMPILE | 0.048418 | 0.086715 | 0.091809 | — | — | — |
| test_regex_error.test | COMPILE | 0.310952 | 0.705524 | 1.050715 | — | — | — |
| test_regex_iterator.test | COMPILE | 0.582624 | 1.287327 | 1.931532 | — | — | — |
| test_regex_primitives.test | COMPILE | 0.916474 | 2.012887 | 2.393337 | — | — | — |
| test_regex_token_iterator.test | COMPILE | 0.536989 | 1.261849 | 1.655614 | — | — | — |
| test_regex_traits.test | COMPILE | 0.331937 | 0.668047 | 1.088756 | — | — | — |
| test_sub_match.test | COMPILE | 0.282884 | 0.463311 | 0.593946 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.093× | 1.097×, 1.088× | 0.995× |
| clang | 0.940× | 0.937×, 0.942× | 1.002× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43609274616).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| c_traits.test | 2 | 0.010441s | 0.014692s | 0.009784s | byte equality observed |
| c_traits_u.test | 2 | 0.011414s | 0.015571s | 0.010206s | byte equality observed |
| misc1.test | 7 | 0.003327s | 0.003221s | 0.003239s | byte equality observed |
| misc2.test | 8 | 0.003014s | 0.002838s | 0.002945s | byte equality observed |
| regress.test | 2 | 0.013774s | 0.017397s | 0.011119s | byte equality observed |
| regress_u.test | 2 | 0.014190s | 0.017549s | 0.011944s | byte equality observed |
| test1.test | 8 | 0.002642s | 0.002588s | 0.002582s | byte equality observed |
| test10.test | 9 | 0.002664s | 0.002555s | 0.002577s | byte equality observed |
| test10u.test | 6 | 0.002614s | 0.002611s | 0.002583s | byte equality observed |
| test11.test | 9 | 0.002644s | 0.002596s | 0.002536s | byte equality observed |
| test11u.test | 7 | 0.002677s | 0.002688s | 0.002597s | byte equality observed |
| test1u.test | 8 | 0.002663s | 0.002659s | 0.002598s | byte equality observed |
| test2.test | 8 | 0.002714s | 0.002758s | 0.002587s | byte equality observed |
| test2u.test | 8 | 0.002728s | 0.002726s | 0.002640s | byte equality observed |
| test3.test | 8 | 0.002697s | 0.002769s | 0.002560s | byte equality observed |
| test3u.test | 8 | 0.002734s | 0.002773s | 0.002611s | byte equality observed |
| test4.test | 8 | 0.002673s | 0.002610s | 0.002550s | byte equality observed |
| test4u.test | 6 | 0.002681s | 0.002714s | 0.002591s | byte equality observed |
| test5.test | 6 | 0.002697s | 0.002716s | 0.002620s | byte equality observed |
| test5u.test | 7 | 0.002695s | 0.002746s | 0.002624s | byte equality observed |
| test6.test | 8 | 0.002662s | 0.002622s | 0.002602s | byte equality observed |
| test6u.test | 8 | 0.002661s | 0.002636s | 0.002619s | byte equality observed |
| test7.test | 8 | 0.002669s | 0.002601s | 0.002555s | byte equality observed |
| test7u.test | 9 | 0.002647s | 0.002673s | 0.002635s | byte equality observed |
| test8.test | 9 | 0.002652s | 0.002562s | 0.002543s | byte equality observed |
| test8u.test | 8 | 0.002640s | 0.002591s | 0.002561s | byte equality observed |
| test9.test | 8 | 0.002683s | 0.002673s | 0.002573s | byte equality observed |
| test9u.test | 8 | 0.002657s | 0.002633s | 0.002616s | byte equality observed |
| test_actions.test | 7 | 0.003407s | 0.003383s | 0.003286s | byte equality observed |
| test_assert.test | 8 | 0.002769s | 0.002734s | 0.002685s | byte equality observed |
| test_assert_with_placeholder.test | 9 | 0.002630s | 0.002589s | 0.002512s | byte equality observed |
| test_cycles.test | 8 | 0.002711s | 0.002642s | 0.002562s | byte equality observed |
| test_dynamic.test | 9 | 0.002599s | 0.002545s | 0.002586s | byte equality observed |
| test_dynamic_grammar.test | 8 | 0.002825s | 0.002789s | 0.002810s | byte equality observed |
| test_format.test | 8 | 0.002769s | 0.002746s | 0.002682s | byte equality observed |
| test_non_char.test | 8 | 0.002752s | 0.002710s | 0.002652s | byte equality observed |
| test_skip.test | 8 | 0.002580s | 0.002613s | 0.002539s | byte equality observed |
| test_static.test | 9 | 0.002411s | 0.002390s | 0.002404s | byte equality observed |
| test_symbols.test | 7 | 0.003042s | 0.003057s | 0.002999s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43609274616).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
