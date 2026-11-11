# libs/xpressive/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| regress.test | RUN | 2.519868 | 13.865516 | 11.149163 | 0.008592 | 0.010306 | 0.007751 |
| regress_u.test | RUN | 2.656908 | 14.234620 | 11.102982 | 0.008924 | 0.010191 | 0.008061 |
| c_traits.test | RUN | 2.435761 | 13.603451 | 10.698034 | 0.007485 | 0.008920 | 0.006625 |
| c_traits_u.test | RUN | 2.659468 | 14.058600 | 11.079698 | 0.007389 | 0.009678 | 0.007281 |
| test1.test | RUN | 0.877017 | 3.449236 | 3.128285 | 0.002691 | 0.002767 | 0.002999 |
| test2.test | RUN | 1.118708 | 4.861060 | 4.654181 | 0.002904 | 0.002884 | 0.002838 |
| test3.test | RUN | 1.134606 | 5.298153 | 4.651931 | 0.002791 | 0.002832 | 0.002948 |
| test4.test | RUN | 1.008822 | 4.389154 | 3.774410 | 0.002659 | 0.002708 | 0.002952 |
| test5.test | RUN | 1.064897 | 4.737459 | 4.133605 | 0.002709 | 0.002703 | 0.002851 |
| test6.test | RUN | 0.946492 | 3.875311 | 3.448865 | 0.002798 | 0.003510 | 0.002826 |
| test7.test | RUN | 0.926324 | 3.849486 | 3.478475 | 0.002804 | 0.002635 | 0.002883 |
| test8.test | RUN | 0.939899 | 3.658625 | 3.369656 | 0.002606 | 0.002765 | 0.002824 |
| test9.test | RUN | 1.015078 | 3.996125 | 3.827051 | 0.002734 | 0.002593 | 0.002765 |
| test10.test | RUN | 0.972034 | 3.744264 | 3.597034 | 0.002836 | 0.002667 | 0.002724 |
| test11.test | RUN | 0.927283 | 3.681715 | 3.504434 | 0.002686 | 0.002941 | 0.002596 |
| test1u.test | RUN | 0.874567 | 3.560643 | 3.071418 | 0.002552 | 0.002757 | 0.002786 |
| test2u.test | RUN | 1.166668 | 5.248969 | 4.633601 | 0.002927 | 0.002787 | 0.002945 |
| test3u.test | RUN | 1.157829 | 5.390344 | 4.838132 | 0.002796 | 0.002904 | 0.002770 |
| test4u.test | RUN | 1.031964 | 4.073398 | 3.846212 | 0.002631 | 0.002804 | 0.002820 |
| test5u.test | RUN | 1.046171 | 4.464555 | 4.254441 | 0.002621 | 0.002833 | 0.002820 |
| test6u.test | RUN | 0.992541 | 3.713216 | 3.485962 | 0.002808 | 0.002811 | 0.002997 |
| test7u.test | RUN | 0.981083 | 3.901219 | 3.622266 | 0.002593 | 0.002774 | 0.002841 |
| test8u.test | RUN | 0.918944 | 3.627741 | 3.372726 | 0.002629 | 0.002814 | 0.002890 |
| test9u.test | RUN | 1.017033 | 3.927545 | 3.841504 | 0.002796 | 0.002890 | 0.002817 |
| test10u.test | RUN | 1.006441 | 3.725163 | 3.516942 | 0.002792 | 0.002896 | 0.002788 |
| test11u.test | RUN | 0.967557 | 3.777994 | 3.628240 | 0.003008 | 0.003070 | 0.002700 |
| misc1.test | RUN | 1.843575 | 9.708546 | 8.161344 | 0.003539 | 0.003465 | 0.003487 |
| misc2.test | RUN | 1.859759 | 9.173293 | 7.790699 | 0.003020 | 0.003076 | 0.003184 |
| test_format.test | RUN | 1.998833 | 11.286775 | 9.078816 | 0.002997 | 0.002959 | 0.002962 |
| test_cycles.test | RUN | 0.915703 | 3.342988 | 2.795204 | 0.002714 | 0.002827 | 0.002832 |
| test_non_char.test | RUN | 1.592755 | 7.705483 | 6.758651 | 0.002967 | 0.002848 | 0.002938 |
| test_static.test | RUN | 0.802610 | 3.044895 | 2.557383 | 0.002826 | 0.002549 | 0.002858 |
| test_actions.test | RUN | 1.881892 | 9.246225 | 8.415415 | 0.003472 | 0.003794 | 0.003685 |
| test_assert.test | RUN | 1.115461 | 3.891746 | 3.862592 | 0.002949 | 0.002905 | 0.002825 |
| test_assert_with_placeholder.test | RUN | 1.039339 | 3.622228 | 3.527398 | 0.002619 | 0.002765 | 0.002503 |
| test_symbols.test | RUN | 2.011819 | 9.124893 | 9.364038 | 0.003980 | 0.003502 | 0.003809 |
| test_dynamic.test | RUN | 1.272884 | 6.273660 | 5.397521 | 0.002909 | 0.002467 | 0.002909 |
| test_dynamic_grammar.test | RUN | 1.358189 | 6.535870 | 5.539792 | 0.003155 | 0.002982 | 0.003067 |
| test_skip.test | RUN | 1.451414 | 6.699755 | 6.422286 | 0.002535 | 0.002413 | 0.002815 |
| multiple_defs.test | LINK | 2.479421 | 11.855163 | 10.800877 | — | — | — |
| test_basic_regex.test | COMPILE | 0.549946 | 0.961606 | 1.302137 | — | — | — |
| test_match_results.test | COMPILE | 0.368027 | 0.652972 | 0.938230 | — | — | — |
| test_regex_algorithms.test | COMPILE | 0.369168 | 0.634050 | 0.902916 | — | — | — |
| test_regex_compiler.test | COMPILE | 0.567356 | 1.039073 | 1.337908 | — | — | — |
| test_regex_constants.test | COMPILE | 0.036031 | 0.045146 | 0.052783 | — | — | — |
| test_regex_error.test | COMPILE | 0.196551 | 0.327928 | 0.493894 | — | — | — |
| test_regex_iterator.test | COMPILE | 0.362156 | 0.631979 | 0.852114 | — | — | — |
| test_regex_primitives.test | COMPILE | 0.496560 | 1.038258 | 1.135337 | — | — | — |
| test_regex_token_iterator.test | COMPILE | 0.320452 | 0.643616 | 0.777649 | — | — | — |
| test_regex_traits.test | COMPILE | 0.206684 | 0.327720 | 0.446613 | — | — | — |
| test_sub_match.test | COMPILE | 0.180109 | 0.209146 | 0.343747 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.063×† | 1.076×, 1.050× | 0.933× |
| clang | 0.930× | 0.934×, 0.927× | 1.008× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1361 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| c_traits.test | 3 | 0.006592s | 0.008430s | 0.005977s | byte equality observed |
| c_traits_u.test | 3 | 0.006846s | 0.008673s | 0.006273s | byte equality observed |
| misc1.test | 9 | 0.002533s | 0.002347s | 0.002507s | byte equality observed |
| misc2.test | 10 | 0.002236s | 0.002098s | 0.002219s | byte equality observed |
| regress.test | 3 | 0.008278s | 0.009541s | 0.006982s | byte equality observed |
| regress_u.test | 3 | 0.008266s | 0.009715s | 0.006995s | byte equality observed |
| test1.test | 10 | 0.001966s | 0.001902s | 0.001928s | byte equality observed |
| test10.test | 10 | 0.001933s | 0.001894s | 0.001928s | byte equality observed |
| test10u.test | 10 | 0.001969s | 0.001865s | 0.001942s | byte equality observed |
| test11.test | 10 | 0.001962s | 0.001867s | 0.001881s | byte equality observed |
| test11u.test | 10 | 0.001933s | 0.001844s | 0.001942s | byte equality observed |
| test1u.test | 10 | 0.001962s | 0.001860s | 0.001938s | byte equality observed |
| test2.test | 10 | 0.001981s | 0.001927s | 0.001995s | byte equality observed |
| test2u.test | 10 | 0.002019s | 0.001963s | 0.002020s | byte equality observed |
| test3.test | 10 | 0.001981s | 0.001934s | 0.001979s | byte equality observed |
| test3u.test | 10 | 0.002001s | 0.001943s | 0.002000s | byte equality observed |
| test4.test | 10 | 0.002004s | 0.001903s | 0.001964s | byte equality observed |
| test4u.test | 10 | 0.001973s | 0.001889s | 0.001966s | byte equality observed |
| test5.test | 10 | 0.001922s | 0.001918s | 0.001977s | byte equality observed |
| test5u.test | 10 | 0.001965s | 0.001940s | 0.001967s | byte equality observed |
| test6.test | 10 | 0.001964s | 0.001901s | 0.001970s | byte equality observed |
| test6u.test | 10 | 0.001990s | 0.001892s | 0.001978s | byte equality observed |
| test7.test | 10 | 0.001970s | 0.001877s | 0.001946s | byte equality observed |
| test7u.test | 10 | 0.001938s | 0.001906s | 0.001926s | byte equality observed |
| test8.test | 10 | 0.001966s | 0.001852s | 0.001967s | byte equality observed |
| test8u.test | 10 | 0.002000s | 0.001862s | 0.001949s | byte equality observed |
| test9.test | 10 | 0.001978s | 0.001873s | 0.001933s | byte equality observed |
| test9u.test | 8 | 0.001999s | 0.001911s | 0.001954s | byte equality observed |
| test_actions.test | 8 | 0.002494s | 0.002392s | 0.002420s | byte equality observed |
| test_assert.test | 10 | 0.002051s | 0.001953s | 0.002068s | byte equality observed |
| test_assert_with_placeholder.test | 10 | 0.001962s | 0.001849s | 0.001892s | byte equality observed |
| test_cycles.test | 10 | 0.001966s | 0.001890s | 0.001929s | byte equality observed |
| test_dynamic.test | 10 | 0.001953s | 0.001889s | 0.001930s | byte equality observed |
| test_dynamic_grammar.test | 10 | 0.002074s | 0.002027s | 0.002063s | byte equality observed |
| test_format.test | 10 | 0.002054s | 0.001966s | 0.002063s | byte equality observed |
| test_non_char.test | 10 | 0.002073s | 0.001924s | 0.002005s | byte equality observed |
| test_skip.test | 10 | 0.001942s | 0.001854s | 0.001927s | byte equality observed |
| test_static.test | 10 | 0.001886s | 0.001744s | 0.001872s | byte equality observed |
| test_symbols.test | 9 | 0.002263s | 0.002194s | 0.002289s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
