# libs/xpressive/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| regress.test | RUN | 4.042943 | 23.814604 | 19.897684 | 0.014955 | 0.019551 | 0.013980 |
| regress_u.test | RUN | 4.352745 | 24.654271 | 20.071580 | 0.015342 | 0.019952 | 0.013279 |
| c_traits.test | RUN | 3.980179 | 23.364316 | 19.137891 | 0.012691 | 0.015462 | 0.010120 |
| c_traits_u.test | RUN | 4.211788 | 24.437994 | 19.849889 | 0.011856 | 0.017614 | 0.012290 |
| test1.test | RUN | 1.576916 | 6.224860 | 6.193041 | 0.003754 | 0.003840 | 0.003728 |
| test2.test | RUN | 2.013002 | 8.673412 | 8.871839 | 0.003777 | 0.003840 | 0.003725 |
| test3.test | RUN | 2.011238 | 9.025870 | 8.933911 | 0.004088 | 0.004262 | 0.003866 |
| test4.test | RUN | 1.779781 | 7.232913 | 7.543577 | 0.003530 | 0.004352 | 0.003864 |
| test5.test | RUN | 1.843922 | 7.906529 | 7.878473 | 0.003515 | 0.004049 | 0.003690 |
| test6.test | RUN | 1.712205 | 6.721414 | 6.828854 | 0.003717 | 0.003700 | 0.003625 |
| test7.test | RUN | 1.651254 | 6.909905 | 6.929658 | 0.003626 | 0.004373 | 0.003859 |
| test8.test | RUN | 1.621115 | 6.728364 | 6.806942 | 0.003649 | 0.004124 | 0.003877 |
| test9.test | RUN | 1.765641 | 7.223614 | 7.535596 | 0.004035 | 0.003989 | 0.003814 |
| test10.test | RUN | 1.758685 | 6.967146 | 6.978265 | 0.003852 | 0.004205 | 0.003981 |
| test11.test | RUN | 1.653420 | 6.709718 | 7.047158 | 0.003549 | 0.004042 | 0.003863 |
| test1u.test | RUN | 1.572516 | 6.206517 | 6.193141 | 0.003876 | 0.004085 | 0.004041 |
| test2u.test | RUN | 2.041680 | 9.393460 | 9.378075 | 0.003945 | 0.004569 | 0.003813 |
| test3u.test | RUN | 2.076275 | 9.528977 | 9.689363 | 0.003662 | 0.004451 | 0.004025 |
| test4u.test | RUN | 1.797106 | 7.366552 | 7.631058 | 0.003687 | 0.004197 | 0.004154 |
| test5u.test | RUN | 1.887506 | 8.226888 | 8.449825 | 0.003721 | 0.004288 | 0.003953 |
| test6u.test | RUN | 1.771729 | 6.779399 | 7.113928 | 0.003593 | 0.004175 | 0.003770 |
| test7u.test | RUN | 1.769236 | 7.137752 | 7.357381 | 0.004061 | 0.003616 | 0.003986 |
| test8u.test | RUN | 1.661579 | 6.680364 | 6.770469 | 0.003734 | 0.004406 | 0.004077 |
| test9u.test | RUN | 1.813691 | 7.197927 | 7.640275 | 0.003846 | 0.004176 | 0.003468 |
| test10u.test | RUN | 1.807727 | 6.722906 | 7.171026 | 0.003956 | 0.004168 | 0.003976 |
| test11u.test | RUN | 1.767750 | 6.836010 | 7.301170 | 0.004081 | 0.004046 | 0.003889 |
| misc1.test | RUN | 3.028726 | 16.777975 | 15.129997 | 0.004814 | 0.004769 | 0.004909 |
| misc2.test | RUN | 3.053063 | 16.051461 | 14.842400 | 0.004144 | 0.004676 | 0.004372 |
| test_format.test | RUN | 3.296710 | 19.624817 | 16.310688 | 0.004022 | 0.004137 | 0.004046 |
| test_cycles.test | RUN | 1.555659 | 6.115645 | 5.835300 | 0.003865 | 0.004286 | 0.003873 |
| test_non_char.test | RUN | 2.661759 | 13.537388 | 12.667113 | 0.004091 | 0.004225 | 0.004174 |
| test_static.test | RUN | 1.455602 | 5.639915 | 5.110537 | 0.003824 | 0.003552 | 0.003980 |
| test_actions.test | RUN | 3.178090 | 16.638746 | 15.723501 | 0.005202 | 0.005348 | 0.005326 |
| test_assert.test | RUN | 1.914796 | 7.146215 | 7.306205 | 0.004247 | 0.003880 | 0.003498 |
| test_assert_with_placeholder.test | RUN | 1.826687 | 6.716162 | 6.872583 | 0.003844 | 0.003734 | 0.003612 |
| test_symbols.test | RUN | 3.496480 | 16.315324 | 18.010729 | 0.005051 | 0.004949 | 0.005520 |
| test_dynamic.test | RUN | 2.190692 | 10.843656 | 10.218424 | 0.003756 | 0.003978 | 0.003624 |
| test_dynamic_grammar.test | RUN | 2.285004 | 11.528855 | 10.530725 | 0.004078 | 0.004400 | 0.004135 |
| test_skip.test | RUN | 2.604905 | 11.992083 | 12.468681 | 0.003795 | 0.003780 | 0.003932 |
| multiple_defs.test | LINK | 4.365949 | 20.260857 | 20.423651 | — | — | — |
| test_basic_regex.test | COMPILE | 1.024930 | 1.880685 | 3.049825 | — | — | — |
| test_match_results.test | COMPILE | 0.701935 | 1.273683 | 2.008477 | — | — | — |
| test_regex_algorithms.test | COMPILE | 0.675041 | 1.263649 | 2.019722 | — | — | — |
| test_regex_compiler.test | COMPILE | 1.020264 | 1.965447 | 2.994955 | — | — | — |
| test_regex_constants.test | COMPILE | 0.066196 | 0.081519 | 0.102146 | — | — | — |
| test_regex_error.test | COMPILE | 0.372609 | 0.692676 | 1.129000 | — | — | — |
| test_regex_iterator.test | COMPILE | 0.660215 | 1.202363 | 2.027187 | — | — | — |
| test_regex_primitives.test | COMPILE | 1.031385 | 2.006344 | 2.663743 | — | — | — |
| test_regex_token_iterator.test | COMPILE | 0.629249 | 1.282222 | 1.958918 | — | — | — |
| test_regex_traits.test | COMPILE | 0.389255 | 0.645119 | 1.072074 | — | — | — |
| test_sub_match.test | COMPILE | 0.335508 | 0.535397 | 0.732860 | — | — | — |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.075× | 1.073×, 1.076× | 1.004× |
| clang | 0.914× | 0.911×, 0.918× | 1.008× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| c_traits.test | 2 | 0.010555s | 0.014576s | 0.009229s | byte equality observed |
| c_traits_u.test | 2 | 0.011624s | 0.015584s | 0.009640s | byte equality observed |
| misc1.test | 7 | 0.003410s | 0.003223s | 0.003119s | byte equality observed |
| misc2.test | 8 | 0.003066s | 0.002846s | 0.002780s | byte equality observed |
| regress.test | 2 | 0.014413s | 0.017206s | 0.011690s | byte equality observed |
| regress_u.test | 2 | 0.014535s | 0.017448s | 0.012076s | byte equality observed |
| test1.test | 8 | 0.002735s | 0.002610s | 0.002582s | byte equality observed |
| test10.test | 9 | 0.002670s | 0.002598s | 0.002566s | byte equality observed |
| test10u.test | 8 | 0.002728s | 0.002655s | 0.002653s | byte equality observed |
| test11.test | 9 | 0.002701s | 0.002634s | 0.002581s | byte equality observed |
| test11u.test | 8 | 0.002688s | 0.002616s | 0.002618s | byte equality observed |
| test1u.test | 9 | 0.002660s | 0.002748s | 0.002605s | byte equality observed |
| test2.test | 8 | 0.002732s | 0.002755s | 0.002618s | byte equality observed |
| test2u.test | 9 | 0.002802s | 0.002722s | 0.002640s | byte equality observed |
| test3.test | 8 | 0.002765s | 0.002778s | 0.002672s | byte equality observed |
| test3u.test | 9 | 0.002754s | 0.002790s | 0.002684s | byte equality observed |
| test4.test | 8 | 0.002764s | 0.002745s | 0.002523s | byte equality observed |
| test4u.test | 8 | 0.002810s | 0.002675s | 0.002617s | byte equality observed |
| test5.test | 8 | 0.002712s | 0.002740s | 0.002586s | byte equality observed |
| test5u.test | 6 | 0.002720s | 0.002727s | 0.002693s | byte equality observed |
| test6.test | 8 | 0.002733s | 0.002632s | 0.002555s | byte equality observed |
| test6u.test | 9 | 0.002694s | 0.002685s | 0.002646s | byte equality observed |
| test7.test | 8 | 0.002698s | 0.002709s | 0.002600s | byte equality observed |
| test7u.test | 8 | 0.002695s | 0.002638s | 0.002599s | byte equality observed |
| test8.test | 8 | 0.002664s | 0.002563s | 0.002630s | byte equality observed |
| test8u.test | 8 | 0.002678s | 0.002648s | 0.002676s | byte equality observed |
| test9.test | 8 | 0.002699s | 0.002720s | 0.002580s | byte equality observed |
| test9u.test | 8 | 0.002725s | 0.002631s | 0.002541s | byte equality observed |
| test_actions.test | 7 | 0.003418s | 0.003426s | 0.003185s | byte equality observed |
| test_assert.test | 8 | 0.002802s | 0.002736s | 0.002735s | byte equality observed |
| test_assert_with_placeholder.test | 8 | 0.002669s | 0.002615s | 0.002482s | byte equality observed |
| test_cycles.test | 8 | 0.002707s | 0.002665s | 0.002580s | byte equality observed |
| test_dynamic.test | 8 | 0.002686s | 0.002620s | 0.002518s | byte equality observed |
| test_dynamic_grammar.test | 7 | 0.002929s | 0.002834s | 0.002793s | byte equality observed |
| test_format.test | 8 | 0.002770s | 0.002688s | 0.002699s | byte equality observed |
| test_non_char.test | 8 | 0.002819s | 0.002677s | 0.002691s | byte equality observed |
| test_skip.test | 8 | 0.002687s | 0.002523s | 0.002512s | byte equality observed |
| test_static.test | 8 | 0.002467s | 0.002419s | 0.002388s | byte equality observed |
| test_symbols.test | 7 | 0.003038s | 0.002963s | 0.002896s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
