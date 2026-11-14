# libs/xpressive/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| regress.test | RUN | 2.466179 | 16.582228 | 12.493683 | 0.008608 | 0.010332 | 0.008000 |
| regress_u.test | RUN | 2.642027 | 16.915319 | 12.639659 | 0.008893 | 0.010646 | 0.008446 |
| c_traits.test | RUN | 2.500447 | 16.034918 | 12.006895 | 0.007060 | 0.008651 | 0.006872 |
| c_traits_u.test | RUN | 2.667590 | 16.852740 | 12.444156 | 0.007632 | 0.009632 | 0.007542 |
| test1.test | RUN | 0.874117 | 4.045506 | 3.565494 | 0.002267 | 0.002100 | 0.002207 |
| test2.test | RUN | 1.168055 | 5.878065 | 5.210737 | 0.001883 | 0.002052 | 0.002067 |
| test3.test | RUN | 1.173468 | 5.834017 | 5.356775 | 0.001990 | 0.002198 | 0.002219 |
| test4.test | RUN | 1.024384 | 4.755023 | 4.320707 | 0.002242 | 0.002091 | 0.002008 |
| test5.test | RUN | 1.105394 | 5.160591 | 4.677634 | 0.001859 | 0.002132 | 0.002177 |
| test6.test | RUN | 0.966034 | 4.345053 | 3.970476 | 0.002006 | 0.002093 | 0.002242 |
| test7.test | RUN | 0.952036 | 4.425848 | 3.993553 | 0.002129 | 0.002133 | 0.002125 |
| test8.test | RUN | 0.942836 | 4.314678 | 3.863879 | 0.001854 | 0.002012 | 0.002246 |
| test9.test | RUN | 1.018465 | 4.684194 | 4.393090 | 0.001948 | 0.002194 | 0.001971 |
| test10.test | RUN | 0.992105 | 4.385091 | 4.065183 | 0.001919 | 0.001966 | 0.002211 |
| test11.test | RUN | 0.964323 | 4.221312 | 3.995174 | 0.002076 | 0.001840 | 0.002046 |
| test1u.test | RUN | 0.884955 | 4.005435 | 3.469144 | 0.002136 | 0.002043 | 0.002044 |
| test2u.test | RUN | 1.198526 | 5.963355 | 5.434666 | 0.002124 | 0.002290 | 0.002174 |
| test3u.test | RUN | 1.217790 | 6.082831 | 5.606625 | 0.002043 | 0.002073 | 0.002246 |
| test4u.test | RUN | 1.023445 | 4.673880 | 4.425956 | 0.002446 | 0.002063 | 0.002178 |
| test5u.test | RUN | 1.123558 | 5.219300 | 4.903532 | 0.002023 | 0.002092 | 0.002285 |
| test6u.test | RUN | 1.005180 | 4.389715 | 4.087077 | 0.001924 | 0.002139 | 0.002497 |
| test7u.test | RUN | 0.973379 | 4.590393 | 4.166646 | 0.002252 | 0.001965 | 0.002044 |
| test8u.test | RUN | 0.941457 | 4.252806 | 3.917413 | 0.002034 | 0.002114 | 0.002077 |
| test9u.test | RUN | 1.026557 | 4.583956 | 4.406663 | 0.002020 | 0.002049 | 0.002220 |
| test10u.test | RUN | 0.999104 | 4.356935 | 4.034880 | 0.002087 | 0.002130 | 0.002224 |
| test11u.test | RUN | 0.998433 | 4.364833 | 4.184422 | 0.002127 | 0.002166 | 0.002141 |
| misc1.test | RUN | 1.813668 | 11.319924 | 9.348670 | 0.002588 | 0.002480 | 0.002596 |
| misc2.test | RUN | 1.860625 | 10.817276 | 8.862993 | 0.002138 | 0.002218 | 0.002321 |
| test_format.test | RUN | 1.934846 | 13.316954 | 10.347851 | 0.002154 | 0.002264 | 0.002211 |
| test_cycles.test | RUN | 0.825322 | 3.888574 | 3.208064 | 0.001996 | 0.002140 | 0.002186 |
| test_non_char.test | RUN | 1.613027 | 9.265420 | 7.587473 | 0.002086 | 0.002037 | 0.002233 |
| test_static.test | RUN | 0.771356 | 3.562766 | 2.897145 | 0.002272 | 0.002069 | 0.002045 |
| test_actions.test | RUN | 1.947513 | 10.930119 | 9.406897 | 0.002564 | 0.002450 | 0.002546 |
| test_assert.test | RUN | 1.090552 | 4.610192 | 4.252890 | 0.002130 | 0.002221 | 0.002206 |
| test_assert_with_placeholder.test | RUN | 1.015779 | 4.199842 | 3.867542 | 0.002086 | 0.002045 | 0.002109 |
| test_symbols.test | RUN | 1.999596 | 10.637949 | 10.460789 | 0.002605 | 0.002510 | 0.002521 |
| test_dynamic.test | RUN | 1.268865 | 7.347148 | 6.044344 | 0.002087 | 0.002166 | 0.002357 |
| test_dynamic_grammar.test | RUN | 1.345802 | 7.798828 | 6.329193 | 0.002272 | 0.002176 | 0.002288 |
| test_skip.test | RUN | 1.523230 | 7.712184 | 7.151792 | 0.002065 | 0.002033 | 0.001921 |
| multiple_defs.test | LINK | 2.504554 | 14.129682 | 12.102938 | — | — | — |
| test_basic_regex.test | COMPILE | 0.521161 | 1.076958 | 1.510753 | — | — | — |
| test_match_results.test | COMPILE | 0.343248 | 0.737640 | 0.985142 | — | — | — |
| test_regex_algorithms.test | COMPILE | 0.347769 | 0.720024 | 0.984302 | — | — | — |
| test_regex_compiler.test | COMPILE | 0.522562 | 1.174651 | 1.493455 | — | — | — |
| test_regex_constants.test | COMPILE | 0.025263 | 0.046441 | 0.052555 | — | — | — |
| test_regex_error.test | COMPILE | 0.186093 | 0.397974 | 0.517669 | — | — | — |
| test_regex_iterator.test | COMPILE | 0.334430 | 0.717240 | 0.960659 | — | — | — |
| test_regex_primitives.test | COMPILE | 0.521198 | 1.184307 | 1.359122 | — | — | — |
| test_regex_token_iterator.test | COMPILE | 0.317740 | 0.742551 | 0.992891 | — | — | — |
| test_regex_traits.test | COMPILE | 0.194166 | 0.390813 | 0.481435 | — | — | — |
| test_sub_match.test | COMPILE | 0.155250 | 0.319624 | 0.334435 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.078× | 1.077×, 1.080× | 1.005× |
| clang | 0.962× | 0.961×, 0.963× | 1.010× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1364 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43559814672).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| c_traits.test | 3 | 0.004939s | 0.006422s | 0.004883s | byte equality observed |
| c_traits_u.test | 3 | 0.005330s | 0.006892s | 0.005147s | byte equality observed |
| misc1.test | 9 | 0.001507s | 0.001521s | 0.001456s | byte equality observed |
| misc2.test | 10 | 0.001358s | 0.001365s | 0.001315s | byte equality observed |
| regress.test | 3 | 0.006056s | 0.007279s | 0.005548s | byte equality observed |
| regress_u.test | 3 | 0.006213s | 0.007539s | 0.005864s | byte equality observed |
| test1.test | 10 | 0.001262s | 0.001270s | 0.001218s | byte equality observed |
| test10.test | 9 | 0.001262s | 0.001293s | 0.001274s | byte equality observed |
| test10u.test | 10 | 0.001275s | 0.001264s | 0.001250s | byte equality observed |
| test11.test | 10 | 0.001285s | 0.001272s | 0.001235s | byte equality observed |
| test11u.test | 10 | 0.001289s | 0.001295s | 0.001249s | byte equality observed |
| test1u.test | 10 | 0.001272s | 0.001298s | 0.001220s | byte equality observed |
| test2.test | 10 | 0.001305s | 0.001319s | 0.001274s | byte equality observed |
| test2u.test | 10 | 0.001306s | 0.001333s | 0.001249s | byte equality observed |
| test3.test | 10 | 0.001319s | 0.001340s | 0.001269s | byte equality observed |
| test3u.test | 10 | 0.001308s | 0.001355s | 0.001295s | byte equality observed |
| test4.test | 10 | 0.001290s | 0.001290s | 0.001252s | byte equality observed |
| test4u.test | 10 | 0.001313s | 0.001286s | 0.001266s | byte equality observed |
| test5.test | 10 | 0.001298s | 0.001313s | 0.001270s | byte equality observed |
| test5u.test | 10 | 0.001299s | 0.001321s | 0.001254s | byte equality observed |
| test6.test | 10 | 0.001292s | 0.001305s | 0.001209s | byte equality observed |
| test6u.test | 10 | 0.001287s | 0.001295s | 0.001230s | byte equality observed |
| test7.test | 10 | 0.001289s | 0.001290s | 0.001218s | byte equality observed |
| test7u.test | 10 | 0.001273s | 0.001283s | 0.001249s | byte equality observed |
| test8.test | 10 | 0.001280s | 0.001283s | 0.001226s | byte equality observed |
| test8u.test | 10 | 0.001279s | 0.001267s | 0.001253s | byte equality observed |
| test9.test | 10 | 0.001293s | 0.001293s | 0.001219s | byte equality observed |
| test9u.test | 10 | 0.001289s | 0.001277s | 0.001278s | byte equality observed |
| test_actions.test | 10 | 0.001501s | 0.001507s | 0.001431s | byte equality observed |
| test_assert.test | 10 | 0.001339s | 0.001348s | 0.001327s | byte equality observed |
| test_assert_with_placeholder.test | 10 | 0.001283s | 0.001277s | 0.001223s | byte equality observed |
| test_cycles.test | 10 | 0.001296s | 0.001271s | 0.001245s | byte equality observed |
| test_dynamic.test | 10 | 0.001285s | 0.001256s | 0.001245s | byte equality observed |
| test_dynamic_grammar.test | 10 | 0.001354s | 0.001337s | 0.001326s | byte equality observed |
| test_format.test | 10 | 0.001339s | 0.001345s | 0.001285s | byte equality observed |
| test_non_char.test | 10 | 0.001308s | 0.001287s | 0.001261s | byte equality observed |
| test_skip.test | 10 | 0.001261s | 0.001283s | 0.001223s | byte equality observed |
| test_static.test | 10 | 0.001251s | 0.001263s | 0.001230s | byte equality observed |
| test_symbols.test | 10 | 0.001458s | 0.001423s | 0.001413s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43559814672).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
