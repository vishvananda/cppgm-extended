# libs/xpressive/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| regress.test | RUN | 3.173965 | 17.612816 | 14.048818 | 0.010813 | 0.013818 | 0.009824 |
| regress_u.test | RUN | 3.359167 | 18.053696 | 14.435004 | 0.010941 | 0.013100 | 0.010583 |
| c_traits.test | RUN | 3.177036 | 17.181699 | 13.762187 | 0.008237 | 0.011384 | 0.008315 |
| c_traits_u.test | RUN | 3.372031 | 17.976791 | 14.112924 | 0.009063 | 0.012145 | 0.009109 |
| test1.test | RUN | 1.171960 | 4.499296 | 4.306699 | 0.003022 | 0.003216 | 0.003218 |
| test2.test | RUN | 1.516458 | 6.442218 | 6.226774 | 0.003146 | 0.002932 | 0.003000 |
| test3.test | RUN | 1.520254 | 6.404049 | 6.364109 | 0.003008 | 0.002923 | 0.002916 |
| test4.test | RUN | 1.354777 | 5.279526 | 5.327559 | 0.003006 | 0.003304 | 0.003208 |
| test5.test | RUN | 1.413806 | 5.739745 | 5.530506 | 0.003259 | 0.003117 | 0.003352 |
| test6.test | RUN | 1.275655 | 4.786940 | 4.794270 | 0.003126 | 0.003322 | 0.003265 |
| test7.test | RUN | 1.234084 | 4.930888 | 4.660207 | 0.002955 | 0.003149 | 0.003069 |
| test8.test | RUN | 1.226125 | 4.753489 | 4.510238 | 0.002872 | 0.003190 | 0.003039 |
| test9.test | RUN | 1.333632 | 5.205980 | 5.101328 | 0.003023 | 0.003088 | 0.002774 |
| test10.test | RUN | 1.321412 | 4.890322 | 4.751094 | 0.003116 | 0.003010 | 0.003262 |
| test11.test | RUN | 1.247196 | 4.748767 | 4.692132 | 0.003082 | 0.003321 | 0.003170 |
| test1u.test | RUN | 1.169784 | 4.478096 | 4.133992 | 0.003085 | 0.003037 | 0.002789 |
| test2u.test | RUN | 1.567295 | 6.687940 | 6.294750 | 0.003184 | 0.003281 | 0.003192 |
| test3u.test | RUN | 1.548405 | 6.830581 | 6.435126 | 0.003010 | 0.003429 | 0.003448 |
| test4u.test | RUN | 1.364800 | 5.180216 | 5.097488 | 0.003342 | 0.003240 | 0.003132 |
| test5u.test | RUN | 1.437439 | 5.815157 | 5.706440 | 0.002867 | 0.003436 | 0.003180 |
| test6u.test | RUN | 1.345282 | 4.870614 | 4.862555 | 0.003098 | 0.003317 | 0.003122 |
| test7u.test | RUN | 1.290396 | 5.110471 | 4.904213 | 0.003043 | 0.003387 | 0.003156 |
| test8u.test | RUN | 1.243142 | 4.756177 | 4.560620 | 0.002977 | 0.003272 | 0.003175 |
| test9u.test | RUN | 1.331181 | 5.095373 | 5.158456 | 0.003100 | 0.003314 | 0.003294 |
| test10u.test | RUN | 1.360070 | 4.845270 | 4.718109 | 0.003123 | 0.003389 | 0.003219 |
| test11u.test | RUN | 1.312141 | 4.869632 | 4.900600 | 0.003259 | 0.003327 | 0.003174 |
| misc1.test | RUN | 2.384233 | 12.232756 | 10.658998 | 0.004231 | 0.004233 | 0.004070 |
| misc2.test | RUN | 2.390287 | 11.619256 | 10.303569 | 0.003655 | 0.003652 | 0.003618 |
| test_format.test | RUN | 2.569679 | 14.147433 | 11.617113 | 0.003410 | 0.003476 | 0.005550 |
| test_cycles.test | RUN | 1.180049 | 4.392269 | 3.988520 | 0.003101 | 0.003277 | 0.002780 |
| test_non_char.test | RUN | 2.080685 | 9.904492 | 8.702368 | 0.003281 | 0.003371 | 0.003217 |
| test_static.test | RUN | 1.058923 | 3.976843 | 3.444568 | 0.003107 | 0.003208 | 0.002821 |
| test_actions.test | RUN | 2.443105 | 11.931174 | 10.865162 | 0.003951 | 0.004489 | 0.003816 |
| test_assert.test | RUN | 1.425595 | 5.109488 | 5.112162 | 0.003578 | 0.003655 | 0.003150 |
| test_assert_with_placeholder.test | RUN | 1.347549 | 4.739518 | 4.540873 | 0.002790 | 0.003090 | 0.003219 |
| test_symbols.test | RUN | 2.635672 | 11.634930 | 12.228911 | 0.004199 | 0.004066 | 0.004344 |
| test_dynamic.test | RUN | 1.684375 | 7.848645 | 7.067967 | 0.003046 | 0.003104 | 0.003198 |
| test_dynamic_grammar.test | RUN | 1.796580 | 8.420619 | 7.194938 | 0.003233 | 0.003573 | 0.003393 |
| test_skip.test | RUN | 1.916382 | 8.623687 | 8.613182 | 0.003097 | 0.003195 | 0.002938 |
| multiple_defs.test | LINK | 3.298336 | 15.702508 | 14.097955 | — | — | — |
| test_basic_regex.test | COMPILE | 0.769796 | 1.291276 | 1.935528 | — | — | — |
| test_match_results.test | COMPILE | 0.496164 | 0.884384 | 1.365552 | — | — | — |
| test_regex_algorithms.test | COMPILE | 0.519623 | 0.890854 | 1.299140 | — | — | — |
| test_regex_compiler.test | COMPILE | 0.758419 | 1.496350 | 1.916376 | — | — | — |
| test_regex_constants.test | COMPILE | 0.047770 | 0.063651 | 0.068259 | — | — | — |
| test_regex_error.test | COMPILE | 0.267417 | 0.487198 | 0.672480 | — | — | — |
| test_regex_iterator.test | COMPILE | 0.503875 | 0.913361 | 1.249675 | — | — | — |
| test_regex_primitives.test | COMPILE | 0.718080 | 1.432204 | 1.687310 | — | — | — |
| test_regex_token_iterator.test | COMPILE | 0.446333 | 0.901176 | 1.145822 | — | — | — |
| test_regex_traits.test | COMPILE | 0.283551 | 0.478788 | 0.681922 | — | — | — |
| test_sub_match.test | COMPILE | 0.245401 | 0.391967 | 0.429513 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.093× | 1.104×, 1.083× | 0.988× |
| clang | 0.934× | 0.935×, 0.933× | 1.003× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1363 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| c_traits.test | 3 | 0.007159s | 0.010334s | 0.006504s | byte equality observed |
| c_traits_u.test | 3 | 0.008064s | 0.010814s | 0.007176s | byte equality observed |
| misc1.test | 8 | 0.002678s | 0.002589s | 0.002570s | byte equality observed |
| misc2.test | 9 | 0.002405s | 0.002255s | 0.002320s | byte equality observed |
| regress.test | 3 | 0.009709s | 0.011724s | 0.008186s | byte equality observed |
| regress_u.test | 3 | 0.009592s | 0.011980s | 0.008382s | byte equality observed |
| test1.test | 10 | 0.002098s | 0.002078s | 0.002022s | byte equality observed |
| test10.test | 10 | 0.002077s | 0.002043s | 0.002022s | byte equality observed |
| test10u.test | 10 | 0.002075s | 0.002019s | 0.002007s | byte equality observed |
| test11.test | 10 | 0.002088s | 0.002040s | 0.002020s | byte equality observed |
| test11u.test | 10 | 0.002103s | 0.002066s | 0.002055s | byte equality observed |
| test1u.test | 9 | 0.002083s | 0.002068s | 0.002043s | byte equality observed |
| test2.test | 10 | 0.002132s | 0.002138s | 0.002082s | byte equality observed |
| test2u.test | 10 | 0.002139s | 0.002122s | 0.002083s | byte equality observed |
| test3.test | 10 | 0.002135s | 0.002078s | 0.002088s | byte equality observed |
| test3u.test | 10 | 0.002152s | 0.002170s | 0.002088s | byte equality observed |
| test4.test | 10 | 0.002106s | 0.002053s | 0.001994s | byte equality observed |
| test4u.test | 10 | 0.002106s | 0.002060s | 0.002066s | byte equality observed |
| test5.test | 10 | 0.002131s | 0.002098s | 0.002083s | byte equality observed |
| test5u.test | 10 | 0.002136s | 0.002108s | 0.002070s | byte equality observed |
| test6.test | 10 | 0.002075s | 0.002065s | 0.002003s | byte equality observed |
| test6u.test | 10 | 0.002094s | 0.002049s | 0.002078s | byte equality observed |
| test7.test | 10 | 0.002095s | 0.002086s | 0.002003s | byte equality observed |
| test7u.test | 8 | 0.002110s | 0.002093s | 0.002068s | byte equality observed |
| test8.test | 7 | 0.002114s | 0.002025s | 0.002017s | byte equality observed |
| test8u.test | 10 | 0.002107s | 0.002052s | 0.001985s | byte equality observed |
| test9.test | 9 | 0.002102s | 0.002125s | 0.001991s | byte equality observed |
| test9u.test | 10 | 0.002094s | 0.002044s | 0.002030s | byte equality observed |
| test_actions.test | 8 | 0.002668s | 0.002659s | 0.002559s | byte equality observed |
| test_assert.test | 9 | 0.002183s | 0.002180s | 0.002162s | byte equality observed |
| test_assert_with_placeholder.test | 10 | 0.002040s | 0.002028s | 0.002014s | byte equality observed |
| test_cycles.test | 10 | 0.002141s | 0.002060s | 0.002056s | byte equality observed |
| test_dynamic.test | 10 | 0.002079s | 0.002046s | 0.002031s | byte equality observed |
| test_dynamic_grammar.test | 10 | 0.002236s | 0.002196s | 0.002202s | byte equality observed |
| test_format.test | 9 | 0.002205s | 0.002157s | 0.002133s | byte equality observed |
| test_non_char.test | 10 | 0.002182s | 0.002111s | 0.002088s | byte equality observed |
| test_skip.test | 10 | 0.002080s | 0.002022s | 0.002045s | byte equality observed |
| test_static.test | 9 | 0.001893s | 0.001903s | 0.001879s | byte equality observed |
| test_symbols.test | 9 | 0.002372s | 0.002455s | 0.002324s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
