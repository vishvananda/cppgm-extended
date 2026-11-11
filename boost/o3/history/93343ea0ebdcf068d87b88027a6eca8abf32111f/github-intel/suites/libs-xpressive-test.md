# libs/xpressive/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| regress.test | RUN | 4.147401 | 23.381756 | 17.080117 | 0.015008 | 0.016783 | 0.012028 |
| regress_u.test | RUN | 4.445643 | 24.012837 | 17.525625 | 0.014958 | 0.017420 | 0.012695 |
| c_traits.test | RUN | 4.156038 | 22.994543 | 16.562054 | 0.010512 | 0.014146 | 0.009758 |
| c_traits_u.test | RUN | 4.433496 | 23.821900 | 17.263141 | 0.011849 | 0.015124 | 0.010858 |
| test1.test | RUN | 1.477853 | 5.954439 | 5.197758 | 0.003165 | 0.002894 | 0.002868 |
| test2.test | RUN | 1.890867 | 8.367428 | 7.569964 | 0.003165 | 0.003126 | 0.002950 |
| test3.test | RUN | 1.901775 | 8.418401 | 7.631247 | 0.002785 | 0.003144 | 0.002786 |
| test4.test | RUN | 1.695558 | 6.855524 | 6.252303 | 0.002792 | 0.003052 | 0.002827 |
| test5.test | RUN | 1.771281 | 7.447915 | 6.769959 | 0.003022 | 0.003003 | 0.002899 |
| test6.test | RUN | 1.597717 | 6.319613 | 5.705350 | 0.003023 | 0.002960 | 0.002802 |
| test7.test | RUN | 1.548274 | 6.488732 | 5.755524 | 0.003047 | 0.003095 | 0.002838 |
| test8.test | RUN | 1.543497 | 6.318765 | 5.522524 | 0.003015 | 0.003118 | 0.002773 |
| test9.test | RUN | 1.660645 | 6.792196 | 6.256483 | 0.003013 | 0.002889 | 0.002803 |
| test10.test | RUN | 1.689663 | 6.479115 | 5.873492 | 0.002971 | 0.002833 | 0.002598 |
| test11.test | RUN | 1.583331 | 6.180718 | 5.841170 | 0.002797 | 0.002944 | 0.002789 |
| test1u.test | RUN | 1.511231 | 5.861532 | 5.051749 | 0.002147 | 0.002940 | 0.002868 |
| test2u.test | RUN | 1.950975 | 8.556879 | 7.784856 | 0.003172 | 0.003131 | 0.002851 |
| test3u.test | RUN | 1.988384 | 8.805336 | 7.940620 | 0.003117 | 0.003101 | 0.002943 |
| test4u.test | RUN | 1.707044 | 6.812113 | 6.272786 | 0.003020 | 0.003032 | 0.002933 |
| test5u.test | RUN | 1.808523 | 7.626250 | 7.023991 | 0.002951 | 0.003046 | 0.002879 |
| test6u.test | RUN | 1.651508 | 6.324355 | 5.904631 | 0.003085 | 0.002977 | 0.002872 |
| test7u.test | RUN | 1.610221 | 6.678130 | 5.998915 | 0.003078 | 0.002966 | 0.002839 |
| test8u.test | RUN | 1.551121 | 6.193072 | 5.593510 | 0.002809 | 0.002981 | 0.002954 |
| test9u.test | RUN | 1.682061 | 6.717805 | 6.309685 | 0.003105 | 0.002966 | 0.002969 |
| test10u.test | RUN | 1.657514 | 6.301553 | 5.715168 | 0.002984 | 0.002852 | 0.002813 |
| test11u.test | RUN | 1.626618 | 6.322502 | 5.892541 | 0.002974 | 0.003002 | 0.002869 |
| misc1.test | RUN | 3.029328 | 16.023002 | 12.688963 | 0.003750 | 0.003675 | 0.003525 |
| misc2.test | RUN | 3.020485 | 15.206855 | 12.347830 | 0.003429 | 0.003292 | 0.003084 |
| test_format.test | RUN | 3.241219 | 18.533455 | 14.102189 | 0.003162 | 0.003078 | 0.002932 |
| test_cycles.test | RUN | 1.418538 | 5.676287 | 4.680448 | 0.003013 | 0.003008 | 0.002867 |
| test_non_char.test | RUN | 2.633896 | 12.945795 | 10.742306 | 0.003116 | 0.003090 | 0.002902 |
| test_static.test | RUN | 1.317663 | 5.213043 | 4.152676 | 0.002970 | 0.002821 | 0.002818 |
| test_actions.test | RUN | 3.085984 | 15.693782 | 13.064746 | 0.003741 | 0.003638 | 0.003548 |
| test_assert.test | RUN | 1.793071 | 6.819676 | 6.076486 | 0.003172 | 0.002939 | 0.003024 |
| test_assert_with_placeholder.test | RUN | 1.685754 | 6.108971 | 5.588082 | 0.003002 | 0.002270 | 0.002826 |
| test_symbols.test | RUN | 3.225250 | 15.298697 | 14.744741 | 0.003647 | 0.003535 | 0.003631 |
| test_dynamic.test | RUN | 2.114500 | 10.408399 | 8.582533 | 0.002985 | 0.002996 | 0.002741 |
| test_dynamic_grammar.test | RUN | 2.194385 | 11.101569 | 8.689357 | 0.003266 | 0.003131 | 0.003054 |
| test_skip.test | RUN | 2.408715 | 11.236181 | 10.228789 | 0.002865 | 0.002932 | 0.002836 |
| multiple_defs.test | LINK | 4.181416 | 20.014361 | 17.003373 | — | — | — |
| test_basic_regex.test | COMPILE | 0.921521 | 1.694015 | 2.283228 | — | — | — |
| test_match_results.test | COMPILE | 0.612402 | 1.159170 | 1.586233 | — | — | — |
| test_regex_algorithms.test | COMPILE | 0.626369 | 1.165126 | 1.562932 | — | — | — |
| test_regex_compiler.test | COMPILE | 0.919213 | 1.782252 | 2.340439 | — | — | — |
| test_regex_constants.test | COMPILE | 0.054342 | 0.074204 | 0.075919 | — | — | — |
| test_regex_error.test | COMPILE | 0.327385 | 0.615212 | 0.835518 | — | — | — |
| test_regex_iterator.test | COMPILE | 0.613584 | 1.117574 | 1.491417 | — | — | — |
| test_regex_primitives.test | COMPILE | 0.822791 | 1.793769 | 1.884628 | — | — | — |
| test_regex_token_iterator.test | COMPILE | 0.543606 | 1.145735 | 1.496001 | — | — | — |
| test_regex_traits.test | COMPILE | 0.347456 | 0.611550 | 0.813744 | — | — | — |
| test_sub_match.test | COMPILE | 0.298550 | 0.485394 | 0.607022 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.074× | 1.074×, 1.073× | 1.012× |
| clang | 0.920× | 0.918×, 0.922× | 1.004× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| c_traits.test | 3 | 0.009453s | 0.012918s | 0.009170s | byte equality observed |
| c_traits_u.test | 2 | 0.010873s | 0.013485s | 0.010046s | byte equality observed |
| misc1.test | 9 | 0.002639s | 0.002439s | 0.002495s | byte equality observed |
| misc2.test | 10 | 0.002303s | 0.002201s | 0.002283s | byte equality observed |
| regress.test | 2 | 0.013357s | 0.015073s | 0.010751s | byte equality observed |
| regress_u.test | 2 | 0.013851s | 0.015791s | 0.011541s | byte equality observed |
| test1.test | 10 | 0.002155s | 0.002108s | 0.002031s | byte equality observed |
| test10.test | 10 | 0.002098s | 0.002024s | 0.002005s | byte equality observed |
| test10u.test | 10 | 0.002081s | 0.002035s | 0.001964s | byte equality observed |
| test11.test | 10 | 0.002095s | 0.002046s | 0.001988s | byte equality observed |
| test11u.test | 10 | 0.002096s | 0.002077s | 0.002069s | byte equality observed |
| test1u.test | 10 | 0.002112s | 0.002037s | 0.001994s | byte equality observed |
| test2.test | 10 | 0.002151s | 0.002167s | 0.002087s | byte equality observed |
| test2u.test | 10 | 0.002156s | 0.002148s | 0.002097s | byte equality observed |
| test3.test | 10 | 0.002177s | 0.002174s | 0.002059s | byte equality observed |
| test3u.test | 10 | 0.002161s | 0.002196s | 0.002111s | byte equality observed |
| test4.test | 10 | 0.002163s | 0.002144s | 0.001991s | byte equality observed |
| test4u.test | 10 | 0.002139s | 0.002117s | 0.002084s | byte equality observed |
| test5.test | 10 | 0.002141s | 0.002126s | 0.002084s | byte equality observed |
| test5u.test | 10 | 0.002200s | 0.002109s | 0.002047s | byte equality observed |
| test6.test | 10 | 0.002117s | 0.002081s | 0.002008s | byte equality observed |
| test6u.test | 10 | 0.002140s | 0.002129s | 0.002042s | byte equality observed |
| test7.test | 10 | 0.002104s | 0.002093s | 0.002036s | byte equality observed |
| test7u.test | 10 | 0.002112s | 0.002057s | 0.002020s | byte equality observed |
| test8.test | 9 | 0.002133s | 0.002046s | 0.002010s | byte equality observed |
| test8u.test | 10 | 0.002100s | 0.002078s | 0.002005s | byte equality observed |
| test9.test | 10 | 0.002177s | 0.002055s | 0.002029s | byte equality observed |
| test9u.test | 10 | 0.002137s | 0.002110s | 0.002068s | byte equality observed |
| test_actions.test | 9 | 0.002535s | 0.002520s | 0.002405s | byte equality observed |
| test_assert.test | 10 | 0.002180s | 0.002135s | 0.002045s | byte equality observed |
| test_assert_with_placeholder.test | 10 | 0.002083s | 0.002030s | 0.002009s | byte equality observed |
| test_cycles.test | 10 | 0.002109s | 0.002049s | 0.002005s | byte equality observed |
| test_dynamic.test | 10 | 0.002103s | 0.002074s | 0.002052s | byte equality observed |
| test_dynamic_grammar.test | 10 | 0.002225s | 0.002191s | 0.002123s | byte equality observed |
| test_format.test | 10 | 0.002217s | 0.002113s | 0.002118s | byte equality observed |
| test_non_char.test | 10 | 0.002147s | 0.002061s | 0.002085s | byte equality observed |
| test_skip.test | 10 | 0.002093s | 0.002023s | 0.002003s | byte equality observed |
| test_static.test | 9 | 0.002019s | 0.001864s | 0.001976s | byte equality observed |
| test_symbols.test | 9 | 0.002094s | 0.002089s | 0.001917s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
