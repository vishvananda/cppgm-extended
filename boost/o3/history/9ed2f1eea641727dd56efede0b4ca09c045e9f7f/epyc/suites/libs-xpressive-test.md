# libs/xpressive/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| regress.test | RUN | 2.759149 | 17.837367 | 13.905965 | 0.010659 | 0.013543 | 0.009691 |
| regress_u.test | RUN | 2.847445 | 18.211687 | 14.064851 | 0.010586 | 0.013312 | 0.009611 |
| c_traits.test | RUN | 2.709003 | 17.441304 | 13.462001 | 0.008250 | 0.011621 | 0.007494 |
| c_traits_u.test | RUN | 2.945826 | 18.070746 | 14.033389 | 0.008808 | 0.012548 | 0.008352 |
| test1.test | RUN | 1.054313 | 4.562510 | 4.186584 | 0.002914 | 0.003172 | 0.002719 |
| test2.test | RUN | 1.401580 | 6.377123 | 6.084063 | 0.002828 | 0.003436 | 0.003074 |
| test3.test | RUN | 1.377472 | 6.491035 | 6.232657 | 0.003133 | 0.003047 | 0.003005 |
| test4.test | RUN | 1.196031 | 5.318089 | 5.119735 | 0.003090 | 0.003360 | 0.003234 |
| test5.test | RUN | 1.318478 | 5.733101 | 5.449258 | 0.002901 | 0.003051 | 0.003017 |
| test6.test | RUN | 1.171506 | 4.845366 | 4.717839 | 0.002956 | 0.003256 | 0.002989 |
| test7.test | RUN | 1.139936 | 4.965891 | 4.660609 | 0.002845 | 0.003089 | 0.003103 |
| test8.test | RUN | 1.115038 | 4.787194 | 4.535310 | 0.002801 | 0.003304 | 0.003024 |
| test9.test | RUN | 1.209871 | 5.235906 | 5.079627 | 0.003036 | 0.002986 | 0.003168 |
| test10.test | RUN | 1.187194 | 4.910408 | 4.769002 | 0.002995 | 0.003101 | 0.003012 |
| test11.test | RUN | 1.151068 | 4.779059 | 4.748540 | 0.002774 | 0.003209 | 0.003019 |
| test1u.test | RUN | 1.039292 | 4.532009 | 4.155181 | 0.002958 | 0.003038 | 0.003036 |
| test2u.test | RUN | 1.413118 | 6.600905 | 6.425601 | 0.003144 | 0.003135 | 0.003209 |
| test3u.test | RUN | 1.434924 | 6.758659 | 6.679277 | 0.002896 | 0.003150 | 0.003278 |
| test4u.test | RUN | 1.210397 | 5.263822 | 5.194774 | 0.003143 | 0.003205 | 0.003078 |
| test5u.test | RUN | 1.316457 | 5.904468 | 5.810342 | 0.002862 | 0.003062 | 0.003177 |
| test6u.test | RUN | 1.199527 | 4.875884 | 4.837114 | 0.003069 | 0.003245 | 0.003214 |
| test7u.test | RUN | 1.173411 | 5.111913 | 4.992139 | 0.003214 | 0.003303 | 0.003211 |
| test8u.test | RUN | 1.144834 | 4.808666 | 4.639344 | 0.003157 | 0.003326 | 0.003072 |
| test9u.test | RUN | 1.230710 | 5.191710 | 5.212561 | 0.003139 | 0.003195 | 0.003159 |
| test10u.test | RUN | 1.211889 | 4.950034 | 4.818074 | 0.003086 | 0.003285 | 0.003178 |
| test11u.test | RUN | 1.231522 | 4.926306 | 4.924085 | 0.003293 | 0.003022 | 0.003121 |
| misc1.test | RUN | 2.105838 | 12.315108 | 10.569892 | 0.004069 | 0.004151 | 0.004146 |
| misc2.test | RUN | 2.138766 | 11.708689 | 10.257898 | 0.003469 | 0.003597 | 0.003559 |
| test_format.test | RUN | 2.208296 | 14.184670 | 11.679484 | 0.003110 | 0.003343 | 0.003325 |
| test_cycles.test | RUN | 1.010074 | 4.365850 | 3.893201 | 0.003162 | 0.003297 | 0.003187 |
| test_non_char.test | RUN | 1.848430 | 9.901449 | 8.765985 | 0.003472 | 0.003117 | 0.003404 |
| test_static.test | RUN | 0.954222 | 4.032130 | 3.477815 | 0.003210 | 0.003254 | 0.002937 |
| test_actions.test | RUN | 2.279342 | 12.040935 | 11.084790 | 0.004246 | 0.004135 | 0.003727 |
| test_assert.test | RUN | 1.321193 | 5.279902 | 5.079208 | 0.002971 | 0.003096 | 0.003378 |
| test_assert_with_placeholder.test | RUN | 1.230650 | 4.654589 | 4.675873 | 0.002905 | 0.002864 | 0.003088 |
| test_symbols.test | RUN | 2.388675 | 11.841786 | 12.404669 | 0.004294 | 0.004159 | 0.004472 |
| test_dynamic.test | RUN | 1.477738 | 7.828211 | 7.134466 | 0.003247 | 0.002836 | 0.003333 |
| test_dynamic_grammar.test | RUN | 1.584876 | 8.308556 | 7.251965 | 0.003209 | 0.003191 | 0.003275 |
| test_skip.test | RUN | 1.806863 | 8.723096 | 8.667346 | 0.003142 | 0.003226 | 0.003126 |
| multiple_defs.test | LINK | 2.907957 | 15.584328 | 14.040045 | — | — | — |
| test_basic_regex.test | COMPILE | 0.671087 | 1.298389 | 1.922607 | — | — | — |
| test_match_results.test | COMPILE | 0.439918 | 0.919026 | 1.231724 | — | — | — |
| test_regex_algorithms.test | COMPILE | 0.439676 | 0.904718 | 1.297055 | — | — | — |
| test_regex_compiler.test | COMPILE | 0.691351 | 1.479165 | 1.918954 | — | — | — |
| test_regex_constants.test | COMPILE | 0.036813 | 0.064491 | 0.069335 | — | — | — |
| test_regex_error.test | COMPILE | 0.236925 | 0.488626 | 0.689730 | — | — | — |
| test_regex_iterator.test | COMPILE | 0.434981 | 0.913814 | 1.226340 | — | — | — |
| test_regex_primitives.test | COMPILE | 0.671233 | 1.452434 | 1.764374 | — | — | — |
| test_regex_token_iterator.test | COMPILE | 0.411697 | 0.905728 | 1.224980 | — | — | — |
| test_regex_traits.test | COMPILE | 0.250264 | 0.484939 | 0.680244 | — | — | — |
| test_sub_match.test | COMPILE | 0.212878 | 0.397013 | 0.336085 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.116× | 1.128×, 1.103× | 1.001× |
| clang | 0.946× | 0.946×, 0.946× | 1.014× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1363 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43601267198).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| c_traits.test | 3 | 0.007170s | 0.010403s | 0.006615s | byte equality observed |
| c_traits_u.test | 3 | 0.007898s | 0.010768s | 0.006960s | byte equality observed |
| misc1.test | 7 | 0.002647s | 0.002590s | 0.002602s | byte equality observed |
| misc2.test | 9 | 0.002346s | 0.002270s | 0.002339s | byte equality observed |
| regress.test | 3 | 0.009060s | 0.011901s | 0.008056s | byte equality observed |
| regress_u.test | 3 | 0.009256s | 0.012052s | 0.008368s | byte equality observed |
| test1.test | 8 | 0.002123s | 0.002053s | 0.002041s | byte equality observed |
| test10.test | 8 | 0.002060s | 0.001990s | 0.001951s | byte equality observed |
| test10u.test | 10 | 0.002064s | 0.002018s | 0.002016s | byte equality observed |
| test11.test | 10 | 0.002062s | 0.002029s | 0.001985s | byte equality observed |
| test11u.test | 10 | 0.002070s | 0.002041s | 0.002000s | byte equality observed |
| test1u.test | 10 | 0.002063s | 0.002029s | 0.002011s | byte equality observed |
| test2.test | 10 | 0.002113s | 0.002108s | 0.002030s | byte equality observed |
| test2u.test | 10 | 0.002109s | 0.002119s | 0.002039s | byte equality observed |
| test3.test | 10 | 0.002099s | 0.002112s | 0.002055s | byte equality observed |
| test3u.test | 10 | 0.002101s | 0.002136s | 0.002098s | byte equality observed |
| test4.test | 10 | 0.002085s | 0.002053s | 0.001990s | byte equality observed |
| test4u.test | 10 | 0.002079s | 0.002060s | 0.002056s | byte equality observed |
| test5.test | 10 | 0.002067s | 0.002078s | 0.001998s | byte equality observed |
| test5u.test | 10 | 0.002073s | 0.002091s | 0.002046s | byte equality observed |
| test6.test | 10 | 0.002051s | 0.002053s | 0.002008s | byte equality observed |
| test6u.test | 10 | 0.002066s | 0.002053s | 0.002010s | byte equality observed |
| test7.test | 10 | 0.002049s | 0.002047s | 0.002003s | byte equality observed |
| test7u.test | 10 | 0.002057s | 0.002120s | 0.002022s | byte equality observed |
| test8.test | 10 | 0.002047s | 0.002076s | 0.002024s | byte equality observed |
| test8u.test | 10 | 0.002069s | 0.002054s | 0.002012s | byte equality observed |
| test9.test | 8 | 0.002056s | 0.002090s | 0.002009s | byte equality observed |
| test9u.test | 9 | 0.002081s | 0.002069s | 0.002034s | byte equality observed |
| test_actions.test | 7 | 0.002635s | 0.002618s | 0.002524s | byte equality observed |
| test_assert.test | 8 | 0.002153s | 0.002145s | 0.002124s | byte equality observed |
| test_assert_with_placeholder.test | 10 | 0.002018s | 0.002016s | 0.001947s | byte equality observed |
| test_cycles.test | 10 | 0.002063s | 0.002036s | 0.002013s | byte equality observed |
| test_dynamic.test | 9 | 0.002073s | 0.002048s | 0.001957s | byte equality observed |
| test_dynamic_grammar.test | 9 | 0.002220s | 0.002122s | 0.002159s | byte equality observed |
| test_format.test | 10 | 0.002175s | 0.002091s | 0.002031s | byte equality observed |
| test_non_char.test | 10 | 0.002132s | 0.002091s | 0.002078s | byte equality observed |
| test_skip.test | 10 | 0.002018s | 0.001991s | 0.001965s | byte equality observed |
| test_static.test | 10 | 0.001962s | 0.001921s | 0.001918s | byte equality observed |
| test_symbols.test | 9 | 0.002388s | 0.002317s | 0.002401s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43601267198).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
