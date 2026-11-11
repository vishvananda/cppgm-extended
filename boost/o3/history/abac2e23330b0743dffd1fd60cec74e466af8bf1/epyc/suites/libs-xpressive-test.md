# libs/xpressive/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| regress.test | RUN | 4.343270 | 24.301789 | 21.402917 | 0.015508 | 0.019994 | 0.014052 |
| regress_u.test | RUN | 4.561371 | 24.767156 | 20.916118 | 0.015918 | 0.019112 | 0.013909 |
| c_traits.test | RUN | 4.364694 | 24.268468 | 20.214380 | 0.010583 | 0.016369 | 0.011495 |
| c_traits_u.test | RUN | 4.570456 | 24.768763 | 20.615008 | 0.012971 | 0.017775 | 0.012741 |
| test1.test | RUN | 1.609099 | 6.319199 | 6.650961 | 0.003812 | 0.003926 | 0.003872 |
| test2.test | RUN | 2.113618 | 8.799921 | 9.184495 | 0.003948 | 0.003976 | 0.004112 |
| test3.test | RUN | 2.073036 | 9.003032 | 9.545844 | 0.003319 | 0.004393 | 0.004085 |
| test4.test | RUN | 1.882587 | 7.344861 | 7.650774 | 0.003814 | 0.004059 | 0.003369 |
| test5.test | RUN | 1.933216 | 7.798549 | 8.378916 | 0.003498 | 0.003898 | 0.004139 |
| test6.test | RUN | 1.762353 | 6.654049 | 7.131260 | 0.003542 | 0.003589 | 0.003925 |
| test7.test | RUN | 1.707125 | 6.880270 | 7.197772 | 0.003860 | 0.004216 | 0.003784 |
| test8.test | RUN | 1.703197 | 6.517920 | 6.951376 | 0.003711 | 0.003658 | 0.003688 |
| test9.test | RUN | 1.813402 | 7.413606 | 7.891971 | 0.003805 | 0.004086 | 0.003946 |
| test10.test | RUN | 1.843899 | 6.776848 | 7.326136 | 0.003775 | 0.004242 | 0.004036 |
| test11.test | RUN | 1.772761 | 6.664376 | 7.096586 | 0.003772 | 0.003950 | 0.003378 |
| test1u.test | RUN | 1.627072 | 6.148744 | 6.355686 | 0.003759 | 0.003658 | 0.003560 |
| test2u.test | RUN | 2.121924 | 9.381026 | 9.612404 | 0.003980 | 0.003765 | 0.003812 |
| test3u.test | RUN | 2.158702 | 9.380102 | 9.736061 | 0.004048 | 0.003658 | 0.003849 |
| test4u.test | RUN | 1.868190 | 7.094423 | 7.619349 | 0.003855 | 0.004052 | 0.004049 |
| test5u.test | RUN | 1.959068 | 7.914114 | 8.607515 | 0.003621 | 0.004288 | 0.004015 |
| test6u.test | RUN | 1.828033 | 6.715846 | 7.407359 | 0.003806 | 0.004312 | 0.003938 |
| test7u.test | RUN | 1.777241 | 7.141367 | 7.564875 | 0.003750 | 0.003974 | 0.003996 |
| test8u.test | RUN | 1.707079 | 6.616154 | 6.878766 | 0.003961 | 0.003793 | 0.004404 |
| test9u.test | RUN | 1.844486 | 7.025714 | 7.950633 | 0.003725 | 0.004362 | 0.003476 |
| test10u.test | RUN | 1.833000 | 6.827035 | 7.317957 | 0.003863 | 0.004460 | 0.004038 |
| test11u.test | RUN | 1.790608 | 6.798555 | 7.379522 | 0.004021 | 0.004141 | 0.003897 |
| misc1.test | RUN | 3.202229 | 16.737848 | 15.680705 | 0.004923 | 0.005152 | 0.005268 |
| misc2.test | RUN | 3.262660 | 16.203100 | 15.573962 | 0.004513 | 0.004602 | 0.004461 |
| test_format.test | RUN | 3.539947 | 19.382575 | 17.006528 | 0.004411 | 0.004218 | 0.004143 |
| test_cycles.test | RUN | 1.565176 | 6.255218 | 5.911993 | 0.003761 | 0.004256 | 0.003942 |
| test_non_char.test | RUN | 2.829224 | 13.890411 | 12.897219 | 0.004108 | 0.004236 | 0.004104 |
| test_static.test | RUN | 1.490673 | 5.503882 | 5.452761 | 0.004029 | 0.003771 | 0.003711 |
| test_actions.test | RUN | 3.337949 | 16.727785 | 16.406145 | 0.005216 | 0.005761 | 0.005534 |
| test_assert.test | RUN | 2.017691 | 7.071488 | 7.633967 | 0.004164 | 0.004108 | 0.004244 |
| test_assert_with_placeholder.test | RUN | 1.843233 | 6.471729 | 7.023090 | 0.003831 | 0.003816 | 0.003894 |
| test_symbols.test | RUN | 3.555868 | 16.273282 | 18.092096 | 0.005349 | 0.005578 | 0.005656 |
| test_dynamic.test | RUN | 2.335472 | 10.637189 | 10.599758 | 0.004012 | 0.003937 | 0.003696 |
| test_dynamic_grammar.test | RUN | 2.454008 | 11.444759 | 10.635739 | 0.004102 | 0.004175 | 0.004370 |
| test_skip.test | RUN | 2.668947 | 12.013270 | 12.709994 | 0.003793 | 0.004073 | 0.003681 |
| multiple_defs.test | LINK | 4.560855 | 21.446660 | 20.996435 | — | — | — |
| test_basic_regex.test | COMPILE | 1.063523 | 1.902459 | 3.035524 | — | — | — |
| test_match_results.test | COMPILE | 0.706659 | 1.302176 | 2.078992 | — | — | — |
| test_regex_algorithms.test | COMPILE | 0.687463 | 1.319848 | 1.974990 | — | — | — |
| test_regex_compiler.test | COMPILE | 1.047076 | 2.121947 | 2.938402 | — | — | — |
| test_regex_constants.test | COMPILE | 0.065873 | 0.087542 | 0.097004 | — | — | — |
| test_regex_error.test | COMPILE | 0.380018 | 0.685274 | 1.095535 | — | — | — |
| test_regex_iterator.test | COMPILE | 0.692008 | 1.251899 | 2.037434 | — | — | — |
| test_regex_primitives.test | COMPILE | 0.989843 | 2.051231 | 2.644530 | — | — | — |
| test_regex_token_iterator.test | COMPILE | 0.654409 | 1.267368 | 1.696938 | — | — | — |
| test_regex_traits.test | COMPILE | 0.400776 | 0.625662 | 1.119440 | — | — | — |
| test_sub_match.test | COMPILE | 0.307070 | 0.369825 | 0.735824 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.091× | 1.086×, 1.097× | 1.006× |
| clang | 0.907× | 0.900×, 0.915× | 1.003× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| c_traits.test | 2 | 0.010669s | 0.014985s | 0.009422s | byte equality observed |
| c_traits_u.test | 2 | 0.011211s | 0.015919s | 0.009602s | byte equality observed |
| misc1.test | 6 | 0.003557s | 0.003441s | 0.003308s | byte equality observed |
| misc2.test | 8 | 0.003218s | 0.003009s | 0.002868s | byte equality observed |
| regress.test | 2 | 0.014615s | 0.017581s | 0.011313s | byte equality observed |
| regress_u.test | 2 | 0.014718s | 0.017878s | 0.012439s | byte equality observed |
| test1.test | 8 | 0.002775s | 0.002738s | 0.002660s | byte equality observed |
| test10.test | 8 | 0.002760s | 0.002686s | 0.002590s | byte equality observed |
| test10u.test | 9 | 0.002703s | 0.002689s | 0.002678s | byte equality observed |
| test11.test | 8 | 0.002765s | 0.002688s | 0.002638s | byte equality observed |
| test11u.test | 8 | 0.002784s | 0.002702s | 0.002696s | byte equality observed |
| test1u.test | 8 | 0.002773s | 0.002736s | 0.002670s | byte equality observed |
| test2.test | 8 | 0.002849s | 0.002851s | 0.002696s | byte equality observed |
| test2u.test | 8 | 0.002830s | 0.002826s | 0.002682s | byte equality observed |
| test3.test | 8 | 0.002850s | 0.002821s | 0.002635s | byte equality observed |
| test3u.test | 8 | 0.002812s | 0.002807s | 0.002716s | byte equality observed |
| test4.test | 8 | 0.002766s | 0.002784s | 0.002687s | byte equality observed |
| test4u.test | 8 | 0.002798s | 0.002803s | 0.002666s | byte equality observed |
| test5.test | 8 | 0.002823s | 0.002750s | 0.002705s | byte equality observed |
| test5u.test | 6 | 0.002860s | 0.002812s | 0.002619s | byte equality observed |
| test6.test | 7 | 0.002771s | 0.002674s | 0.002586s | byte equality observed |
| test6u.test | 8 | 0.002786s | 0.002784s | 0.002642s | byte equality observed |
| test7.test | 9 | 0.002836s | 0.002770s | 0.002629s | byte equality observed |
| test7u.test | 8 | 0.002794s | 0.002735s | 0.002665s | byte equality observed |
| test8.test | 9 | 0.002748s | 0.002717s | 0.002560s | byte equality observed |
| test8u.test | 8 | 0.002763s | 0.002723s | 0.002681s | byte equality observed |
| test9.test | 8 | 0.002778s | 0.002724s | 0.002701s | byte equality observed |
| test9u.test | 8 | 0.002762s | 0.002802s | 0.002589s | byte equality observed |
| test_actions.test | 7 | 0.003501s | 0.003503s | 0.003367s | byte equality observed |
| test_assert.test | 8 | 0.002905s | 0.002837s | 0.002842s | byte equality observed |
| test_assert_with_placeholder.test | 9 | 0.002758s | 0.002667s | 0.002632s | byte equality observed |
| test_cycles.test | 8 | 0.002820s | 0.002674s | 0.002671s | byte equality observed |
| test_dynamic.test | 8 | 0.002788s | 0.002672s | 0.002609s | byte equality observed |
| test_dynamic_grammar.test | 6 | 0.003012s | 0.002925s | 0.002792s | byte equality observed |
| test_format.test | 8 | 0.002942s | 0.002773s | 0.002700s | byte equality observed |
| test_non_char.test | 8 | 0.002906s | 0.002807s | 0.002734s | byte equality observed |
| test_skip.test | 8 | 0.002688s | 0.002651s | 0.002602s | byte equality observed |
| test_static.test | 8 | 0.002605s | 0.002465s | 0.002557s | byte equality observed |
| test_symbols.test | 7 | 0.002902s | 0.002917s | 0.002691s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
