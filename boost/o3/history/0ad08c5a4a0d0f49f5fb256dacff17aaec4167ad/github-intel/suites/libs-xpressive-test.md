# libs/xpressive/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| regress.test | RUN | 4.158218 | 22.606806 | 16.833983 | 0.013725 | 0.013820 | 0.011530 |
| regress_u.test | RUN | 4.532664 | 23.148298 | 17.180897 | 0.013671 | 0.014481 | 0.011982 |
| c_traits.test | RUN | 4.206782 | 22.273243 | 16.206796 | 0.010113 | 0.012252 | 0.009809 |
| c_traits_u.test | RUN | 4.493966 | 22.984064 | 16.817314 | 0.011042 | 0.012161 | 0.010459 |
| test1.test | RUN | 1.425755 | 5.608129 | 4.909982 | 0.003205 | 0.003153 | 0.002714 |
| test2.test | RUN | 1.882395 | 8.096857 | 7.126880 | 0.003228 | 0.003168 | 0.002903 |
| test3.test | RUN | 1.839910 | 8.096918 | 7.251708 | 0.003118 | 0.003080 | 0.002865 |
| test4.test | RUN | 1.656282 | 6.565689 | 5.924442 | 0.003169 | 0.003179 | 0.002850 |
| test5.test | RUN | 1.712366 | 7.150922 | 6.417400 | 0.003029 | 0.002961 | 0.002895 |
| test6.test | RUN | 1.560508 | 6.062435 | 5.387950 | 0.003130 | 0.003014 | 0.002866 |
| test7.test | RUN | 1.516559 | 6.127304 | 5.433802 | 0.003082 | 0.003022 | 0.002874 |
| test8.test | RUN | 1.505148 | 5.948256 | 5.243785 | 0.003090 | 0.003032 | 0.002987 |
| test9.test | RUN | 1.623591 | 6.457750 | 5.986945 | 0.003041 | 0.003045 | 0.002981 |
| test10.test | RUN | 1.616134 | 6.086454 | 5.557829 | 0.003169 | 0.002465 | 0.002877 |
| test11.test | RUN | 1.526657 | 5.889352 | 5.457703 | 0.002934 | 0.003006 | 0.002832 |
| test1u.test | RUN | 1.431848 | 5.594845 | 4.699363 | 0.002951 | 0.003067 | 0.003007 |
| test2u.test | RUN | 1.929446 | 8.219449 | 7.365246 | 0.003268 | 0.002904 | 0.003053 |
| test3u.test | RUN | 1.906416 | 8.447930 | 7.596242 | 0.003121 | 0.003039 | 0.003009 |
| test4u.test | RUN | 1.667255 | 6.600427 | 5.935515 | 0.003126 | 0.003032 | 0.002963 |
| test5u.test | RUN | 1.747703 | 7.329632 | 6.669874 | 0.003193 | 0.003115 | 0.002958 |
| test6u.test | RUN | 1.602056 | 6.068492 | 5.540582 | 0.003254 | 0.003118 | 0.002978 |
| test7u.test | RUN | 1.560563 | 6.386271 | 5.699031 | 0.002789 | 0.003001 | 0.002870 |
| test8u.test | RUN | 1.515867 | 5.946880 | 5.259840 | 0.002983 | 0.002942 | 0.002802 |
| test9u.test | RUN | 1.599893 | 6.423694 | 5.973495 | 0.003163 | 0.003091 | 0.003013 |
| test10u.test | RUN | 1.623503 | 6.052765 | 5.449473 | 0.003177 | 0.002959 | 0.002875 |
| test11u.test | RUN | 1.569549 | 6.146617 | 5.719072 | 0.003092 | 0.002983 | 0.002894 |
| misc1.test | RUN | 3.045353 | 15.568962 | 12.465976 | 0.003536 | 0.003679 | 0.003552 |
| misc2.test | RUN | 2.986395 | 14.786696 | 11.991530 | 0.003339 | 0.003311 | 0.003140 |
| test_format.test | RUN | 3.319791 | 18.295943 | 13.851255 | 0.003705 | 0.003091 | 0.003087 |
| test_cycles.test | RUN | 1.376251 | 5.461325 | 4.368710 | 0.002867 | 0.002998 | 0.002828 |
| test_non_char.test | RUN | 2.638420 | 12.691616 | 10.237831 | 0.003269 | 0.002920 | 0.002906 |
| test_static.test | RUN | 1.260331 | 4.966134 | 3.976885 | 0.003121 | 0.002849 | 0.002803 |
| test_actions.test | RUN | 3.018908 | 15.001761 | 12.483133 | 0.003727 | 0.003765 | 0.003556 |
| test_assert.test | RUN | 1.730839 | 6.547646 | 5.830260 | 0.003138 | 0.003126 | 0.003179 |
| test_assert_with_placeholder.test | RUN | 1.609038 | 5.853139 | 5.177601 | 0.002931 | 0.002989 | 0.002938 |
| test_symbols.test | RUN | 3.267917 | 14.672930 | 14.220827 | 0.003895 | 0.003408 | 0.003767 |
| test_dynamic.test | RUN | 2.127337 | 10.104773 | 8.176456 | 0.003049 | 0.003128 | 0.003029 |
| test_dynamic_grammar.test | RUN | 2.233317 | 10.670116 | 8.490165 | 0.003139 | 0.003135 | 0.003223 |
| test_skip.test | RUN | 2.368911 | 10.709564 | 9.743181 | 0.002853 | 0.003024 | 0.002911 |
| multiple_defs.test | LINK | 4.180045 | 19.406517 | 16.376201 | — | — | — |
| test_basic_regex.test | COMPILE | 0.863547 | 1.540649 | 2.073787 | — | — | — |
| test_match_results.test | COMPILE | 0.575582 | 1.041966 | 1.347508 | — | — | — |
| test_regex_algorithms.test | COMPILE | 0.579844 | 1.047056 | 1.369097 | — | — | — |
| test_regex_compiler.test | COMPILE | 0.882864 | 1.701464 | 2.042784 | — | — | — |
| test_regex_constants.test | COMPILE | 0.050289 | 0.070558 | 0.070339 | — | — | — |
| test_regex_error.test | COMPILE | 0.313451 | 0.572497 | 0.712338 | — | — | — |
| test_regex_iterator.test | COMPILE | 0.569177 | 1.077217 | 1.345814 | — | — | — |
| test_regex_primitives.test | COMPILE | 0.831509 | 1.717979 | 1.752667 | — | — | — |
| test_regex_token_iterator.test | COMPILE | 0.509329 | 1.091013 | 1.346107 | — | — | — |
| test_regex_traits.test | COMPILE | 0.327775 | 0.573159 | 0.724336 | — | — | — |
| test_sub_match.test | COMPILE | 0.257998 | 0.463367 | 0.557323 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.032× | 1.032×, 1.031× | 1.003× |
| clang | 0.932× | 0.929×, 0.935× | 1.004× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| c_traits.test | 3 | 0.008665s | 0.011100s | 0.008507s | byte equality observed |
| c_traits_u.test | 3 | 0.009645s | 0.011591s | 0.008866s | byte equality observed |
| misc1.test | 8 | 0.002594s | 0.002447s | 0.002422s | byte equality observed |
| misc2.test | 9 | 0.002295s | 0.002184s | 0.002205s | byte equality observed |
| regress.test | 3 | 0.012074s | 0.012753s | 0.009884s | byte equality observed |
| regress_u.test | 2 | 0.012125s | 0.013043s | 0.010328s | byte equality observed |
| test1.test | 10 | 0.002118s | 0.002050s | 0.002011s | byte equality observed |
| test10.test | 10 | 0.002056s | 0.002022s | 0.001981s | byte equality observed |
| test10u.test | 10 | 0.002077s | 0.002029s | 0.002012s | byte equality observed |
| test11.test | 10 | 0.002087s | 0.002059s | 0.001960s | byte equality observed |
| test11u.test | 10 | 0.002101s | 0.002064s | 0.001980s | byte equality observed |
| test1u.test | 9 | 0.002092s | 0.002059s | 0.001993s | byte equality observed |
| test2.test | 9 | 0.002106s | 0.002108s | 0.002011s | byte equality observed |
| test2u.test | 10 | 0.002141s | 0.002132s | 0.002033s | byte equality observed |
| test3.test | 10 | 0.002146s | 0.002102s | 0.002036s | byte equality observed |
| test3u.test | 10 | 0.002147s | 0.002153s | 0.002018s | byte equality observed |
| test4.test | 10 | 0.002105s | 0.002085s | 0.001979s | byte equality observed |
| test4u.test | 10 | 0.002146s | 0.002065s | 0.002065s | byte equality observed |
| test5.test | 9 | 0.002130s | 0.002107s | 0.002022s | byte equality observed |
| test5u.test | 10 | 0.002165s | 0.002106s | 0.002053s | byte equality observed |
| test6.test | 10 | 0.002120s | 0.002046s | 0.002017s | byte equality observed |
| test6u.test | 9 | 0.002109s | 0.002028s | 0.002002s | byte equality observed |
| test7.test | 10 | 0.002103s | 0.002051s | 0.002002s | byte equality observed |
| test7u.test | 9 | 0.002098s | 0.002106s | 0.002019s | byte equality observed |
| test8.test | 10 | 0.002081s | 0.002042s | 0.001991s | byte equality observed |
| test8u.test | 10 | 0.002100s | 0.002006s | 0.002012s | byte equality observed |
| test9.test | 10 | 0.002100s | 0.002083s | 0.001995s | byte equality observed |
| test9u.test | 10 | 0.002108s | 0.002078s | 0.002017s | byte equality observed |
| test_actions.test | 9 | 0.002459s | 0.002461s | 0.002376s | byte equality observed |
| test_assert.test | 10 | 0.002161s | 0.002092s | 0.002072s | byte equality observed |
| test_assert_with_placeholder.test | 10 | 0.002081s | 0.002023s | 0.001952s | byte equality observed |
| test_cycles.test | 6 | 0.002088s | 0.002089s | 0.002051s | byte equality observed |
| test_dynamic.test | 6 | 0.002142s | 0.002024s | 0.002032s | byte equality observed |
| test_dynamic_grammar.test | 9 | 0.002220s | 0.002136s | 0.002095s | byte equality observed |
| test_format.test | 9 | 0.002222s | 0.002137s | 0.002036s | byte equality observed |
| test_non_char.test | 10 | 0.002158s | 0.002085s | 0.001991s | byte equality observed |
| test_skip.test | 10 | 0.002057s | 0.002016s | 0.001943s | byte equality observed |
| test_static.test | 10 | 0.001873s | 0.001773s | 0.001789s | byte equality observed |
| test_symbols.test | 9 | 0.002251s | 0.002151s | 0.002091s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
